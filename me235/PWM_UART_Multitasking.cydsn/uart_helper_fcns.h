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
 * 
 * IMPORTANT: this assumes you'll use the interrupt handler below, Interrupt_Handler_UART_Receive.
 */


// This is called an include guard. See https://en.wikipedia.org/wiki/Include_guard for more info.
// This is a macro also.
#ifndef UART_HELPER_FCNS_H
#define UART_HELPER_FCNS_H
    
// Cypress' project.h already has its own include guard.
#include <project.h>

// Handler for receiving UART data. Does the following:
// 1) Parses the command received
// 2) Sets the PWM block parameters
// 3) Sends a response back over UART, with the new settings confirmed.
CY_ISR( Interrupt_Handler_UART_Receive );

// Another helper that does the writing to the PWM and UART upon receipt of a newline,
// making the ISR cleaner.
void Write_PWM_and_UART();

#endif //UART_HELPER_FCNS_H

/* [] END OF FILE */
