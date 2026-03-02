#ifndef ADCS_MATH_H
#define ADCS_MATH_H

#include "adcs_types.h"
#include <math.h>

// Vector Operations
void Vector3_Normalize(Vector3_t* v);
float Vector3_Dot(Vector3_t a, Vector3_t b);
Vector3_t Vector3_Cross(Vector3_t a, Vector3_t b);

// Ensures the quaternion has a magnitude of 1.0
void Quat_Normalize(Quaternion_t* q);

// Combines two rotations: res = q_left * q_right
void Quat_Multiply(const Quaternion_t* q_left, const Quaternion_t* q_right, Quaternion_t* res);

// Converts a quaternion to a simple Rotation Matrix (used for vector transformation)
void Quat_To_RotationMatrix(const Quaternion_t* q, float R[3][3]);

void Quat_Conjugate(const Quaternion_t* q, Quaternion_t* res);

#endif