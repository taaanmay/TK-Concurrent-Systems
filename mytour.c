#include "mytour.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <alloca.h>
//#include <omp.h>



float my_tour_sqr(float x)
{
  return x*x;
}

float my_tour_dist(const point cities[], int i, int j) {
  return sqrt(my_tour_sqr(cities[i].x-cities[j].x)+
	      my_tour_sqr(cities[i].y-cities[j].y));
}

void sequential_find_tour(const point cities[], int tour[], int ncities)
{
  int i,j;
  char *visited = alloca(ncities);
  int ThisPt, ClosePt=0;
  float CloseDist;
  int endtour=0;
  
  for (i=0; i<ncities; i++)
    visited[i]=0;
  ThisPt = ncities-1;
  visited[ncities-1] = 1;
  tour[endtour++] = ncities-1;
  
  for (i=1; i<ncities; i++) {
    CloseDist = DBL_MAX;
    for (j=0; j<ncities-1; j++) {
      if (!visited[j]) {
	      float temp_dist = my_tour_dist(cities, ThisPt, j);
        if (temp_dist < CloseDist) {
	        CloseDist = temp_dist;
	        ClosePt = j;
	      }
      }
    }
    tour[endtour++] = ClosePt;
    visited[ClosePt] = 1;
    ThisPt = ClosePt;
  }
}

void vectorised_find_tour(const point cities[], int tour[], int ncities)
{
  int i,j;
  char *visited = alloca(ncities);
  int ThisPt, ClosePt=0;
  float CloseDist;
  int endtour=0;
  
  #pragma omp parallel for
  for (i=0; i<ncities; i++)
    { visited[i]=0; }

  ThisPt = ncities-1;
  visited[ncities-1] = 1;
  tour[endtour++] = ncities-1;
  
  for (i=1; i<ncities; i++) {
    CloseDist = DBL_MAX;
    for (j=0; j<ncities-1; j++) {
      if (!visited[j]) {
	      float temp_dist = my_tour_dist(cities, ThisPt, j);
        if (temp_dist < CloseDist) {
	        CloseDist = temp_dist;
	        ClosePt = j;
	      }
      }
    }
    tour[endtour++] = ClosePt;
    visited[ClosePt] = 1;
    ThisPt = ClosePt;
  }
}


void my_tour(const point cities[], int tour[], int ncities)
{
  if (ncities < 10000) {
    sequential_find_tour(cities, tour, ncities);
  }else{
    vectorised_find_tour(cities, tour, ncities);
  }
}