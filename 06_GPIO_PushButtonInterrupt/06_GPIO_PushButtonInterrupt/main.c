// This program sets up GPIO pin 1 as an input pin, and configures it to
// trigger an interrupt whenever a falling edge is detected. GPIO pin 1 is 
// connected to Switch B on the Traffic Light board, which should be connected
// to the Pi using Port B on the Pi Hat. Switch B should be set so that it is
// pulled high.


// Include files
#include "uart.h"
#include "sysreg.h"
#include "gpio.h"
#include "gic.h"


// Function prototypes
void init_GPIO1_to_fallingEdgeInterrupt();

// Declare a global shared variable
unsigned int sharedValue;



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function first prints out the values of some system
//                  registers for diagnostic purposes. It then initializes GPIO
//                  pin 1 to be an input pin that generates an interrupt (IRQ
//                  exception) whenever a falling edge occurs on the pin. The
//                  function then goes into an infinite loop, where the shared
//                  global variable is continually checked. If the interrupt
//                  service routine changes the shared variable, then this is
//                  detected in the loop, and the current value is printed out.
//
////////////////////////////////////////////////////////////////////////////////

void main()
{
    unsigned int localValue, value, el, i;

    
    // Set up the UART serial port
    uart_init();

    // Print out initial values before setting GIC, etc.
    uart_puts("Initial Values:\n");
    
    // Query the current exception level
    el = getCurrentEL();
    
    // Print out the exception level
    uart_puts("  Exception level:    0x");
    uart_puthex(el);
    uart_puts("\n");
    
    // Get the SPSel value
    value = getSPSel();
    
    // Print out the SPSel value
    uart_puts("  SPSel:              0x");
    uart_puthex(value);
    uart_puts("\n");
        
    // Query the current DAIF flag values
    value = getDAIF();
    
    // Print out the DAIF flag values
    uart_puts("  DAIF flags:         0x");
    uart_puthex(value);
    uart_puts("\n");

    // Print out initial values the GPREN0 register (rising-edge interrupt
    // enable register)
    value = *GPREN0;
    uart_puts("  GPREN0:             0x");
    uart_puthex(value);
    uart_puts("\n");
    
    // Print out initial values the GPFEN0 register (falling-edge interrupt
    // enable register)
    value = *GPFEN0;
    uart_puts("  GPFEN0:             0x");
    uart_puthex(value);
    uart_puts("\n");
    

    // Initialize the sharedValue global variable and and set the local variable
    // to be same value
    localValue = sharedValue = 0;
    

    // Set and print out new values
    uart_puts("\nResetting to new values:\n");
    
    // Set up GPIO pin 1 to input and so that it triggers an interrupt when a
    // falling edge is detected
    init_GPIO1_to_fallingEdgeInterrupt();
    
    // Enable IRQ Exceptions on the CPU core
    enableIRQ();
    
    // Query the current DAIF flag values
    value = getDAIF();
    
    // Print out the DAIF flag values
    uart_puts("  DAIF flags:         0x");
    uart_puthex(value);
    uart_puts("\n");

    // Print out new value of the GPREN0 register (rising-edge interrupt enable
    // register)
    uart_puts("  GPREN0:             0x");
    value = *GPREN0;
    uart_puthex(value);
    uart_puts("\n");
    
    // Print out new value of the GPFEN0 register (falling-edge interrupt enable
    // register)
    uart_puts("  GPFEN0:             0x");
    value = *GPFEN0;
    uart_puthex(value);
    uart_puts("\n\n");

    
    // Set up the Generic Interrupt Controller

    // First set all interrupts to the highest priority (0x00). Each register
    // holds the priorities for 4 interrupts each, so we are setting 64
    // interrupts in total (numbered 0 - 63).
    for (i = 0; i < 16; i++) {
		*(GIC_GICD_IPRIORITYR + (i * 4)) = 0x00000000;
    }

    // Set supported SPIs targets to CPU 0 only (0x01, or 0b00000001). Each
    // register holds the targets for 4 interrupts each, so we are setting 32
    // interrupts in total (numbered 32 - 63).
    for (i = 8; i < 16; i++) {
		*(GIC_GICD_ITARGETSR + (i * 4)) = 0x01010101;
    }

    // Set PPIs and supported SPIs to be edge-triggered and using the 1-N model
    // (0b11). Each register holds the configuration for 16 interrupts each, so
    // we are are setting 48 interrupts in total (numbered 16 - 63). Interrupts
    // 0 - 15 (SGIs) are read-only (default to edge-triggered, N-N model).
    for (i = 1; i < 4; i++) {
		*(GIC_GICD_ICFGR + (i * 4)) = 0xFFFFFFFF;
    }

    // Enable Bank 0 GPIO interrupts in the GIC. Bit 17 in GICD_ISENABLER1 is
    // for Bank 0 (pins 0 - 27).
    uart_puts("Enabling Bank 0 GPIO interrupts (pins 0 - 27) in GIC:\n");
    *(GIC_GICD_ISENABLER + (1 * 4)) = 0x00020000;

    // Print out enabled interrupts
	value = *(GIC_GICD_ISENABLER + (0 * 4));
	uart_puts("  GICD_ISENABLER0:    0x");
	uart_puthex(value);
	uart_puts("\n");
	value = *(GIC_GICD_ISENABLER + (1 * 4));
	uart_puts("  GICD_ISENABLER1:    0x");
	uart_puthex(value);
	uart_puts("\n\n");

    // If in EL3 (secure-code), globally enable both Group 0 and Group 1
    // forwarding from the GICD to a GICC.
    if (el == 0x3) {
		*GIC_GICD_CTLR = 0x3;
		uart_puts("Enabling GIC forwarding of Group 0 and 1 interrupts.\n");
    } else {
		// Globally enable forwarding of Group 1 (non-secure) interrupts only
		// from the GICD to the GICC if in EL0, EL1, or EL2 (non-secure code).
		*GIC_GICD_CTLR = 0x1;
		uart_puts("Enabling GIC forwarding of Group 1 interrupts.\n");
    }
    value = *GIC_GICD_CTLR;
    uart_puts("  GICD_CTLR:          0x");
    uart_puthex(value);
    uart_puts("\n\n");


    
    // Print out a message to the console
    uart_puts("\nInfinite loop starting:\n");
    
    // Loop forever, waiting for interrupts to change the shared value
    while (1) {
		// Check to see if the shared value was changed by an interrupt
		if (localValue != sharedValue) {
			// Update the local variable
			localValue = sharedValue;

			// Print out the shared value
			uart_puts("\nsharedValue is:  ");
			uart_puthex(sharedValue);
			uart_puts("\n");
		}

        // Delay a little using a busy loop
        value = 0x0000FFFF;
    	while (value--) {
      	    asm volatile("nop");
    	}
    }
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO1_to_fallingEdgeInterrupt
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 1 to an input pin without any
//                  internal pull-up or pull-down resistors. Note that a pull-
//                  up resistor must be used externally on the Traffic Light
//                  board connected to the pin. Be sure that the pin high
//                  level is 3.3V (definitely NOT 5V). GPIO pin 1 is also set
//                  to trigger an interrupt on a falling edge.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO1_to_fallingEdgeInterrupt()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 0
    r = *GPFSEL0;

    // Clear bits 3 - 5. This is the field FSEL1, which maps to GPIO pin 1.
    // We clear the bits by ANDing with a 000 bit pattern in the field. This
    // sets the pin to be an input pin.
    r &= ~(0x7 << 3);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 0
    *GPFSEL0 = r;


    // Disable the pull-up/pull-down control line for GPIO pin 1

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 2-3 in this bit pattern, since this maps to GPIO pin 1.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 2);

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;

    // Set pin 1 to so that it generates an interrupt on a falling edge. We do
    // so by setting bit 1 in the GPIO Falling Edge Detect Enable Register 0 to
    // a 1 value (p. 91 in the Broadcom BCM2711 ARM Peripherals manual).
    *GPFEN0 = (0x1 << 1);
}
