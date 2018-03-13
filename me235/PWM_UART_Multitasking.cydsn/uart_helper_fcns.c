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

// Definition of the UART ISR
// We use the same line for the function definition, with the CY_ISR macro.
// Compare this to tutorial 6, with "pythagorean"
CY_ISR( Interrupt_Handler_UART_Receive){
    // We assume this ISR is called when a byte is received.
    // See how the IDE doesn't give any errors, as long as we include project.h here.
    uint8 received_byte = UART_for_USB_GetChar();
}

/* [] END OF FILE */
