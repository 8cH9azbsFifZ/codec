/*---------------------------------------------------------------------------*\
                                                                          
  FILE........: tnlp.c                                                  
  AUTHOR......: David Rowe                                            
  DATE CREATED: 23/3/93                                        
                                                               
  Test program for non linear pitch estimation functions.  
                                                                   
\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2009 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define N 160		/* frame size */
#define M 320		/* pitch analysis window size */
#define PITCH_MIN 20
#define PITCH_MAX 133
#define TNLP

int frames;

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "four1.c"
#include "nlpl.c"

/*---------------------------------------------------------------------------*\

                                    FUNCTIONS

\*---------------------------------------------------------------------------*/

void swap(buf,n)
short buf[];	/* array of speech samples */
int n;		/* number of speech samples */
{
  int i;
  short a,b;
  
  for(i=0; i<n; i++) {
    a = buf[i] & 0xff;
    b = (buf[i] >> 8) & 0xff;
    buf[i] = (a << 8) | b;
  }
}

void short_to_float(b,s,n)
short b[];	/* buffer of short speech samples */
float s[];	/* buffer of float speech samples */
int n;		/* number of speech samples */
{
  int i;
  
  for(i=0; i<n; i++)
    s[i] = (float)b[i];
}

/*---------------------------------------------------------------------------*\

                                    MAIN

\*---------------------------------------------------------------------------*/

void main(argc,argv)
int argc;
char *argv[];
{
  FILE *fin,*fout;
  short buf[N];
  float Sn[N];
  float pitch;
  int i;
  int pbin;

  if (argc == 3) {

    /* Input file */

    if ((fin = fopen(argv[1],"rb")) == NULL) {
      printf("Error opening input speech file: %s\n",argv[1]);
      exit(1);
    }

    /* Output file */

    if ((fout = fopen(argv[2],"wt")) == NULL) {
      printf("Error opening output text file: %s\n",argv[2]);
      exit(1);
    }

    frames = 0;
    pbin = 102;
    while(fread(buf,sizeof(short),N,fin)) {
      frames++;
      short_to_float(buf,Sn,N);
      nlpl(Sn,N,M,N-NTAP/2,PITCH_MIN,PITCH_MAX,&pitch,&pbin);

      /* Compensate for delay in C version compared to Matlab */

      if (frames > 2)
	fprintf(fout,"%f\n",pitch);

      printf("frame: %d  pitch: %f\n",frames,pitch);

    }
    fprintf(fout,"0\n0\n");

    fclose(fin);
    fclose(fout);
  }
  else
    printf("\nusage: tnlp InputFile OutputFile\n");
}
 

