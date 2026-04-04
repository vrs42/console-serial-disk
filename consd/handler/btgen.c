/* Program to massage a palbart listing to produce
** a table of octal numbers to include in a C
** program.
**
** based on hlpgen.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 256

int main(int argc, char**argv)
{
int l;
unsigned address,data;
char line[MAXLINE];
FILE *inf,*otf;
  inf=stdin;
  otf=stdout;
  if (argc>3) {
    printf("Usage: btgen [infile [outfile]]\n");
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

      l=strlen(line);	/* get the length */
      if (l<=2) continue;		/* this is a blank line or a formfeed */
      if (line[11]!='*') continue;
      if (line[7]!='7') continue;

      sscanf(line, "%*6c%5o* %4o", &address, &data);

      fprintf( otf, "%05o,  /* %05o */\n", data, address );
    }
  } while(!feof(inf));

  exit(0);
}
