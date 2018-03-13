/* ========================================
 *
 * Copyright Andrew P. Sabelhaus, 2018 
 * See README and LICENSE for more details.
 *
 * ========================================
*/

// Code adapted heavily from Cypress' example CE95277 ADC and UART,
// but all re-written by Drew.

// We wrote the declarations in the header file,
// and define them here.
// Including this header file in both main.c and here will allow the functions
// to be accessible both places (one to define them, one to use them).

// Unlike in the header files, we don't need to add include guards here.
// That's because the guards are already present in the .h files themselves.
#include "uart_helper_fcns.h"
#include <project.h>
// stdio.h provides the sprintf function for working with strings.
#include "stdio.h"

// We're going to define the size of the character array for the transmission
// back to the PC. This is in bytes: if a uint8 is one character, we can store
// a string of this length. E.g. 16 gives us 16 characters, including any actual integers.
// This solves exactly the same problem as in the supplement to tutorial 6!!!
// Drew used 32 here for a VERY SPECIFIC REASON. Compare to the size of the period of the PWM, as returned from ReadPeriod().
// How many bytes will we expect to need to send back? How many are "taken up" by the PWM period?
#define TRANSMISSION_LENGTH 32

// See tutorial 7 supplement for discussion on "static".

// Also, keep the buffer as a global variable instead of creating it inside the ISR.
// This is for efficiency, and since we'll need to send back a string of characters with numbers also inside it.
static char transmission_buffer[TRANSMISSION_LENGTH];

// Since the UART only sends single bytes, we encounter a problem with setting numerical values.
// Specifically, the byte send is interpreted as a character, NOT a number.
// See the ASCII table for a bit more intuition: for example, to set a period between 100 and 200, we'd need to 
// type in the characters between 'd' and 'weird L bar thing that isn't on Drew's keyboard. 
// https://www.asciitable.com/
// That's not OK. Instead, let's take in each character, convert it to a uint8, and add to a uint16 that will represent
// the total period. To do so, we need to store two things:
// (1) the sum total of the period
static uint16 period = 0;
// (2) the number of characters (bytes) received. For example, if I want to type in the number "175", 
// we'd need to do the following: 5 * 10^0, 7 * 10^1, 1 * 10^2, where the (0,1,2) are the number received.
// We'll assume below that a "newline" ends a transmission.
static uint8 num_chars_received = 0;

// Definition of the UART ISR
// We use the same line for the function definition, with the CY_ISR macro.
// Compare this to tutorial 6, with "pythagorean"
CY_ISR( Interrupt_Handler_UART_Receive){
    // We assume this ISR is called when a byte is received.
    // See how the IDE doesn't give any errors, as long as we include project.h here.
    uint8 received_byte = UART_for_USB_GetChar();
    
    // Next, we need to deal with what was received, in the following way.
    // If a new line is received (the \n character, or ASCII values 10 or 12 or 13 depending on if your computer is Windows/Linux/Mac),
    // then finally set the period.
    // Otherwise, add to the uint16 we're keeping track of.
    // Luckily enough, C allows us to "switch" on uint8s, since characters are also numbers via the ASCII table.
    switch( received_byte )
    {
        // Look up switch-case statements.
        // We're going to take advantage of the "flow" of switch-case to do the same thing for characters 10, 12, 13,
        // which could all be newlines, by not "break"-ing until the end of the third case.
        case 10:
            // flow downward, no specific code for 10
        case 12:
            // flow downward, no specific code for 12
        case 13:
            // This code will run if any of 10, 12, 13 are input (any type of new line.)
            // Since the PSoC received a new line...
            write_PWM_and_UART();
            // This helper will also reset the period we're tracking, and the num chars received.
            // By "break"-ing, the next case is not executed.
            break;
        default:
            // The "default" case is "anything else." Assume it's a number.
            // TO-DO: validate (does C use assertions???) that we're only receiving a byte that's a 0-9 number,
            // e.g. between 48 and 57.
            // First, multiply the received number by its decimal place (e.g. num chars received.)
            // This may look unusual. Remember that the >> operator raises the left argument to the power-of-two of the right argument.
            // For example, we're multiplying by 10, so that' x * 10 = x*8 + x*2 = x * (2^3) + x * (2^1).
            // Drew's head hurt when trying to bitshift-multiply by 10^n instead of just 10, so a for loop lazily raises the values for me.
            /*
            uint8 i; 
            for( i=0; i <= num_chars_received; i++) {
                // note the less than or equal to. Think: why need this here?
                period += (received_byte << 3) + (received_byte << 1);
            }
            */
            break;
        }
            
    }
    
    // We're going to assume that the character we receive is an integer, 
    // representing the period.
    // For a transmission of 8 bits, we can only set the period between 0 and 255.
    // This is OK for how we've configured the PWM for our servo: 
    // our period can only be between 100 and 200 anyway (1 ms for min angle, 2 ms for max angle).
    // To get better resolution here, you'll need to do a handful of things:
    // (1) change the clock of the PWM so that we'll need a uint16 to represent 1 ms to 2 ms of "on"
    // (2) receive two bytes (as uint8) from the UART, keep track of the first one
    // (3) once two are received, combine them into a uint16
    // (4) write the uint16 to the PWM's period, as with the code below.    
    PWM_Servo_WritePeriod( received_byte );
    
    // Send back the period that was just written.
    // In C, to concatenate a number (integer) and a string (characters), you need to...
    // (1) store the result, as confirmed by the PWM component.
    uint16 period_written = PWM_Servo_ReadPeriod();
    // (2) Concatenate this period with a string of characters describing what we did
    // Requires stdio.h (standard input/output) for the sprintf function.
    sprintf( transmission_buffer, "PWM now has a period of: %i \r\n", period_written);
    // (3) send the byte back to your PC
    UART_for_USB_PutString( transmission_buffer );    
    // (4) Send another little string prompting the next input.
    UART_for_USB_PutString( "Type a number between 100 and 200 (min and max period for a 100 kHz clock).\r\n");
}

// Helper function that does the writing to the PWM and UART.
// makes the ISR code easier to understand.
void write_PWM_and_UART(){
    
}

/* [] END OF FILE */
