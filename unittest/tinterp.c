/*---------------------------------------------------------------------------*\
                                                                          
  FILE........: tinterp.c                                                  
  AUTHOR......: David Rowe                                            
  DATE CREATED: 22/8/10                                        
                                                               
  Tests interpolation functions.
                                                                   
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
#include "interp.h"

int main() {
    MODEL  prev, next, interp;
    int    l;
    FILE  *f;

    f = fopen("interp.txt","wt");

    prev.Wo = PI/20;
    prev.L = PI/prev.Wo;
    prev.voiced = 1;
    for(l=1;l<=prev.L; l++) 
	prev.A[l] = 1000.0;

    next.Wo = PI/30;
    next.L = PI/next.Wo;
    next.voiced = 1;
    for(l=1;l<=next.L; l++) 
	next.A[l] = 2000.0;

    interp.voiced = 1.0;
    interpolate(&interp, &prev, &next);
    printf("Wo = %f\n", interp.Wo);
 
    for(l=1; l<=interp.L; l++)
	fprintf(f, "%f\n", interp.A[l]);

    fclose(f);
    return 0;
}
