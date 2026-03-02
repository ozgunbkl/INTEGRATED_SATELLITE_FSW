#ifndef ADCS_CONTROL_H
#define ADCS_CONTROL_H

#include "adcs_types.h"

// Calculates the magnetic moment command (Am^2) to stop tumbling
// mag_now: Current magnetometer reading
// mag_prev: Previous magnetometer reading
// dt: Time step between readings
Vector3_t Control_BDot(Vector3_t mag_now, Vector3_t mag_prev, float dt);
Vector3_t Control_PD(Quaternion_t q_error, Vector3_t omega);

#endif