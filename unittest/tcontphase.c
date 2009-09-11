/*---------------------------------------------------------------------------*\
                                                                          
  FILE........: tcontphase.c                                                  
  AUTHOR......: David Rowe                                            
  DATE CREATED: 11/9/09                                        
                                                               
  Test program for developing continuous phase track synthesis algorithms.
                                                                   
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

#define N  80		/* frame size          */
#define F 160           /* frames to synthesis */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "sine.h"
#include "dump.h"
#include "synth.h"

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
    FILE *fout;
    short buf[N];
    int   i,j; 
    int   dump;
    float phi_prev[MAX_AMP];
    float Wo_prev;
    
    if (argc < 2) {
	printf("\nusage: tcontphase OutputRawSpeechFile\n");
        exit(0);
    }

    /* Output file */

    if ((fout = fopen(argv[1],"wb")) == NULL) {
      printf("Error opening output speech file: %s\n",argv[1]);
      exit(1);
    }

    dump = switch_present("--dump",argc,argv);
    if (dump) 
      dump_on(argv[dump+1]);

    init_decoder();

    for(i=0; i<MAX_AMP; i++)
	phi_prev[i] = 0.0;
    Wo_prev = 0.0;
	
    model.L      = 1;
    model.A[1]   = 1000;
    model.Wo     = PI*(50.0/4000.0);
    model.phi[1] = 0;

    frames = 0;
    for(j=0; j<F; j++) {
	frames++;

	synthesise_continuous_phase(Pn, &model, Sn_, 1, &Wo_prev, phi_prev);
	for(i=0; i<N; i++)
	    buf[i] = Sn_[i];
	fwrite(buf,sizeof(short),N,fout);
    }

    fclose(fout);
    if (dump) dump_off();

    return 0;
}
 

