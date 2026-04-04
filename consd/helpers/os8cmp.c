/* compare two os/8 images.
**  compares the system area of csd0.img with rk0.img
*/

#include <stdio.h>
#include <stdlib.h>


#define CSD_BLKCNT	4095	/* max image block count */
int csd_dsk[CSD_BLKCNT][256];	/* consolidated csd images */

#define RK_BLKCNT	3248	/* block count of an RK05 image */
int rk_dsk[2][RK_BLKCNT][256];	/* RK05 platter as a memory image */

FILE *inf1, *inf2;

void disk_open()
{
int half;	/* walk through RK05 parts */
int blk;	/* walk through blocks */
int i,j,k;	/* indexes and temps */
int patch_bt;	/* flag to indicate if the boot block needs to be patched with the handler */

  /* open the CSD image */
  inf1=fopen( "csd0.img", "r+" );
  if( inf1==NULL ){
    fprintf( stderr, "SERVER: can't open csd0.img for read/write\r\n" );
    exit(1);
  } else {	/* read the image into memory */
    for( blk=0; blk<CSD_BLKCNT; blk++ ){	/* read all the blocks */
      for( i=0; i<256; i++ ){	/* fetch all the words in the block */
        j=fgetc( inf1 );	/* get the first byte making up the word */
        if( j==EOF ) j=0;	/* handle short images?? */
        if( (j<0) || (j>0377) ){		/* we have a problem */
          fprintf( stderr, "SERVER: invalid csd0.img\r\n" );
          exit(1);
        }
        k=fgetc( inf1 );	/* get the second byte of the word */
        if( k==EOF ) k=0;	/* handle short images */
        if( (k<0) || (k>017) ){	/* we have a problem */
          fprintf( stderr, "SERVER: invalid csd0.img\r\n" );
          exit(1);
        }
        csd_dsk[blk][i]= (k<<8) | j;		/* assemble the word */
      }
    }
  }
  fclose( inf1);

  /* open the rk05 image */
  inf2=fopen( "rk0.img", "r+" );
  if( inf2==NULL ){
    fprintf( stderr, "SERVER: can't open rk0.img for read/write\r\n" );
    exit(1);
  } else {
    /* read the image into memory */
    for( half=0; half<2; half++){
      for( blk=0; blk<RK_BLKCNT; blk++ ){	/* read all the blocks */
        for( i=0; i<256; i++ ){		/* fetch all the words in each block */
          j=fgetc( inf2 );	/* get the first byte making up the word */
          if( j==EOF ) j=0;	/* handle short images */
          if( (j<0) || (j>0377) ){	/* we have a problem */
            fprintf( stderr, "SERVER: disk_open invalid rk0.img half=%o Blk=%04o wrd=%03o j=%04o\r\n",
              half, blk, i, j );
            j=0;
          }
          k=fgetc( inf2 );	/* get the second byte making up the word */
          if( k==EOF ) k=0;	/* handle short images */
          if( (k<0) || (k>017) ){	/* we have a problem */
            fprintf( stderr, "SERVER: disk_open invalid rk05.img half=%o Blk=%04o wrd=%03o k=%02o\r\n",
              half, blk, i, k );
            k=0;
          }
          rk_dsk[half][blk][i]= (k<<8) | j;	/* assemble the word */
        }
      }
    }
  }
  fclose( inf2);

  /* this will need to be switched to the CSD0 image */
  half=0;	/* really only makes sense to use the first image. */
  /* make certain this is a system image */
  if( rk_dsk[half][1][1]!=070 ){		/* it is not a system image */
      fprintf( stderr, "SERVER: rk05.img does not appear to be a system image.\r\n" );
      exit(1);
  }
}

int main()
{
  disk_open();
  exit(0);
}
