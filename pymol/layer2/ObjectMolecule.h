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

#include<Python.h>

#include"Object.h"
#include"AtomInfo.h"
#include"Vector.h"
#include"Color.h"
#include"Symmetry.h"

#define cUndoMask 0x7

typedef struct ObjectMolecule {
  Object Obj;
  struct CoordSet **CSet;
  int NCSet;
  int *Bond;
  AtomInfoType *AtomInfo;
  int NAtom;
  int NBond;
  int DiscreteFlag,NDiscrete;
  int *DiscreteAtmToIdx;
  struct CoordSet **DiscreteCSet;
  int CurCSet;
  char Color[3];
  float FractionExposed;
  int SeleBase; /* for internal usage by  selector & only valid during selection process */
  CSymmetry *Symmetry;
  int *Neighbor;
  float *UndoCoord[cUndoMask+1];
  int UndoState[cUndoMask+1];
  int UndoNIndex[cUndoMask+1];
  int UndoIter;
} ObjectMolecule;

typedef struct ObjectMoleculeOpRec {
  unsigned long code;
  Vector3f v1,v2;
  int cs1;
  int i1,i2,i3,*vc1,*i1VLA;
  float f1,*f1VLA;
  double d[3][3];
  float *vv1,*vv2;
  char *charVLA;
  char *s1;
  float ttt[16];
  int nvv1,nvv2;
} ObjectMoleculeOpRec;

/* these four letter code are left over from an 
   earlier multicharacter constant implementation
   and should be replaced with something more verbose */

#define OMOP_PDB1 1
#define OMOP_AVRT 2
#define OMOP_SFIT 3
#define OMOP_COLR 4
#define OMOP_VISI 5
#define OMOP_TTTF 6
#define OMOP_ALTR 7
#define OMOP_CSOC 8
#define OMOP_SUMC 9
#define OMOP_VERT 10
#define OMOP_SVRT 11
#define OMOP_MOME 12
#define OMOP_INVA 13
#define OMOP_MDST 14
#define OMOP_MNMX 15
#define OMOP_AlterState 16
#define OMOP_Flag 17
#define OMOP_LABL 18
#define OMOP_Identify    19
#define OMOP_Remove 20
#define OMOP_Protect 21
#define OMOP_Mask 22

#include"CoordSet.h"

ObjectMolecule *ObjectMoleculeNew(int discreteFlag);
void ObjectMoleculeSort(ObjectMolecule *I);
ObjectMolecule *ObjectMoleculeCopy(ObjectMolecule *obj);

ObjectMolecule *ObjectMoleculeLoadPDBFile(ObjectMolecule *obj,char *fname,int frame,int discrete);
ObjectMolecule *ObjectMoleculeLoadMOLFile(ObjectMolecule *obj,char *fname,int frame,int discrete);
ObjectMolecule *ObjectMoleculeLoadMMDFile(ObjectMolecule *obj,char *fname,
                                          int frame,char *sepPrefix,int discrete);
ObjectMolecule *ObjectMoleculeLoadChemPyModel(ObjectMolecule *I,PyObject *model,int frame,int discrete);
ObjectMolecule *ObjectMoleculeLoadCoords(ObjectMolecule *I,PyObject *coords,int frame);

ObjectMolecule *ObjectMoleculeReadMOLStr(ObjectMolecule *obj,char *molstr,int frame,int discrete);
ObjectMolecule *ObjectMoleculeReadPDBStr(ObjectMolecule *obj,char *molstr,int frame,int discrete);
ObjectMolecule *ObjectMoleculeReadMMDStr(ObjectMolecule *I,char *MMDStr,int frame,int discrete);
void ObjectMoleculeExtendIndices(ObjectMolecule *I);

void ObjectMoleculeInvalidate(ObjectMolecule *I,int rep,int level);

void ObjectMoleculeSeleOp(ObjectMolecule *I,int sele,ObjectMoleculeOpRec *op);

CoordSet *ObjectMoleculeGetCoordSet(ObjectMolecule *I,int setIndex);
void ObjectMoleculeBlindSymMovie(ObjectMolecule *I);
void ObjectMoleculeMerge(ObjectMolecule *I,AtomInfoType *ai,CoordSet *cs,int bondSearchFlag);
void ObjectMoleculeUpdateNonbonded(ObjectMolecule *I);
void ObjectMoleculeUpdateNeighbors(ObjectMolecule *I);
int ObjectMoleculeMoveAtom(ObjectMolecule *I,int state,int index,float *v,int mode);
int ObjectMoleculeGetAtomVertex(ObjectMolecule *I,int state,int index,float *v);
int ObjectMoleculeGetAtomIndex(ObjectMolecule *I,int sele);
void ObjectMoleculeTransformSelection(ObjectMolecule *I,int state,int sele,float *TTT);

void ObjectMoleculeInferChemFromNeighGeom(ObjectMolecule *I,int state);
void ObjectMoleculeInferChemForProtein(ObjectMolecule *I,int state);
void ObjectMoleculeInferChemFromBonds(ObjectMolecule *I,int state);
void ObjectMoleculePurge(ObjectMolecule *I);

int ObjectMoleculeAddBond(ObjectMolecule *I,int sele0,int sele1,int order);
int ObjectMoleculeRemoveBonds(ObjectMolecule *I,int sele1,int sele2);

void ObjectMoleculeSaveUndo(ObjectMolecule *I,int state);
void ObjectMoleculeUndo(ObjectMolecule *I,int dir);
void ObjectMoleculeReplaceAtom(ObjectMolecule *I,int index,AtomInfoType *ai);

#endif











