#ifndef FDIR_SERVICE_H
#define FDIR_SERVICE_H

#include "satellite_types.h" // For SystemMode_t
#include <stdint.h>
#include <stdbool.h>



// Defined Fault Models

typedef enum{
    FAULT_INFO,
    FAULT_WARNING,
    FAULT_CRITICAL,
    FAULT_FATAL
} FaultSeverity_t;

typedef enum {
    SRC_PAYLOAD,
    SRC_EPS,
    SRC_COMMS,
    SRC_ARCHIVE,
    SRC_CDH,
    SRC_ADCS,
    SRC_COUNT
} FaultSource_t;

typedef struct {
    FaultSource_t source;
    FaultSeverity_t severity;
    uint16_t fault_code;
    uint32_t timestamp;
} FaultReport_t;


// FDIR Public API
void FDIR_Init(void);
void FDIR_ReportFault(FaultReport_t report);
void FDIR_Process(void);    // The "Engine" called in the main loop
SystemMode_t FDIR_GetCurrentMode(void);
bool FDIR_HasActiveFaults(void);
void FDIR_SetSystemMode(SystemMode_t new_mode);

void FDIR_ProcessCommand(const uint8_t* payload, uint16_t len);

#endif