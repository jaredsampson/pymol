/* 
A* -------------------------------------------------------------------
B* This file contains source code for the PyMOL computer program
C* copyright 1998-2000 by Warren Lyford Delano of DeLano Scientific. 
D* -------------------------------------------------------------------
E* It is unlawful to modify or remove this copyright notice.
F* -------------------------------------------------------------------
G* Please see the accompanying LICENSE file for further information. 
H* -------------------------------------------------------------------
I* Additional authors of this source file include:
-* 
-* 
-*
Z* -------------------------------------------------------------------
*/
#ifndef _H_Triangle
#define _H_Triangle

#include"Vector.h"

typedef struct {
  int *Tri;

} TriangleSurfaceRec;

TriangleSurfaceRec *TrianglePointsToSurface(float *v,int n,float cutoff);

#endif
