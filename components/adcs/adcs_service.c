#include "adcs_service.h"
#include "adcs_sim.h"
#include "adcs_control.h"
#include "adcs_math.h"
#include"adcs_types.h"
#include "fdir_service.h"
#include "time_service.h"
#include "commands.h"
#include "state_manager.h"
#include <stdio.h>
#include <math.h>

#define MAX_SAFE_ANGULAR_VELOCITY 3000.0f  // deg/s
#define STABLE_THRESHOLD 0.25f  // rad/s (roughly 8.5 degrees per second)
static Vector3_t g_last_dipole_cmd = {0.0f, 0.0f, 0.0f};
static int log_divider = 0;


static ADCSMode_t current_mode = ADCS_MODE_SAFE;
static bool actuators_enabled = false;
static Vector3_t last_mag_reading = {0, 0, 0};
static Vector3_t current_torque = {0, 0, 0};
static const float DT = 0.1f;  // 100 ms cycle time


void ADCS_Init(void) {
    current_mode = ADCS_MODE_DETUMBLE;  // Start with stopping the spin
    actuators_enabled = true;
    printf("ADCS: Initialized in DETUMBLE mode.\n");
}

void ADCS_SetMode(ADCSMode_t mode) {
    // Valid transition check 
    if(current_mode == ADCS_MODE_SAFE && mode != ADCS_MODE_DETUMBLE){
        printf("ADCS: Rejected transition. Must detumble from SAFE mode first.\n");
        return;
    }
    current_mode = mode;
}

void ADCS_Process(void) {
    // 1. Read sensors (Inputs)
    Vector3_t gyro = SIM_ReadGyroscope();
    Vector3_t mag_now = SIM_ReadMagnetometer();

    if(current_mode == ADCS_MODE_DETUMBLE){
        // Safety Check: If angular velocity too high, go to SAFE mode
        if(fabs(gyro.x) > MAX_SAFE_ANGULAR_VELOCITY || fabs(gyro.y) > MAX_SAFE_ANGULAR_VELOCITY || fabs(gyro.z) > MAX_SAFE_ANGULAR_VELOCITY) {
            FaultReport_t adcs_fault = {
                .source = SRC_ADCS,
                .severity = FAULT_WARNING,
                .fault_code = ADCS_ERR_SATURATION,
                .timestamp = TIME_GetSeconds()
            };
            FDIR_ReportFault(adcs_fault);
            printf("ADCS: !!! SENSOR SATURATION DETECTED !!! Reporting to FDIR.\n");
            
        }
    }    
    // 2. Logic based on Mode
    switch (current_mode){
        case ADCS_MODE_DETUMBLE:
            // Check if we are stable enough to switch to Pointing
            float total_spin = sqrt(gyro.x*gyro.x + gyro.y*gyro.y + gyro.z*gyro.z);

            if(total_spin < STABLE_THRESHOLD) {
                current_mode = ADCS_MODE_NOMINAL;
                set_system_mode(MODE_NOMINAL);
                g_last_dipole_cmd = (Vector3_t){0,0,0}; 
                printf("ADCS: >>> SUCCESS <<< Magnitude: %.3f. Switching to NOMINAL.\n", total_spin);
            }
            else {
                g_last_dipole_cmd = Control_BDot(mag_now, last_mag_reading, DT);
                printf("ADCS: [Detumbling] B-Dot Command: X:%.2f Y:%.2f Z:%.2f\n", 
                        g_last_dipole_cmd.x, g_last_dipole_cmd.y, g_last_dipole_cmd.z);
            }
            break;
        
        case ADCS_MODE_NOMINAL:
            // 1. Get current orientation from virtual sensor
            Quaternion_t q_current = SIM_ReadEstimatedAttitude();
            // 2. Define where to point (target)
            Quaternion_t q_target = {1.0f, 0.0f, 0.0f, 0.0f}; // Pointing to identity
            
            // 3. Calculate error
            Quaternion_t q_curr_inv;
            Quaternion_t q_error;

            Quat_Conjugate(&q_current, &q_curr_inv);
            Quat_Multiply(&q_target, &q_curr_inv, &q_error);

            // 4. Run PD controller
            Vector3_t torque_cmd = Control_PD(q_error, gyro);

            // 5. Output debugging
            log_divider++;
            if (log_divider >= 20) { // Only print every 20 cycles (approx every 2 seconds)
                printf("ADCS: [Nominal] Stable and Pointing. Error: %.3f\n", q_error.q0);
                log_divider = 0;
            }
            current_torque = Control_PD(q_error, gyro);
            break;
            
        case ADCS_MODE_SAFE:
            actuators_enabled = false;
            break;

        default:
            break;
    }

    // 3. Update Memory for next cycle
    last_mag_reading = mag_now;
}

void ADCS_GetTelemetry(ADCSTelemetry_t* telemetry) {
    telemetry->mode = current_mode;
    telemetry->angular_velocity = SIM_ReadGyroscope();
    telemetry->attitude = SIM_ReadEstimatedAttitude();
    telemetry->last_torque = current_torque;
    telemetry->actuators_enabled = actuators_enabled;
}


// This function is CALLED by the Router when a packet for ADCS arrives
void ADCS_ProcessCommand(const uint8_t* payload, uint16_t len) {
    if (len < 1) {
        printf("ADCS ERROR: Command packet too short!\n");
        return;
    }

    uint8_t command_id = payload[0]; // The first byte is the "What to do"

    switch (command_id) {
        case ADCS_CMD_SET_MODE:
            if (len >= 2) {
                // The second byte (payload[1]) is the mode value
                ADCSMode_t requested_mode = (ADCSMode_t)payload[1]; 
                ADCS_SetMode(requested_mode); 
                
                printf("ADCS: SetMode command processed.\n");
            }
            break;

        case ADCS_CMD_RESET_ESTIMATION:
            // Logic to reset the filters/quaternions
            printf("ADCS: Orientation filters reset.\n");
            break;

        default:
            printf("ADCS: Unknown Command ID 0x%02X recieved.\n", command_id);
            break;
    }
}


Vector3_t ADCS_GetLastCommand(void) {
    // If in Nominal, return the PD torque, if in Detumble, return the B-Dot dipole
    if (current_mode == ADCS_MODE_NOMINAL) {
        return current_torque;
    }
    return g_last_dipole_cmd;
}

// Body for the ADCS Mode Getter
ADCSMode_t ADCS_GetMode(void) {
    return current_mode; 
}