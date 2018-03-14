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
// stdio.h provides the sprintf and sscanf functions for working with strings.
#include "stdio.h"

// We're going to define the size of the character array for the transmission
// back to the PC. This is in bytes: if a uint8 is one character, we can store
// a string of this length. E.g. 16 gives us 16 characters, including any integers represented as characters.
// This solves exactly the same problem as in the supplement to tutorial 6!!!
// Drew used 128 here, like with receive, because it's 
// "big enough." Compare to the size of the period of the PWM, as returned from ReadPeriod(), 
// and the string of text I put down below.
// How many bytes (characters) will we expect to need to send back? How many are "taken up" by the PWM period?
#define TRANSMIT_LENGTH 128
// We're also going to keep a buffer of received characters, since multiple bytes are needed.
// The size of this here is the number of characters you can type into TeraTerm before pressing enter.
// George suggested 128, which is way many more than you'll need. 
// Note the example of "make the array larger than you need just in case because it doesn't really hurt"
#define RECEIVE_LENGTH 128

// See tutorial 7 supplement for discussion on "static".

// Also, keep the buffer as a global variable instead of creating it inside the ISR.
// This is for efficiency, and since we'll need to send back a string of characters with numbers also inside it.
static char transmit_buffer[TRANSMIT_LENGTH];
// similarly, we want a receive buffer, for taking in multiple charactes as they are sent to the PSoC.
static char receive_buffer[RECEIVE_LENGTH];

// Since the UART only sends single bytes, we encounter a problem with setting numerical values.
// Specifically, the byte send is interpreted as a character, NOT a number.
// See the ASCII table for a bit more intuition: for example, to set a period between 100 and 200, we'd need to 
// type in the characters between 'd' and 'weird L bar thing that isn't on Drew's keyboard. 
// https://www.asciitable.com/
// That's not OK. Instead, let's take in each character, store it in a buffer.
// The num_chars_received integer allows us to index into the string buffer (since it's an array of char).
static uint8 num_chars_received = 0;

// In the example that Drew got from George, we used a character to split

// the period, as recorded in the helpers below. By declaring with global scope, we
// increase efficiency.
static uint16 period = 0;

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
        // Students: look up switch-case statements. to understand this more.
        // We're going to take advantage of the "flow" of switch-case to do the same thing for characters 10, and 13, (\n and \r),
        // which could all be newlines, by not "break"-ing until the end of the third case.
        case '\r':
            // flow downward, no specific code for carriage return
        case '\n':
            // This code will run if the received byte is either a carriage return or a newline.
            // Since the PSoC received a new line...
            // First, terminate the string. This is for the use of sscanf below.
            receive_buffer[num_chars_received] = '\0';
            Write_PWM_and_UART();
            // This helper will also reset the period we're tracking, and the num chars received.
            // By "break"-ing, the next case is not executed.
            break;
        default:
            // The "default" case is "anything else", which is "store another character."
            // Add to the received buffer.
            receive_buffer[num_chars_received] = received_byte;
            // We need to increment the counter. i++ does this without an equals sign for assignment
            num_chars_received++;
            break;
        // end of case statement.
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
    
    
    
}

// Helper function that does the writing to the PWM and UART.
// makes the ISR code easier to understand.
void Write_PWM_and_UART(){
    // OK, so now, we have a string in the receive buffer of the form "p:somenumber".
    // Read in the integer after the p:
    sscanf( receive_buffer, "p:%d", &period);
    // TO-DO: checking on output of sscanf to confirm the user wrote a p:
    // sscanf requires the address-of (&) for the variable to be written.
    // Now, set the period, 
    PWM_Servo_WritePeriod( period );
    
    // and send back the period that was just written, for confirmation.
    // In C, to concatenate a number (integer) and a string (characters), you need to...
    // (1) store the result, as confirmed by the PWM component.
    uint16 period_written = PWM_Servo_ReadPeriod();
    // (2) Concatenate this period with a string of characters describing what we did
    // Requires stdio.h (standard input/output) for the sprintf function.
    sprintf( transmit_buffer, "PWM now has a period of: %i \r\n", period_written);
    // (3) send the byte back to your PC
    UART_for_USB_PutString( transmit_buffer );    
    // (4) Send another little string prompting the next input.
    UART_for_USB_PutString( "Set the period by typing p: then a new period, between 100 and 200 (min and max period for a servo on a 100 kHz clock).\r\n");
    // Note that we don't have to reset the buffer here, since sscanf only reads up until the first '\0'.
}

/* [] END OF FILE */
