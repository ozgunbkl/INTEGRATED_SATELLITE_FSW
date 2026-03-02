#include "adcs_sim.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

// Internal "True" state of the simulation
static Vector3_t true_angular_velocity = {0.5f, -0.2f, 0.1f}; // rad/s
static Vector3_t world_mag_field = {20.0f, 0.0f, 40.0f}; // uT (North-ish)
static Quaternion_t true_attitude = {1.0f, 0.0f, 0.0f, 0.0f}; // Identity (Perfectly aligned)
static Vector3_t local_mag_sim = {20.0f, 0.0f, 40.0f};

static float g_altitude_km = 548.26f; 
const float DRAG_COEFF = 0.1f; // How fast we fall

// Resource Management
static float g_fuel_level = 100.0f;     // Percent (%)


// Helper to add 1% random noise
static float add_noise(float value, float amlitude) {
    float noise = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f;  // -1 to 1
    return value + (noise * amlitude);
}

Vector3_t SIM_ReadMagnetometer(void) {
    // SIMULATION TRICK: Rotate the local field based on how fast we are spinning
    // This makes the B-Dot controller "see" the tumble
    local_mag_sim.x += true_angular_velocity.y * 0.01f; 
    local_mag_sim.y -= true_angular_velocity.x * 0.01f;
    
    // Normalize it roughly so it stays around the 40-50uT range
    float mag_magnitude = sqrt(local_mag_sim.x*local_mag_sim.x + local_mag_sim.y*local_mag_sim.y + local_mag_sim.z*local_mag_sim.z);
    local_mag_sim.x = (local_mag_sim.x / mag_magnitude) * 45.0f;
    local_mag_sim.y = (local_mag_sim.y / mag_magnitude) * 45.0f;
    local_mag_sim.z = (local_mag_sim.z / mag_magnitude) * 45.0f;

    Vector3_t mag;
    mag.x = add_noise(local_mag_sim.x, 0.5f);
    mag.y = add_noise(local_mag_sim.y, 0.5f);
    mag.z = add_noise(local_mag_sim.z, 0.5f);
    return mag;
}

Vector3_t SIM_ReadGyroscope(void) {
    Vector3_t gyro;
    // Returns the "True" velocity plus some sensor bias/noise
    gyro.x = add_noise(true_angular_velocity.x, 0.01f);
    gyro.y = add_noise(true_angular_velocity.y, 0.01f);
    gyro.z = add_noise(true_angular_velocity.z, 0.01f);
    return gyro;
}

void SIM_SetTrueAngularVelocity(float x, float y, float z) {
    true_angular_velocity.x = x;
    true_angular_velocity.y = y;
    true_angular_velocity.z = z;
}

Quaternion_t SIM_ReadEstimatedAttitude(void) {
    return true_attitude;
}

void SIM_SetTrueAttitude(float q0, float q1, float q2, float q3) {
    true_attitude.q0 = q0;
    true_attitude.q1 = q1;
    true_attitude.q2 = q2;
    true_attitude.q3 = q3;
}


// This function simulates the physical interaction in space

void SIM_UpdatePhysics(Vector3_t dipole_cmd, float dt) {
    // Tuning constant: How much torque the magnetorquers actually exert.
    // In space, this is tiny. For our sim, we use a value that makes 
    // it visible in the console.
    float effectiveness = 0.2f; 
    g_altitude_km -= 0.005f;

    // PHYSICS: Torque fights angular velocity.
    // We subtract the command (multiplied by effectiveness) from the true velocity.
    true_angular_velocity.x -= dipole_cmd.x * effectiveness;
    true_angular_velocity.y -= dipole_cmd.y * effectiveness;
    true_angular_velocity.z -= dipole_cmd.z * effectiveness;

    // SPACE DAMPING: A tiny bit of "friction" to help the simulation settle.
    // 0.95 means it loses 5% of its speed every 100ms naturally.
    true_angular_velocity.x *= 0.95f;
    true_angular_velocity.y *= 0.95f;
    true_angular_velocity.z *= 0.95f;
}


float SIM_GetAltitude(void) {
    return g_altitude_km;
}


void SIM_ApplyBoost(float km_boost) {
    if (g_fuel_level > 0.0f) {
        // 1. Move the satellite up
        g_altitude_km += km_boost;

        // 2. Consume fuel 
        // Example: 10km boost costs 1.5% fuel
        float fuel_cost = (km_boost / 10.0f) * 1.5f; 
        g_fuel_level -= fuel_cost;

        // Safety check
        if (g_fuel_level < 0.0f) g_fuel_level = 0.0f;

        printf("PROPULSION: Burn successful. Fuel Remaining: %.1f%%\n", g_fuel_level);
    } else {
        printf("PROPULSION: [REJECTED] Tank empty. Altitude cannot be raised!\n");
    }
}

float ADCS_GetFuelLevel(void) {
    return g_fuel_level;
}