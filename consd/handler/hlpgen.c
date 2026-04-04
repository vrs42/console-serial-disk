#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 256

int main(int argc, char**argv)
{
int i,l,p,t;
unsigned address,data;
unsigned instruction[33];
unsigned helpcode[33];
char line[MAXLINE];
FILE *inf,*otf;
  inf=stdin;
  otf=stdout;
  if (argc>3) {
    printf("Usage: hlpgen [infile [outfile]]\n");
    exit(1);
  }

  if (argc>2) {
    otf=fopen(argv[2], "w");
    /* do error checking here? */
  }

  if (argc>1) {
    inf=fopen(argv[1], "r");
    /* do error checking here? */
  }

  do {
    if (fgets(line, MAXLINE, inf)==line) {
      sscanf(line, "%*6c%5o %4o", &address, &data);
      instruction[address]=data;
      if (data&0740) {	/* if these bits are set we can't convert it */
        printf("ADDR=%05o DATA=%04o\n", address, data);
      }
    }
  } while(!feof(inf));

  instruction[address+1]=0;

  for(i=1;i<=address;i++) {
    t=instruction[i];
    helpcode[i]=((t<<3)&0370)|((t>>10)&03)|((instruction[i+1]&01000)>>7);
  }

  p=0;	/* previous link */
  for(i=1;i<=address;i++) {
    t=helpcode[i];
    l=(t>>2)&1;		/* new link */
    t= ((t>>3)&037) | ((t<<10)&06000) | (p<<9);
    p=l;
    if (instruction[i]==t) fprintf(otf, "  %04o, /* %02o  %04o */\n", helpcode[i], i, instruction[i]);
    else printf("ERROR: Addr=%04o Orig=%04o Help=%03o Recv=%04o\n", i, instruction[i], helpcode[i], t);
  }

  if (p) printf("Final link not zero.\n");

  exit(0);
}
