#include <stdio.h>
#include <math.h>
#include "atmosphere.h"

//Resolution of the output image
#define W 640
#define H 480

//Viewport angular size (+-)
#define XLIM 3.0
#define YLIM 2.0

//Sun distance above the horizon
#define SUN_HEIGHT 1

//Utility function for mapping from pixel to angular coordinates
float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * [main description]
 */
int main() {
  vec3 r0 = (vec3){0,6372e3,0};     //
  vec3 pSun = (vec3){0,SUN_HEIGHT,-1};
  vec3 rd;
  vec3 color;

  //Print the image header
  printf("P3\n"); //color image
  printf("%d %d\n",W,H); //dimensions
  printf("255\n"); //max values

  int ix, iy;
  for(iy=0;iy<H;iy++) {
   for(ix=0;ix<W;ix++) {
     //Calculate the ray direction for the given (x,y) "pixel"
     rd = (vec3){
       map(ix, 0, W, -XLIM, XLIM),
       map(iy, 0, H, YLIM, -YLIM),
       -1
     };

     //Find the atmosphereic colour, and scale to [0,255]
     color = atmosphere(r0, rd, pSun, ATMOSPHERE_EARTH);
     color = mul(expose(color), 255);

     //Output the pixel
     if (ix) printf("   "); //Spacing if needed.
     printf("%3d %3d %3d", (int)round(color.x), (int)round(color.y), (int)round(color.z));
   }
   printf("\n");
 }
}
