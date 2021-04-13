#include "mytour.h"
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <alloca.h>
#include <omp.h>



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
  //char *visited = alloca(ncities);
  int ThisPt, ClosePt=0;
  float CloseDist;
  int endtour=0;
  
  /*
  for (i=0; i<ncities; i++)
    visited[i]=0;
  */

  //Intialising visited array  
  int no_visited = sizeof(float)*ncities + sizeof(float)*(ncities % 4);
  float visited[no_visited];

  __m128 zero_4 =_mm_set1_ps(0);
  #pragma omp parallel for 
  for (i=0; i<no_visited; i+=4){
    _mm_store_ps(&visited[i],zero_4);
  }  
  
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
  int i,j, index;
  int ThisPt, ClosePt=0;
  float CloseDist;
  int endtour=0;


  //Intialising visited array  
  int no_visited = sizeof(float)*ncities + sizeof(float)*(ncities % 4);
  float visited[no_visited];

  __m128 zero_4 =_mm_set1_ps(0);
  #pragma omp parallel for 
  for (i=0; i<no_visited; i+=4){
    _mm_store_ps(&visited[i],zero_4);
  }

  // Change length for the cases where length is not exactly divisible by 4
  int modified_len = ncities + (4 - (ncities % 4));

  // Extract the x and y values for each city
  float *city_X = malloc(sizeof(float) * modified_len);
  float *city_Y = malloc(sizeof(float) * modified_len);
  #pragma omp parallel for
  for (i = 0; i < ncities; i++) {
      city_X[i] = cities[i].x;
      city_Y[i] = cities[i].y;
   }

  ThisPt = ncities-1;           // Start from the last city
  visited[ncities-1] = 1;       // Marks that the last city has been visited
  tour[endtour++] = ncities-1;  // ?? tour[0] = last city   

  __m128 thisPointX;
  __m128 thisPointY;
  __m128 difference_X;
  __m128 difference_Y;
  __m128 current_X;
  __m128 current_Y;
  __m128 diff_square_X;
  __m128 diff_square_Y;
  __m128 distance;
  
  
  for (i=1; i<ncities; i++) {
    
  // Calculate Cost for neighbors for this array
  float *costArray = malloc(sizeof(float) * modified_len);  

  #pragma omp parallel for private(index, thisPointX, thisPointY, difference_X, difference_Y, current_X, current_Y, diff_square_X, diff_square_Y, distance)
  for (index = 0; index < modified_len; index += 4){
    thisPointX = _mm_set1_ps(city_X[ThisPt]);
    current_X  = _mm_load_ps(&city_Y[index]);
    
    thisPointY = _mm_set1_ps(city_X[ThisPt]);
    current_Y  = _mm_load_ps(&city_Y[index]);

    difference_X = _mm_sub_ps(thisPointX, current_X);
    difference_Y = _mm_sub_ps(thisPointY, current_Y);  
    
    diff_square_X = _mm_mul_ps(difference_X, difference_X);
    diff_square_Y = _mm_mul_ps(difference_Y, difference_Y);

    distance = _mm_sqrt_ps(_mm_add_ps(diff_square_X,diff_square_Y));
    _mm_store_ps(&costArray[index],distance);
  }

    CloseDist = DBL_MAX;        // Close Distance initialised to maximum floating number
    for (j=0; j<ncities-1; j++) {
      if (!visited[j] && costArray[j] < CloseDist) {
	      CloseDist = costArray[j];
	      ClosePt = j;
	    }
    }
    tour[endtour++] = ClosePt;
    visited[ClosePt] = 1;
    ThisPt = ClosePt;
    free(costArray);
  }
}


void my_tour(const point cities[], int tour[], int ncities)
{
  if (ncities < 20000) {
    sequential_find_tour(cities, tour, ncities);
  }else{
    vectorised_find_tour(cities, tour, ncities);
  }
}
