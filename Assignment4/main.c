

// Include files
#include "uart.h"
#include "gpio.h"
#include "systimer.h"

// Function prototypes
unsigned short get_SNES();
void init_GPIO9_to_output();
void set_GPIO9();
void clear_GPIO9();
void init_GPIO11_to_output();
void set_GPIO11();
void clear_GPIO11();
void init_GPIO10_to_input();
unsigned int get_GPIO10();



void main()
{
    unsigned short data, currentState = 0xFFFF;
	

    // Set up the UART serial port
    uart_init();
    
    // Set up GPIO pin 9 for output (LATCH output)
    init_GPIO9_to_output();
    
    // Set up GPIO pin 11 for output (CLOCK output)
    init_GPIO11_to_output();
    
    // Set up GPIO pin 10 for input (DATA input)
    init_GPIO10_to_input();
    
    // Clear the LATCH line (GPIO 9) to low
    clear_GPIO9();
    
    // Set CLOCK line (GPIO 11) to high
    set_GPIO11();
    
       
    // Print out a message to the console
    uart_puts("SNES Controller Program starting.\n");
    
    // Loop forever, reading from the SNES controller 30 times per second
    while (1) {
    	// Read data from the SNES controller
		data = get_SNES();

		// Write out data if the state of the controller has changed
		if (data != currentState) {
			// Write the data out to the console in hexadecimal
			uart_puts("0x");
			uart_puthex(data);
			uart_puts("\n");

			// Record the state of the controller
			currentState = data;
		}
    	
		// Delay 1/30th of a second
		microsecond_delay(33333);
    }
}


unsigned short get_SNES()
{
    int i;
    unsigned short data = 0;
    unsigned int value;
	
	
    // Set LATCH to high for 12 microseconds. This causes the controller to
    // latch the values of button presses into its internal register. The first
    // serial bit also becomes available on the DATA line.
    set_GPIO9();
    microsecond_delay(12);
    clear_GPIO9();
	
    // Output 16 clock pulses, and read 16 bits of serial data
	for (i = 0; i < 16; i++) {
		// Delay 6 microseconds (half a cycle)
		microsecond_delay(6);
		
		// Clear the CLOCK line (creates a falling edge)
		clear_GPIO11();
		
		// Read the value on the input DATA line
		value = get_GPIO10();
		
		// Store the bit read. Note we convert a 0 (which indicates a button
		// press) to a 1 in the returned 16-bit integer. Unpressed buttons will
		// be encoded as a 0.
		if (value == 0) {
	    	data |= (0x1 << i);
		}
		
		// Delay 6 microseconds (half a cycle)
		microsecond_delay(6);
		
		// Set the CLOCK to 1 (creates a rising edge). This causes the
		// controller to output the next bit, which we read half a cycle later.
		set_GPIO11();
    }
	
    // Return the encoded data
    return data;
}


void init_GPIO9_to_output()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 0
    r = *GPFSEL0;

    // Clear bits 27 - 29. This is the field FSEL9, which maps to GPIO pin 9.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << 27);

    // Set the field FSEL9 to 001, which sets pin 9 to an output pin. We do so
    // by ORing the bit pattern 001 into the field.
    r |= (0x1 << 27);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 0
    *GPFSEL0 = r;


    // Disable the pull-up/pull-down control line for GPIO pin 9:

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 18-19 in this bit pattern, since this maps to GPIO pin 9.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 18);

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;
}


void set_GPIO9()
{
    register unsigned int r;
	  
    // Put a 1 into the SET9 field of the GPIO Pin Output Set Register 0
    r = (0x1 << 9);
    *GPSET0 = r;
}



void clear_GPIO9()
{
    register unsigned int r;
	  
    // Put a 1 into the CLR9 field of the GPIO Pin Output Clear Register 0
    r = (0x1 << 9);
    *GPCLR0 = r;
}



void init_GPIO11_to_output()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL1;

    // Clear bits 3 - 5. This is the field FSEL11, which maps to GPIO pin 11.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << 3);

    // Set the field FSEL11 to 001, which sets pin 9 to an output pin. We do so
    // by ORing the bit pattern 001 into the field.
    r |= (0x1 << 3);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 1
    *GPFSEL1 = r;

    
    // Disable the pull-up/pull-down control line for GPIO pin 11:

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 22-23 in this bit pattern, since this maps to GPIO pin 11.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 22);

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;
}



void set_GPIO11()
{
    register unsigned int r;
	  
    // Put a 1 into the SET11 field of the GPIO Pin Output Set Register 0
    r = (0x1 << 11);
    *GPSET0 = r;
}


void clear_GPIO11()
{
    register unsigned int r;
	  
    // Put a 1 into the CLR11 field of the GPIO Pin Output Clear Register 0
    r = (0x1 << 11);
    *GPCLR0 = r;
}



void init_GPIO10_to_input()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 1
    r = *GPFSEL1;

    // Clear bits 0 - 2. This is the field FSEL10, which maps to GPIO pin 10.
    // We clear the bits by ANDing with a 000 bit pattern in the field. This
    // sets the pin to be an input pin.
    r &= ~(0x7 << 0);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 1
    *GPFSEL1 = r;


    // Disable the pull-up/pull-down control line for GPIO pin 10:

    // Get the current bit pattern of the GPPUPPDN0 register
    r = *GPPUPPDN0;

    // Zero out bits 20-21 in this bit pattern, since this maps to GPIO pin 10.
    // The bit pattern 00 disables pullups/pulldowns.
    r &= ~(0x3 << 20);

    // Write the modified bit pattern back to the GPPUPPDN0 register
    *GPPUPPDN0 = r;
}



unsigned int get_GPIO10()
{
    register unsigned int r;
	  
	  
    // Get the current contents of the GPIO Pin Level Register 0
    r = *GPLEV0;
	  
    // Isolate pin 10, and return its value (a 0 if low, or a 1 if high)
    return ((r >> 10) & 0x1);
}
