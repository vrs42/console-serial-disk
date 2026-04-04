/* Serial port receiver test */

/* You run a little program on the PDP-8 which sends all
** codes.  This program receives them and if an error
** occurs it halts and reports what it saw.
**
** Here is the program for the PDP-8
**
** 0200 6046 TLS
** 0201 7001 IAC
** 0202 6041 TSF
** 0203 5202 JMP .-1
** 0204 5200 JMP 0200
**
** Start the recvtst program first, then on the PDP-8
**
** LOAD ADDR 0200
** Start or Clear Continue depending on which 8 you are using
**
** every 256 characters a . will be printed.  If something is
** not configured correctly you will get an expected and got
** message.  This probably means one of the serial ports is
** not configured for 8 bit no parity and 1 stop bit.  It could
** also mean the baud rate is wrong somewhere or if it runs a
** little while before it gets an error it could mean the baud
** rate is a little off on one side or the other.
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int debug_mode=0;	/* initialize debug to off */

/* the serial port will need to be defined for each installation. */
#define PDP_SP		"/dev/ttyUSB0"

/* The baud rate will need to match the PDP-8 baud rate.
** And while the console serial disk should work with any matching
** baud rate, it will be nearly useless at speeds below 9600 baud.
*/
#define PDP_BAUD	B9600

/* Use 0 for one stop bit and CSTOPB for two */
#define PDP_STOP	0

int pdp_fd;	/* descriptor that talks to the PDP-8 */

struct termios pdp_init;
struct termios pdp_set;

#include "pdp.c"

int main( int argc, char *argv[] )
{
int i,val;

unsigned char chin, chout;

/* open the pdp-8 serial port */

  pdp_setup();

  while( 1 ){
    for( i=0; i<256; i++ ){
      do {
        val=pdp_getch();
      } while( val == -1 );
      if( val != i ) {
        fprintf( stderr, "expected %04o got %04o\n", i, val );
        pdp_reset();
        exit(1);
      }
    }
    fprintf( stderr, "." );
  }

  pdp_reset();
  exit(0);
}
