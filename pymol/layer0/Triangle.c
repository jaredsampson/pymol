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

/* glossary:

	active edge = an edge with one triangle
	closed edge = an edge with two triangles

	closed vertex = one which is surrounded by a cycle of triangles
	active vertex = (opposite of closed)

*/

#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<values.h>

#include"Base.h"
#include"Triangle.h"
#include"Map.h"
#include"MemoryDebug.h"
#include"Err.h"
#include"Setting.h"
#include"Ortho.h"

float TestLine[10000];
int NTestLine = 0;

typedef struct {
  int index;
  int value;
  int next;
} LinkType;

typedef struct {
  int vert3,tri1;
  int vert4,tri2;
} EdgeRec;

typedef struct {
  int *activeEdge; /* active edges */
  int nActive;
  int *edgeStatus;
  int *vertActive;
  int *vertWeight;
  int *tri;
  int nTri;
  float *vNormal; /* normal vector for first triangle of an active edge */
  EdgeRec *edge;
  int nEdge;
  MapType *map;
  LinkType *link;
  int nLink;
  int N;
} TriangleSurfaceRec;

TriangleSurfaceRec TriangleSurface;

static int TriangleEdgeStatus(int i1,int i2) 
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int l,low,high;
  low = ( i1>i2 ? i2 : i1 );
  high = ( i1>i2 ? i1 : i2 );

  l=I->edgeStatus[low]; 
  while(l) {
	 if(I->link[l].index == high) 
		return(I->link[l].value); /* <0 closed, 0 open, >0 then has index for blocking vector */
	 l=I->link[l].next;
  }
  return(0);
}

static int *TriangleMakeStripVLA(float *v,float *vn,int n) 
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int *tFlag;
  int c,a,cc=0;
  int *s,*sc,*strip;
  int state,s01,i0,i1,i2,tc,t0=0;
  int *t;
  int done;
  int dir,dcnt;
  int flag;
  strip=VLAlloc(int,I->nTri*4); /* strip VLA is count,vert,vert,...count,vert,vert...zero */
  tFlag=Alloc(int,I->nTri);  
  for(a=0;a<I->nTri;a++) 
	 tFlag[a]=0;
  s=strip;
  done = false;
  while(!done) {
	 done = true;
	 t=I->tri;
	 dir=0;
	 for(a=0;a<I->nTri;a++) {
		if(!tFlag[a]) {
		  tc=a;
		  flag=false;
		  dcnt=0;
		  while(dcnt<3) {
			 i0 = *(t+3*tc+(dir%3));
			 i1 = *(t+3*tc+((dir+1)%3));
			 state = TriangleEdgeStatus(i0,i1);
			 if(state) {
				s01 = abs(state);
				t0 = I->edge[s01].tri1;
				if(!tFlag[t0])
				  flag=true;
				else if(state<0) {
				  t0 = I->edge[s01].tri2;
				  if(!tFlag[t0])
					 flag=true;
				}
			 }
			 if(!flag) {
				dir++;
				dcnt++;
			 } else {
				c=0;
				sc = s++;
				*(s++)=i0;
				*(s++)=i1;
				while(1) {
				  state = TriangleEdgeStatus(s[-2],s[-1]);
				  if(!state) break;
				  s01 = abs(state);
				  /*			 printf("a: %i %i %i\n",a,I->edge[s01].tri1,I->edge[s01].tri2);*/
				  
				  t0 = I->edge[s01].tri1;
				  if(!tFlag[t0])
					 i2 = I->edge[s01].vert3;
				  else
					 {
						if(state>=0) break;
						t0 = I->edge[s01].tri2;
						i2 = I->edge[s01].vert4;
						/*				printf("second to %i i2 %i  [t0] %i \n",t0,i2,tFlag[t0]);*/
					 } 
				  if(tFlag[t0]) break;
				  *(s++) = i2;
				  tFlag[t0]=true;
				  c++;
				  done=false;
				}
				if(!c)
				  s=sc;
				else {
				  *sc = c; 
				  cc+=c;
				}
				/*				if(c>1) printf("strip %i %i\n",c,cc);*/
				dcnt=0;
				tc=t0;			 
				flag=false;
			 }
		  }
		}
	 }

	 /* fail-safe check in case of bad connectivity...*/
	 
	 for(a=0;a<I->nTri;a++) 
		if(!tFlag[a]) {
		  /*		  printf("missed %i %i %i\n",*(I->tri+3*a),*(I->tri+3*a+1), *(I->tri+3*a+2));*/
		  *(s++) = 1;
		  *(s++) = *(I->tri+3*a);
		  *(s++) = *(I->tri+3*a+1);
		  *(s++) = *(I->tri+3*a+2);
		}

	 *s=0; /* terminate strip list */
  }
  FreeP(tFlag);
  /* shrink strip */
  return(strip);
}


static void TriangleAdjustNormals(float *v,float *vn,int n)
{
  TriangleSurfaceRec *I=&TriangleSurface;
  /* points all normals to the average of the intersecting triangles in order to maximum surface smoothness */
  float *tNorm = NULL;
  int *vFlag=NULL;
  float *v0,*v1,*v2,*tn,vt1[3],vt2[3],*vn0,*tn0,*tn1,*tn2;
  int a,*t,i0,i1,i2;

  tNorm=Alloc(float,3*I->nTri);
  vFlag=Alloc(int,n);
  for(a=0;a<n;a++) {
	 vFlag[a]=0;
  }
  /* first, calculate and store all triangle normals */
  t=I->tri;
  tn=tNorm;
  for(a=0;a<I->nTri;a++) {
	 vFlag[t[0]]=1;
	 vFlag[t[1]]=1;
	 vFlag[t[2]]=1;
	 v0 = v+(*(t++))*3;
	 v1 = v+(*(t++))*3;
	 v2 = v+(*(t++))*3;
	 subtract3f(v1,v0,vt1);
	 subtract3f(v2,v0,vt2);
	 cross_product3f(vt1,vt2,tn);
	 normalize3f(tn);
	 tn+=3;
  }
  /* clear normals */
  vn0=vn;
  for(a=0;a<n;a++) 
	 if(vFlag[a]) {
		*(vn0++)=0.0;
		*(vn0++)=0.0;
		*(vn0++)=0.0;
	 } else
		vn0+=3;
  
  /* sum */
  tn=tNorm;
  t=I->tri;
  for(a=0;a<I->nTri;a++) {
	 i0 = *(t++);
	 i1 = *(t++);
	 i2 = *(t++);
	 tn0 = vn+i0*3;
	 tn1 = vn+i1*3;
	 tn2 = vn+i2*3;
	 add3f(tn,tn0,tn0);
	 add3f(tn,tn1,tn1);
	 add3f(tn,tn2,tn2);
	 tn+=3;
  }
  /* normalize */
  vn0=vn;
  for(a=0;a<n;a++) {
	 if(vFlag[a])
		normalize3f(vn0);
	 vn0+=3;
  }
  FreeP(vFlag);
  FreeP(tNorm);
}



static int TriangleActivateEdges(int low)
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int l;
  l=I->edgeStatus[low]; 
  while(l) {
    if(I->link[l].value>0) {
      VLACheck(I->activeEdge,int,I->nActive*2+1);
      I->activeEdge[I->nActive*2] = low;
      I->activeEdge[I->nActive*2+1] = I->link[l].index;
      I->nActive++;
    }
	 l=I->link[l].next;
  }
  return(0);
}

static void TriangleEdgeSetStatus(int i1,int i2,int value) 
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int l,low,high;
  low = ( i1>i2 ? i2 : i1 );
  high = ( i1>i2 ? i1 : i2 );

  /*  printf("set: %i %i %i\n",i1,i2,value);*/
  l=I->edgeStatus[low]; 
  while(l) {
	 if(I->link[l].index == high) {
		I->link[l].value = value; 
		return;
	 }
	 l=I->link[l].next;
  }
  if(!l) {
	 VLACheck(I->link,LinkType,I->nLink);
	 I->link[I->nLink].next = I->edgeStatus[low];
	 I->edgeStatus[low] = I->nLink;
	 /*	 printf("offset %i value %i index %i\n",I->nLink,value,high);*/
	 I->link[I->nLink].index = high;
	 I->link[I->nLink].value = value; 
	 I->nLink++;
  }
}

static void TriangleMove(from,to)
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int i0,i1,i2,s01,s02,s12;

  i0=I->tri[from*3];
  i1=I->tri[from*3+1];
  i2=I->tri[from*3+2];

  s01 = TriangleEdgeStatus(i0,i1);
  s02 = TriangleEdgeStatus(i0,i2);
  s12 = TriangleEdgeStatus(i1,i2);

#define TMoveMacro(x) {\
  if(x>0) { \
	 if(I->edge[x].tri1==from) \
		I->edge[x].tri1=to; \
	 else if(I->edge[x].tri2==from) \
		I->edge[x].tri2=to; \
  } else if(x<0) { \
	 x=-x; \
	 if(I->edge[x].tri1==from) \
		I->edge[x].tri1=to; \
	 else if(I->edge[x].tri2==from) \
		I->edge[x].tri2=to; \
  }}\

  TMoveMacro(s01)
  TMoveMacro(s02)
  TMoveMacro(s12)
  
  I->tri[to*3]=i0;
  I->tri[to*3+1]=i1;
  I->tri[to*3+2]=i2;
}

#define max_edge_len 2.5

static void AddActive(int i1,int i2) {
  int t;

  TriangleSurfaceRec *I=&TriangleSurface;
  if(i1>i2) {
    t=i1;
    i1=i2;
    i2=t;
  }
  VLACheck(I->activeEdge,int,I->nActive*2+1);
  I->activeEdge[I->nActive*2] = i1;
  I->activeEdge[I->nActive*2+1] = i2;
  I->nActive++;
  if(I->vertActive[i1]<0) I->vertActive[i1]=0;
  I->vertActive[i1]++;
  if(I->vertActive[i2]<0) I->vertActive[i2]=0;
  I->vertActive[i2]++;
}

static void TriangleAdd(int i0,int i1,int i2,float *tNorm,float *v,float *vn)
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int s01,s12,s02,it;
  float *v0,*v1,*v2,*n0,*n1,*n2,*ft;
  float vt[3],vt1[3],vt2[3],vt3[3];

  v0 = v+3*i0;
  v1 = v+3*i1;
  v2 = v+3*i2;
  n0 = vn+3*i0;
  n1 = vn+3*i1;
  n2 = vn+3*i2;

  /* make sure the triangle obeys the right hand rule */

  subtract3f(v1,v0,vt1);
  subtract3f(v2,v0,vt2);
  cross_product3f(vt1,vt2,vt);
  if(dot_product3f(vt,tNorm)<0.0) { /* if wrong, then interchange */
	 it=i1; i1=i2; i2=it;
	 ft=v1; v1=v2; v2=ft;
	 ft=n1; n1=n2; n2=ft;
	 }
  /* now, bend the normals a bit so that they line up better with the
     actual triangles drawn */
  add3f(n0,n1,vt3);
  add3f(n2,vt3,vt3);
  I->vertWeight[i0]++;
  scale3f(n0,I->vertWeight[i0],n0);
  add3f(tNorm,n0,n0);
  normalize3f(n0);
  I->vertWeight[i1]++;
  scale3f(n1,I->vertWeight[i1],n1);
  add3f(tNorm,n1,n1);
  normalize3f(n1);
  I->vertWeight[i2]++;
  scale3f(n2,I->vertWeight[i2],n2);
  add3f(tNorm,n2,n2);
  normalize3f(n2);

  s01 = TriangleEdgeStatus(i0,i1);
  s02 = TriangleEdgeStatus(i0,i2);
  s12 = TriangleEdgeStatus(i1,i2);
  
  /* create a new triangle */
  VLACheck(I->tri,int,(I->nTri*3)+2);
  I->tri[I->nTri*3] = i0;
  I->tri[I->nTri*3+1] = i1;
  I->tri[I->nTri*3+2] = i2;
  /*  	printf("creating %i %i %i\n",i0,i1,i2);*/
  if(s01) {
	 if(s01>0) {
		I->edge[s01].vert4 = i2;
		I->edge[s01].tri2 = I->nTri;
		TriangleEdgeSetStatus(i0,i1,-s01);
		I->vertActive[i0]--; /* deactivate when all active edges are closed */
		I->vertActive[i1]--;
	 } else {
		printf("s01 negative\n");
		while(1);
	 }
  } else {
	 VLACheck(I->edge,EdgeRec,I->nEdge);
	 I->edge[I->nEdge].vert3 = i2;
	 I->edge[I->nEdge].tri1 = I->nTri;
	 VLACheck(I->vNormal,float,(I->nEdge*3)+2);
	 copy3f(tNorm,I->vNormal+I->nEdge*3);
	 TriangleEdgeSetStatus(i0,i1,I->nEdge);
	 I->nEdge++;
    AddActive(i0,i1);
  }
  if(s02) {
	 if(s02>0) {
		I->edge[s02].vert4 = i1;
		I->edge[s02].tri2 = I->nTri;
		TriangleEdgeSetStatus(i0,i2,-s02);
		I->vertActive[i0]--; /* deactivate when all active edges are closed */
		I->vertActive[i2]--;
	 } else {
		printf("s02 negative\n");
		while(1);
	 }
  } else {
	 VLACheck(I->edge,EdgeRec,I->nEdge);
	 I->edge[I->nEdge].vert3 = i1;
	 I->edge[I->nEdge].tri1 = I->nTri;
	 VLACheck(I->vNormal,float,(I->nEdge*3)+2);
	 copy3f(tNorm,I->vNormal+I->nEdge*3);
	 TriangleEdgeSetStatus(i0,i2,I->nEdge);
	 I->nEdge++;
    AddActive(i0,i2);
  }
  if(s12) {
	 if(s12>0) {
		I->edge[s12].vert4 = i0;
		I->edge[s12].tri2 = I->nTri;
		TriangleEdgeSetStatus(i1,i2,-s12); 
		I->vertActive[i1]--; /* deactivate when all active edges are closed */
		I->vertActive[i2]--;
	 } else {
		printf("s12 negative\n");
		while(1);
	 }
  } else {
	 VLACheck(I->edge,EdgeRec,I->nEdge);
	 I->edge[I->nEdge].vert3 = i0;
	 I->edge[I->nEdge].tri1 = I->nTri;
	 VLACheck(I->vNormal,float,(I->nEdge*3)+2);
	 copy3f(tNorm,I->vNormal+I->nEdge*3);
	 TriangleEdgeSetStatus(i1,i2,I->nEdge);
	 I->nEdge++;
    AddActive(i1,i2);
  }
  I->nTri++;

}

static void TriangleBuildObvious(int i1,int i2,float *v,float *vn,int n)
{
  /* this routine builds obvious, easy triagles where the closest point 
  * to the edge is always tried */

  TriangleSurfaceRec *I=&TriangleSurface;
  MapType *map;
  float *v1,*v2,*v0,*v4,vt[3],vt1[3],vt2[3],vt3[3],vt4[3],*n0,*n1,*n2,tNorm[3];
  int i0,s01,s02,s12,i,j,h,k,l,i4;
  float dif,minDist,d1,d2,dp;
  int flag;
  int used = -1;

  map=I->map;
  s12 = TriangleEdgeStatus(i1,i2);
  if(s12>0) used = I->edge[s12].vert3;
  if(s12>=0) {
    minDist = MAXFLOAT;
    i0=-1;
    v1=v+i1*3; v2=v+i2*3;
    MapLocus(map,v1,&h,&k,&l);
	 i=*(MapEStart(map,h,k,l));
    if(i) {
      j=map->EList[i++];
      while(j>=0) {
        if((j!=i1)&&(j!=i2)&&(j!=used)) 
          {
            v0 = v+3*j;
            d1 = diff3f(v0,v1);
            d2 = diff3f(v0,v2);
            dif= ( d2 > d1 ? d2 : d1 );
				if(dif<minDist)
				  {
					 minDist = dif;
					 i0=j; 
				  }
			 }
		  j=map->EList[i++];
		}
		if(i0>=0) {
		  s01 = TriangleEdgeStatus(i0,i1); s02 = TriangleEdgeStatus(i0,i2);
		  if(I->vertActive[i0]>0) {
			 if(!((s01>0)||(s02>0)))
				i0=-1; /* don't allow non-adjacent joins to active vertices */
		  }
		}
		if(i0>=0) {
		  v0 = v+3*i0;
		  flag=false;
		  if(I->vertActive[i0]) {
			 if((s01>=0)&&(s02>=0)) flag=true;
			 if(flag) { /* are all normals pointing in generally the same direction? */
				n0 = vn+3*i0; n1 = vn+3*i1; n2 = vn+3*i2;							 
				add3f(n0,n1,vt1);
				add3f(n2,vt1,vt2);
				normalize3f(vt2);
				if(dot_product3f(n0,vt2)<0.1) flag=false;
				if(dot_product3f(n1,vt2)<0.1) flag=false;
				if(dot_product3f(n2,vt2)<0.1) flag=false;
			 } /*printf("pass normal sums %i\n",flag);*/
			 if(flag) { /* does the sum of the normals point in the same direction as the triangle? */
				subtract3f(v1,v0,vt3);
				subtract3f(v2,v0,vt4);
				cross_product3f(vt3,vt4,tNorm); 
				normalize3f(tNorm); 							 
				dp = dot_product3f(vt2,tNorm);
				if(dp<0) scale3f(tNorm,-1.0,tNorm);
				if(fabs(dp)<0.1) flag = false;
			 } /*printf("pass tNorm  %i\n",flag);*/
			 if(flag) {
				if(s12>0) if(dot_product3f(I->vNormal+s12*3,tNorm)<0.1) flag=false; 
				if(s01>0) if(dot_product3f(I->vNormal+s01*3,tNorm)<0.1) flag=false; 
				if(s02>0) if(dot_product3f(I->vNormal+s02*3,tNorm)<0.1) flag=false; 
			 } /*printf("pass compare tNorm %i\n",flag);*/
			 if(flag) { /* are all the Blocking vectors pointing outward, and are the triangle normals consistent? */
				if(s12>0) {
				  i4 = I->edge[s12].vert3;
				  v4=v+i4*3;
				  subtract3f(v0,v1,vt1);
				  subtract3f(v4,v1,vt2);
				  subtract3f(v1,v2,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}			 
				if(s01>0) {
				  i4 = I->edge[s01].vert3;
				  v4=v+i4*3;
				  subtract3f(v2,v0,vt1);
				  subtract3f(v4,v0,vt2);
				  subtract3f(v0,v1,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}
				if(s02>0) {
				  i4 = I->edge[s02].vert3;
				  v4=v+i4*3;
				  subtract3f(v1,v0,vt1);
				  subtract3f(v4,v0,vt2);
				  subtract3f(v0,v2,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}
			 } /*printf("pass blocking %i\n",flag);*/
		  }
		  if(flag) TriangleAdd(i0,i1,i2,tNorm,v,vn);
		}
	 }
  }
}

static void TriangleBuildSecondPass(int i1,int i2,float *v,float *vn,int n)
{

  /* in this version, the closest active point is tried.  Closed points
	  are skipped. */

  TriangleSurfaceRec *I=&TriangleSurface;
  MapType *map;
  float *v1,*v2,*v0,*v4,vt[3],vt1[3],vt2[3],vt3[3],vt4[3],*n0,*n1,*n2,tNorm[3];
  int i0,s01,s02,s12,i,j,h,k,l,i4;
  float dif,minDist,d1,d2,dp;
  int flag;
  int used = -1;

  map=I->map;
  s12 = TriangleEdgeStatus(i1,i2);
  if(s12>0) used = I->edge[s12].vert3;
  if(s12>=0) {
    minDist = MAXFLOAT;
    i0=-1;
    v1=v+i1*3; v2=v+i2*3;
    MapLocus(map,v1,&h,&k,&l);
	 i=*(MapEStart(map,h,k,l));
    if(i) {
      j=map->EList[i++];
      while(j>=0) {
        if((j!=i1)&&(j!=i2)&&(j!=used)&&(I->vertActive[j])) 
			 /* eliminate closed vertices from consideration - where vertactive is 0 */
          {
            v0 = v+3*j;
            d1 = diff3f(v0,v1);
            d2 = diff3f(v0,v2);
            dif= ( d2 > d1 ? d2 : d1 );
				if(dif<minDist)
				  {
					 minDist = dif;
					 i0=j; 
				  }
			 }
		  j=map->EList[i++];
		}
		if(i0>=0) {
		  s01 = TriangleEdgeStatus(i0,i1); s02 = TriangleEdgeStatus(i0,i2);
		  if(I->vertActive[i0]>0) {
			 if(!((s01>0)||(s02>0)))
				i0=-1; /* don't allow non-adjacent joins to active vertices */
		  }
		}
		if(i0>=0) {
		  v0 = v+3*i0;
		  flag=false;
		  if(I->vertActive[i0]) {
			 if((s01>=0)&&(s02>=0)) flag=true;
			 if(flag) { /* are all normals pointing in generally the same direction? */
				n0 = vn+3*i0; n1 = vn+3*i1; n2 = vn+3*i2;							 
				add3f(n0,n1,vt1);
				add3f(n2,vt1,vt2);
				normalize3f(vt2);
				if(dot_product3f(n0,vt2)<0.1) flag=false;
				if(dot_product3f(n1,vt2)<0.1) flag=false;
				if(dot_product3f(n2,vt2)<0.1) flag=false;
			 } /*printf("pass normal sums %i\n",flag);*/
			 if(flag) { /* does the sum of the normals point in the same direction as the triangle? */
				subtract3f(v1,v0,vt3);
				subtract3f(v2,v0,vt4);
				cross_product3f(vt3,vt4,tNorm); 
				normalize3f(tNorm); 							 
				dp = dot_product3f(vt2,tNorm);
				if(dp<0) scale3f(tNorm,-1.0,tNorm);
				if(fabs(dp)<0.1) flag = false;
			 } /*printf("pass tNorm  %i\n",flag);*/
			 if(flag) {
				if(s12>0) if(dot_product3f(I->vNormal+s12*3,tNorm)<0.1) flag=false; 
				if(s01>0) if(dot_product3f(I->vNormal+s01*3,tNorm)<0.1) flag=false; 
				if(s02>0) if(dot_product3f(I->vNormal+s02*3,tNorm)<0.1) flag=false; 
			 } /*printf("pass compare tNorm %i\n",flag);*/
			 if(flag) { /* are all the Blocking vectors pointing outward, and are the triangle normals consistent? */
				if(s12>0) {
				  i4 = I->edge[s12].vert3;
				  v4=v+i4*3;
				  subtract3f(v0,v1,vt1);
				  subtract3f(v4,v1,vt2);
				  subtract3f(v1,v2,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}			 
				if(s01>0) {
				  i4 = I->edge[s01].vert3;
				  v4=v+i4*3;
				  subtract3f(v2,v0,vt1);
				  subtract3f(v4,v0,vt2);
				  subtract3f(v0,v1,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}
				if(s02>0) {
				  i4 = I->edge[s02].vert3;
				  v4=v+i4*3;
				  subtract3f(v1,v0,vt1);
				  subtract3f(v4,v0,vt2);
				  subtract3f(v0,v2,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}
			 } /*printf("pass blocking %i\n",flag);*/
		  }
		  if(flag) TriangleAdd(i0,i1,i2,tNorm,v,vn);
		}
	 }
  }
}

static void TriangleBuildSingle(int i1,int i2,float *v,float *vn,int n)
{

  TriangleSurfaceRec *I=&TriangleSurface;
  MapType *map;
  float *v1,*v2,*v0,*v4,vt[3],vt1[3],vt2[3],vt3[3],vt4[3],*n0,*n1,*n2,tNorm[3];
  int i0,s01,s02,s12,i,j,h,k,l,i4;
  float dif,minDist,d1,d2,dp;
  int flag;
  int used = -1;

  map=I->map;
  s12 = TriangleEdgeStatus(i1,i2);
  if(s12>0) used = I->edge[s12].vert3;
  if(s12>=0) {
    minDist = MAXFLOAT;
    i0=-1;
    v1=v+i1*3; v2=v+i2*3;
    MapLocus(map,v1,&h,&k,&l);
	 i=*(MapEStart(map,h,k,l));
    if(i) {
      j=map->EList[i++];
      while(j>=0) {
        if((j!=i1)&&(j!=i2)&&(j!=used)&&(I->vertActive[j]))
          {
            v0 = v+3*j;
            d1 = diff3f(v0,v1);
            d2 = diff3f(v0,v2);
            dif= ( d2 > d1 ? d2 : d1 );
				if(dif<minDist)
				  {
					 minDist = dif;
					 i0=j; 
				  }
			 }
		  j=map->EList[i++];
		}
		if(i0>=0) {
		  v0 = v+3*i0;
		  flag=false;
		  s01 = TriangleEdgeStatus(i0,i1); s02 = TriangleEdgeStatus(i0,i2);
		  if(I->vertActive[i0]) {
			 if((s01>=0)&&(s02>=0)) flag=true;
			 if(flag) { /* are all normals pointing in generally the same direction? */
				n0 = vn+3*i0; n1 = vn+3*i1; n2 = vn+3*i2;							 
				add3f(n0,n1,vt1);
				add3f(n2,vt1,vt2);
				normalize3f(vt2);
				if(dot_product3f(n0,vt2)<0.1) flag=false;
				if(dot_product3f(n1,vt2)<0.1) flag=false;
				if(dot_product3f(n2,vt2)<0.1) flag=false;
			 } /*printf("pass normal sums %i\n",flag);*/
			 if(flag) { /* does the sum of the normals point in the same direction as the triangle? */
				subtract3f(v1,v0,vt3);
				subtract3f(v2,v0,vt4);
				cross_product3f(vt3,vt4,tNorm); 
				normalize3f(tNorm); 							 
				dp = dot_product3f(vt2,tNorm);
				if(dp<0) scale3f(tNorm,-1.0,tNorm);
				if(fabs(dp)<0.1) flag = false;
			 } /*printf("pass tNorm  %i\n",flag);*/
			 if(flag) {
				if(s12>0) if(dot_product3f(I->vNormal+s12*3,tNorm)<0.1) flag=false; 
				if(s01>0) if(dot_product3f(I->vNormal+s01*3,tNorm)<0.1) flag=false; 
				if(s02>0) if(dot_product3f(I->vNormal+s02*3,tNorm)<0.1) flag=false; 
			 } /*printf("pass compare tNorm %i\n",flag);*/
			 if(flag) { /* are all the Blocking vectors pointing outward, and are the triangle normals consistent? */
				if(s12>0) {
				  i4 = I->edge[s12].vert3;
				  v4=v+i4*3;
				  subtract3f(v0,v1,vt1);
				  subtract3f(v4,v1,vt2);
				  subtract3f(v1,v2,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}			 
				if(s01>0) {
				  i4 = I->edge[s01].vert3;
				  v4=v+i4*3;
				  subtract3f(v2,v0,vt1);
				  subtract3f(v4,v0,vt2);
				  subtract3f(v0,v1,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}
				if(s02>0) {
				  i4 = I->edge[s02].vert3;
				  v4=v+i4*3;
				  subtract3f(v1,v0,vt1);
				  subtract3f(v4,v0,vt2);
				  subtract3f(v0,v2,vt);
				  normalize3f(vt);
				  remove_component3f(vt1,vt,vt3);
				  remove_component3f(vt2,vt,vt4);
				  normalize3f(vt3);
				  normalize3f(vt4);
				  if(dot_product3f(vt3,vt4)>0.0) flag=false;
				}
			 } /*printf("pass blocking %i\n",flag);*/
		  }
		  if(flag) TriangleAdd(i0,i1,i2,tNorm,v,vn);
		}
	 }
  }
}


static void TriangleBuildThirdPass(int i1,int i2,float *v,float *vn,int n)
{
  /* This routine fills in triangles surrounded by three active edges

	*/

  TriangleSurfaceRec *I=&TriangleSurface;
  MapType *map;
  float *v1,*v2,*v0,vt1[3],vt2[3],vt3[3],vt4[3],*n0,*n1,*n2,tNorm[3];
  int i0,s01,s02,s12,i,j,h,k,l;
  float dif,minDist,d1,d2,dp;
  int used = -1;

  map=I->map;
  s12 = TriangleEdgeStatus(i1,i2);
  if(s12>0) used = I->edge[s12].vert3;
  if(s12>=0) {
    minDist = MAXFLOAT;
    i0=-1;
    v1=v+i1*3; v2=v+i2*3;
    MapLocus(map,v1,&h,&k,&l);
	 i=*(MapEStart(map,h,k,l));
    if(i) {
      j=map->EList[i++];
      while(j>=0) {
        if((j!=i1)&&(j!=i2)&&(j!=used)&&(I->vertActive[j]))
          {
            v0 = v+3*j;
            d1 = diff3f(v0,v1);
            d2 = diff3f(v0,v2);
            dif= ( d2 > d1 ? d2 : d1 );
				if(dif<minDist)
				  {
					 minDist = dif;
					 i0=j; 
				  }
			 }
		  j=map->EList[i++];
		}
		if(i0>=0) {
		  v0 = v+3*i0;
		  s01 = TriangleEdgeStatus(i0,i1); s02 = TriangleEdgeStatus(i0,i2);
		  /* if all three edges are active */
		  if((s12>0)&&(s01>0)&&(s02>0)) { 
			 n0 = vn+3*i0; n1 = vn+3*i1; n2 = vn+3*i2;							 
			 add3f(n0,n1,vt1);
			 add3f(n2,vt1,vt2);
			 subtract3f(v1,v0,vt3);
			 subtract3f(v2,v0,vt4);
			 cross_product3f(vt3,vt4,tNorm); 
			 normalize3f(tNorm); 							 
			 dp = dot_product3f(vt2,tNorm);
			 if(dp<0) scale3f(tNorm,-1.0,tNorm);
			 TriangleAdd(i0,i1,i2,tNorm,v,vn);
		  }
		}
	 }
  }
}


static void TriangleBuildLast(int i1,int i2,float *v,float *vn,int n)
{
  /* this routine is a hack to fill in the odd-ball situations */

  TriangleSurfaceRec *I=&TriangleSurface;
  MapType *map;
  float *v1,*v2,*v0,vt1[3],vt2[3],vt3[3],vt4[3],*n0,*n1,*n2,tNorm[3];
  int i0,s01,s02,s12,i,j,h,k,l;
  float dif,minDist,d1,d2,dp;
  int used = -1;
  int both_active;
  map=I->map;
  s12 = TriangleEdgeStatus(i1,i2);
  if(s12>0) used = I->edge[s12].vert3;
  if(s12>=0) {
    minDist = MAXFLOAT;
    i0=-1;
	 both_active = (I->vertActive[i1]&&I->vertActive[i2]);
    v1=v+i1*3; v2=v+i2*3;
    MapLocus(map,v1,&h,&k,&l);
	 i=*(MapEStart(map,h,k,l));
    if(i) {
      j=map->EList[i++];
      while(j>=0) {
        if((j!=i1)&&(j!=i2)&&(j!=used)&&(I->vertActive[j]>0))
          {
            v0 = v+3*j;
            d1 = diff3f(v0,v1);
            d2 = diff3f(v0,v2);
            dif= ( d2 > d1 ? d2 : d1 );
				if(dif<minDist)
				  {
					 minDist = dif;
					 i0=j; 
				  }
			 }
		  j=map->EList[i++];
		}
		if(i0>=0) {
		  v0 = v+3*i0;
		  s01 = TriangleEdgeStatus(i0,i1); s02 = TriangleEdgeStatus(i0,i2);
		  /* if all three edges are active */
		  if(((s12>0)&&(((s01>0)&&(s02>=0))||((s01>=0)&&(s02>0))))||
			  ((s01>0)&&(s02>0))) { 
			 n0 = vn+3*i0; n1 = vn+3*i1; n2 = vn+3*i2;							 
			 add3f(n0,n1,vt1);
			 add3f(n2,vt1,vt2);
			 subtract3f(v1,v0,vt3);
			 subtract3f(v2,v0,vt4);
			 cross_product3f(vt3,vt4,tNorm); 
			 normalize3f(tNorm); 							 
			 dp = dot_product3f(vt2,tNorm);
			 if(dp<0) scale3f(tNorm,-1.0,tNorm);
			 TriangleAdd(i0,i1,i2,tNorm,v,vn);
		  }
		}
	 }
  }
}



static void FollowActives(float *v,float *vn,int n,int mode)
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int i1,i2;
  int cnt;

  cnt = SettingGet(cSetting_test1);
  
  OrthoBusyFast((I->N*2)+I->nTri,I->N*4);

  while(I->nActive&&(I->nTri<cnt)) {
    I->nActive--;
    i1 = I->activeEdge[I->nActive*2];
    i2 = I->activeEdge[I->nActive*2+1];
	 switch(mode) {
	 case 0:
		TriangleBuildObvious(i1,i2,v,vn,n);
		break;
	 case 1:
		TriangleBuildSecondPass(i1,i2,v,vn,n);
		break;
	 case 2:
		TriangleBuildThirdPass(i1,i2,v,vn,n);
		break;
	 case 3:
		TriangleBuildLast(i1,i2,v,vn,n);
		break;
	 }
  }
}

static void TriangleFill(float *v,float *vn,int n)
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int lastTri,lastTri2,lastTri3;
  int a,i,j,h,k,l;
  float dif,minDist,*v0;
  int i1,i2=0;
  MapType *map;

  
  map=I->map;

  lastTri3=-1;
  while(lastTri3!=I->nTri) {
	 lastTri3=I->nTri;

	 I->nActive=0;
    while((!I->nActive)&&(I->nTri==lastTri3))
      {
        i1=-1;
        minDist = MAXFLOAT;
        
        for(a=0;a<n;a++) 
          if(!I->edgeStatus[a])
            {
              v0=v+a*3;
              MapLocus(map,v0,&h,&k,&l);
              i=*(MapEStart(map,h,k,l));
              if(i) {
                j=map->EList[i++];
                while(j>=0) {
                  if(j!=a) 
                    {
                      dif=diff3f(v+3*j,v0);
                      if(dif<minDist)
                        if(I->vertActive[a]==-1)
                          if(TriangleEdgeStatus(a,j)>=0) /* can we put a triangle here? */
                            {
                              minDist = dif;
                              i1=a;
                              i2=j;
                            }
                    }
                  j=map->EList[i++];
                }
              }
            }
	 
        if(i1>=0) {
          if(I->vertActive[i1]<0) I->vertActive[i1]--;
          VLACheck(I->activeEdge,int,I->nActive*2+1);
          I->activeEdge[I->nActive*2] = i1;
          I->activeEdge[I->nActive*2+1] = i2;
          I->nActive=1;
          lastTri=I->nTri;
          FollowActives(v,vn,n,0);
          while(lastTri!=I->nTri) {
            lastTri=I->nTri;
            for(a=0;a<n;a++) 
              if(I->vertActive[a])
                TriangleActivateEdges(a);
            FollowActives(v,vn,n,0);
          }
        } else break;
	 }
	 /*		printf("Follow actives 1 %i\n",I->nTri);*/
	 lastTri=I->nTri-1;
	 while(lastTri!=I->nTri) {
		lastTri=I->nTri;
		for(a=0;a<n;a++) 
		  if(I->vertActive[a])
			 TriangleActivateEdges(a);
		FollowActives(v,vn,n,1);
	 }	 
	 lastTri2=I->nTri-1;
	 while(lastTri2!=I->nTri) {
		lastTri2=I->nTri;
		for(a=0;a<n;a++) 
		  if(I->vertActive[a])
			 {
				TriangleActivateEdges(a);
				if(I->nActive) {
				  /*				  printf("Build single %i %i\n",I->nTri,I->nActive);*/
				  I->nActive--;
				  i1 = I->activeEdge[I->nActive*2];
				  i2 = I->activeEdge[I->nActive*2+1];
				  TriangleBuildSingle(i1,i2,v,vn,n);
				  /*				  printf("Follow actives 1  %i %i\n", I->nTri, I->nActive);*/
				  FollowActives(v,vn,n,1);
				}
			 }
	 }
	 /*	 printf("Follow Actives 2\n");*/
	 for(a=0;a<n;a++) 
		if(I->vertActive[a])
		  TriangleActivateEdges(a);
	 FollowActives(v,vn,n,2);
	 /*	 printf("Follow Actives 3\n");*/
	 lastTri=I->nTri-1;
	 while(lastTri!=I->nTri) {
		lastTri=I->nTri;
		for(a=0;a<n;a++) 
		  if(I->vertActive[a])
			 TriangleActivateEdges(a);
		FollowActives(v,vn,n,3); /* this is a sloppy, forcing tesselation */
	 }
  }
}

static void TriangleFixProblems(float *v,float *vn,int n) 
{
  TriangleSurfaceRec *I=&TriangleSurface;
  int problemFlag;
  int a,l,e;
  int i0,i1,i2,s01,s02,s12;
  int *pFlag = NULL;
  int *vFlag = NULL;
  problemFlag=false;

  pFlag=Alloc(int,n);  
  vFlag=Alloc(int,n);  
  for(a=0;a<n;a++) {
	 vFlag[a]=0;
	 if(I->vertActive[a]) {
		pFlag[a]=1;
		problemFlag=true;
	 } else {
		pFlag[a]=0;
	 }
  }
  if(problemFlag) 
	 {
		a=0;
		while(a<I->nTri) {
		  if(((pFlag[I->tri[a*3]]&&(pFlag[I->tri[a*3+1]]))||
				(pFlag[I->tri[a*3+1]]&&(pFlag[I->tri[a*3+2]]))||
				(pFlag[I->tri[a*3]]&&(pFlag[I->tri[a*3+2]]))))
			 {
				i0=I->tri[a*3];
				i1=I->tri[a*3+1];
				i2=I->tri[a*3+2];
				
				s01=TriangleEdgeStatus(i0,i1);
				if(s01<0) 
				  {
					 s01=-s01; 
					 if(I->edge[s01].tri2!=a) {
						I->edge[s01].tri1=I->edge[s01].tri2;
						I->edge[s01].vert3=I->edge[s01].vert4;
					 }
				  } else s01=0;
				TriangleEdgeSetStatus(i0,i1,s01);

				s02=TriangleEdgeStatus(i0,i2);
				if(s02<0) {
				  s02=-s02; 
					 if(I->edge[s02].tri2!=a) {
						I->edge[s02].tri1=I->edge[s02].tri2;
						I->edge[s02].vert3=I->edge[s02].vert4;
					 }
				} else s02=0;
				TriangleEdgeSetStatus(i0,i2,s02);

				s12=TriangleEdgeStatus(i1,i2);
				if(s12<0) {
				  s12=-s12; 
					 if(I->edge[s12].tri2!=a) {
						I->edge[s12].tri1=I->edge[s12].tri2;
						I->edge[s12].vert3=I->edge[s12].vert4;
					 }
				} else s12=0;
				TriangleEdgeSetStatus(i1,i2,s12);

				I->nTri--;
				TriangleMove(I->nTri,a);

				vFlag[i0]=true;
				vFlag[i1]=true;
				vFlag[i2]=true;
			 }
		  a++;
		}
		
		/* now go through the complicated step of resetting vertex activities */

		for(a=0;a<n;a++) 
		  if(vFlag[a]) 
			 I->vertActive[a]=-1;
		
		for(a=0;a<n;a++) {
		  l=I->edgeStatus[a]; 
		  while(l) {
			 if(I->link[l].value>0) {
				if(vFlag[a]) {
				  e=a;
				  if(I->vertActive[e]<0) I->vertActive[e]=0;
				  I->vertActive[e]++;
				} 
				if(vFlag[I->link[l].index]) {
				  e=I->link[l].index;
				  if(I->vertActive[e]<0) I->vertActive[e]=0;
				  I->vertActive[e]++;
				}
			 } if (I->link[l].value<0) {
				if(vFlag[a]) {
				  e=a;
				  if(I->vertActive[e]<0) I->vertActive[e]=0;
				} 
				if(vFlag[I->link[l].index]) {
				  e=I->link[l].index;
				  if(I->vertActive[e]<0) I->vertActive[e]=0;
				}
			 }
			 l=I->link[l].next;
		  }
		}
		
		TriangleAdjustNormals(v,vn,n);
		TriangleFill(v,vn,n);

	 }
  FreeP(vFlag);
  FreeP(pFlag);
}

int *TrianglePointsToSurface(float *v,float *vn,int n,float cutoff,int *nTriPtr,int **stripPtr,float *extent)
{
  int l;
  TriangleSurfaceRec *I=&TriangleSurface;
  MapType *map;
  int a;
  int cnt;

  I->N=n;
  I->nActive = 0;
  I->activeEdge=VLAlloc(int,1000);

  I->link=VLAlloc(LinkType,n*2);
  I->nLink = 1;

  I->nEdge = 1;

  I->vNormal=VLAlloc(float,n*2);
  I->edge=VLAlloc(EdgeRec,n*2);

  I->tri=VLAlloc(int,n);
  I->nTri = 0;

  I->map=MapNew(cutoff,v,n,extent);
  MapSetupExpress(I->map);
  map=I->map;
  
  I->edgeStatus = Alloc(int,n);
  for(a=0;a<n;a++) {
	 I->edgeStatus[a]=0;
  }

  I->vertActive = Alloc(int,n);
  for(a=0;a<n;a++) {
	 I->vertActive[a]=-1;
  }

  I->vertWeight = Alloc(int,n);
  for(a=0;a<n;a++) {
	 I->vertWeight[a]=2;
  }

  TriangleFill(v,vn,n);

  /*  for(a=0;a<n;a++) 
	 if(I->vertActive[a])
	 printf("before fix %i %i\n",a,I->vertActive[a]);
  */

  TriangleFixProblems(v,vn,n);  

  /*  for(a=0;a<n;a++) 
	 if(I->vertActive[a])
		printf("after fix %i %i\n",a,I->vertActive[a]);
  for(a=0;a<n;a++) 
	 if(I->vertActive[a])
		{
		  TestLine[NTestLine*6]=0;
		  TestLine[NTestLine*6+1]=0;
		  TestLine[NTestLine*6+2]=0;
		  copy3f(v+3*a,TestLine+NTestLine*6+3);
		  NTestLine++;
		  printf("never %i %i\n",a,I->vertActive[a]);
		}
  for(a=0;a<n;a++) {
	 l=I->edgeStatus[a]; 
	 while(l) {
		if(I->link[l].value>=0)
		  printf("edge open %i-%i => %i \n",a,I->link[l].index,I->link[l].value);
		l=I->link[l].next;
	 }
  }
  */

  TriangleAdjustNormals(v,vn,n);

  *(stripPtr) = TriangleMakeStripVLA(v,vn,n);

  cnt = SettingGet(cSetting_test1);
  if(I->nTri>cnt)
	 I->nTri=cnt;

  (*nTriPtr)=I->nTri;
  VLAFreeP(I->activeEdge);
  VLAFreeP(I->link);
  VLAFreeP(I->vNormal);
  VLAFreeP(I->edge);
  FreeP(I->edgeStatus);
  FreeP(I->vertActive);
  FreeP(I->vertWeight);
  MapFree(map);
  return(I->tri);
}

