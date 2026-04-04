/* pdp-8 serial port support */

/* function to get a char from the pdp-8 */
int pdp_getch()
{
int cnt;
unsigned char ch;

  cnt = read(pdp_fd, &ch, 1);
  if( cnt==0 ) return -1;
  return (int) ch;
}

/* function to get a 12 bit value from the PDP-8
**
** the 8 sends the 12 bits with this subroutine:
**
** SEND8,  .-.
**         TSF
**         JMP .-1
**         TLS
**         JMP I SEND8
**
** SEND12, .-.
**         JMS SEND8
**         CLL RTR
**         RTR
**         JMS SEND8
**         CLA CLL
**         JMP I SEND12
**
** which means we have 4 bits that are the same
** in each character and we can sort of error or
** validation check those bits.
*/
int pdp_get12()
{
int val1, val2;
  do {
    val1=pdp_getch();
  } while( val1 == -1 );
  do {
    val2=pdp_getch();
  } while( val2 == -1 );

  if( (val1&0360) != ((val2<<4)&0360) ){
    fprintf( stderr, "SERVER: validation failure in pdp_get12(), val1=%04o, val2=%04o\n", val1, val2);
  }

  return ((val2&0360)<<4) | (val1&0377);
}

/* pdp_putch sends an 8 bit character to the PDP-8 */
void pdp_putch(int val)
{
int i;
unsigned char ch;
  ch = val & 0377;
  do {
    i= write( pdp_fd, &ch, 1) ;
  } while( i != 1 );
}

/* pdp_put12 sends a 12 bit value to the PDP-8
**
** The first byte sent is the upper 8 bits.
** and the second byte sent is the lower 4 bits.
**
** The PDP-8 received the characters with the following routine:
**
** GET12,  .-.
**         KSF
**         JMP .-1
**         KRB
**         CLL RTL
**         RTL
**         DCA TEMP
**         KSF
**         JMP .-1
**         KRB
**         TAD TEMP
**         JMP I GET12
**
** This joins the two characters into a 12 bit value.
*/
void pdp_put12( int value )
{
  pdp_putch( (value>>4) & 0377 );	/* send upper 8 bits */
  pdp_putch( value & 0017 );		/* send the lower 4 bits */
}

void pdp_setup(){
/* O_RDWR is Read Write mode.
** O_NOCTTY specifies that we are not the controlling termnal.
** O_NDELAY means o ignore DCD.
*/
  pdp_fd = open( PDP_SP , O_RDWR | O_NOCTTY | O_NDELAY );
  if(pdp_fd == -1) {
    fprintf( stderr, "Can't open %s\n", PDP_SP );
    exit(1);
  }

/* read the initial pdp-8 serial port settings */
  if( tcgetattr(pdp_fd, &pdp_init) < 0 ) {
    fprintf( stderr, "tcgetattr failed for %s\n", PDP_SP );
    exit(1);
  }

/* set the baud rate, stop bits and parity */
  pdp_set = pdp_init;	/* make changes to initial value */

  cfsetispeed(&pdp_set, PDP_BAUD);	/* set the input speed */
  cfsetospeed(&pdp_set, PDP_BAUD);	/* set the output speed */

/*  pdp_set.c_cflag = CS8 | CREAD | HUPCL | CLOCAL ; */

/* enable the receiver and local mode. */
  pdp_set.c_cflag |= ( CREAD | CLOCAL );

/* Set to 8 data bits */
  pdp_set.c_cflag &= ~CSIZE;
  pdp_set.c_cflag |= CS8;
 
/* Disable parity checking */
  pdp_set.c_cflag &= ~PARENB;

/* Select the number of stop bits */
  pdp_set.c_cflag &= ~CSTOPB;	/* clear the bit */
  pdp_set.c_cflag |= PDP_STOP;	/* set it if set */

/* disable hardware flow control */
  pdp_set.c_cflag &= ~CRTSCTS;

/* choose Raw input */
  pdp_set.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

/* disable parity checking */
  pdp_set.c_iflag &= ~INPCK;

/* disable incoming and outgoing software flow control */
  pdp_set.c_iflag &= ~(IXON | IXOFF | IXANY);

/* disable CR to LF conversion on input */
  pdp_set.c_iflag &= ~ICRNL;

/* Turn off output post processing */
  pdp_set.c_oflag &= ~OPOST;


  pdp_set.c_cc[VMIN] = 0;
  pdp_set.c_cc[VTIME] = 0;

  if (tcsetattr(pdp_fd, TCSANOW, &pdp_set) < 0) {
    fprintf( stderr, "tcsetattr failed\n");
    exit(1);
  }

/* flush the pdp serial port buffer */
  tcflush(pdp_fd,TCIOFLUSH);
}


void pdp_reset(){
/* put it back to initial settings */
  tcsetattr(pdp_fd, TCSANOW, &pdp_init);
  close( pdp_fd );
}
