/* Serial port loopback test */

/* The first way to use this program is with a wire
** between pins 2 and 3 on the server serial port.
** this tests that the server s able to send and
** receive characters and if there is any quastion
** about the serial port do this test first.
**
** For the final test you toggle a short program into
** the PDP-8 which echoes every character it receives.
** This is an end to end test of all the hardware.
** Here is the program:
**
** 0200 6031 KSF
** 0201 5200 JMP .-1
** 0202 6036 KRB
** 0203 6046 TLS
** 0204 5200 JMP 0200
**
** The program waits for a character, reads it and
** then sends it back to this program.  Character is
** read and compared to what was sent.  If the read is
** blocking that is detected and reported.  Finally
** the number of times read was called before receiving
** the last character is reported.  I mostly did this
** to find out how much extra processor is available.
**
** A hard loop is a physical wire between pins 2 and 3
** on the RS-232 connector.
** A soft loop is the program in the PDP-8 echoing the
** character.
**
** Final test results:
** Linux raspberrypi 5.15.32-v8+ #1538 SMP PREEMPT Thu Mar 31 19:40:39 BST 2022 aarch64 GNU/Linux
** on Rpi400 at 19200 baud (hard loop) this is between 138 and 161.
** on Rpi400 at  9600 baud (hard loop) this is between 275 and 293.
** on Rpi400 at 19200 baud (soft loop) this is between 265 and 294.
** on Rpi400 at  9600 baud (soft loop) this is between 514 and 551.

** on Rpi3B+ at 19200 baud (hard loop) this is between 122 and 135.
** on Rpi3B+ at  9600 baud (hard loop) this is between 125 and 134.
** on Rpi3B+ at 19200 baud (soft loop) this is between 126 and 136.
** on Rpi3B+ at  9600 baud (soft loop) this is between 252 and 262.

*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/* the serial port will need to be defined for each installation. */
#define PDP_SP		"/dev/ttyUSB0"

/* The baud rate will need to match the PDP-8 baud rate.
** And while the console serial disk should work with any matching
** baud rate, it will be nearly useless at speeds below 9600 baud.
*/
#define PDP_BAUD	B19200

int pdp_fd;	/* descriptor that talks to the PDP-8 */

struct termios pdp_init;
struct termios pdp_set;

/* function to get a char from the pdp-8 */
int get_pdp8()
{
int cnt;
unsigned char ch;

  cnt = read(pdp_fd, &ch, 1);
  if (cnt==0) return -1;
  return ch;
}

int main( int argc, char *argv[] )
{
int i,j,k,l;

unsigned char chin, chout;

/* open the pdp-8 serial port */

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

/* Select one stop bit */
  pdp_set.c_cflag &= ~CSTOPB;

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


/* Send a counting sequence to be displayed in the AC.
** Either jumper tx to rx for a physical loopback or run an
** echo program.
**
** 0200	6031	KSF
** 0201 5200	JMP 200
** 0202 6036	KRB
** 0203 6046	TLS
** 0204 5200	JMP 200
*/

  for( i=0; i<8192; i++) {
    chout = i&0377;
    j = write(pdp_fd, &chout, 1);
    if(j!=1){
      fprintf(stderr, "write returned %d.  i=%d\n", j,i );
      exit(1);
    }
    k=0;
    while(read(pdp_fd, &chin, 1)==0)k++;
    if(k==0){
      fprintf(stderr, "it appears read is blocking\n");
      exit(1);
    }
    if(chin!=chout){
      fprintf(stderr, "chout=%03o chin=%03o\n", chout, chin);
      exit(1);
    } 
  }
  printf("read didn't block %d times on the last character\n", k);
  exit(1);

  close( pdp_fd );
  exit(0);
}
