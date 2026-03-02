// src/eps_control.c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "eps_control.h"
#include "state_manager.h"
#include "watchdog.h"
#include "eps_telemetry.h"
#include "eps_commands.h"
#include "fdir_service.h"
#include "time_service.h"
#include <stdio.h>

#define CRITICAL_BUS_VOLTAGE 2.5f

float Get_Battery_Voltage(void);

void vEPSMonitoringTask(void *pvParameters) {
    printf("CDH EPS Monitor: Now linked to EPS Subsystem Telemetry.\n");

    for(;;) {
        // 1. GET REAL DATA from the EPS Subsystem (not a local variable)
        if(xSemaphoreTake(xEPSDataMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            float real_voltage = g_eps_telemetry.f_BusVoltage;
            xSemaphoreGive(xEPSDataMutex);
            SystemMode_t current_mode = get_system_mode();

            // 2. GLOBAL FDIR LOGIC
            // If the EPS Subsystem is struggling, CDH triggers the Global Critical Mode
            if(real_voltage < CRITICAL_BUS_VOLTAGE && current_mode != MODE_CRITICAL) {
                
                // --- NEW: REPORT TO THE SYSTEM-WIDE LOG ---
                FaultReport_t global_pwr_fault = {
                    .source = SRC_EPS,
                    .severity = FAULT_CRITICAL,
                    .fault_code = FAULT_EPS_LOW_VOLTAGE,
                    .timestamp = TIME_GetSeconds()
                };
                FDIR_ReportFault(global_pwr_fault);

                // 3. GLOBAL RECOVERY
                set_system_mode(MODE_CRITICAL);
                printf("CDH: Emergency! Satellite forced to MODE_CRITICAL.\n");
            }

            else if (real_voltage > 6.0f && current_mode == MODE_CRITICAL) {
                set_system_mode(MODE_NOMINAL);
                printf("CDH: Power Recovery detected (%.2fV). Transitioning to MODE_NOMINAL.\n", real_voltage);
            }
        }

        watchdog_pet(WDT_TASK_EPS_MON);
        
        // Check once per second (10s was way too slow for safety!)
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}


void vEPS_SetSafeModePower(int mode_id) {
    printf("STUB CALLED: EPS received power command (Mode ID: %d).\n", mode_id);
}

static float simulated_v = 8.0f; // Start healthy
float Get_Battery_Voltage(void) {
    return simulated_v; 
}