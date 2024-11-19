// This program blinks the activity LED on the Pi 4 on and off, and is useful
// for showing that the boot process is working. Note that it takes about 5
// seconds for the Pi 4 to boot up from the SD card. After that, the LED should
// blink on and off.


// Include header file
#include "gpio.h"


// Main routine. This is called by startup code in the start.s file.
 
void main()
{
    register volatile unsigned int r, counter;


    // Set GPIO pin 42 to be an output. This pin is connected to the activity
    // LED on the Pi 4.

    // Get the current contents of GPIO Function Select Register 4
    r = *GPFSEL4;

    // Clear bits 6-8. This is the field FSEL42, which maps to GPIO pin 42. We
    // clear bits in the field by ANDing its contents with a 000 bit pattern.
    // All other bits in the register are untouched.
    r &= ~(0x7 << 6);

    // Set the field FSEL42 to the 001 bit pattern, which sets GPIO pin 42 to
    // be an output. We do so by ORing the 001 bit pattern into the field.
    r |= (0x1 << 6);

    // Write the modified bit pattern back to the GPIO Function Select
    // Register 4
    *GPFSEL4 = r;

    // Loop forever, blinking the LED on and off
    while (1)
    {
        // Use a busy loop to delay a short amount of time
        for (counter = 0; counter < 250000; counter++)
            ;

        // Set GPIO pin 42 to low (off). Pin 42 is specified with bit 10 in the
        // GPIO Pin Output Clear Register 1.
        *GPCLR1 = (1 << 10);

        // Use a busy loop to delay a short amount of time
        for (counter = 0; counter < 250000; counter++)
            ;

        // Set GPIO pin 42 to high (on). Pin 42 is specified with bit 10 in the
        // GPIO Pin Output Set Register 1.
        *GPSET1 = (1 << 10);
    }
}
