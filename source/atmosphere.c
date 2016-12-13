#include <stdio.h>
#include <math.h>
#include "atmosphere.h"

//Internal constants
#define PI 3.141592
#define ISTEPS 16
#define JSTEPS 8

//Utility functions
void printvec3(vec3 a) { printf("(%.3f, %.3f, %.3f)", a.x, a.y, a.z); }
float dot(vec3 a, vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
vec3 mul(vec3 a, float b) { return (vec3){a.x*b, a.y*b, a.z*b}; }
float length(vec3 a) { return sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }
vec3 normalize(vec3 a) { return mul(a, 1/length(a)); }
vec3 add(vec3 a, vec3 b) { return (vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }
float min(float a, float b) { return a<b?a:b; }
vec3 expose(vec3 a) { return (vec3){1.0-exp(-a.x), 1.0-exp(-a.y), 1.0-exp(-a.z)}; }

/**
 * Calculate ray-sphere intersection
 * Assume that the sphere is centred at the origin.
 * No intersection when result.x > result.y
 * @param  r0 Ray origin
 * @param  rd Normalised ray direction
 * @param  sr Sphere radius
 * @return A vec2 with the distances. x=minimum, y=maximum.
 */
vec2 rsi(vec3 r0, vec3 rd, float sr) {
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return (vec2){1e5,-1e5};
    return (vec2){
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    };
}

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
vec3 atmosphere(vec3 r0, vec3 rd, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {
    int i,j;

    // Normalize the sun and view directions.
    pSun = normalize(pSun);
    rd = normalize(rd);

    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, rd, rAtmos);                                  // How far in this direction is the edge of the atmosphere?
    if (p.x > p.y) return (vec3){0,0,0};                          // If the ray will not contact the atmosphere, return BLACK.
    p.y = min(p.y, rsi(r0, rd, rPlanet).x);                        // Does the ray hit the planet? If so, then shorten the distance.
    float iStepSize = (p.y - p.x) / (float)ISTEPS;                // Divide that distance into the configured num of steps.
                                                                  // NOTE: Not sure why it's using p.x as well; p.x would be distance behind the observer...

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = (vec3){0,0,0};
    vec3 totalMie = (vec3){0,0,0};

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(rd, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (i = 0; i < ISTEPS; i++) {

        // Calculate the primary ray sample position.
        vec3 iPos = add(r0, mul(rd, (iTime + iStepSize * 0.5)));

        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / (float)JSTEPS;

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (j = 0; j < JSTEPS; j++) {

            // Calculate the secondary ray sample position.
            vec3 jPos = add(iPos, mul(pSun, (jTime + jStepSize * 0.5)));

            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attn = (vec3){
          exp(-(kMie * (iOdMie + jOdMie) + kRlh.x * (iOdRlh + jOdRlh))),
          exp(-(kMie * (iOdMie + jOdMie) + kRlh.y * (iOdRlh + jOdRlh))),
          exp(-(kMie * (iOdMie + jOdMie) + kRlh.z * (iOdRlh + jOdRlh)))
        };

        // Accumulate scattering.
        totalRlh = add(totalRlh, mul(attn, odStepRlh));
        totalMie = add(totalMie, mul(attn, odStepMie));

        // Increment the primary ray time.
        iTime += iStepSize;
    }

    // Calculate and return the final color.
    return (vec3){
      iSun * (pRlh * kRlh.x * totalRlh.x + pMie * kMie * totalMie.x),
      iSun * (pRlh * kRlh.y * totalRlh.y + pMie * kMie * totalMie.y),
      iSun * (pRlh * kRlh.z * totalRlh.z + pMie * kMie * totalMie.z)
    };
}
