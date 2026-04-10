/* code sent to the 8 during OS/8 boot.
*/

unsigned char boot2code[]={
/* First send the help codes for the boot 2 loader.
** The console help loader starts loading at address 1.
  HELP	  ADR  MEMORY
  CODE	       VALUE */
  0000, /* 01  0000 */
  0004, /* 02  0000 */
  0327, /* 03  7032 */
  0127, /* 04  7012 */
  0214, /* 05  1021 */
  0211, /* 06  3021 */
  0317, /* 07  6031 */
  0072, /* 10  5007 */
  0367, /* 11  6036 */
  0067, /* 12  7006 */
  0067, /* 13  7006 */
  0021, /* 14  3002 */
  0317, /* 15  6031 */
  0152, /* 16  5015 */
  0367, /* 17  6036 */
  0024, /* 20  1002 */
  0011, /* 21  3001 */
  0011, /* 22  2001 */
  0005, /* 23  2000 */
  0076, /* 24  5007 */
  0032  /* 25  5003 */
};

void do_boot()
{
int i,l;

  l=sizeof(boot2code);	/* find out how many bytes of help code to send. */

  /* Step 1 is to send the boot2 help codes. */
  if( debug_mode ){
    fprintf( stderr, "SERVER: sending %d bytes of boot2 help code\r\n", l );
  }
  for( i=0; i<l; i++) pdp_putch( boot2code[i] );

  /* Step 2 is to feed it the boot block from csd0 */
  if( debug_mode ){
    fprintf( stderr, "SERVER: sending the boot block\r\n" );
  }
#ifdef PQS8
  /* Execution has reached boot2. WC and CA are zero. */
  /* Load WC and CA suitably to load the boot block at 07600. */
  /* Include a few extra words in WC, so that locations 0-4 */
  /* will also be loaded. */
  pdp_put12(07572); pdp_put12(07577);
#endif
  for( i=0; i<BLKSIZE; i++) pdp_put12( csd_dsk[0][0][i] );
#ifdef PQS8
  /* Load WC, CA, and BTTMP redundantly, then a branch to 07600. */
  pdp_put12(07773); pdp_put12(00001); pdp_put12(00000);
  pdp_put12(05403); pdp_put12(07600);
  /* At this point, WC wraps, causing a restart at 00003. */
  /* We've just placed a JMP at 00003, which will start the O/S. */
sleep(1);
#endif

/* boot complete */
    
}
