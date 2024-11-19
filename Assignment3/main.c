// Ali Akbari 30171539
// CPSC 359 Assignment 3

#include "gpio.h"
#include "uart.h"
#include "gic.h"
#include "sysreg.h"

// Global variables to store the current state
volatile int state = 1; // Start in State 1

// Function Prototypes
void init_GPIO(void);
void configure_interrupts(void);
void delay(unsigned int cycles);
void state1_logic(void);
void state2_logic(void);

// Interrupt Service Routines (ISRs)
void button_A_ISR(void);
void button_B_ISR(void);

// Exception handler for IRQs
void irq_handler(void);

void main() {
    uart_init();         // Initialize UART
    init_GPIO();         // Initialize GPIO pins
    configure_interrupts(); // Set up interrupts

    // Enable IRQ Exceptions on the CPU
    enableIRQ();

    while (1) {
        if (state == 1) {
            state1_logic(); // Execute State 1 logic
        } else {
            state2_logic(); // Execute State 2 logic
        }
    }
}

// GPIO Initialization
void init_GPIO() {
    init_GPIO16_to_output(); // Green LED
    init_GPIO12_to_output(); // Yellow LED
    init_GPIO4_to_output();  // Red LED

    init_GPIO0_to_input();   // Button A
    init_GPIO1_to_input();   // Button B
}

// Configure Interrupts
void configure_interrupts() {
    // Enable rising-edge interrupt for GPIO 0 (Button A)
    *GPREN0 |= (1 << 0);

    // Enable falling-edge interrupt for GPIO 1 (Button B)
    *GPFEN0 |= (1 << 1);

    // Enable GPIO interrupts in the GIC
    *(GIC_GICD_ISENABLER + (1 * 4)) = 0x00020000; // Enable Bank 0 GPIO interrupts

    // Enable forwarding of Group 1 interrupts
    *GIC_GICD_CTLR = 0x1;
}

// Busy-loop delay
void delay(unsigned int cycles) {
    for (volatile unsigned int i = 0; i < cycles; i++) {
        __asm__ volatile("nop");
    }
}

// LED Control Logic for State 1
void state1_logic() {
    set_GPIO16(); // Turn on Green LED
    delay(500000); // Delay ~0.5 seconds
    clear_GPIO16();

    set_GPIO12(); // Turn on Yellow LED
    delay(500000); // Delay ~0.5 seconds
    clear_GPIO12();

    set_GPIO4(); // Turn on Red LED
    delay(500000); // Delay ~0.5 seconds
    clear_GPIO4();
}

// LED Control Logic for State 2
void state2_logic() {
    set_GPIO4(); // Turn on Red LED
    delay(250000); // Delay ~0.25 seconds
    clear_GPIO4();

    set_GPIO12(); // Turn on Yellow LED
    delay(250000); // Delay ~0.25 seconds
    clear_GPIO12();

    set_GPIO16(); // Turn on Green LED
    delay(250000); // Delay ~0.25 seconds
    clear_GPIO16();
}

// ISR for Button A (State 1)
void button_A_ISR() {
    state = 1;
    *GPEDS0 = (1 << 0); // Clear interrupt flag for GPIO 0
}

// ISR for Button B (State 2)
void button_B_ISR() {
    state = 2;
    *GPEDS0 = (1 << 1); // Clear interrupt flag for GPIO 1
}

// GPIO interrupt handler (called by the exception vector)
void irq_handler() {
    // Check for interrupt on GPIO 0 (Button A)
    if (*GPEDS0 & (1 << 0)) {
        button_A_ISR();
    }
    // Check for interrupt on GPIO 1 (Button B)
    if (*GPEDS0 & (1 << 1)) {
        button_B_ISR();
    }
}


void init_GPIO16_to_output() {
    unsigned int r = *GPFSEL1;       // Read GPIO Function Select Register 1
    r &= ~(0x7 << 18);               // Clear FSEL16 bits (GPIO16)
    r |= (0x1 << 18);                // Set FSEL16 to output mode (0b001)
    *GPFSEL1 = r;                    // Write back to the register
}

void init_GPIO12_to_output() {
    unsigned int r = *GPFSEL1;       // Read GPIO Function Select Register 1
    r &= ~(0x7 << 6);                // Clear FSEL12 bits (GPIO12)
    r |= (0x1 << 6);                 // Set FSEL12 to output mode (0b001)
    *GPFSEL1 = r;                    // Write back to the register
}

void init_GPIO4_to_output() {
    unsigned int r = *GPFSEL0;       // Read GPIO Function Select Register 0
    r &= ~(0x7 << 12);               // Clear FSEL4 bits (GPIO4)
    r |= (0x1 << 12);                // Set FSEL4 to output mode (0b001)
    *GPFSEL0 = r;                    // Write back to the register
}

void init_GPIO0_to_input() {
    unsigned int r = *GPFSEL0;       // Read GPIO Function Select Register 0
    r &= ~(0x7 << 0);                // Clear FSEL0 bits (GPIO0)
    *GPFSEL0 = r;                    // Set GPIO0 to input mode (0b000)
}

void init_GPIO1_to_input() {
    unsigned int r = *GPFSEL0;       // Read GPIO Function Select Register 0
    r &= ~(0x7 << 3);                // Clear FSEL1 bits (GPIO1)
    *GPFSEL0 = r;                    // Set GPIO1 to input mode (0b000)
}


void set_GPIO16() {
    *GPSET0 = (1 << 16);             // Set GPIO16 high
}

void clear_GPIO16() {
    *GPCLR0 = (1 << 16);             // Set GPIO16 low
}

void set_GPIO12() {
    *GPSET0 = (1 << 12);             // Set GPIO12 high
}

void clear_GPIO12() {
    *GPCLR0 = (1 << 12);             // Set GPIO12 low
}

void set_GPIO4() {
    *GPSET0 = (1 << 4);              // Set GPIO4 high
}

void clear_GPIO4() {
    *GPCLR0 = (1 << 4);              // Set GPIO4 low
}
    