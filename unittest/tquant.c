/*---------------------------------------------------------------------------*\
                                                                          
  FILE........: tquant.c                                                  
  AUTHOR......: David Rowe                                            
  DATE CREATED: 22/8/10                                        
                                                               
  Generates quantisation curves for plotting on Octave.
                                                                   
\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2010 David Rowe

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "defines.h"
#include "dump.h"
#include "quantise.h"

int main() {
    int    i,c,bit;
    FILE  *f;
    float  Wo,Wo_dec, error, step_size;
    char   bits[WO_BITS];
    int    code, nbits, code_in, code_out;

    /* output pitch quant curve for plotting */

    f = fopen("quant_pitch.txt","wt");

    for(Wo=0.9*(TWO_PI/P_MAX); Wo<=1.1*(TWO_PI/P_MIN); Wo += 0.001) {
	nbits = 0;
	encode_Wo(bits, &nbits, Wo);
        code = 0;
	for(i=0; i<WO_BITS; i++) {
	    code <<= 1;
	    code |= bits[i];
	}
	fprintf(f, "%f %d\n", Wo, code);
    }

    fclose(f);

    /* check for all pitch codes we get 1:1 match between encoder
       and decoder Wo levels */

    for(c=0; c<WO_LEVELS; c++) {
	code_in = c;
	for(i=0; i<WO_BITS; i++) {
	    bit = (code_in >> (WO_BITS-1-i)) & 0x1;
	    bits[i] = bit;
	}
	nbits = 0;
	Wo = decode_Wo(bits, &nbits);
	nbits = 0;

	memset(bits, sizeof(bits), 0);
        encode_Wo(bits, &nbits, Wo);
        code_out = 0;
	for(i=0; i<WO_BITS; i++) {
	    code_out <<= 1;
	    code_out |= bits[i];
	}
	if (code_in != code_out)
	    printf("  Wo %f code_in %d code_out %d\n", Wo, 
		   code_in, code_out);
    }

    /* measure quantisation error stats and compare to expected.  Also
       plot histogram of error file to check. */

    f = fopen("quant_pitch_err.txt","wt");
    step_size = ((TWO_PI/P_MIN) - (TWO_PI/P_MAX))/WO_LEVELS;

    for(Wo=TWO_PI/P_MAX; Wo<0.99*TWO_PI/P_MIN; Wo += 0.0001) {
	nbits = 0; encode_Wo(bits, &nbits, Wo);
	nbits = 0; Wo_dec = decode_Wo(bits, &nbits);
	error = Wo - Wo_dec;
	if (error > (step_size/2.0)) {
	    printf("error: %f  step_size/2: %f\n", error, step_size/2.0);
	    exit(0);
	}
	fprintf(f,"%f\n",error);
    }
    printf("OK\n");

    fclose(f);
    return 0;
}
