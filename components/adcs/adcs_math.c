#include "adcs_math.h"
#include <math.h>

#define EPSILON 1e-6f

// --- Vector Math ---

void Vector3_Normalize(Vector3_t* v) {
    float mag = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    if (mag > EPSILON) {
        v->x /= mag;
        v->y /= mag;
        v->z /= mag;
    }
}

float Vector3_Dot(Vector3_t a, Vector3_t b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}

Vector3_t Vector3_Cross(Vector3_t a, Vector3_t b) {
    Vector3_t res;
    res.x = a.y * b.z - a.z * b.y;
    res.y = a.z * b.x - a.x * b.z;
    res.z = a.x * b.y - a.y * b.x;
    return res;
}

// --- Quaternion Math ---

void Quat_Normalize(Quaternion_t* q) {
    float mag = sqrtf(q->q0*q->q0 + q->q1*q->q1 + q->q2*q->q2 + q->q3*q->q3);
    if (mag > EPSILON) {
        q->q0 /= mag; q->q1 /= mag; q->q2 /= mag; q->q3 /= mag;
    }
}

void Quat_Multiply(const Quaternion_t* a, const Quaternion_t* b, Quaternion_t* res) {
    // Standard Hamilton product
    res->q0 = a->q0*b->q0 - a->q1*b->q1 - a->q2*b->q2 - a->q3*b->q3;
    res->q1 = a->q0*b->q1 + a->q1*b->q0 + a->q2*b->q3 - a->q3*b->q2;
    res->q2 = a->q0*b->q2 - a->q1*b->q3 + a->q2*b->q0 + a->q3*b->q1;
    res->q3 = a->q0*b->q3 + a->q1*b->q2 - a->q2*b->q1 + a->q3*b->q0;
}

void Quat_Conjugate(const Quaternion_t* q, Quaternion_t* res) {
    res->q0 = q->q0;   // Scalar part remains the same
    res->q1 = -q->q1;  // Vector part is negated
    res->q2 = -q->q2;
    res->q3 = -q->q3;
}

