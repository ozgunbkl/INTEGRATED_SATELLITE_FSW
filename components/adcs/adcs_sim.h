#ifndef ADCS_SIM_H
#define ADCS_SIM_H

#include "adcs_types.h"

// Simulates reading a 3-axis Magnetometer
// Returns a vector in micro-Teslas (uT)
Vector3_t SIM_ReadMagnetometer(void);

// Simulates reading a 3-axis Gyroscope
// Returns angular velocity in degrees per second (deg/s)
Vector3_t SIM_ReadGyroscope(void);

// Force a specific physics state for testing/simulation
void SIM_SetTrueAngularVelocity(float x, float y, float z);

Quaternion_t SIM_ReadEstimatedAttitude(void);
void SIM_SetTrueAttitude(float q0, float q1, float q2, float q3);

void SIM_UpdatePhysics(Vector3_t dipole_cmd, float dt);

void SIM_ApplyBoost(float km_boost);
float SIM_GetAltitude(void);
float ADCS_GetFuelLevel(void);

#endif