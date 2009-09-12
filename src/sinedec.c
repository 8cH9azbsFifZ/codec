/*---------------------------------------------------------------------------*\

  FILE........: sinedec.c
  AUTHOR......: David Rowe
  DATE CREATED: 20/2/95

  Decoder program for sinudoidal codec.

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

#include <assert.h>
#include <string.h>
#include "sine.h"
#include "quantise.h"
#include "dump.h"
#include "phase.h"
#include "lpc.h"
#include "synth.h"

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
  FILE *fmodel;		/* file of model parameters from encoder */
  FILE *fout;		/* output speech file */
  FILE *fin;		/* input speech file */
  short buf[N];		/* input/output buffer */
  int i;		/* loop variable */
  int length;		/* number of frames so far */

  char  out_file[MAX_STR];
  int   arg;
  float snr;
  float sum_snr;

  int lpc_model, order;
  int lsp, lsp_quantiser;
  float ak[LPC_MAX_ORDER+1];

  int dump;
  
  int phase, phase_model;
  float prev_Wo, ex_phase;

  if (argc < 3) {
    printf("usage: sinedec InputFile ModelFile [-o OutputFile] [-o lpc Order]\n");
    printf("       [--dump DumpFilePrefix]\n");
    exit(0);
  }

  /* Interpret command line arguments -------------------------------------*/

  /* Input file */

  if ((fin = fopen(argv[1],"rb")) == NULL) {
    printf("Error opening input speech file: %s\n",argv[1]);
    exit(1);
  }

  /* Model parameter file */

  if ((fmodel = fopen(argv[2],"rb")) == NULL) {
    printf("Error opening model file: %s\n",argv[2]);
    exit(1);
  }

  /* Output file */

  if ((arg = switch_present("-o",argc,argv))) {
    if ((fout = fopen(argv[arg+1],"wb")) == NULL) {
      printf("Error opening output speech file: %s\n",argv[arg+1]);
      exit(1);
    }
    strcpy(out_file,argv[arg+1]);
  }
  else
    fout = NULL;

  /* Length (no. of frames) */

  if ((length = switch_present("-l",argc,argv))) {
    length = atoi(argv[length+1]);
    if (length < 0) {
      printf("Error in length: %d\n",length);
      exit(1);
    }
  }
  else
    length = 32000;

  lpc_model = 0;
  if ((arg = switch_present("--lpc",argc,argv))) {
      lpc_model = 1;
      order = atoi(argv[arg+1]);
      if ((order < 4) || (order > 20)) {
        printf("Error in lpc order: %d\n", order);
        exit(1);
      }	  
  }

  dump = switch_present("--dump",argc,argv);
  if (dump) 
      dump_on(argv[dump+1]);

  lsp = switch_present("--lsp",argc,argv);
  lsp_quantiser = 0;
  if (lsp) 
      lsp_quantiser = atoi(argv[lsp+1]);

  /* phase_model 0: zero phase
     phase_model 1: 1st order polynomial */
  phase = switch_present("--phase",argc,argv);
  if (phase) {
      phase_model = atoi(argv[phase+1]);
      assert((phase_model == 0) || (phase_model == 1));
  }

  /* Initialise ------------------------------------------------------------*/

  init_decoder();
  init_encoder();
  Nw = 220;
  make_window(Nw);
  quantise_init();

  /* Main loop ------------------------------------------------------------*/

  frames = 0;
  sum_snr = 0;
  while(fread(&model,sizeof(model),1,fmodel)) {
    frames++;

    /* Read input speech */

    fread(buf,sizeof(short),N,fin);
    for(i=0; i<M-N; i++)
      Sn[i] = Sn[i+N];
    for(i=0; i<N; i++)
      Sn[i+M-N] = buf[i];
    dump_Sn(Sn);
    dft_speech(); dump_Sw(Sw);   

    dump_model(&model);

    /* optional LPC model amplitudes */

    if (lpc_model) {
	snr = lpc_model_amplitudes(Sn, &model, order, lsp_quantiser, ak);
	sum_snr += snr;
        dump_quantised_model(&model);
    }

    /* optional phase modelling */

    if (phase) {
	float Wn[M];		        /* windowed speech samples */
	float Rk[PHASE_LPC_ORD+1];	/* autocorrelation coeffs  */
        COMP  H[MAX_AMP];               /* LPC freq domain samples */
	float n_min;
	COMP  min_Am;
	
	dump_phase(&model.phi[0]);

	if (!lpc_model) {
	    /* Determine LPC model using time domain LPC if we don't have
	       any LPCs yet */

	    for(i=0; i<M; i++)
		Wn[i] = Sn[i]*w[i];
	    autocorrelate(Wn,Rk,M,PHASE_LPC_ORD);
	    levinson_durbin(Rk,ak,PHASE_LPC_ORD);
	}
	else
	    assert(order == PHASE_LPC_ORD);

	dump_ak(ak, PHASE_LPC_ORD);
	snr = phase_model_first_order(ak, H, &n_min, &min_Am);

	dump_snr(snr);
	if (phase_model == 0) {
	    /* just to make sure we are not cheating - kill all phases */
	    for(i=0; i<MAX_AMP; i++)
	    	model.phi[i] = 0;
	    phase_synth_zero_order(snr, H, &prev_Wo, &ex_phase);
	}

	if (phase_model == 1) {
	    phase_synth_first_order(snr, H, n_min, min_Am);
            dump_phase_(&model.phi[0]);
        }
    }

    /* Synthesise speech */

    if (fout != NULL) {

	synthesise_mixed(Pn,&model,Sn_);

	/* Save output speech to disk */

	for(i=0; i<N; i++) {
	    if (Sn_[i] > 32767.0)
		buf[i] = 32767;
	    else if (Sn_[i] < -32767.0)
		buf[i] = -32767;
	    else
		buf[i] = Sn_[i];
	}
	fwrite(buf,sizeof(short),N,fout);
    }    
  }

  if (fout != NULL)
    fclose(fout);

  if (lpc_model)
      printf("SNR av = %5.2f dB\n", sum_snr/frames);

  if (dump)
      dump_off();

  return 0;
}

