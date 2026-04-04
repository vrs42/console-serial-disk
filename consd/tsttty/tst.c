#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#define TTY_PATH	"/dev/tty"

int tty_fd;	/* file descriptor for the newly opened console. */

struct termios tty_init;
struct termios tty_set;

void nullfunc(int i){
  return;
}

void put_tty( unsigned char ch ){
int i;
  i=write( tty_fd, &ch, 1 );
  if( i != 1 ) {
    fprintf( stderr, "write returned a %d\n", i );
  }
}

void main()
{
int cnt;
int i,j;
unsigned char buff[256];
unsigned char ch;

  /* open the tty read/write and make it not a controlling tty and no delay */
  tty_fd = open( TTY_PATH, O_RDWR|O_NOCTTY|O_NDELAY );
  if( tty_fd == -1 ){
    fprintf( stderr, "unable to open %s for RDWR\n", TTY_PATH );
    exit(1);
  }

  /* get the original settings */
  if( tcgetattr( tty_fd, &tty_init ) < 0 ){
    fprintf( stderr, "tcgetattr failed for %s\n", TTY_PATH );
    exit(1);
  }

  /* copy the original settings */
  tty_set = tty_init;

  /* make changes */
  /* enable the receiver and local mode. */
  tty_set.c_cflag |= ( CREAD | CLOCAL );

/* choose Raw input */
  tty_set.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

/* disable incoming and outgoing software flow control */
  tty_set.c_iflag &= ~(IXON | IXOFF | IXANY);

/* disable CR to LF conversion on input */
  tty_set.c_iflag &= ~ICRNL;

/* Turn off output post processing */
  tty_set.c_oflag &= ~OPOST;


  tty_set.c_cc[VMIN] = 0;
  tty_set.c_cc[VTIME] = 0;

  /* activate the new settings */
  if( tcsetattr( tty_fd, TCSANOW, &tty_set ) <0 ){
    fprintf( stderr, "tcsetattr failed\n", TTY_PATH );
    close( tty_fd );
    exit( 1 );
  }

  /* should be a delay between each character showing unbuffered output */
  for(ch='A'; ch<='Z'; ch++) {
    put_tty( ch );	/* put the character on the console */
    for(i=0; i<10000000; i++) nullfunc(i);	/* wait a while */
  }
  put_tty( '\r' );
  put_tty( '\n' );

  /* Make sure CR does not also do a NL. Should be only a Z when done */
  for(ch='A'; ch<='Z'; ch++) {
    put_tty( ch );	/* put the character on the console */
    put_tty( '\r' );
  }
  put_tty( '\n' );

  /* make sure a newline only does a NL */
  for(ch='A'; ch<='Z'; ch++) {
    put_tty( ch );	/* put the character on the console */
    put_tty( '\n' );
  }
  put_tty( '\r' );
  put_tty( '\n' );

  /* If the above all works then we can test the keyboard portion */

  for( i=0; i<10; i++ ){
    while( 0 == (cnt=read( tty_fd, buff, 256 )) );
    if( cnt == -1 ) {
      fprintf( stderr, "read returned -1 meaning no character?\n" );
      break;
    }
    if( cnt == 1 ){
      ch=buff[0];
      if( ch<040 || ch>0176 ) {		/* it is a control code */
        fprintf( stderr, "(%03o)", ch );
      } else put_tty( ch );	/* just echo the character */
    } else {	/* show the codes received */
      fprintf( stderr, "\r\n%d chars received:", cnt );
      for( j=0; j<cnt; j++ )  fprintf( stderr, "(%03o)", buff[j] );
      fprintf( stderr, "\r\n" );
      break;
    }
  }

  /* put the tty back to the original settings */
  if( tcsetattr( tty_fd, TCSADRAIN, &tty_init ) <0 ){
    fprintf( stderr, "tcsetattr failed to restore %s\n", TTY_PATH );
    fprintf( stderr, "The terminal might be in an unusable state.\n" );
  }
  close( tty_fd );
  exit(0);
}
