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
#ifndef _H_ObjectMesh
#define _H_ObjectMesh

#include"ObjectMap.h"

typedef struct {
  ObjectMap *Map;
  int *N;
  float *V;
  int Range[6];
  float ExtentMin[3],ExtentMax[3];
  int ExtentFlag;
  float Level,Radius;
  int ResurfaceFlag;
  int DotFlag;
} ObjectMeshState;

typedef struct ObjectMesh {
  Object Obj;
  ObjectMeshState *State;
  int NState;
} ObjectMesh;

ObjectMesh *ObjectMeshFromBox(ObjectMesh *obj,ObjectMap *map,int state,float *mn,float *mx,float level,int dotFlag);
void ObjectMeshDump(ObjectMesh *I,char *fname,int state);

#endif

