// Ali Akbari 30171539
// CPSC 359 Assignment 3 

#include "gpio.h"
#include "uart.h"
#include "gic.h"
#include "sysreg.h"

/* GPIO Pin Assignments */
#define BTN_A 0
#define BTN_B 1
#define LED_GREEN 16
#define LED_YELLOW 12
#define LED_RED 4

/* State Identifiers */
#define SLOW_MODE 0   // Slow LED sequence
#define FAST_MODE 1   // Fast LED sequence

/* GPIO Interrupt ID */
#define GPIO_IRQ_ID 96

/* Shared State Variable (Used in ISR and Main) */
volatile unsigned int currentState;

/* Function Prototypes */
// GPIO Functions
void activate_LED(unsigned int pin);
void deactivate_LED(unsigned int pin);
void configure_GPIO_as_output(unsigned int pin);

// Interrupt Configuration
void configure_interrupt_controller(void);
void handle_interrupt(void);
void setup_GPIO0_interrupt(void);
void setup_GPIO1_interrupt(void);

// Helper Functions
unsigned int read_GPIO0_state(void);
unsigned int read_GPIO1_state(void);
void wait_cycles(unsigned int cycles);

// LED Sequences
void run_slow_sequence(void);
void run_fast_sequence(void);

/* Main Program */
void main()
{
    unsigned int localState;

    // Initialize GPIO Pins and State
    localState = currentState = SLOW_MODE;
    configure_GPIO_as_output(LED_GREEN);
    configure_GPIO_as_output(LED_YELLOW);
    configure_GPIO_as_output(LED_RED);

    // Setup GPIO Interrupts
    setup_GPIO0_interrupt();
    setup_GPIO1_interrupt();
    configure_interrupt_controller();

    // Turn off all LEDs initially
    deactivate_LED(LED_GREEN);
    deactivate_LED(LED_YELLOW);
    deactivate_LED(LED_RED);

    while (1)
    {
        // Detect state changes
        if (localState != currentState)
        {
            // Turn off all LEDs and update state
            deactivate_LED(LED_GREEN);
            deactivate_LED(LED_YELLOW);
            deactivate_LED(LED_RED);
            localState = currentState;
        }

        // Execute LED sequence based on the state
        if (localState == SLOW_MODE)
        {
            // Execute the slow LED blinking sequence
            run_slow_sequence();
        }
        else if (localState == FAST_MODE)
        {
            // Execute the fast LED blinking sequence
            run_fast_sequence();
        }
    }
}

/* Interrupt Service Routine */
void IRQ_handler()
{
    unsigned int ack = *GIC_GICC_IAR; // Acknowledge interrupt
    unsigned int irqID = ack & 0x3FF; // Extract the interrupt ID

    // Check if the interrupt was triggered by GPIO
    if (irqID == GPIO_IRQ_ID)
    {
        // Read the state of both buttons
        unsigned int buttonA_state = read_GPIO0_state();
        unsigned int buttonB_state = read_GPIO1_state();

        // Update shared state based on button inputs
        if (buttonA_state == 1)
        {
            currentState = SLOW_MODE;
        }
        else if (buttonB_state == 0)
        {
            currentState = FAST_MODE;
        }

        // Clear interrupt flags for both buttons
        *GPEDS0 = (1 << BTN_A) | (1 << BTN_B); // Clear interrupt flags
    }

    // Signal end of interrupt
    *GIC_GICC_EOIR = ack; // End interrupt
}

/* GPIO Interrupt Configurations */
void setup_GPIO0_interrupt()
{
    unsigned int reg;

    // Set Button A (GPIO 0) as input with pull-down
    reg = *GPFSEL0;
    reg &= ~(0x7 << 0); // Clear function bits for GPIO 0
    *GPFSEL0 = reg;

    reg = *GPPUPPDN0;
    reg &= ~(0x3 << (BTN_A * 2)); // Clear pull settings
    reg |= (0x2 << (BTN_A * 2));  // Enable pull-down
    *GPPUPPDN0 = reg;

    *GPREN0 |= (1 << BTN_A); // Enable rising-edge detection for GPIO 0
    *GPEDS0 = (1 << BTN_A);  // Clear any pending events
}

void setup_GPIO1_interrupt()
{
    unsigned int reg;

    // Set Button B (GPIO 1) as input with pull-up
    reg = *GPFSEL0;
    reg &= ~(0x7 << 3); // Clear function bits for GPIO 1
    *GPFSEL0 = reg;

    reg = *GPPUPPDN0;
    reg &= ~(0x3 << (BTN_B * 2)); // Clear pull settings
    reg |= (0x1 << (BTN_B * 2));  // Enable pull-up
    *GPPUPPDN0 = reg;

    *GPFEN0 |= (1 << BTN_B); // Enable falling-edge detection for GPIO 1
    *GPEDS0 = (1 << BTN_B);  // Clear any pending events
}

/* Interrupt Controller Setup */
void configure_interrupt_controller()
{
    // Disable the controller
    *GIC_GICD_CTLR = 0x0;

    // Clear and set interrupt settings
    *(GIC_GICD_ICENABLER + (GPIO_IRQ_ID / 32)) = (1 << (GPIO_IRQ_ID % 32));
    *((volatile unsigned char *)GIC_GICD_IPRIORITYR + GPIO_IRQ_ID) = 0xA0;
    *((volatile unsigned char *)GIC_GICD_ITARGETSR + GPIO_IRQ_ID) = 0x1;

    // Calculate the index and shift for the interrupt configuration register
    unsigned int cfgIndex = GPIO_IRQ_ID / 16;
    unsigned int cfgShift = (GPIO_IRQ_ID % 16) * 2;
    unsigned int cfgValue = *(GIC_GICD_ICFGR + cfgIndex);
    cfgValue &= ~(0x3 << cfgShift);
    cfgValue |= (0x2 << cfgShift);
    *(GIC_GICD_ICFGR + cfgIndex) = cfgValue;

    // Enable the interrupt
    *(GIC_GICD_ISENABLER + (GPIO_IRQ_ID / 32)) = (1 << (GPIO_IRQ_ID % 32));
    *GIC_GICD_CTLR = 0x1;
    *GIC_GICC_PMR = 0xFF;
    *GIC_GICC_CTLR = 0x1;

    enableIRQ(); // Enable CPU IRQs
}

/* GPIO Helper Functions */
void configure_GPIO_as_output(unsigned int pin)
{
    // Configure a GPIO pin as output by setting its function bits
    volatile unsigned int *gpio_reg = GPFSEL0 + (pin / 10);
    unsigned int shift = (pin % 10) * 3;

    *gpio_reg &= ~(0x7 << shift);
    *gpio_reg |= (0x1 << shift);
}

// Turn on an LED
void activate_LED(unsigned int pin)
{
    *GPSET0 = (1 << pin);
}

// Turn off an LED
void deactivate_LED(unsigned int pin)
{
    *GPCLR0 = (1 << pin);
}

// Read the state of Button A
unsigned int read_GPIO0_state()
{
    return (*GPLEV0 & 0x1);
}

// Read the state of Button B
unsigned int read_GPIO1_state()
{
    return ((*GPLEV0 >> 1) & 0x1);
}

/* Delay Function */
void wait_cycles(unsigned int cycles)
{
    for (unsigned int i = 0; i < cycles; i++)
    {
        asm volatile("nop");
    }
}

/* LED Sequences */
void run_slow_sequence()
{
    activate_LED(LED_GREEN);
    wait_cycles(5000000); // ~0.5 seconds
    deactivate_LED(LED_GREEN);

    activate_LED(LED_YELLOW);
    wait_cycles(5000000);
    deactivate_LED(LED_YELLOW);

    activate_LED(LED_RED);
    wait_cycles(5000000);
    deactivate_LED(LED_RED);
}

void run_fast_sequence()
{
    activate_LED(LED_RED);
    wait_cycles(2500000); // ~0.25 seconds
    deactivate_LED(LED_RED);

    activate_LED(LED_YELLOW);
    wait_cycles(2500000);
    deactivate_LED(LED_YELLOW);

    activate_LED(LED_GREEN);
    wait_cycles(2500000);
    deactivate_LED(LED_GREEN);
}
