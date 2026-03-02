// src/adcs/adcs_manager.c

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "adcs_service.h" // Access to ADCS_Init and ADCS_Process
#include "adcs_sim.h"     // To set an initial "Tumble" for testing
#include "satellite_types.h" // For SystemMode_t and other types
#include "watchdog.h"     // To pet the watchdog for this task
#include "adcs_types.h" 
#include "eps_telemetry.h" // To check EPS status for situational events
#include <stdio.h>

void vADCS_Manager_Task(void *pvParameters) {
    // 1. Initialize the internal ADCS state
    ADCS_Init(); 

    // 2. Setup a "Tumble" so we can watch the B-Dot work in the console
    // Setting spin to 10 deg/s on X and -5 deg/s on Y
    SIM_SetTrueAngularVelocity(10.0f, -5.0f, 0.5f);

    printf("ADCS TASK: Started. Initial spin applied for simulation.\n");

    for(;;) {
        // 3. RUN THE ENGINE
        // This calls the math, reads sim sensors, and runs B-Dot/PD
        ADCS_Process();

        float current_alt = SIM_GetAltitude();

        if (current_alt < 500.0f) {
            // This is the "Orbit Maintenance" situational event
            printf("ADCS_TASK: Low Altitude Detected (%.2f km)!\n", current_alt);
            
            // Get the status of both systems
            ADCSMode_t adcs_st = ADCS_GetMode();
            eps_mode_t eps_st = EPS_GetMode();

            float alt_before = current_alt; // Record starting point
            // We only trigger the boost if the satellite is stable
            if (adcs_st == ADCS_MODE_NOMINAL && eps_st == EPS_MODE_NOMINAL) {
                SIM_ApplyBoost(50.0f);
                printf("ADCS_TASK: Maintenance Complete. Altitude Boosted.\n");
                // MANEUVER SUMMARY PRINT
                printf("\n============================================\n");
                printf("🚀 ORBIT MANEUVER EXECUTED\n");
                printf("Pre-Burn Alt:  %.2f km\n", alt_before);
                printf("Post-Burn Alt: %.2f km\n", SIM_GetAltitude());
                printf("Status:        ORBIT RAISED SUCCESSFULLY\n");
                printf("============================================\n\n");
            } else {
                // Log specifically why we didn't boost
                if (adcs_st != ADCS_MODE_NOMINAL) {
                    printf("ADCS_TASK: [REJECTED] Satellite is TUMBLING. Boost inhibited.\n");
                }
                if (eps_st != EPS_MODE_NOMINAL) {
                    printf("ADCS_TASK: [REJECTED] EPS in SAFE MODE. Power too low for boost.\n");
                }
            }
        }
        // Every few seconds, report the altitude
        static int orbit_report_timer = 0;
        if (orbit_report_timer++ >= 50) { 
            printf(">>> ORBIT: Altitude: %.2f km | Status: %s <<<\n", 
                    current_alt, 
                    (current_alt < 500.0f) ? "CRITICAL (Low Orbit)" : "NOMINAL");
            
            // FDIR: If we get too low, maybe force a mode change to conserve power
            if (current_alt < 480.0f) {
                printf("FDIR: ALTITUDE EMERGENCY! Prepare for re-entry.\n");
            }
            orbit_report_timer = 0;
        }

        // 4. GET THE CMD: We need the dipole command just calculated by B-Dot
        // (Assuming ADCS_GetLastCommand() returns a Vector3_t of your magnetorquer output)
        Vector3_t current_cmd = ADCS_GetLastCommand();

        // 5. APPLY PHYSICS: Feed the command back into the simulation
        // 0.1f is our DT (10Hz)
        SIM_UpdatePhysics(current_cmd, 0.1f);

        watchdog_pet(WDT_TASK_ADCS);

        // 6. Update frequency: 10Hz (100ms) matches DT = 0.1f definition
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}