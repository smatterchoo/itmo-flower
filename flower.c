#include <stdio.h>
#include <stdlib.h>
#include "edx-io.h"
#include "math.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

#define ten5 100000

int gs[100000]; //#seeds planted by gardener
int gl[100000]; //left or starting bed for gardener
int gr[100000]; //right or ending bed for gardener
int n; // num beds
int ng; // num gardeners

int at[ten5];
int slope[ten5];
int prev[ten5];
int delta[ten5];
int nd; //num of delta rows

/*
1..B
beds  1---2---3---4---5---6---7---8---9--10
g1    3   3   3   3
g2                4   4   4   4
g3                            5   5   5   5
i1    i   i   i   i   i   i   i   i   i   i

so i1= 3+3+3+7+4+4+9+5+5+5

cumulative would make it easier, so just subtract to get range vs summing

beds  0---1---2---3---4---5---6---7---8---9--10
cum   0   3   6   9   16  20  24  33  38  43 48

so to inspect 1-10, cum(10)-cum(0) or 48

test.. inspect 3-6 should be 2*3+3*4=18.. cum(6)-cum(2)=24-6=18

more compact representation

range:1-3 prev:0 slope:3  so eg [2]=6
range:4 prev:9 slope:7 so [4]=16
range:5-6 prev:16 slope:4 so [6]=24
etc

to get ranges efficiently.. break all ranges into 2
eg +3 from 1..4 impact on overall slope is +3 @ 1, -3 @ 5
eg +4 from 4..7 .. becomes +4 @ 4, -4 @ 8
sort on the "@"'s

@1, +3  [this will be the delta table]
@4, +4
@5, -3
@7, +5
@8, -4
@11, -5

you can accumulate these

@1, +3
@4, +7
@5, +4
@7, +9
@8, +5
@11, 0

and you can use these slopes to get prevs

@1, +3, 0
@4, +7, 9 (3 from @1,@2,@3 count, * +3 slope + prev @1= 9)
@5, +4, 16 (my prev = prev prev + n * prev slope)
...

*/

//sorts the dat array, applying all swaps to the "co"sorted array too
void qqsort(int s,int e, int *dat, int *co) {
  int s_ = s;
  int e_ = e;
  int tmp,tmp2;
  int mi = (s+e)/2;
  int m = dat[mi];
  while (s_ <= e_) {
    while (dat[s_] < m && s_ <= e) { s_++;}
    while (m < dat[e_] && e_ >= 0) { e_--;}
    if (s_ <= e_) {
      tmp=dat[s_];      tmp2=co[s_];
      dat[s_]=dat[e_];  co[s_]=co[e_];
      dat[e_]=tmp;      co[e_]=tmp2;
      s_++;
      e_--;
    }
  }
  if (s <= e_ && e_ != e) { qqsort(s, e_, dat, co); }
  if (s_ <= e && s_ != s) { qqsort(s_, e, dat, co); }
}

// eg if the @ list has vals 2,4,7,11
// and i is 8, start w at[2]=7
//  if i is 7, start w at[2]=7
//  if i is 1, return 0 (not in range)
//  or if i is 20, start w at[3]=11
// THEN
// based on where you start, return prev[] + slope*(i-start)
int cumulative(int val, int right) {
  int n = nd; // numdeltas is the same as the number of @'s
  int peek = n/2; // where to look next, start in middle
  while (peek >=0 && peek < n) {
    //    fprintf(stderr,"in peek %d looking for %d \n",peek,val);
    if (at[peek]==val || at[peek]<val && peek<n-1 && at[peek+1]>val) {
      break;}
    else if (at[peek]>val) {
      peek = MIN(peek/2, peek-1);}
    else { // at[peek]<val..
      peek = MAX((peek+n)/2,peek+1); } // eg peek=2, n=4.. 6/2=3
  }
  //  fprintf(stderr,"peek %d looking for %d \n",peek,val);
  if (peek < 0) {
    return 0;
  }
  if (peek >= n) { // case when looking for val beyond last "at"
    peek=n-1; // calc as usual, from last val
  }
  return prev[peek]+slope[peek]*(val-at[peek]+(right>0?1:0));

}

int main() {
  
  edx_open();
  n=edx_next_i32();
  
  ng=edx_next_i32();

  nd=0;
  //  fprintf(stderr,"ng %d\n",ng);
  for (int i=0;i<ng;i++){
    //    fprintf(stderr,"g %d %d \n",at[i],delta[i]);
    int s=edx_next_i32(); // seeds
    at[nd]= edx_next_i32(); // left
    delta[nd++]=s;
    at[nd]= edx_next_i32()+1; // right
    delta[nd++]=-s;
  }

  /*  for (int i=0;i<nd;i++) {
    fprintf(stderr,"@ %d delta %d\n",at[i],delta[i]);
  }
  fprintf(stderr,"\n");*/
  
  qqsort(0,nd-1,at,delta);

  //cumulative.. eg if you have @2 3 then @5 1
  //@2 prev is 0, @5 prev is 9 (3 from 2,3,4)
  //@2 slope is 3, @5 slope is 4 (since we started adding 3 at 2 then also 1 at 5)
  prev[0]=0;
  slope[0]=delta[0];
  for (int i=1;i<nd;i++) {
    prev[i]=prev[i-1]+(at[i]-at[i-1])*slope[i-1];
    slope[i]=slope[i-1]+delta[i];
  }

  //  for (int i=0;i<nd;i++) {
    //    fprintf(stderr,"@ %d prev %d slope %d\n",at[i],prev[i],slope[i]);
  //  }
  //  fprintf(stderr,"\n");
  
  int ni=edx_next_i32(); // num inspectors
  
  for (int i=0;i<ni;i++){
    int accum=0;
    int il=cumulative(edx_next_i32(),0);
    int ir=cumulative(edx_next_i32(),1);
    //    fprintf(stderr,"start %d end %d\n",il,ir);
    //need to find the last @ before il
    //and the last @ before ir
    edx_printf("%d \n",ir-il);
  }

  return 0;
}
