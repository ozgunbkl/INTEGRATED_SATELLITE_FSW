#ifndef ADCS_TYPES_H
#define ADCS_TYPES_H

#include <stdint.h>

typedef struct {
    float x, y, z;
} Vector3_t;

typedef struct {
    float q0;  // Scalar part
    float q1, q2, q3;  // Vector part
} Quaternion_t;


typedef enum {
    ADCS_MODE_SAFE,
    ADCS_MODE_DETUMBLE,
    ADCS_MODE_SUN_SAFE,
    ADCS_MODE_NOMINAL,
    ADCS_MODE_PAYLOAD_POINT
} ADCSMode_t;

#endif