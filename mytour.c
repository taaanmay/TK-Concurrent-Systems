#include "mytour.h"
#include <alloca.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <sys/time.h>
#include "sales.h"
#include <omp.h>
#include <emmintrin.h>
#include <xmmintrin.h>



/* Changes Made :
Added SIMD instructions and used OpenMP.
1) Firstly, Parallelised the for loop which initialises the visited array using Parallel For loop. 
2) Then, calculated the modified length called `modified_len` to accomodate the cases where ncities is not exactly divisibe by 4.
3) Created 2 arrays called city_X and city_Y which stores the x and y coordinates of every city. Used another parallel for loop for this loop. 
4) Created __m128 variables to calculate the cost and distance. Using Parallel For Loop and SIMD variables, code was parallelised and the distance was found much quicker.
   Stored the cost in an array called `cost_array` which is used later in the code in comparing the costs.
5) We get guaranteed speedups when ncities > 35000. So, we run the sequential code when ncities is less than 30,000 and modified code is run otherwise. 
 
Remarks:
1) Major difference is seen when there a large number of cities as parallelisation for a small dataset incorporates larger computational cost. 
2) Answers found through the sequential and parallel way are exactly same and accuracy is maintained.

*/



float local_sqr(float x)
{
  return x*x;
}

float local_dist(const point cities[], int i, int j) {
  return sqrt(local_sqr(cities[i].x-cities[j].x) + local_sqr(cities[i].y-cities[j].y));
}

void sequential_tour(const point cities[], int tour[], int ncities)
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
		float temp_distance = local_dist(cities, ThisPt, j);
		if (temp_distance < CloseDist) {
	  		CloseDist = temp_distance;
	  		ClosePt = j;
		}
      }
    }
    tour[endtour++] = ClosePt;
    visited[ClosePt] = 1;
    ThisPt = ClosePt;
  }
}

// Parallelised and Vectorised Code
void modified_find_tour(const point cities[], int tour[], int ncities)
{
  
	
  int i,j, index;
  int ThisPt, ClosePt=0;
  float CloseDist;
  int endtour=0;


  // Visited Array is initialised
  // Used SIMD and OpenMP for vecotrisation and parallelisation
  int len_visited = sizeof(float)*ncities + sizeof(float)*(ncities % 4);
  float visited[len_visited];

  __m128 zero_4 =_mm_set1_ps(0);
  #pragma omp for 
  for (i = 0; i < len_visited; i = i+4){
    _mm_store_ps(&visited[i], zero_4);
  }

  // Obtain modifiedlength for cases when ncities is not exactly divisible by 4
  int modified_len = ncities + (4 - (ncities % 4));

  // X and Y coordinate extracted for computing distance
  float *cityx = malloc(sizeof(float) * modified_len);
  float *cityy = malloc(sizeof(float) * modified_len);
  #pragma omp parallel for
  for (i = 0; i < ncities; i++) {
      cityx[i] = cities[i].x;
      cityy[i] = cities[i].y;
   }

  ThisPt = ncities-1;           
  visited[ncities-1] = 1;       
  tour[endtour++] = ncities-1;  

  // Variables initialised for SIMD
  __m128 this_Pt_X;
  __m128 this_Pt_Y;
  __m128 diff_X;
  __m128 diff_Y;
  __m128 current_X;
  __m128 current_Y;
  __m128 diff_square_X;
  __m128 diff_square_Y;
  __m128 distance;
  
  
  for (i=1; i<ncities; i++) {
    
  // Calculate Cost
  float *costArray = malloc(sizeof(float) * modified_len);  

  #pragma omp parallel for private(index, this_Pt_X, this_Pt_Y, current_X, current_Y, diff_X, diff_Y, diff_square_X, diff_square_Y, distance)
  for (index = 0; index < modified_len; index += 4){
    this_Pt_X = _mm_set1_ps(cityx[ThisPt]);
    current_X  = _mm_load_ps(&cityx[index]);
    
    this_Pt_Y = _mm_set1_ps(cityy[ThisPt]);
    current_Y  = _mm_load_ps(&cityy[index]);

    diff_X = _mm_sub_ps(this_Pt_X, current_X);
    diff_Y = _mm_sub_ps(this_Pt_Y, current_Y);  
    
    diff_square_X = _mm_mul_ps(diff_X, diff_X);
    diff_square_Y = _mm_mul_ps(diff_Y, diff_Y);

    distance = _mm_sqrt_ps(_mm_add_ps(diff_square_X,diff_square_Y));
    _mm_store_ps(&costArray[index],distance);
  }

    CloseDist = DBL_MAX;        
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
  // Parallelised Code will be used when number of cities is greater than 30,000
  if(ncities >= 30000){
  	modified_find_tour(cities, tour, ncities);	
  }else{
  	sequential_tour(cities, tour, ncities);	
  }
  
}
