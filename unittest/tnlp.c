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

#define N 80		/* frame size */
#define M 320		/* pitch analysis window size */
#define PITCH_MIN 20
#define PITCH_MAX 160
#define TNLP

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "nlp.h"
#include "dump.h"

int   frames;

/*---------------------------------------------------------------------------*\
                                                                             
 switch_present()                                                            
                                                                             
 Searches the command line arguments for a "switch".  If the switch is       
 found, returns the command line argument where it ws found, else returns    
 NULL.                                                                       
                                                                             
\*---------------------------------------------------------------------------*/

int switch_present(sw,argc,argv)
  char sw[];     /* switch in string form */
  int argc;      /* number of command line arguments */
  char *argv[];  /* array of command line arguments in string form */
{
  int i;       /* loop variable */

  for(i=1; i<argc; i++)
    if (!strcmp(sw,argv[i]))
      return(i);

  return 0;
}

/*---------------------------------------------------------------------------*\

                                    MAIN

\*---------------------------------------------------------------------------*/

int main(argc,argv)
int argc;
char *argv[];
{
    FILE *fin,*fout;
    short buf[N];
    float pitch;
    int   i; 
    int   dump;
    
    if (argc < 3) {
	printf("\nusage: tnlp InputRawSpeechFile OutputPitchTextFile "
	       "[--dump DumpFile]\n");
        exit(0);
    }

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

    dump = switch_present("--dump",argc,argv);
    if (dump) 
      dump_on(argv[dump+1]);

    init_encoder();
    make_window(NW);

    /* align with current version of sinenc.c, fix this later */

    frames = 0;
    while(fread(buf,sizeof(short),N,fin)) {
      frames++;

      /* Update input speech buffers */

      for(i=0; i<M-N; i++)
        Sn[i] = Sn[i+N];
      for(i=0; i<N; i++)
        Sn[i+M-N] = buf[i];
      dft_speech();
      dump_Sn(Sn); dump_Sw(Sw); 

      nlp(Sn,N,M,PITCH_MIN,PITCH_MAX,&pitch,Sw);

      fprintf(fout,"%f\n",pitch);

      printf("frame: %d  pitch: %f\n",frames,pitch);
    }

    fclose(fin);
    fclose(fout);
    if (dump) dump_off();

    return 0;
}
 

