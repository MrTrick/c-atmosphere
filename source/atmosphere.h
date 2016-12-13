#ifndef _ATMOSPHERE_H_
#define _ATMOSPHERE_H_

//Vector types
typedef struct {float x, y;} vec2;
typedef struct {float x, y, z;} vec3;

//Utility functions
void printvec3(vec3 a);
float dot(vec3 a, vec3 b);
vec3 mul(vec3 a, float b);
float length(vec3 a);
vec3 normalize(vec3 a);
vec3 add(vec3 a, vec3 b);
float min(float a, float b);
vec3 expose(vec3 a);

/**
 * Define a "standard" earthlike set of parameters to use in atmosphere()
 */
#define ATMOSPHERE_EARTH 22.0, 6371e3, 6471e3, (vec3){5.5e-6,13.0e-6,22.4e-6}, 21e-6, 8e3, 1.2e3, 0.758

/**
 * Calculate the color of the sky!
 * If the standard earth values are desired, can be called using:
 *   atmosphere(r0, rd, pSun, ATMOSPHERE_EARTH);
 *
 * @param  r0      Ray origin (eg: (vec3){0,6372e3,0})
 * @param  rd       Normalized ray direction
 * @param  pSun    Direction of the sun (eg: (vec3){0,0.1,-1})
 * @param  iSun    Intensity of the sun (eg: 22.0)
 * @param  rPlanet Radius of the planet in meters (eg: 6371e3)
 * @param  rAtmos  Radius of the atmosphere in meters (eg: 6471e3)
 * @param  kRlh    Rayleigh scattering coefficient (eg: (vec3){5.5e-6, 13.0e-6, 22.4e-6})
 * @param  kMie    Mie scattering coefficient (eg: 21e-6)
 * @param  shRlh   Rayleigh scale height (eg: 8e3)
 * @param  shMie   Mie scale height (eg: 1.2e3)
 * @param  g       Mie preferred scattering direction (eg: 0.758)
 * @return         [description]
 */
vec3 atmosphere(vec3 r0, vec3 rd, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g);

#endif
