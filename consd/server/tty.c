/* display a character on the glass teletype */

void tty_putch( int value )
{
unsigned char ch;
  if( warn_mode && (value<0200) ){
    fprintf( stderr, "SERVER: warning, char has value less than 0200.  Ch=%04o\n", value );
  }
  value=value&0177;			/* strip the MSB */
  ch=(unsigned char) toupper(value);	/* teletypes only print upper case */
  if( ch<040 || ch==0177 ) {		/* it is a control character */
    switch( ch ){
    case 011:		/* Tab */
      ch=' ';		/* print a space for a Tab */
    case 007:		/* bell */
    case 012:		/* linefeed */
    case 015:		/* carriage return */
      write( tty_fd, &ch, 1 );
    }
  } else {				/* it is displayable */
    write( tty_fd, &ch, 1 );
  }
}


/* define function key return values. */

/* regular function keys */

#define KEY_F1 1000
#define KEY_F2 1001
#define KEY_F3 1002
#define KEY_F4 1003
#define KEY_F5 1004
#define KEY_F6 1005
#define KEY_F7 1006
#define KEY_F8 1007
#define KEY_F9 1008
#define KEY_F10 1009
#define KEY_F11 1010	/* Mobaxterm goes to full screen instead of sending this. */
#define KEY_F12 1011

/* shifted function keys */

#define KEY_SH_F1 1012	/* Moba Sends 1010 instead of this */
#define KEY_SH_F2 1013	/* Moba Sends 1011 instead of this */
#define KEY_SH_F3 1014
#define KEY_SH_F4 1015
#define KEY_SH_F5 1016
#define KEY_SH_F6 1017
#define KEY_SH_F7 1018
#define KEY_SH_F8 1019
#define KEY_SH_F9 1020
#define KEY_SH_F10 1021
#define KEY_SH_F11 1022
#define KEY_SH_F12 1023

/* ctrl function keys */

#define KEY_CT_F1 1024
#define KEY_CT_F2 1025
#define KEY_CT_F3 1026
#define KEY_CT_F4 1027
#define KEY_CT_F5 1028	/* Moba sends F5 code instead of this */
#define KEY_CT_F6 1029	/* Moba sends F6 code instead of this */
#define KEY_CT_F7 1030	/* Moba sends F7 code instead of this */
#define KEY_CT_F8 1031	/* Moba sends F8 code instead of this */
#define KEY_CT_F9 1032	/* Moba sends F9 code instead of this */
#define KEY_CT_F10 1033	/* Moba sends F10 code instead of this */
#define KEY_CT_F11 1034	/* Moba sends F11 code instead of this */
#define KEY_CT_F12 1035	/* Moba sends F12 code instead of this */

struct termios tty_init;
struct termios tty_set;

/* function to get a char from the console keyboard
**
** Map       F1 through F12 to 1000 through 1011
** Map shift F1 through F12 to 1012 through 1023
** Map ctrl  F1 through F4  to 1024 through 1027
** Rpi terminal does a dropdown menu for F10.
** Mobaxterm does not generate all of these uniquely.
*/
int get_key()
{
int cnt;
int i;
#define MAXSTR	8
unsigned char ch[MAXSTR];

  cnt = read(tty_fd, &ch, MAXSTR);
  if (cnt<=0) return -1;
  if (cnt==1) return (int) ch[0];
  if (cnt==3 && ch[0]==033 && ch[1]==0117) {	/* F1 through F4 */
    switch(ch[2]){
    case 0120: return KEY_F1;	/* F1 */
    case 0121: return KEY_F2;	/* F2 */
    case 0122: return KEY_F3;	/* F3 */
    case 0123: return KEY_F4;	/* F4 */
    default: 
      fprintf(stderr,"Gr1 ch[2]=%04o\n", ch[2]);
      return -1;
    }
  }
  if( (cnt==4) && (ch[0]==033) && (ch[1]==0133) && (ch[2]==063) && (ch[3]==0176) ) {
    return 0177;	/* decode the DEL key as a RUBOUT */
  }
  if (cnt==5 && ch[0]==033 && ch[1]==0133 && ch[2]==061 && ch[4]==0176) {
    switch(ch[3]){
    case 065: return KEY_F5;	/* F5 */
    case 067: return KEY_F6;	/* F6 */
    case 070: return KEY_F7;	/* F7 */
    case 071: return KEY_F8;	/* F8 */
    default: 
      fprintf(stderr, "Gr2 ch[3]=%04o\n", ch[3]);
      return -1;
    }
  }
  if (cnt==5 && ch[0]==033 && ch[1]==0133 && ch[2]==062 && ch[4]==0176) {
    switch(ch[3]){
    case 060: return KEY_F9;	/* F9 */
    case 061: return KEY_F10;	/* F10 */
    case 063: return KEY_F11;	/* F11 */
    case 064: return KEY_F12;	/* F12 */
    case 065: return KEY_SH_F3;	/* Shift F3 */
    case 066: return KEY_SH_F4;	/* Shift F4 */
    case 070: return KEY_SH_F5;	/* Shift F5 */
    case 071: return KEY_SH_F6;	/* Shift F6 */
    default: 
      fprintf(stderr, "Gr3 ch[3]=%04o\n", ch[3]);
      return -1;
    }
  }
  if (cnt==5 && ch[0]==033 && ch[1]==0133 && ch[2]==063 && ch[4]==0176) {
    switch(ch[3]){
    case 061: return KEY_SH_F7;	/* Shift F7 */
    case 062: return KEY_SH_F8;	/* Shift F8 */
    case 063: return KEY_SH_F9;	/* Shift F9 */
    case 064: return KEY_SH_F10;	/* Shift F10 */
    default: 
      fprintf(stderr, "Gr4 ch[3]=%04o\n", ch[3]);
      return -1;
    }
  }
  if (cnt==5 && ch[0]==033 && ch[1]==0133 && ch[2]==064 && ch[4]==0176) {
    switch(ch[3]){
    case 062: return KEY_SH_F11;	/* Shift F11 */
    case 063: return KEY_SH_F12;	/* Shift F12 */
    default: 
      fprintf(stderr, "Gr5 ch[3]=%04o\n", ch[3]);
      return -1;
    }
  }
  if (cnt==6 && ch[0]==033 && ch[1]==0133 && ch[2]==061 && ch[3]==073 && ch[4]==065) {
    switch(ch[5]){
    case 0120: return KEY_CT_F1;	/* Ctrl F1 */
    case 0121: return KEY_CT_F2;	/* Ctrl F2 */
    case 0122: return KEY_CT_F3;	/* Ctrl F3 */
    case 0123: return KEY_CT_F4;	/* Ctrl F4 */
    default: 
      fprintf(stderr, "Gr6 ch[5]=%04o\n", ch[5]);
      return -1;
    }
  }
  /* Rpi console generates these */
  if (cnt==6 && ch[0]==033 && ch[1]==0133 && ch[2]==061 && ch[3]==073 && ch[4]==062) {
    switch(ch[5]){
    case 0120: return KEY_SH_F1;	/* Shift F1 */
    case 0121: return KEY_SH_F2;	/* Shift F2 */
    case 0122: return KEY_SH_F3;	/* Shift F3 */
    case 0123: return KEY_SH_F4;	/* Shift F4 */
    default: 
      fprintf(stderr, "Gr7 ch[5]=%04o\n", ch[5]);
      return -1;
    }
  }
  /* Rpi console generates these */
  if (cnt==7 && ch[0]==033 && ch[1]==0133 && ch[2]==061 && ch[4]==073 && ch[5]==062 && ch[6]==0176) {
    switch(ch[3]){
    case 065: return KEY_SH_F5;	/* Shift F5 */
    case 067: return KEY_SH_F6;	/* Shift F6 */
    case 070: return KEY_SH_F7;	/* Shift F7 */
    case 071: return KEY_SH_F8;	/* Shift F8 */
    default: 
      fprintf(stderr, "Gr8 ch[3]=%04o\n", ch[3]);
      return -1;
    }
  }
  /* Rpi console generates these */
  if (cnt==7 && ch[0]==033 && ch[1]==0133 && ch[2]==062 && ch[4]==073 && ch[5]==062 && ch[6]==0176) {
    switch(ch[3]){
    case 060: return KEY_SH_F9;	/* Shift F9 */
    case 061: return KEY_SH_F10;	/* Shift F10 */
    case 063: return KEY_SH_F11;	/* Shift F11 */
    case 064: return KEY_SH_F12;	/* Shift F12 */
    default: 
      fprintf(stderr, "Gr9 ch[3]=%04o\n", ch[3]);
      return -1;
    }
  }
  fprintf(stderr, "in get_key cnt=%d", cnt);
  for(i=0;i<cnt;i++) fprintf(stderr, " ch[%d]=%04o", i, ch[i]);
  fprintf(stderr, "\n");
  return -1;
}

void tty_setup()
{
/* open the connected Unix console non blocking */
  tty_fd = open( "/dev/tty" , O_RDWR|O_NOCTTY|O_NDELAY );
  if(tty_fd == -1) {
    fprintf( stderr, "Can't open /dev/tty\n" );
    exit(1);
  }

/* read the initial /dev/tty settings */
  if( tcgetattr(tty_fd, &tty_init) < 0 ) {
    fprintf( stderr, "tcgetattr failed for /dev/tty\n" );
    exit(1);
  }

  tty_set = tty_init;

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

  if (tcsetattr(tty_fd, TCSANOW, &tty_set) < 0) {
    fprintf( stderr, "tcsetattr failed for the Unix console\n");
    exit(1);
  }
}


void tty_reset()
{
/* ok, put the keyboard back */
  if (tcsetattr(tty_fd, TCSANOW, &tty_init) < 0) {
    fprintf( stderr, "tcsetattr failed to restore the console keyboard\n");
    exit(1);
  }
}
