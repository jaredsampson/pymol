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

#include"os_predef.h"
#include"os_std.h"
#include"os_gl.h"

#include"Base.h"
#include"OOMac.h"
#include"RepNonbondedSphere.h"
#include"Color.h"
#include"Sphere.h"
#include"Setting.h"
#include"main.h"

typedef struct RepNonbondedSphere {
  Rep R;
  float *V;
  float *VC;
  SphereRec *SP;
  int N,NC;
  float *VP;
  Pickable *P;
  int NP;

} RepNonbondedSphere;

#include"ObjectMolecule.h"

void RepNonbondedSphereRender(RepNonbondedSphere *I,CRay *ray,Pickable **pick);
void RepNonbondedSphereFree(RepNonbondedSphere *I);

void RepNonbondedSphereInit(void)
{
}

void RepNonbondedSphereFree(RepNonbondedSphere *I)
{
  FreeP(I->VP);
  RepFree(&I->R);
  FreeP(I->VC);
  FreeP(I->V);
  OOFreeP(I);
}

void RepNonbondedSphereRender(RepNonbondedSphere *I,CRay *ray,Pickable **pick)
{
  float *v=I->V;
  int c=I->N;
  int cc=0;
  int a;
  SphereRec *sp;
  int i,j;
  Pickable *p;

  if(ray) {
	 v=I->VC;
	 c=I->NC;
	 while(c--) {
		ray->fColor3fv(ray,v);
		v+=3;
		ray->fSphere3fv(ray,v,*(v+3));
		v+=4;
	 }
  } else if(pick&&PMGUI) {

	 i=(*pick)->index;

	 v=I->VP;
	 c=I->NP;
	 p=I->R.P;
	 
	 glBegin(GL_LINES);
	 
	 while(c--) {

		i++;

		if(!(*pick)[0].ptr) {
		  /* pass 1 - low order bits */

		  glColor3ub((uchar)((i&0xF)<<4),(uchar)((i&0xF0)|0x8),(uchar)((i&0xF00)>>4)); 
		  VLACheck((*pick),Pickable,i);
		  p++;
		  (*pick)[i] = *p; /* copy object and atom info */
		} else { 
		  /* pass 2 - high order bits */

		  j=i>>12;

		  glColor3ub((uchar)((j&0xF)<<4),(uchar)((j&0xF0)|0x8),(uchar)((j&0xF00)>>4)); 

		}			 

		glVertex3fv(v);
		v+=3;
		glVertex3fv(v);
		v+=3;
		glVertex3fv(v);
		v+=3;
		glVertex3fv(v);
		v+=3;
		glVertex3fv(v);
		v+=3;
		glVertex3fv(v);
		v+=3;

	 }
	 glEnd();

	 (*pick)[0].index = i;

  } else if(PMGUI) {
    sp=I->SP;
    while(c--)
      {
        glColor3fv(v);
        v+=3;
        for(a=0;a<sp->NStrip;a++) {
          glBegin(GL_TRIANGLE_STRIP);
          cc=sp->StripLen[a];
          while(cc--) {
            glNormal3fv(v);
            v+=3;
            glVertex3fv(v);
            v+=3;
          }
          glEnd();
        }
      }
  }
}

Rep *RepNonbondedSphereNew(CoordSet *cs)
{
  ObjectMolecule *obj;
  int a,c,d,c1;
  float *v,*v0,*vc;
  float nonbonded_size;
  int *q, *s;
  SphereRec *sp = TempPyMOLGlobals->Sphere->Sphere[0];
  int ds;
  int *active=NULL;
  AtomInfoType *ai;
  int nSphere = 0;
  int a1;
  float *v1;
  float tmpColor[3];
  OOAlloc(RepNonbondedSphere);
  obj = cs->Obj;

  active = Alloc(int,cs->NIndex);
  
  for(a=0;a<cs->NIndex;a++) {
    ai = obj->AtomInfo+cs->IdxToAtm[a];
    active[a] =(!ai->bonded) && (ai->visRep[ cRepNonbondedSphere ]);
	if(active[a]) {
		if(ai->masked)
			active[a]=-1;
		else
			active[a]=1;
	}
    if(active[a]) nSphere++;
  }
  if(!nSphere) {
    OOFreeP(I);
    FreeP(active);
    return(NULL); /* skip if no dots are visible */
  }
  
  nonbonded_size = SettingGet_f(cs->Setting,obj->Obj.Setting,cSetting_nonbonded_size);
  
  /* get current dot sampling */
  ds = SettingGet_i(cs->Setting,obj->Obj.Setting,cSetting_dot_density);
  ds=1;
  if(ds<0) ds=0;
  if(ds>3) ds=3;
  sp = TempPyMOLGlobals->Sphere->Sphere[ds];

  RepInit(&I->R);
  I->R.fRender=(void (*)(struct Rep *, CRay *, Pickable **))RepNonbondedSphereRender;
  I->R.fFree=(void (*)(struct Rep *))RepNonbondedSphereFree;
  I->R.fRecolor=NULL;
  I->N=0;
  I->NC=0;
  I->V=NULL;
  I->VC=NULL;
  I->SP=NULL;
  I->NP=0;
  I->VP=NULL;
  I->R.P=NULL;

  /* raytracing primitives */

  I->VC=(float*)mmalloc(sizeof(float)*nSphere*7);
  ErrChkPtr(I->VC);
  I->NC=0;

  v=I->VC; 

  for(a=0;a<cs->NIndex;a++)
	 {
      if(active[a])
		  {
			 I->NC++;
			 c1=*(cs->Color+a);
			 v0 = cs->Coord+3*a;			 
          if(ColorCheckRamped(c1)) {
            ColorGetRamped(c1,v0,tmpColor);
            vc = tmpColor;
          } else {
            vc = ColorGet(c1);
          }
			 *(v++)=*(vc++);
			 *(v++)=*(vc++);
			 *(v++)=*(vc++);
			 *(v++)=*(v0++);
			 *(v++)=*(v0++);
			 *(v++)=*(v0++);
			 *(v++)=nonbonded_size;
		  }
	 }

  if(I->NC) 
	 I->VC=ReallocForSure(I->VC,float,(v-I->VC));
  else
	 I->VC=ReallocForSure(I->VC,float,1);

  I->V=(float*)mmalloc(sizeof(float)*nSphere*(3+sp->NVertTot*6));
  ErrChkPtr(I->V);

  /* rendering primitives */

  I->N=0;
  I->SP=sp;
  v=I->V;

  for(a=0;a<cs->NIndex;a++)
	 {
		if(active[a])
		  {
			 c1=*(cs->Color+a);
			 v0 = cs->Coord+3*a;
			 vc = ColorGet(c1);

          if(ColorCheckRamped(c1)) {
            ColorGetRamped(c1,v0,tmpColor);
            vc = tmpColor;
          } else {
            vc = ColorGet(c1);
          }

			 *(v++)=*(vc++);
			 *(v++)=*(vc++);
			 *(v++)=*(vc++);

          q=sp->Sequence;
          s=sp->StripLen;
          
          for(d=0;d<sp->NStrip;d++)
            {
              for(c=0;c<(*s);c++)
                {
                  *(v++)=sp->dot[*q][0]; /* normal */
                  *(v++)=sp->dot[*q][1];
                  *(v++)=sp->dot[*q][2];
                  *(v++)=v0[0]+nonbonded_size*sp->dot[*q][0]; /* point */
                  *(v++)=v0[1]+nonbonded_size*sp->dot[*q][1];
                  *(v++)=v0[2]+nonbonded_size*sp->dot[*q][2];
                  q++;
                }
              s++;
            }
			 I->N++;
		  }
	 }
  
  if(I->N) 
    I->V=ReallocForSure(I->V,float,(v-I->V));
  else
    I->V=ReallocForSure(I->V,float,1);

  /* use pickable representation from nonbonded */
  if(SettingGet_f(cs->Setting,obj->Obj.Setting,cSetting_pickable)) {
    I->VP=(float*)mmalloc(sizeof(float)*nSphere*18);
    ErrChkPtr(I->VP);
    
    I->R.P=Alloc(Pickable,cs->NIndex+1);
    ErrChkPtr(I->R.P);
    
    v=I->VP;
    
    for(a=0;a<cs->NIndex;a++) 
      if(active[a]>0) {
        
        I->NP++;

        a1=cs->IdxToAtm[a];
        
        I->R.P[I->NP].ptr = (void*)obj;
        I->R.P[I->NP].index = a1;
        I->R.P[I->NP].bond = -1;
        v1 = cs->Coord+3*a;
        
        *(v++)=v1[0]-nonbonded_size;
        *(v++)=v1[1];
        *(v++)=v1[2];
        *(v++)=v1[0]+nonbonded_size;
        *(v++)=v1[1];
        *(v++)=v1[2];
        *(v++)=v1[0];
        *(v++)=v1[1]-nonbonded_size;
        *(v++)=v1[2];
        *(v++)=v1[0];
        *(v++)=v1[1]+nonbonded_size;
        *(v++)=v1[2];
        *(v++)=v1[0];
        *(v++)=v1[1];
        *(v++)=v1[2]-nonbonded_size;
        *(v++)=v1[0];
        *(v++)=v1[1];
        *(v++)=v1[2]+nonbonded_size;
      }
    I->R.P = Realloc(I->R.P,Pickable,I->NP+1);
    I->R.P[0].index = I->NP;
    I->VP = Realloc(I->VP,float,I->NP*21);
  }

  FreeP(active);
  return((void*)(struct Rep*)I);
}


