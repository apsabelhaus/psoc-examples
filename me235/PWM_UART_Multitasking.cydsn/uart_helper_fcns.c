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
// "big enough." Compare to the size of the data of the PWM, as returned from ReadPeriod() for example, 
// and the string of text I put down below.
// How many bytes (characters) will we expect to need to send back? How many are "taken up" by the PWM data?
#define TRANSMIT_LENGTH 128
// We're also going to keep a buffer of received characters, since multiple bytes are needed.
// The size of this here is the number of characters you can type into TeraTerm before pressing enter.
// George suggested 128, which is way many more than you'll need. 
// Note the example of "make the array larger than you need just in case because it doesn't really hurt"
#define RECEIVE_LENGTH 128

// See tutorial 7 supplement for discussion on "static".

// Since the UART only sends single bytes, we encounter a problem with setting numerical values.
// Specifically, the byte sent is interpreted as a character, NOT a number.
// See the ASCII table for a bit more intuition: for example, to set a data between 100 and 200, we'd need to 
// type in the characters between 'd' and 'weird L bar thing that isn't on Drew's keyboard. 
// https://www.asciitable.com/
// That's not OK. Instead, let's take in the numbers as characters, not try to re-interpret them, and store them in a buffer.
// The num_chars_received integer allows us to index into the string buffer (since it's an array of char).
static uint8 num_chars_received = 0;

// Also, keep the buffer as a global variable instead of creating it inside the ISR.
// This is for efficiency, and since we'll need to send back a string of characters with numbers also inside it.
static char transmit_buffer[TRANSMIT_LENGTH];
// similarly, we want a receive buffer, for taking in multiple characters as they are sent to the PSoC.
static char receive_buffer[RECEIVE_LENGTH];

// the data, as recorded in the helpers below. By declaring with global scope, we
// increase efficiency. Used for both period and duty cycle.
static uint16 data = 0;

// We're also going to let people adjust the duty cycle (compare value).
// So, we need to store a character representing the mode.
static char mode;

/**
 * Definition of the UART ISR
 * We use the same line for the function definition, with the CY_ISR macro.
 * Compare this to tutorial 6, with "pythagorean"
 * This function also repeats characters back to the terminal, so you can see what you're typing.
 */
CY_ISR( Interrupt_Handler_UART_Receive){
    // We assume this ISR is called when a byte is received.
    // See how the IDE doesn't give any errors, as long as we include project.h here.
    uint8 received_byte = UART_for_USB_GetChar();
    
    //DEBUGGING
    /*
    UART_for_USB_PutString( "Received a character: ");
    UART_for_USB_PutChar( received_byte );
    UART_for_USB_PutString("\r\n");
    */
    
    // Next, we need to deal with what was received, in the following way.
    // If a new line is received (the \n character, or ASCII values 10 or 13 depending on if your computer is Windows/Linux/Mac),
    // then finally set the data.
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
            // Print back the newline/carriage return, to complete the "respond back to the terminal" code
            UART_for_USB_PutString("\r\n");
            // From George: strings in C are arrays of characters, and require an "end string" character at the end, 
            // so these library functions can know "how many characters are in the string."
            // So, append the end string character, which is:
            receive_buffer[num_chars_received] = '\0';
            // Call the helper function to actually parse the whole received string, now
            // that a newline has been received.
            Write_PWM_and_UART();
            // This helper will also reset the data we're tracking, and the num chars received.
            // By "break"-ing, the next case is not executed.
            break;
        case 'x':
            // Added functionality: if the user types an x, then the PWM stops.
            UART_for_USB_PutString("\r\nStopping PWM.\r\n");
            PWM_Servo_Stop();
            // Reset the buffer. We'll just start writing from the start again.
            num_chars_received = 0;
            break;
        case 'e':
            // Similarly, type e to enable.
            UART_for_USB_PutString("\r\nRestarting PWM.\r\n");
            PWM_Servo_Start();
            // Reset the buffer. We'll just start writing from the start again.
            num_chars_received = 0;
            break;
        default:
            // The "default" case is "anything else", which is "store another character."
            // Add to the received buffer.
            receive_buffer[num_chars_received] = received_byte;
            // Respond back to the terminal
            UART_for_USB_PutChar( received_byte );
            // We need to increment the counter. i++ does this without an equals sign for assignment
            num_chars_received++;
            break;
        // end of case statement.
    }
}

/**
 *Helper function that does the writing to the PWM and UART.
 * makes the ISR code easier to understand.
 */
void Write_PWM_and_UART(){
    // OK, so now, we have a string in the receive buffer,
    // ideally of the form "(p/d) : somenumber".
    
    // If you need help seeing what was stored, uncomment
    // the following lines to reply with the string that was received:
    /*
    UART_for_USB_PutString("Received the string: ");
    UART_for_USB_PutString( receive_buffer );
    UART_for_USB_PutString("\r\n");
    */
    
    // Read in both the mode (p or d) and the integer afterward
    // The %c specified is for a single character
    // The %hu specifier is for unsigned shorts (16 bit integers)
    // A good resource on scanf and printf specifiers is https://www.tutorialspoint.com/c_standard_library/c_function_sscanf.htm
    // Let's also check: did we store both a char and a uint16? Check the number of 'things' pulled out of the string.
    int num_var_filled;
    // actually scan in the character and integer. See documentation for the sscanf function.
    // sscanf requires the address-of (&) for the variable to be written.
    num_var_filled = sscanf( receive_buffer, "%c : %hu", &mode, &data);
    // Need to check: was anything received? Equivalently, did sscanf find exactly one character, and one uint16?
    if( num_var_filled != 2){
        UART_for_USB_PutString("Error! incorrect data. Did you type a number after a (p or d), a colon, and the spaces between?\r\n");
        data = 0;
    }
    
    // Depending on the mode, write either the period or the duty cycle:
    switch( mode )
    {
        case 'p':
            // Now, set the period, 
            PWM_Servo_WritePeriod( data );
            // and send back the data that was just written, for confirmation.
            // In C, to concatenate a number (integer) and a string (characters), you need to...
            // (1) store the result, as confirmed by the PWM component.
            uint16 period_written = PWM_Servo_ReadPeriod();
            // (2) Concatenate this data with a string of characters describing what we did
            // Requires stdio.h (standard input/output) for the sprintf function.
            sprintf( transmit_buffer, "PWM now has a period of: %i \r\n", period_written);
            break;
        case 'd':
            // Instead, set the compare value, the duty cycle in clock ticks.
            PWM_Servo_WriteCompare( data );
            // Like with the period:
            uint16 duty_written = PWM_Servo_ReadCompare();
            sprintf( transmit_buffer, "PWM now has a duty cycle (in clock ticks) of: %i \r\n", duty_written);
            break;
        default:
            // Print an error message if any other character besides a p or d was typed
            sprintf( transmit_buffer, "Error! You didn't type a p or d. \r\n");
            mode = 0;
            break;
    } 
    
    // send the byte back to your PC so you know what you set
    UART_for_USB_PutString( transmit_buffer );    
    // to make this easier to read, send another newline.
    UART_for_USB_PutString("\r\n");
    // Reset the indexing into the array
    num_chars_received = 0;
    // and just in case let's do the data too.
    data = 0;
    // Note that we don't have to reset the buffer here, since sscanf only reads up until the first '\0'.
}

/* [] END OF FILE */
