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

#include <string.h>
#include "sine.h"

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

  char out_file[MAX_STR];
  int arg;

  if (argc < 3) {
    printf("usage: sinedec InputFile ModelFile [-o OutputFile]\n");
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

  /* Initialise ------------------------------------------------------------*/

  init_decoder();
  init_encoder();
  Nw = 220;
  make_window(Nw);

  /* Main loop ------------------------------------------------------------*/

  frames = 0;
  while(fread(&model,sizeof(model),1,fmodel) /*&& frames < 1200*/) {
    frames++;

    /* Read input speech */

    fread(buf,sizeof(short),N,fin);
    for(i=0; i<N+AW_ENC/2; i++)
      Sn[i] = Sn[i+N];
    for(i=0; i<N; i++)
      Sn[i+N+AW_ENC/2] = buf[i];

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

  return 0;
}

