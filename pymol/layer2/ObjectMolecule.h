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
#ifndef _H_ObjectMolecule
#define _H_ObjectMolecule

#include"Object.h"
#include"AtomInfo.h"
#include"Vector.h"
#include"Color.h"

typedef struct ObjectMolecule {
  Object Obj;
  struct CoordSet **CSet;
  int NCSet;
  int *Bond;
  AtomInfoType *AtomInfo;
  int NAtom;
  int NBond;
  int CurCSet;
  char Color[3];
  float FractionExposed;
} ObjectMolecule;

typedef struct ObjectMoleculeOpRec {
  unsigned long code;
  Vector3f v1;
  int i1,i2,i3,*vc1;
  float f1;
  double d[3][3];
  float *vv1,*vv2;
  char *charVLA;
  char *s1;
  float ttt[16];
  int nvv1,nvv2;
} ObjectMoleculeOpRec;

#include"CoordSet.h"

ObjectMolecule *ObjectMoleculeNew(void);
void ObjectMoleculeSort(ObjectMolecule *I);
ObjectMolecule *ObjectMoleculeCopy(ObjectMolecule *obj);

ObjectMolecule *ObjectMoleculeLoadPDBFile(ObjectMolecule *obj,char *fname,int frame);
ObjectMolecule *ObjectMoleculeLoadMOLFile(ObjectMolecule *obj,char *fname,int frame);
ObjectMolecule *ObjectMoleculeLoadMMDFile(ObjectMolecule *obj,char *fname,
                                          int frame,char *sepPrefix);

ObjectMolecule *ObjectMoleculeReadMOLStr(ObjectMolecule *obj,char *molstr,int frame);
ObjectMolecule *ObjectMoleculeReadPDBStr(ObjectMolecule *obj,char *molstr,int frame);
ObjectMolecule *ObjectMoleculeReadMMDStr(ObjectMolecule *I,char *MMDStr,int frame);


void ObjectMoleculeInvalidateRep(ObjectMolecule *I,int rep);

void ObjectMoleculeSeleOp(ObjectMolecule *I,int sele,ObjectMoleculeOpRec *op);

CoordSet *ObjectMoleculeGetCoordSet(ObjectMolecule *I,int setIndex);


#endif











