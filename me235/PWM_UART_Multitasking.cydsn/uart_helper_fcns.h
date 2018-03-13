/* ========================================
 *
 * Copyright Andrew P. Sabelhaus, 2018 
 * See README and LICENSE for more details.
 *
 * ========================================
*/

/**
 * uart_helper_fcns.h
 * A collection of helper functions for the UART communication,
 * when interrupt-driven (for multitasking.)
 * Currently used for setting the PWM block,
 * BUT can be adapted for any other interrupt-driven UART things!
 *
 * Why do we use a separate header here? 
 * The main.c file will get too large and confusing.
 * It's good practice to split off code into separate files for
 * different purposes.
 * 
 * How do I use these functions?
 * Write #include "uart_helper_fcns.h" at the top of your main.c file,
 * and you'll be able to call every function here.
 */

// IMPORTANT: this assumes you'll use the interrupt handler below,
// Interrupt_Handler_UART_Receive.

// Another use of macros: only include a header file if it hasn't been included already.
// We don't want to "double include" header files, but by including project.h,
// the PSoC Creator IDE will allow us to auto-complete, and is needed for the CY_ISR
// macro.
// This is called an include guard. See https://en.wikipedia.org/wiki/Include_guard for more info.
#ifndef UART_HELPER_FCNS_H
#define UART_HELPER_FCNS_H
    
// Here we see an example of how to use other headers inside include guards.
// Since Cypress was smart and used their own include guards in their headers,
// we can #include the project here to get the nice IDE autocomplete features.
// As an example, see http://faculty.cs.niu.edu/~mcmahon/CS241/c241man/node90.html, B.h
// And, you can look for lines in the files included in project.h, see things like "#if !defined(CYDEVICE_H)",
// to convince yourself that Cypress also uses include guards.
#include <project.h>

// Handler for receiving UART data. Does the following:
// 1) Parses the command received
// 2) Sets the PWM block parameters
// 3) Sends a response back over UART, with the new settings confirmed.
// THIS IS ONLY A DECLARATION. The definition is in the .c file.
CY_ISR( Interrupt_Handler_UART_Receive);


#endif //UART_HELPER_FCNS_H

/* [] END OF FILE */
