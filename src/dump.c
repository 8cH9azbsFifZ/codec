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
#include <string.h>

static int dumpon = 0;

static FILE *fsn = NULL;
static FILE *fsw = NULL;
static FILE *fsw_ = NULL;
static FILE *fmodel = NULL;
static FILE *fqmodel = NULL;
static FILE *fpw = NULL;
static FILE *flsp = NULL;
static FILE *fphase = NULL;
static FILE *fphase_ = NULL;

static char  prefix[MAX_STR];

void dump_on(char p[]) {
    dumpon = 1;
    strcpy(prefix, p);
}

void dump_off(){
    if (fsn != NULL)
	fclose(fsn);
    if (fsw != NULL)
	fclose(fsw);
    if (fsw_ != NULL)
	fclose(fsw_);
    if (fmodel != NULL)
	fclose(fmodel);
    if (fqmodel != NULL)
	fclose(fqmodel);
    if (fpw != NULL)
	fclose(fpw);
    if (flsp != NULL)
	fclose(flsp);
    if (fphase != NULL)
	fclose(fphase);
    if (fphase_ != NULL)
	fclose(fphase_);
}

void dump_Sn(float Sn[]) {
    int i;
    char s[MAX_STR];

    if (!dumpon) return;

    if (fsn == NULL) {
	sprintf(s,"%s_sn.txt", prefix);
	fsn = fopen(s, "wt");
	assert(fsn != NULL);
    }

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
    char s[MAX_STR];

    if (!dumpon) return;

    if (fsw == NULL) {
	sprintf(s,"%s_sw.txt", prefix);
	fsw = fopen(s, "wt");
	assert(fsw != NULL);
    }

    for(i=0; i<FFT_ENC/2; i++)
	fprintf(fsw,"%f\t",
		10.0*log10(Sw[i].real*Sw[i].real + Sw[i].imag*Sw[i].imag));
    fprintf(fsw,"\n");    
}

void dump_Sw_(COMP Sw_[]) {
    int i;
    char s[MAX_STR];

    if (!dumpon) return;

    if (fsw_ == NULL) {
	sprintf(s,"%s_sw_.txt", prefix);
	fsw_ = fopen(s, "wt");
	assert(fsw_ != NULL);
    }

    for(i=0; i<FFT_ENC/2; i++)
	fprintf(fsw_,"%f\t",
		10.0*log10(Sw_[i].real*Sw_[i].real + Sw_[i].imag*Sw_[i].imag));
    fprintf(fsw_,"\n");    
}

void dump_model(MODEL *model) {
    int l;
    char s[MAX_STR];

    if (!dumpon) return;

    if (fmodel == NULL) {
	sprintf(s,"%s_model.txt", prefix);
	fmodel = fopen(s, "wt");
	assert(fmodel != NULL);
    }

    fprintf(fmodel,"%f\t%d\t", model->Wo, model->L);    
    for(l=1; l<=model->L; l++)
	fprintf(fmodel,"%f\t",model->A[l]);
    for(l=model->L+1; l<MAX_AMP; l++)
	fprintf(fmodel,"0.0\t");
    for(l=1; l<=model->L; l++)
	fprintf(fmodel,"%f\t",model->v[l]);
    for(l=model->L+1; l<MAX_AMP; l++)
	fprintf(fmodel,"0.0\t");
    fprintf(fmodel,"\n");    
}

void dump_quantised_model(MODEL *model) {
    int l;
    char s[MAX_STR];

    if (!dumpon) return;

    if (fqmodel == NULL) {
	sprintf(s,"%s_qmodel.txt", prefix);
	fqmodel = fopen(s, "wt");
	assert(fqmodel != NULL);
    }

    fprintf(fqmodel,"%f\t%d\t", model->Wo, model->L);    
    for(l=1; l<=model->L; l++)
	fprintf(fqmodel,"%f\t",model->A[l]);
    for(l=model->L+1; l<MAX_AMP; l++)
	fprintf(fqmodel,"0.0\t");
    fprintf(fqmodel,"\n");    
}

void dump_phase(float phase[]) {
    int l;
    char s[MAX_STR];

    if (!dumpon) return;

    if (fphase == NULL) {
	sprintf(s,"%s_phase.txt", prefix);
	fphase = fopen(s, "wt");
	assert(fphase != NULL);
    }

    for(l=1; l<=model.L; l++)
	fprintf(fphase,"%f\t",phase[l]);
    for(l=model.L+1; l<MAX_AMP; l++)
	fprintf(fphase,"%f\t",0.0);
    fprintf(fphase,"\n");    
}

void dump_phase_(float phase_[]) {
    int l;
    char s[MAX_STR];

    if (!dumpon) return;

    if (fphase_ == NULL) {
	sprintf(s,"%s_phase_.txt", prefix);
	fphase_ = fopen(s, "wt");
	assert(fphase_ != NULL);
    }

    for(l=1; l<=model.L; l++)
	fprintf(fphase_,"%f\t",phase_[l]);
    for(l=model.L+1; l<MAX_AMP; l++)
	fprintf(fphase_,"%f\t",0.0);
    fprintf(fphase_,"\n");    
}

void dump_Pw(COMP Pw[]) {
    int i;
    char s[MAX_STR];

    if (!dumpon) return;

    if (fpw == NULL) {
	sprintf(s,"%s_pw.txt", prefix);
	fpw = fopen(s, "wt");
	assert(fpw != NULL);
    }

    for(i=0; i<FFT_DEC/2; i++)
	fprintf(fpw,"%f\t",Pw[i].real);
    fprintf(fpw,"\n");    
}

void dump_lsp(float lsp[]) {
    int i;
    char s[MAX_STR];

    if (!dumpon) return;

    if (flsp == NULL) {
	sprintf(s,"%s_lsp.txt", prefix);
	flsp = fopen(s, "wt");
	assert(flsp != NULL);
    }

    for(i=0; i<10; i++)
	fprintf(flsp,"%f\t",lsp[i]);
    fprintf(flsp,"\n");    
}


