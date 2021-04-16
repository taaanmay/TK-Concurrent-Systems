#include "mytour.h"
#include <alloca.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <sys/time.h>
#include "sales.h"
#include <omp.h>


float local_sqr(float x)
{
  return x*x;
}

float local_dist(const point cities[], int i, int j) {
  return sqrt(local_sqr(cities[i].x-cities[j].x) + local_sqr(cities[i].y-cities[j].y));
}

void find_tour(const point cities[], int tour[], int ncities)
{
  
	
	  int j;
	  char *visited = alloca(ncities);
	  int ThisPt, ClosePt=0;
	  float CloseDist;
	  int endtour=0;
  
  	  int i;
	  #pragma omp parallel for 
	  for (i=0; i<ncities; i++)
	  {
	  	visited[i]=0;
	  }
	    
	  

	  ThisPt = ncities-1;
	  visited[ncities-1] = 1;
	  tour[endtour++] = ncities-1;
	  
	  for (i=1; i<ncities; i++) {
	    CloseDist = DBL_MAX;
	    for (j=0; j<ncities-1; j++) {
	      if (!visited[j]) {
		if (local_dist(cities, ThisPt, j) < CloseDist) {
		  CloseDist = local_dist(cities, ThisPt, j);
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
  find_tour(cities, tour, ncities);
}
