/* virtual disk routines */

#define CSD_MAX		4	/* maximum number of csd images */
#define CSD_BLKCNT	4095	/* max image block count */
int csd_dsk[CSD_MAX][CSD_BLKCNT][256];	/* consolidated csd images */
char csd_name[CSD_MAX][256];	/* file name table */
FILE *csd_stream[CSD_MAX];	/* csd streams */

#define TC_BLKCNT	737	/* block count of a DECtape */
int tc_dsk[TC_BLKCNT][256];	/* pointer to the DECtape image */
char tc_name[256];		/* file name */
FILE *tc_stream;		/* TC0X stream */

#define RK_BLKCNT	3248	/* block count of an RK05 image */
int rk_dsk[2][RK_BLKCNT][256];	/* RK05 platter as a memory image */
char rk_name[256];		/* file name */
FILE *rk_stream;		/* rk05 stream */

/* conversion table from 6 bit pdp-8 version of ASCII
** Note: The first char should be an @ but is used for string end
*/
char sixbit[64]={
' ','A','B','C','D','E','F','G',
'H','I','J','K','L','M','N','O',
'P','Q','R','S','T','U','V','W',
'X','Y','Z','[','\\',']','^','_',
' ','!','"','#','$','%','&','\'',
'(',')','*','+',',','-','.','/',
'0','1','2','3','4','5','6','7',
'8','9',':',';','<','=','>','?'};


int btcode[047]={	/* boot block boot code for verification. */
  07400,        /*  BTWC,   -0400           /WORD COUNT TO TRANSFER */
  00001,        /*  BTCA,   0001            /CURRENT ADDRESS TO STORE WORD.*/
  00000,        /*  BTTMP,  .-.             /THIS IS AN INITIAL DONT CARE VALUE.*/
  07605,        /*  OS8GO,  7605            /OS8 ENTRY POINT*/
  00047,        /*  BTSRC,  0047*/
  07647,        /*  BTDST1, 7647*/
  07600,        /*  BTDST2, 7600*/
  06031,        /*  BOOT2,  KSF             /GET A CHARACTER FROM SERIAL DISK SERVER*/
  05007,        /*          JMP .-1*/
  06036,        /*          KRB*/
  07006,        /*          RTL             /LEFT SHIFT 4 BITS TO BUILD UPPER HALF*/
  07006,        /*          RTL*/
  03002,        /*          DCA BTTMP       /SAVE FOR COMBINE*/
  06031,        /*          KSF             /GET LOWER 6 BITS*/
  05015,        /*          JMP .-1*/
  06036,        /*          KRB*/
  01002,        /*          TAD BTTMP*/
  03401,        /*          DCA I BTCA      /STORE THE WORD IN MEMORY*/
  02001,        /*          ISZ BTCA        /POINT AT NEXT ADDRESS*/
  02000,        /*          ISZ BTWC        /SKIP IF DONE*/
  05007,        /*          JMP BOOT2       /GO DO THE NEXT WORD*/
  01404,        /*  BTCPY1, TAD I BTSRC     /LOAD*/
  06211,        /*          CDF 10          /SWITCH TO FIELD 1*/
  03405,        /*          DCA I BTDST1    /STORE*/
  06201,        /*          CDF 00          /SWITCH TO FIELD 0*/
  02004,        /*          ISZ BTSRC       /BUMP SRC ADDRESS WILL NEVER SKIP*/
  02005,        /*          ISZ BTDST1      /BUMP DST ADDRESS AND SKIP IF DONE*/
  05025,        /*          JMP BTCPY1*/
  01404,        /*  BTCPY2, TAD I BTSRC     /LOAD*/
  03406,        /*          DCA I BTDST2    /STORE*/
  02004,        /*          ISZ BTSRC       /BUMP SRC ADDRESS WILL NEVER SKIP*/
  02006,        /*          ISZ BTDST2      /BUMP DST ADDRESS AND SKIP IF DONE*/
  05034,        /*          JMP BTCPY2*/
  05403,        /*          JMP I OS8GO     /AND GO START OS8*/
  00000,
  00000,
  00000,
  00000,
  00000
};

/* This is the handler code used to validate the boot block.
** I believe the first 7 will always be the same as they are
** the OS/8 entry point.
*/
int hndcode[0144]={	/* SYS Handler loads from 07607 through 07743	*/
04207,  /* 07600	JMS 7607	/ Call the SYS handler		*/
05000,  /* 07601	5000		/ Write 10 blks from field 0	*/
00000,  /* 07602	0000		/ starting at 0			*/
00033,  /* 07603	0033		/ to block 33 on the disk	*/
07602,  /* 07604	CLA HLT		/ Error returns here		*/
06213,  /* 07605	CIF CFD 10	/ Far JMP to field 1		*/
05267,  /* 07606	JMP 7667	/ JMP 17667			*/
/* Handler code follows.
** Manually changed when handler is updated.
*/
00001,  /* 07607 */
04243,  /* 07610 */
00000,  /* 07611 */
06041,  /* 07612 */
05212,  /* 07613 */
06046,  /* 07614 */
05611,  /* 07615 */
00000,  /* 07616 */
04211,  /* 07617 */
07112,  /* 07620 */
07012,  /* 07621 */
04211,  /* 07622 */
07300,  /* 07623 */
05616,  /* 07624 */
00000,  /* 07625 */
06031,  /* 07626 */
05226,  /* 07627 */
06036,  /* 07630 */
07106,  /* 07631 */
07006,  /* 07632 */
03352,  /* 07633 */
06031,  /* 07634 */
05234,  /* 07635 */
06036,  /* 07636 */
01352,  /* 07637 */
05625,  /* 07640 */
00000,  /* 07641 */
04243,  /* 07642 */
00000,  /* 07643 */
07340,  /* 07644 */
03342,  /* 07645 */
01256,  /* 07646 */
03352,  /* 07647 */
06041,  /* 07650 */
05253,  /* 07651 */
05257,  /* 07652 */
02352,  /* 07653 */
05250,  /* 07654 */
02342,  /* 07655 */
07530,  /* 07656 */
06214,  /* 07657 */
07012,  /* 07660 */
07010,  /* 07661 */
01327,  /* 07662 */
06046,  /* 07663 */
07200,  /* 07664 */
01243,  /* 07665 */
04216,  /* 07666 */
07240,  /* 07667 */
06031,  /* 07670 */
07001,  /* 07671 */
03343,  /* 07672 */
06036,  /* 07673 */
04211,  /* 07674 */
04225,  /* 07675 */
03350,  /* 07676 */
04225,  /* 07677 */
03351,  /* 07700 */
04225,  /* 07701 */
07500,  /* 07702 */
05316,  /* 07703 */
03313,  /* 07704 */
02342,  /* 07705 */
06042,  /* 07706 */
06031,  /* 07707 */
05307,  /* 07710 */
02343,  /* 07711 */
06032,  /* 07712 */
07402,  /* 07713 */
01351,  /* 07714 */
05750,  /* 07715 */
07006,  /* 07716 */
07001,  /* 07717 */
03321,  /* 07720 */
07402,  /* 07721 */
07430,  /* 07722 */
05333,  /* 07723 */
04225,  /* 07724 */
03750,  /* 07725 */
02350,  /* 07726 */
00020,  /* 07727 */
02351,  /* 07730 */
05324,  /* 07731 */
05275,  /* 07732 */
01750,  /* 07733 */
02350,  /* 07734 */
07000,  /* 07735 */
04216,  /* 07736 */
02351,  /* 07737 */
05333,  /* 07740 */
05275,  /* 07741 */
00000,  /* 07742 */
00000,  /* 07743 */
};

void disk_open()
{
int d;		/* walk through devices */
int blk;	/* walk through blocks */
int i,j,k;	/* indexes and temps */
int patch_bt;	/* flag to indicate if the boot block needs to be patched with the handler */

  /* open the CSD images */
  for( d=0; d<CSD_MAX; d++ ){
    if( debug_mode ){
      fprintf( stderr, "SERVER: Opening CSD%d image %s\r\n", d, csd_name[d] );
    }
    csd_stream[d]=fopen( csd_name[d], "r+" );
    if( csd_stream[d]==NULL ){
      if( d==0 ) {
        fprintf( stderr, "SERVER: can't open CSD image %s for read/write\r\n", csd_name[d] );
        exit( 1 );	/* exit if this is the system device */
      }
      if( debug_mode )
        fprintf( stderr, "SERVER: can't open CSD image %s for read/write\r\n", csd_name[d] );
    } else {	/* read the image into memory */
      if( debug_mode ){
        fprintf( stderr, "SERVER: Reading the CSD image %s into memory.\r\n", csd_name[d] );
      }
      for( blk=0; blk<CSD_BLKCNT; blk++ ){	/* read through all the blocks */
        for( i=0; i<256; i++ ){			/* fetch all the words in the block */
          j=fgetc( csd_stream[d] );		/* get the first byte making up the word */
          if( j==EOF ) j=0;			/* handler short images?? */
          if( (j<0) || (j>0377) ){		/* we have a problem */
            fprintf( stderr, "SERVER: invalid CSD image %s\r\n", csd_name[d] );
            exit(1);
          }
          k=fgetc( csd_stream[d] );		/* get the second byte of the word */
          if( k==EOF ) k=0;			/* handle short images */
          if( (k<0) || (k>017) ){	/* we have a problem */
            fprintf( stderr, "SERVER: invalid CSD image %s\r\n", csd_name[d] );
            exit(1);
          }
          csd_dsk[d][blk][i]= (k<<8) | j;		/* assemble the word */
        }
      }
    }
  }

  /* open the rk05 image */
  if( debug_mode ){
    fprintf( stderr, "SERVER: Opening RK05 image %s\r\n", rk_name );
  }
  rk_stream=fopen( rk_name, "r+" );
  if( rk_stream==NULL ){
    if( debug_mode )
      fprintf( stderr, "SERVER: can't open RK05 image %s for read/write\r\n", rk_name );
  } else {
    /* read the image into memory */
    if( debug_mode ){
      fprintf( stderr, "SERVER: Reading the RK05 image into memory.\r\n" );
    }
    for( half=0; half<2; half++){
      for( blk=0; blk<RK_BLKCNT; blk++ ){	/* read through all the blocks */
        for( i=0; i<256; i++ ){		/* fetch all the words in each block */
          j=fgetc( rk_stream );		/* get the first byte making up the word */
          if( j==EOF ) j=0;			/* handle short images */
          if( (j<0) || (j>0377) ){	/* we have a problem */
            fprintf( stderr, "SERVER: disk_open invalid image half=%o Blk=%04o wrd=%03o j=%04o\r\n",
              half, blk, i, j );
            j=0;
          }
          k=fgetc( rk_stream );		/* get the second byte making up the word */
          if( k==EOF ) k=0;			/* handle short images */
          if( (k<0) || (k>017) ){	/* we have a problem */
            fprintf( stderr, "SERVER: disk_open invalid image half=%o Blk=%04o wrd=%03o k=%02o\r\n",
              half, blk, i, k );
            k=0;
          }
          rk_dsk[half][blk][i]= (k<<8) | j;		/* assemble the word */
        }
      }
    }
  }

  /* open the DECtape image */
  if( debug_mode ){
    fprintf( stderr, "SERVER: Opening DECTape image %s\r\n", tc_name );
  }
  tc_stream=fopen( tc_name, "r+" );
  if( tc_stream==NULL ){
    if( debug_mode )
      fprintf( stderr, "SERVER: can't open DECtape image %s for read/write\r\n", tc_name );
  } else {
    /* read the image into memory */
    if( debug_mode ){
      fprintf( stderr, "SERVER: Reading the DECtape image into memory.\r\n" );
    }
    for( blk=0; blk<TC_BLKCNT; blk++ ){	/* read through all the blocks */
      for( i=0; i<256; i++ ){		/* fetch all the words in each block */
        j=fgetc( tc_stream );		/* get the first byte making up the word */
        if( j==EOF ) j=0;			/* handle short images */
        if( (j<0) || (j>0377) ){	/* we have a problem */
          fprintf( stderr, "SERVER: disk_open invalid image Blk=%04o wrd=%03o j=%04o\r\n",
            blk, i, j );
          j=0;
        }
        k=fgetc( tc_stream );		/* get the second byte making up the word */
        if( k==EOF ) k=0;			/* handle short images */
        if( (k<0) || (k>017) ){	/* we have a problem */
          fprintf( stderr, "SERVER: disk_open invalid image Blk=%04o wrd=%03o k=%02o\r\n",
            blk, i, k );
          k=0;
        }
        tc_dsk[blk][i]= (k<<8) | j;		/* assemble the word */
        if( (i==127) || (i==255) ) {	/* skip word 129 of each record */
          fgetc( tc_stream );		/* just ignore it for now */
          fgetc( tc_stream );		/* No good way to handle this */
        }
      }
    }
  }

  /* make certain this is a system image */
  if( csd_dsk[0][1][1]!=070 ){		/* it is not a system image */
      fprintf( stderr, "SERVER: %s is not a SYS image.\r\n", csd_name[0] );
      exit(1);
  }

  if( debug_mode )
    fprintf( stderr, "SERVER: Calculating installed handler checksum.\r\n" );

  k=0;
  for( i=0207; i<0343; i++ ){
    k=k+csd_dsk[0][0][i];
  }
  if( debug_mode )
    fprintf( stderr, "SERVER: Installed handler checksum = %d.\r\n", k );

  patch_bt=1;	/* do the patch unless turned off */
  switch( k ){
  case 204190:	/* an RK05 handler Ver 3 */
  case 204196:	/* an RK05 handler Ver 6 */
    fprintf( stderr, "SERVER: %s appears to have an RK05 handler\r\n", csd_name[0] );
    break;
  case 216869:	/* Older CONSYS.BN */
    fprintf( stderr, "SERVER: %s has an old csd SYS handler.\r\n", csd_name[0] );
    break;
  case 216467:	/* CONSYS.BN from 20220802 */
    if( debug_mode )
      fprintf( stderr, "SERVER: %s appears to have CONSYS.BN (20220802)\r\n", csd_name[0] );
    patch_bt=0;	/* don't need to patch the boot block */
    break;
  default:
    fprintf( stderr, "SERVER: Unknown Image type for file %s cksum=%d. Exiting.\r\n", csd_name[0], k );
    exit(1);
  };

  /* verify the boot code on the boot block here.*/
  if( debug_mode )
    fprintf( stderr, "SERVER: Verifying the boot code on the boot block\r\n" );

  for( i=0; i<=046; i++ ){
    if( patch_bt )
      csd_dsk[0][0][i]=btcode[i];	/* patch it in */
    else
      if( csd_dsk[0][0][i]!=btcode[i] ) {
        fprintf( stderr, "SERVER: boot code does not match\r\n" );
        fprintf( stderr, "SERVER: addr %05o expected %05o got %05o\r\n",
                         i, btcode[i], csd_dsk[0][0][i] );
        exit(1);
      }
  }

  /* look at the OS/8 entry point */
  if( debug_mode )
    fprintf( stderr, "SERVER: Examining the OS/8 entry point on the boot block\r\n" );
  
  for( i=0; i<07; i++){
    if( hndcode[i] != csd_dsk[0][0][i+0200] ){
      fprintf( stderr, "addr: %05o expected: %05o got: %05o\r\n",
	i+07600, hndcode[i], csd_dsk[0][0][i+0200] );
    }
  }

  /* look at the handler code */
  if( debug_mode )
    fprintf( stderr, "SERVER: Verifying the handler on the boot block\r\n" );
 
  for( i=07; i<0144; i++ ){
    if( patch_bt )
      csd_dsk[0][0][i+0200]=hndcode[i];	/* patch it in to build initial platter. */
    else
      if( hndcode[i] != csd_dsk[0][0][i+0200] ){
        fprintf( stderr, "SERVER: Boot handler does not match at addr: %05o ", i+07600 );
        fprintf( stderr, "expected %05o and got %05o\r\n",
                          hndcode[i], csd_dsk[0][0][i+200] );
      }
  }

}

void disk_close()
{
int d;		/* device index */
int blk;	/* block index */
int i,j,k;	/* indexes and temps */

  /* do the CSD images */
  for( d=0; d<CSD_MAX; d++ ){
    if( csd_stream[d] != NULL ){
      rewind( csd_stream[d] );
      if( debug_mode ){
        fprintf( stderr, "SERVER: Writing out the CSD%d image.\r\n", d );
      }
      for( blk=0; blk<CSD_BLKCNT; blk++ ){
        for( i=0; i<256; i++ ){
          j=csd_dsk[d][blk][i];
          fputc( j&0377, csd_stream[d] );
          fputc( (j>>8)&0377, csd_stream[d] );
        }
      }
      fclose( csd_stream[d] );
    }
  }

  /* do the RK05 image */
  if( rk_stream != NULL ){
    rewind( rk_stream );
    if( debug_mode ){
      fprintf( stderr, "SERVER: Writing out the RK05 image.\r\n" );
    }
    for( half=0; half<2; half++){
      for( blk=0; blk<RK_BLKCNT; blk++ ){	/* all the blocks */
        for( i=0; i<256; i++ ){		/* all the words in each block */
          j=rk_dsk[half][blk][i];		/* get the word */
          fputc( j&0377, rk_stream );
          fputc( (j>>8)&0377, rk_stream );
        }
      }
    }
    fclose( rk_stream );
  }

  /* do the DECtape image */
  if( tc_stream != NULL ){
    rewind( tc_stream );
    if( debug_mode ){
      fprintf( stderr, "SERVER: Writing out the DECtape image.\r\n" );
    }
    for( blk=0; blk<TC_BLKCNT; blk++ ){	/* all the blocks */
      for( i=0; i<128; i++ ){		/* all the words in each block */
        j=tc_dsk[blk][i];		/* get the word */
        fputc( j&0377, tc_stream );
        fputc( (j>>8)&0377, tc_stream );
      }
      fputc( 0, tc_stream );		/* write the last word on the record */
      fputc( 0, tc_stream );
      for( i=128; i<256; i++ ){		/* all the words in each block */
        j=tc_dsk[blk][i];		/* get the word */
        fputc( j&0377, tc_stream );
        fputc( (j>>8)&0377, tc_stream );
      }
      fputc( 0, tc_stream );		/* write the last word on the record */
      fputc( 0, tc_stream );
    }
    fclose( tc_stream );
  }

}
