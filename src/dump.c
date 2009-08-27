/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: dump.c
  AUTHOR......: David Rowe                                                          
  DATE CREATED: 25/8/09                                                       
                                                                             
  Routines to dump data to text files for Octave analysis.

\*---------------------------------------------------------------------------*/

/*
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

#include "dump.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static int dumpon = 0;

static FILE *fsn;
static FILE *fsw;
static FILE *fmodel;
static FILE *fqmodel;
static FILE *fpw;
static FILE *flsp;

void dump_on(char prefix[]) {
    char s[MAX_STR];

    dumpon = 1;

    sprintf(s,"%s_sn.txt", prefix);
    fsn = fopen(s, "wt");
    assert(fsn != NULL);

    sprintf(s,"%s_sw.txt", prefix);
    fsw = fopen(s, "wt");
    assert(fsw != NULL);

    sprintf(s,"%s_model.txt", prefix);
    fmodel = fopen(s, "wt");
    assert(fmodel != NULL);

    sprintf(s,"%s_qmodel.txt", prefix);
    fqmodel = fopen(s, "wt");
    assert(fqmodel != NULL);

    sprintf(s,"%s_pw.txt", prefix);
    fpw = fopen(s, "wt");
    assert(fpw != NULL);

    sprintf(s,"%s_lsp.txt", prefix);
    flsp = fopen(s, "wt");
    assert(flsp != NULL);
}

void dump_off(){
    fclose(fsn);
    fclose(fsw);
    fclose(fmodel);
    fclose(fqmodel);
    fclose(fpw);
    fclose(flsp);
}

void dump_Sn(float Sn[]) {
    int i;

    if (!dumpon) return;

    /* split across two lines to avoid max line length problems */
    /* reconstruct in Octave */

    for(i=0; i<AW_ENC/2; i++)
	fprintf(fsn,"%f\t",Sn[i]);
    fprintf(fsn,"\n");    
    for(i=AW_ENC/2; i<AW_ENC; i++)
	fprintf(fsn,"%f\t",Sn[i]);
    fprintf(fsn,"\n");    
}

void dump_Sw(COMP Sw[]) {
    int i;

    if (!dumpon) return;

    for(i=0; i<FFT_ENC/2; i++)
	fprintf(fsw,"%f\t",
		10.0*log10(Sw[i].real*Sw[i].real + Sw[i].imag*Sw[i].imag));
    fprintf(fsw,"\n");    
}

void dump_model(MODEL *model) {
    int l;

    if (!dumpon) return;

    fprintf(fmodel,"%f\t%d\t", model->Wo, model->L);    
    for(l=1; l<=model->L; l++)
	fprintf(fmodel,"%f\t",model->A[l]);
    for(l=model->L+1; l<MAX_AMP; l++)
	fprintf(fmodel,"0.0\t");
    fprintf(fmodel,"\n");    
}

void dump_quantised_model(MODEL *model) {
    int l;

    if (!dumpon) return;

    fprintf(fqmodel,"%f\t%d\t", model->Wo, model->L);    
    for(l=1; l<=model->L; l++)
	fprintf(fqmodel,"%f\t",model->A[l]);
    for(l=model->L+1; l<MAX_AMP; l++)
	fprintf(fqmodel,"0.0\t");
    fprintf(fqmodel,"\n");    
}

void dump_Pw(COMP Pw[]) {
    int i;

    if (!dumpon) return;

    for(i=0; i<FFT_DEC/2; i++)
	fprintf(fpw,"%f\t",Pw[i].real);
    fprintf(fpw,"\n");    
}

void dump_lsp(float lsp[]) {
    int i;

    if (!dumpon) return;

    for(i=0; i<10; i++)
	fprintf(flsp,"%f\t",lsp[i]);
    fprintf(flsp,"\n");    
}


