/*---------------------------------------------------------------------------*\
                                                               
  FILE........: sinenc.c                                    
  AUTHOR......: David Rowe                                  
  DATE CREATED: 20/2/95                                       
                                                                             
  Sinusoidal speech encoder program using external (Matlab) pitch estimator.
                                                                             
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

#include "sine.h"
#include "dump.h"
#include <string.h>

/*---------------------------------------------------------------------------*\
                                                                             
 switch_present()                                                            
                                                                             
 Searches the command line arguments for a "switch".  If the switch is       
 found, returns the command line argument where it ws found, else returns    
 NULL.                                                                       
                                                                             
\*---------------------------------------------------------------------------*/

int switch_present(sw,argc,argv)
register char sw[];     /* switch in string form */
register int argc;      /* number of command line arguments */
register char *argv[];  /* array of command line arguments in string form */
{
  register int i;       /* loop variable */

  for(i=1; i<argc; i++)
    if (!strcmp(sw,argv[i]))
      return(i);

  return 0;
}

/*---------------------------------------------------------------------------*\
                                                                            
				    MAIN                                          
                                                                             
\*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
  FILE *fin;		/* input speech sample file */
  FILE *fmodel;		/* output file of model parameters */
  FILE *fp;		/* input text file containing pitch estimates */
  short buf[N];		/* input speech sample buffer */
  int length;		/* number of frames to process */
  float pitch;		/* current pitch estimate from external pitch file */
  int i;		/* loop variable */
  FILE *fref;		/* optional output file with refined pitch estimate */
  int   arg;
  int   dump;
  int   frames;

  if (argc < 5) {
    printf("usage: sinenc InputFile ModelFile Frames PitchFile\n");
    exit(1);
  }

  /* Interpret command line arguments -------------------------------------*/

  if ((fin = fopen(argv[1],"rb")) == NULL) {
    printf("Error opening input file: %s\n",argv[1]);
    exit(1);
  }

  if ((fmodel = fopen(argv[2],"wb")) == NULL) {
    printf("Error opening output model file: %s\n",argv[2]);
    exit(1);
  }

  length = atoi(argv[3]);

  if ((fp = fopen(argv[4],"rt")) == NULL) {
    printf("Error opening input pitch file: %s\n",argv[4]);
    exit(1);
  }

  dump = switch_present("--dump",argc,argv);
  if (dump) 
      dump_on(argv[dump+1]);

  if ((arg == switch_present("--ref",argc,argv))) {
    if ((fref = fopen(argv[arg+1],"wt")) == NULL) {
      printf("Error opening output pitch refinement file: %s\n",argv[5]);
      exit(1);
    }
  }
  else
    fref = NULL;

  init_encoder();
  Nw = 220;
  make_window(Nw);

  /* Main loop ------------------------------------------------------------*/

  frames = 0;
  while((fread(buf,sizeof(short),N,fin) == N) && (frames != length)) {
    frames++;

    /* Update input speech buffers */

    for(i=0; i<N+AW_ENC/2; i++)
      Sn[i] = Sn[i+N];
    for(i=0; i<N; i++)
      Sn[i+N+AW_ENC/2] = buf[i];

    /* Estimate pitch */

    if (frames > 2) {
      fscanf(fp,"%f\n",&pitch);
      if (pitch > P_MAX) pitch = P_MAX;
      if (pitch < P_MIN) pitch = P_MIN;
    }
    else
      pitch = P_MIN;

    /* construct analysis window */

    model.Wo = TWO_PI/pitch;

    /* estimate and model parameters */

    dft_speech(); 
    two_stage_pitch_refinement();
    estimate_amplitudes();
    dump_Sn(Sn); dump_Sw(Sw); dump_Sw_(Sw_); dump_model(&model);

    /* save model parameters */

    if (fref != NULL && frames > 2)
      fprintf(fref,"%f\n",model.Wo);
    fwrite(&model,sizeof(model),1,fmodel);
    printf("frame: %d\r",frames);
  }

  /* close files and exit */

  if (fref != NULL) fclose(fref);
  fclose(fin);
  fclose(fmodel);

  if (dump)
      dump_off();

  return 0;
}

