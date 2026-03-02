#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <stdio.h>
#include "eps_telemetry.h"
#include "eps_commands.h"
#include "fdir_service.h"
#include "time_service.h"




eps_mode_t current_mode = EPS_MODE_NOMINAL;

static float last_solar_power = 0.0f;
static int recovery_boost_timer = 0;


void vEPS_ExecuteLoadShedding(void);
void vEPS_RestoreNominalLoads(void);
float EPS_CalculateSoC(float voltage);


void vEPS_Manager_Task(void *pvParameters){
    while (1) {
        // I need to access the shared telemetry data (g_eps_telemetry)
        if (xSemaphoreTake(xEPSDataMutex, portMAX_DELAY) == pdTRUE) {
            
            EPS_Monitor(g_eps_telemetry.f_BusVoltage); // Check for faults and report to FDIR
            
            float voltage = g_eps_telemetry.f_BusVoltage;
            g_eps_telemetry.f_StateOfCharge = EPS_CalculateSoC(voltage); // Update SOC based on voltage

            // --- Post-Eclipse Detection ---
            float current_solar = g_eps_telemetry.f_SolarInputPower; // Assuming this is in your struct

            if (current_solar > 0.5f && last_solar_power <= 0.1f) {
                // We just hit the sun!
                recovery_boost_timer = 50; // Boost for 50 cycles (approx 5 seconds)
                printf("MANAGER: Sunrise detected! Initializing High-Rate Charge Recovery.\n");
            }
            last_solar_power = current_solar;

            if (recovery_boost_timer > 0) {
                // Logic: In a real satellite, you might change the charging profile here.
                // For your simulation, we'll tell the FDIR or Sim to "increase efficiency"
                g_eps_telemetry.f_SolarInputPower *= 1.8f; // 80% boost during recovery
                recovery_boost_timer--;
            }

            // --- CORE STATE MACHINE LOGIC ---
            switch (current_mode) {

                case EPS_MODE_NOMINAL:
                    // Check for fault condition: LOW VOLTAGE
                    if (g_eps_telemetry.f_BusVoltage < EPS_VOLTAGE_CRITICAL_LOW) {
                        current_mode = EPS_MODE_LOW_VOLTAGE;
                        printf("MANAGER: CRITICAL! Voltage below %.1fV. Entering LOW_VOLTAGE Mode.\n", EPS_VOLTAGE_CRITICAL_LOW);

                        FaultReport_t low_v_fault = {
                            .source = SRC_EPS,
                            .severity = FAULT_CRITICAL,
                            .fault_code = FAULT_EPS_LOW_VOLTAGE,
                            .timestamp = TIME_GetSeconds()
                        };
                        FDIR_ReportFault(low_v_fault);
                    }
                    break;

                case EPS_MODE_LOW_VOLTAGE:
                    // Action: Execute the safety procedure
                    vEPS_ExecuteLoadShedding(); // This will shed loads via bit masking
                    current_mode = EPS_MODE_SAFE;
                    g_eps_telemetry.xFaultFlags.safe_mode_active = 1; // Set the bit field flag
                    break;
                
                case EPS_MODE_SAFE:
                    // Check for recovery condition: Has the voltage improved?
                    if (g_eps_telemetry.f_BusVoltage >= EPS_VOLTAGE_RECOVERY) {
                        vEPS_RestoreNominalLoads();
                        current_mode = EPS_MODE_RECOVERY;
                        g_eps_telemetry.xFaultFlags.safe_mode_active = 0; // Clear the flag
                        printf("MANAGER: Recovery voltage reached. Exiting SAFE Mode.\n");
                    }
                    break;

                case EPS_MODE_RECOVERY:
                    // If the system has stabilized, return to normal operation
                    current_mode = EPS_MODE_NOMINAL;
                    printf("MANAGER: Stability confirmed. Returning to NOMINAL Mode.\n");
                    break;
            }

            // Release the Mutex
            xSemaphoreGive(xEPSDataMutex);
        }

        // The Manager Task runs frequently to respond fast to faults.
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}


void vEPS_ExecuteLoadShedding(void) {
    // 1. Clear (turn OFF) the non-essential loads using Bit Masking.
    // The mask flips the bits we want to clear (1s become 0s) and keeps the rest (0s become 1s).
    // We only clear the PAYLOAD and HEATER bits.

    g_eps_telemetry.ul_PDU_Register &= ~(PDU_LOAD_PAYLOAD_BIT | PDU_LOAD_HEATER_BIT);

    // 2. Verification Print (showing the PDU register's new value)
    printf("MANAGER: --- Load Shedding Executed ---\n");
    printf("MANAGER: PDU State (HEX) : 0x%08lX (Only COMMS should be ON)\n", g_eps_telemetry.ul_PDU_Register);
}

void vEPS_RestoreNominalLoads(void) {
    // 1. Set (turn ON) all non-essential loads using Bit Masking.
    // We use the OR operator to set the PAYLOAD and HEATER bits high.

    g_eps_telemetry.ul_PDU_Register |= (PDU_LOAD_PAYLOAD_BIT | PDU_LOAD_HEATER_BIT);

    // 2. Verification Print (showing the PDU register's new value)
    printf("MANAGER: --- Nominal Loads Restored ---\n");
    printf("MANAGER: PDU State (HEX) : 0x%08lX (All three loads should be ON)\n", g_eps_telemetry.ul_PDU_Register);

}


// This allows other tasks to check if we are in SAFE mode or NOMINAL
eps_mode_t EPS_GetMode(void) {
    return current_mode;
}

float EPS_CalculateSoC(float voltage) {
    const float MAX_V = 8.4f;
    const float MIN_V = 6.0f;
    
    if (voltage >= MAX_V) return 100.0f;
    if (voltage <= MIN_V) return 0.0f;
    
    // Convert Voltage to 0-100% range
    float soc = ((voltage - MIN_V) / (MAX_V - MIN_V)) * 100.0f;
    return soc;
}