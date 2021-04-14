#include "mytour.h"
#include <alloca.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <sys/time.h>
#include "sales.h"
#include <omp.h>


float my_tour_sqr(float x)
{
  return x*x;
}

float my_tour_dist(const point cities[], int i, int j) {
  return sqrt(my_tour_sqr(cities[i].x-cities[j].x) + my_tour_sqr(cities[i].y-cities[j].y));
}

void seq_find_tour(const point cities[], int tour[], int ncities)
{
  int i,j;
  char *visited = alloca(ncities);
  int ThisPt, ClosePt=0;
  float CloseDist;
  int endtour=0;
  
  for (i=0; i<ncities; i++){
    visited[i]=0;
  }
    
  ThisPt = ncities-1;
  visited[ncities-1] = 1;
  tour[endtour++] = ncities-1;
  
  for (i=1; i<ncities; i++) {
    CloseDist = DBL_MAX;
    for (j=0; j<ncities-1; j++) {
      if (!visited[j]) {
	      if (my_tour_dist(cities, ThisPt, j) < CloseDist) {
	         CloseDist = my_tour_dist(cities, ThisPt, j);
	          ClosePt = j;
	      }
      }
    }
    tour[endtour++] = ClosePt;
    visited[ClosePt] = 1;
    ThisPt = ClosePt;
  }
}






void find_tour(const point cities[], int tour[], int ncities)
{
  int i,j;
  char *visited = alloca(ncities);
  int ThisPt, ClosePt=0;
  float CloseDist;
  int endtour=0;
  
  // Parallelise if ncities > 1000
  #pragma omp parallel for
  for (i=0; i<ncities; i++){
    visited[i]=0;
  }
    
  ThisPt = ncities-1;
  visited[ncities-1] = 1;
  tour[endtour++] = ncities-1;
  
  for (i=1; i<ncities; i++) {
    //CloseDist = DBL_MAX;


     		  
 #pragma omp parallel
 {
      // Local variables for parallel
     float tempCloseDist = DBL_MAX;
     int tempClosePt;
      // Parallelise if ncities > 1000
     #pragma omp for
      for (j=0; j<ncities-1; j++) {
        if (!visited[j]) {
          float temp = my_tour_dist(cities, ThisPt, j);
          if ( temp < tempCloseDist) {
              tempCloseDist = temp;
              tempClosePt = j;
          }
        }
      }
   
   #pragma omp critical
   {
      CloseDist = tempCloseDist;
      ClosePt = tempClosePt;
      tour[endtour++] = ClosePt;
      visited[ClosePt] = 1;
      ThisPt = ClosePt;	
    }	    
}
   
    
  }
}



void my_tour(const point cities[], int tour[], int ncities)
{
  find_tour(cities, tour, ncities);
}
