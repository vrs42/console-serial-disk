/* Console port Serial Disk */

#define VERSION	"1.1 (Release)"

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>


//#define PDP_SP		"/dev/ttyUSB0"
#define PDP_SP		"./ttyPDP"

#define PDP_BAUD	B19200

/* use CSTOPB for 2 stop bits and 0 for one */
#define PDP_STOP	0


/* testing defines */
/* #define TESTLOOPBACK */

int pdp_fd;	/* descriptor that talks to the PDP-8 */
int tty_fd;	/* descriptor that talks to the Unix connected terminal */
int con_fd;	/* descriptor that talks to the optional pdp-8 console device */

struct termios pdp_init;	/* for the serial port connected to the 8 */
struct termios pdp_set;

struct termios tty_init;	/* for the unix connected terminal */
struct termios tty_set;

struct termios con_init;	/* for the serial port connected to the physical console */
struct termios con_set;

int boot_mode;		/* if set by command line then do an initial boot */
int debug_mode;		/* if true then display debug messages */
int warn_mode;		/* if true then display warning messages */

int half;	/* which half of the disk? */
int blk;	/* block counter */

#include "pdp.c"	/* pdp serial port support */

#include "disk.c"	/* virtual disk routines */

#include "boot.c"	/* code making up the boot function */

#include "tty.c"	/* Unix console interface routines */

/* do a 12 bit 2's complement */
int twocomp12( int value )
{
  return (((~value)&07777)+1)&07777;
}

/* The handler is in command mode and expecting 3 arguments.
**
** 1) The PDP-8 memory address of the transfer.
** 2) The word count of the transfer in two's comp.  0 transfers 4096.
** 3) The command.
**
** For the memory read operation the command is built as follows:
**
** (((06200 | (FIELD<<3))>>2) | 02000)
** The 6200 is part of the opcode for a CDF instruction. The Field is
** designated by the third octal digit which is why FIELD is left shifted
** by 3 bits.  We right shift the whole thing by 2 bits because the
** handler uses the MSB or sign bit to specify a far JMP command and the
** bit next to that to specify a memory read or write command.  The 2000
** is ored in to tell the handler that this is a memory read.  The handler
** checks the sign bit and if set performs a far JMP.  It then does
** RTL;IAC to put the bits in the correct place and the IAC completes the
** CDF by setting the LSB.  A 6201 is a CDF to field 0 while a 6271 is a
** CDF to field 7.
*/

void pdp_mem_read( int field, int adr, int cnt, int *ptr )
{
int i;		/* counter */
int wdcnt;	/* word count to transfer */

  if( (field<0) || (field>7) ){
    fprintf( stderr, "SERVER: pdp_mem_read field out of range\r\n" );
    return;
  }

  if( (adr<0) || (adr>4095) ){
    fprintf( stderr, "SERVER: pdp_mem_read address out of range\r\n" );
    return;
  }

  if( (cnt<1) || (cnt>4096) ){
    fprintf( stderr, "SERVER: pdp_mem_read count out of range\r\n" );
    return;
  }
//fprintf( stderr, "pdp_mem_read(%o, %o, %o)\r\n", field, adr, cnt );

  /* fix up the count special case */
  wdcnt=cnt;
  if( wdcnt == 4096 ) wdcnt=0;

  pdp_put12( adr );			/* PDP-8 address to be transfered */
  pdp_put12( twocomp12(wdcnt) );	/* number of words to transfer */
  pdp_put12( (((06200 | (field<<3))>>2) | 02000) );	/* read command */

  /* now read the words the handler is sending */
  for( i=0; i<cnt; i++){
    *ptr=pdp_get12();	/* read the word */
    ptr++;
  }
}

void pdp_mem_write( int field, int adr, int cnt, int *ptr )
{
int i;		/* counter */
int wdcnt;	/* word count to transfer */

  if( (field<0) || (field>7) ){
    fprintf( stderr, "SERVER: pdp_mem_write field out of range\r\n" );
    return;
  }

  if( (adr<0) || (adr>4095) ){
    fprintf( stderr, "SERVER: pdp_mem_write address out of range\r\n" );
    return;
  }

  if( (cnt<1) || (cnt>4096) ){
    fprintf( stderr, "SERVER: pdp_mem_write transfer count out of range\r\n" );
    return;
  }

  /* fix up the count special case */
  wdcnt=cnt;
  if( wdcnt == 4096 ) wdcnt=0;		/* transfer a whole field */

  pdp_put12( adr );			/* PDP-8 address to be transfered */
  pdp_put12( twocomp12(wdcnt) );	/* number of words to transfer */
  pdp_put12( (((06200 | (field<<3))>>2) | 00000) );	/* write command */

  for( i=0;i<cnt;i++ ){
    pdp_put12( *ptr );	/* write the word */
    ptr++;
  }
}

/* process a disk server request
**
** The variable passed is the memory field of the caller.
*/
void do_server( int field )
{
int c_addr;		/* handler address called */
int a_addr;		/* handler argument list address */
int cmd;		/* the command to be sent */
int keybuf;		/* char that was in the keyboard buffer */

int i;			/* array index */
int arg_list[3];

int *dsk;		/* pointer to the starting disk sector */
int blkcnt;		/* max block count for selected device */
int patch_jms;		/* true if the JMS SETUP needs to be fixed up */
int JMS_SETUP=04245;	/* EXTREMELY DANGEROUS CONSTANT */

int FCW;		/* OS/8 File Control Word */
int RC;			/* Record Count.  Num of 128 word records to xfer */
int MF;			/* Memory Field where the transfer is to take place */
int DD;			/* Device Dependent (only used on DECTape */

  /* Table entry points will have JMS SETUP for every one except for the 1st
  ** and look something like this:
  **         *0240
  ** CSD0,   VERSION
  ** CSD1,   JMS SETUP
  ** CSD2,   JMS SETUP
  ** CSD3,   JMS SETUP
  **         JMS SETUP
  ** SETUP,  .-.
  **
  ** where SETUP is not expected to return.  For all entry points except the
  ** first the JMS SETUP will need to be put back by the server.  
  */
  patch_jms=1;		/* Unless cleared patch the JMS SETUP back in */
//fprintf( stderr, "do_server(%03o)\r\n", field );

#ifdef PQS8
  c_addr=07640;	/* Call is always here for PQS8 */
#else
  /* get the handler entry point address that was called plus 2 */
  /* this will tell us what device is requested.  */
  c_addr=pdp_get12()-2;	/* fetch and remove offset */
#endif

  /* handler sends us the character that is in the keyboard buffer */
  /* We need this so we can put it back */
  do {
    keybuf=pdp_getch();
  } while( keybuf == -1 );
//fprintf( stderr, "SERVER: keybuf=%05o\r\n", keybuf );

  /* we need the callers address */
  pdp_mem_read( 0, c_addr, 1, &a_addr );
//fprintf( stderr, "SERVER: handler arg list at %05o\r\n", a_addr );

  /* fetch the callers argument list */
  pdp_mem_read( field, a_addr, 3, arg_list );

#ifdef PQS8
/* arg_list[0] == the FCW (File Control Word) */
  FCW=arg_list[1]; /* Remember function word */
  arg_list[1] = arg_list[0]; /* Fix up buffer pointer */
#else
/* arg_list[0] == the FCW (File Control Word) */
  FCW=arg_list[0];
#endif
  RC=(FCW&03700)>>6;	/* 128 word Record transfer Count */
  MF=(FCW&00070)>>3;	/* Memory Field where transfer is to take place */
  DD=(FCW&0007);	/* not used / unit number */
/* arg_list[1] == the pointer to the buffer
** arg_list[2] == the starting block number to read or write
*/

  /* figure out what device we are handling */
  switch( c_addr ) {
  case 07607:		/* SYS:  and CSD0: entry points */
    blkcnt=CSD_BLKCNT;	/* max block count for error check */
    dsk=&csd_dsk[0][arg_list[2]][0];
    patch_jms=0;	/* don't need to patch */
    break;
  case 07641:		/* CSD1: entry point */
    blkcnt=CSD_BLKCNT;	/* max block count for error check */
    dsk=&csd_dsk[1][arg_list[2]][0];
    patch_jms=0;	/* don't need to patch */
    break;
  default:		/* entry points not coresident with SYS handler */
    switch( c_addr&077 ) {
    case 00010:		/* DTA0 entry point */
      patch_jms=0;	/* don't need to patch */
      blkcnt=TC_BLKCNT;
      dsk=&tc_dsk[arg_list[2]][0];
      break;
    case 00020:		/* RKA0 entry point */
      half=0;		/* first half of the RK05 image */
      patch_jms=0;	/* don't need to patch */
      blkcnt=RK_BLKCNT;	/* max block count for error check */
      dsk=&rk_dsk[half][arg_list[2]][0];
      break;
    case 00021:		/* RKB0 entry point */
      half=1;		/* 2nd half of the RK05 image */
      blkcnt=RK_BLKCNT;
      dsk=&rk_dsk[half][arg_list[2]][0];
      break;
    case 00040:		/* CSD0: */
      patch_jms=0;	/* don't need to patch */
    case 00041:		/* CSD1: */
    case 00042:		/* CSD2: */
    case 00043:		/* CSD3: */
      blkcnt=CSD_BLKCNT;
#ifdef PQS8
      patch_jms=0;	/* don't need to patch */
      /* Use the PQS8 unit number */
      dsk=&csd_dsk[DD][arg_list[2]][0];
//fprintf( stderr, "SERVER: called for unit=%05o\r\n", DD );
#else
      dsk=&csd_dsk[c_addr&07][arg_list[2]][0];
#endif
      break;
    default:	/* unknown entry point */
      fprintf( stderr, "SERVER: Unknown handler entry point=%05o\r\n", c_addr );
      /* we need the callers address */
      pdp_mem_read( 0, c_addr, 1, &a_addr );
      /* fetch the callers argument list */
      pdp_mem_read( field, a_addr, 3, arg_list );
      /* Do an error return here */
      pdp_put12( (a_addr+3)&07777 );
      pdp_put12( 07777 );		/* negative number is a fatal error */
      pdp_put12( 06203|(field<<3) );
      /* and finally put the character back in the buffer */
      pdp_putch( keybuf );
      return;
    }
  }

  /* patch the JMS SETUP back in */
  if( patch_jms ){
    pdp_mem_write( 0, c_addr, 1, &JMS_SETUP );
  }

  if( (arg_list[2]+(RC/2)+(RC&1)) >= blkcnt ){	/* Request past end of media */
    if( warn_mode )
      fprintf( stderr, "WARNING: requested operation off end of media\r\n" );
/* it is unclear if this should be a fatal error. */
    /* We need to take the error exit */
    pdp_put12( (a_addr+3)&07777 );
    pdp_put12( 07777 );		/* any negative number is a fatal error */
    pdp_put12( 06203|(field<<3) );
    /* and finally put the character back in the buffer */
    pdp_putch( keybuf );
    return;
  }

  if( debug_mode ){
  /* need to identify the disk device somehow */
    fprintf( stderr, "SERVER: Entry=%04o ", c_addr );
    if( FCW&04000 ){	/* if true then a write operation */
      fprintf( stderr, "Write" );
    } else {
      fprintf( stderr, "Read " );
    }
    fprintf( stderr, " %05o words", RC*128 );
    fprintf( stderr, " @ addr %o", MF );
    fprintf( stderr, "%04o ", arg_list[1] );
    fprintf( stderr, "from Block %05o\r\n", arg_list[2] );
  }

  if( RC == 0 ) RC=32;	/* adjust for full field transfer */
  if( FCW&04000 ){	/* if true then a write operation */
    pdp_mem_read( MF, arg_list[1], RC*128, dsk );
  } else {		/* it is a read operation. */
    pdp_mem_write( MF, arg_list[1], RC*128, dsk );
  }

/* this is the place for the control C check. */
/* Not sure the best way to do that.  */
/* but if we see one we jump to 07605 instead of returning */

#ifdef PQS8
  /* c_adder may still point to a_addr. */
  /* If it does not, then a return address was read */
  /* by the transfer we just finished. */
  i = a_addr;
  pdp_mem_read( 0, c_addr, 1, &a_addr );
  if (a_addr != i)
    a_addr -= 3;	/* 3 because there is no error return */
  a_addr += 3;	/* 3 because there is no error return */
#else
  a_addr += 4;	/* 4 to skip error return */
#endif

  /* send the far jump stuff here to return to caller */
  pdp_put12( (a_addr)&07777 );
  pdp_put12( 0 );
  pdp_put12( 06203|(field<<3) );


  /* and finally put the character back in the keyboard buffer */
  pdp_putch( keybuf );

}

int main( int argc, char *argv[] )
{
int i,j,k;		/* general index */
int key;	/* key pressed on the console */
int pdp_ch;	/* character from the PDP-8 */
char stmp[256];	/* string temp */

FILE *txtfile;	/* a text file to be sent */
int txt_ch;	/* character to be sent text mode */

  boot_mode=0;	/* no initial boot */
  debug_mode=0;	/* turn off debugging */
  warn_mode=0;	/* turn off warnings */

/* initialize the image names */
  for( i=0; i<CSD_MAX; i++ ){
    sprintf( csd_name[i], "csd%d.img", i );
  }
  strcpy( tc_name, "tc0X.img" );
  strcpy( rk_name, "rk05.img" );

/* process the command line */
  for( i=1; i<argc; i++ ){
    if( strcmp("-B", argv[i] )==0 ) {
      boot_mode=1;		/* do an initial boot */
    } else
    if( strcmp("-D", argv[i] )==0 ) {
      debug_mode=1;		/* turn on debugging */
    } else
    if( strcmp("-W", argv[i] )==0 ) {
      warn_mode=1;		/* turn on warnings */
    } else {
      fprintf( stderr, "Usage: csd [options]\n\n" );
      fprintf( stderr, "\t-B\tEnter boot mode on startup.\n" );
      fprintf( stderr, "\t-D\tDisplay debug messages.\n" );
      fprintf( stderr, "\t-W\tDisplay warning messages.\n" );
      exit(1);
    }
  }

  fprintf( stderr, "Console Serial Disk Version %s\r\n", VERSION );

  if( debug_mode ){
    fprintf( stderr, "SERVER: Debug mode enabled.  Command line:\r\n" );
    for( i=0; i<argc; i++) fprintf( stderr, "%s ", argv[i] );
    fprintf( stderr, "\r\n" );
  }

  /* open the system disk */

  disk_open();

/* open the pdp-8 serial port */
  pdp_setup();

/* configure the Unix keyboard and display */
  tty_setup();

/* do the initial boot if requesed on command line */
  if( boot_mode ) do_boot();

/* go into terminal mode just like a teletype. */

  if( debug_mode )
    fprintf( stderr, "SERVER: going into terminal mode\r\n" );

  while( 1 ){	/* be a terminal forever */
    key=get_key();		/* check if a key is pressed */
    if( key != -1 ){		/* we have a key */
      if( key > 0377 ){		/* we have a function key */
        switch( key ){
          case KEY_SH_F12: 	/* time to boot */
            fprintf( stderr, "SERVER: Shift F12 key pressed, sending boot code.\r\n" );
            do_boot();
            break;
          case KEY_F1:		/* time to exit */
            fprintf( stderr, "SERVER: F1 key pressed, updating device images and exiting\r\n" );
            disk_close();
            tty_reset();
            pdp_reset();
            exit(0);
          case KEY_F6:		/* send a text file using echo to rate limit */
				/* fails if a line is longer than 72 chars? */
            fprintf( stderr, "SERVER: F6 key pressed, Send a text file\r\n" );

            txtfile=fopen( "textfile", "r" );
            if( txtfile==NULL ){
              fprintf( stderr, "SERVER: can't open %s for reading\r\n", "textfile" );
              break;
            }

            do {
              txt_ch=fgetc( txtfile );
              if( txt_ch==EOF )break;
              if( txt_ch=='\n' ){
                pdp_putch( '\r'|0200 );	/* send a return */
                while( (pdp_ch=pdp_getch())== -1 );	/* wait for the return */
		if( (pdp_ch&0370) == 020 ){	/* got a server wakeup code */
                  do_server( pdp_ch&007 );
                  while( (pdp_ch=pdp_getch())== -1 );	/* wait for the return */
                }
                if( (pdp_ch&0177)!='\r' ){	/* expected a return */
                  fprintf( stderr, "expected a return and got (%03o)\r\n", pdp_ch );
                  fclose( txtfile );
		  disk_close();
		  tty_reset();
		  pdp_reset();
		  exit(1);
                }
                tty_putch( pdp_ch );
                while( (pdp_ch=pdp_getch())== -1 );	/* wait for the newline */
                if( (pdp_ch&0177)!='\n' ){	/* expected a newline */
                  fprintf( stderr, "expected a newline and got (%03o)\r\n", pdp_ch );
                  fclose( txtfile );
		  disk_close();
		  tty_reset();
		  pdp_reset();
		  exit(1);
                }
                tty_putch( pdp_ch );
              } else {
                pdp_putch( toupper(txt_ch)|0200 );	/* send the char */
                while( -1 == (pdp_ch=pdp_getch()) );	/* wait for the echo */
		if( (pdp_ch&0370) == 020 ){	/* got a server wakeup code */
                  do_server( pdp_ch&007 );
                  while( (pdp_ch=pdp_getch())== -1 );	/* wait for the return */
                }
                if( (pdp_ch&0177) != toupper(txt_ch) ){
                  fprintf( stderr, "expected (%03o) and got (%03o)\r\n", txt_ch, pdp_ch );
		  disk_close();
		  tty_reset();
		  pdp_reset();
		  exit(1);
                }
                tty_putch( pdp_ch );
              }
            } while( 1 );
            pdp_putch( 0232 );	/* send a ctrl Z */

            fclose( txtfile );
            break;
          default:
            fprintf( stderr, "SERVER: Unassigned function key pressed key=%d\r\n", key );
            fprintf( stderr, "Fkeys:\r\n" );
            fprintf( stderr, "  F1     Write disks and exit.\r\n" );
            fprintf( stderr, "  F6     Send a text file to PIP.\r\n" );
            fprintf( stderr, "  SH-F12 Send OS/8 boot to CSD Help loader.\r\n" );
            break;
        }
      } else {			/* more or less a regular character */
        key = toupper(key);	/* convert to upper case. */
        key = key | 0200;	/* Turn on MSB. */
        pdp_putch( key );	/* send it to the pdp-8 */
      }
    }
/* get a char from the PDP here if there is one and handle it. */
    pdp_ch=pdp_getch();		/* see if the PDP-8 sent us something */
    if( pdp_ch != -1 ){		/* we have a character */
      if( (pdp_ch&0370) == 020 ){	/* could be a server wakeup */
        do_server( pdp_ch&007 );
      } else {			/* print the character */
        tty_putch( pdp_ch );	
      }
    }
  }

/* technically we can never get here. */

  tty_reset();

  pdp_reset();

  exit(0);
}
