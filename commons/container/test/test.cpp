#include <stdlib.h>
#include <math.h>

#include <iostream>



int main(   )
{

  int N = 14000000;
  uint64_t t=clock();
/*
 float STEP = 4. * M_PI / N;
  float r = -2.*M_PI; 
  float d = 0;
*/
  double STEP = 4. * M_PI / N;
  double r = -2.*M_PI; 
  double d = 0;

for(int i=0; i < N; i++ ) {
   d=sinf(r);// cos(r);
   r += STEP;
}
  t=clock()-t;
  printf("~%.1f ns per call. %.1f %.f\n",1.0e+9*(double(t)/double(N))/double(CLOCKS_PER_SEC),r,d);

return EXIT_SUCCESS;
}
