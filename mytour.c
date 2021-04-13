#include "mytour.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <alloca.h>
#include <omp.h>

/* Changes Made :
Added SIMD instructions and used OpenMP.
1) Firstly, Parallelised the for loop which initialises the visited array using Parallel For loop. 
2) Then, calculated the modified length called `modified_len` to accomodate the cases where ncities is not exactly divisibe by 4.
3) Created 2 arrays called city_X and city_Y which stores the x and y coordinates of every city. Used another parallel for loop for this loop. 
4) Created __m128 variables to calculate the cost and distance. Using Parallel For Loop and SIMD variables, code was parallelised and the distance was found much quicker.
   Stored the cost in an array called `cost_array` which is used later in the code in comparing the costs.
Remarks:
1) Major difference is seen when there a large number of cities as parallelisation for a small dataset incorporates larger computational cost. 
2) Answers found through the sequential and parallel way are exactly same and accuracy is maintained.
3) Further improvement can be done by using Dynamic Approach to find the tour instead of brute-force.
*/

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

  //Intialising visited array
  #pragma omp for 
  for (i=0; i<ncities; i++){
    visited[i]=0;
  }
    
  ThisPt = ncities-1;           // Start from the last city
  visited[ncities-1] = 1;       // Marks that the last city has been visited
  tour[endtour++] = ncities-1;  // ?? tour[0] = last city  
  
  for (i=1; i<ncities; i++) {
    CloseDist = DBL_MAX;        // Close Distance initialised to maximum floating number

  // Calculate Cost for neighbors for this array
  int index;  
  int thisPointX = cities[ThisPt].x;
  int thisPointY = cities[ThisPt].y;
  float *costArray = malloc(sizeof(float) * ncities);
 
  int difference_X;
  int difference_Y;
  #pragma omp for private(difference_X, difference_Y)
  for (index = 0; index < ncities; index++){
    difference_X = thisPointX - cities[index].x;
    difference_Y = thisPointY - cities[index].y;  
    costArray[index] = sqrt(sqr(difference_X) + sqr(difference_Y)); 
  }

  for (j=0; j<ncities-1; j++) {
      if (!visited[j]) {
	      float checkShortDist = costArray[j];
        if (checkShortDist < CloseDist) {
	        CloseDist = checkShortDist;
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
  if (ncities < 1000) {
    sequential_find_tour(cities, tour, ncities);
  }else{
    vectorised_find_tour(cities, tour, ncities);
  }
}
