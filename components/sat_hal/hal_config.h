// include/hal_config.h

#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H

#include <stdint.h>

#define MAX_VALID_REG_ADDR (255) // Max valid address is 255
#define TOTAL_MOCK_REGS    (MAX_VALID_REG_ADDR + 1) // 256

// --- GPIO Pin Definitions ---
// Define pins using standard 32-bit pin numbering for simplicity
#define PIN_LED_RED         (13)
#define PIN_SUBSYSTEM_PWR   (14) // Used by FDIR/EPS to cut power
#define PIN_I2C_SDA         (21)
#define PIN_I2C_SCL         (22)

// --- Register Definitions ---
// Using 8-bit addresses for mock register access
#define MOCK_REG_GPIO_OUTPUT    (0x04)
#define MOCK_REG_GPIO_INPUT     (0x05)

// I2C Command Register (Used for writing slave address/command)
#define MOCK_REG_I2C_CMD        (0x06) 
// I2C Data Register (Used for storing mock read data or writing data)
#define MOCK_REG_I2C_DATA       (0x07) 
// I2C Status Register (Used for simulating success/failure)
#define MOCK_REG_I2C_STATUS     (0x08) 

// Status Flags for MOCK_REG_I2C_STATUS
#define I2C_STATUS_SUCCESS      (0x0100)
#define I2C_STATUS_NACK         (0x0200) // Simulate no acknowledgment

// --- UART Register Definitions ---
// Simulates the physical data register where the CPU writes data to be sent
#define MOCK_REG_UART_TX_DATA   (0x09) 
// Simulates the physical data register where the CPU reads received data
#define MOCK_REG_UART_RX_DATA   (0x0A) 
// Simulates the status register (used to check if data is available)
#define MOCK_REG_UART_STATUS    (0x0B) 

// Status Flags for MOCK_REG_UART_STATUS
#define UART_STATUS_RX_READY    (0x01) // Data is ready to be read
#define UART_STATUS_TX_BUSY     (0x02) // Transmitter is currently busy
#define UART_STATUS_OVERRUN     (0x04) // Simulated error flag

// --- SPI Register Definitions ---
// Simulates the data buffer for outgoing data
#define MOCK_REG_SPI_TX_DATA    (0x0C) 
// Simulates the data buffer for incoming data
#define MOCK_REG_SPI_RX_DATA    (0x0D) 
// Simulates the control register (often used to initiate transfer or select slave)
#define MOCK_REG_SPI_CONTROL    (0x0E) 

// Status Flags for MOCK_REG_SPI_CONTROL
// Use Bit 0, 1, 2 for SPI flags
#define SPI_CONTROL_TRANSFER_START (0x01) // Flag to initiate transfer
#define SPI_CONTROL_BUSY           (0x02) // Flag indicating transfer in progress
#define SPI_CONTROL_SLAVE_SELECT   (0x04) // Flag to enable a specific slave device

#endif // HAL_CONFIG_H