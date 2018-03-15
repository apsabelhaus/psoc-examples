/* ========================================
 *
 * Copyright Andrew P. Sabelhaus, 2018 
 * See README and LICENSE for more details.
 *
 * ========================================
*/

// Code adapted heavily from Cypress' example CE95277 ADC and UART,
// but all re-written by Drew.

#include "uart_helper_fcns.h"
#include <project.h>
// stdio.h provides the sprintf and sscanf functions for working with strings.
#include "stdio.h"

// We'll store a string for the PSoC-to-PC transmission, with a length of this many characters:
#define TRANSMIT_LENGTH 128
// We'll store a string for the PC-to-PSoC transmission, with a length of this many characters:
#define RECEIVE_LENGTH 128

// Strings of characters are often stored in "buffers" as arrays of characters.
static char transmit_buffer[TRANSMIT_LENGTH];
static char receive_buffer[RECEIVE_LENGTH];

// We'll keep track of new characters as they're received over the UART, so we need to
// index into the buffer
static uint8 num_chars_received = 0;

// the data, as recorded in the helpers below.
// This will be either the period or duty cycle
static uint16 data = 0;

// We need to store a character representing the mode, "set period" = p or "set duty cycle" = d
static char mode;

/**
 * Definition of the UART ISR
 * We use the same line for the function definition, with the CY_ISR macro.
 * Compare this to tutorial 6, with "pythagorean"
 * This function also repeats characters back to the terminal, so you can see what you're typing.
 * Compare to the code at the end of tutorial 7
 */
CY_ISR( Interrupt_Handler_UART_Receive){
    // We assume this ISR is called when a byte is received.
    uint8 received_byte = UART_for_USB_GetChar();
    
    // C allows us to "switch" on uint8s, since characters are also numbers via the ASCII table.
    // either add to the buffer, or finally set the PWM parameters, depending on ...
    switch( received_byte )
    {
        // Students: look up switch-case statements. to understand this more.
        case '\r':
            // flow downward, no specific code for carriage return
        case '\n':
            // newline or carraige return received, so finally set the PWM parameters
            // First, terminate the string. This is for the use of sscanf below.
            receive_buffer[num_chars_received] = '\0';
            // Print back the newline/carriage return, to complete the "respond back to the terminal" code
            UART_for_USB_PutString("\r\n");
            // Call the helper function to actually set the PWM
            Write_PWM_and_UART();
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
 * Helper function that does the writing to the PWM.
 * makes the ISR code easier to understand.
 */
void Write_PWM_and_UART(){
    // OK, so now, we have a string in the receive buffer,
    // ideally of the form "(p/d) : somenumber".
    
    // Read in both the mode (p or d) and the integer afterward. Need to check if both were read in.
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
            /**
             * TUTORIAL 8, TASK 6:
             * Write the period of the PWM.
             * At this point in the code, the variable "data" will store your PWM period as a uint16.
             * hint: look at the API for the PWM component... something about writing and period.
             */
        
            ///////////////// PUT YOUR CODE HERE
        
            // send back the data that was just written, for confirmation:
            // read back from the PWM, and send a message with the period inserted into the string
            uint16 period_written = PWM_Servo_ReadPeriod();
            sprintf( transmit_buffer, "PWM now has a period of: %i \r\n", period_written);
            break;
        case 'd':
            /**
             * TUTORIAL 8, TASK 7:
             * Write the duty cycle of the PWM. This is also called the "compare" value.
             * At this point in the code, the variable "data" will store your PWM duty cycle as a uint16.
             * hint: look at the API for the PWM component... something about writing and compare value.
             */
            
            ///////////////// PUT YOUR CODE HERE
            
            // as with the period, send back what we just did:
            uint16 duty_written = PWM_Servo_ReadCompare();
            sprintf( transmit_buffer, "PWM now has a duty cycle (in clock ticks) of: %i \r\n", duty_written);
            break;
        default:
            // Print an error message if any other character besides a p or d was typed
            sprintf( transmit_buffer, "Error! You didn't type a p or d. \r\n");
            // just in case
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
