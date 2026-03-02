#ifndef ADCS_SERVICE_H
#define ADCS_SERVICE_H

#include "adcs_types.h"
#include "fdir_service.h"  // To report faults to FDIR

// ADCS Specific Error Codes
#define ADCS_ERR_NONE             0x00
#define ADCS_ERR_SATURATION       0xA1  // Sensor/Actuator at physical limit
#define ADCS_ERR_DIVERGENCE       0xA2  // Estimation math is failing
#define ADCS_ERR_TIMEOUT          0xA3  // Sensor stopped responding

#define ADCS_CMD_SET_MODE 0x01


// ADCS Public API
void ADCS_Init(void);
void ADCS_SetMode(ADCSMode_t mode);
void ADCS_Process(void);  //Main periodic task
ADCSMode_t ADCS_GetMode(void);

// For Telemetry
typedef struct {
    ADCSMode_t mode;
    Vector3_t angular_velocity;
    Quaternion_t attitude;
    Vector3_t last_torque;
    bool actuators_enabled;
} ADCSTelemetry_t;


void ADCS_GetTelemetry(ADCSTelemetry_t* telemetry);

void ADCS_ProcessCommand(const uint8_t* payload, uint16_t len);

Vector3_t ADCS_GetLastCommand(void);
#endif