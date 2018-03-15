/* ========================================
 *
 * Copyright Andrew P. Sabelhaus, 2018 
 * See README and LICENSE for more details.
 *
 * ========================================
*/
// For all the libraries on the PSoC itself
#include <project.h>
// Some code that Drew wrote to make the UART communication more easy to read.
#include "uart_helper_fcns.h"

int main()
{
    // enable all interrupts on the PSoC
    CyGlobalIntEnable;
    
    /**
     * TUTORIAL 8, TASK 1: 
     * start the interrupt for the UART, setting the handler (ISR) to Interrupt_Handler_UART_Receive
     * hint: see tutorial 7, or look a the two "start" functions in the API for interrupts.
     */
    
    ////////////////// PUT YOUR CODE HERE
    
    // Start the UART 
    UART_for_USB_Start();
    
    /**
     * TUTORIAL 8, TASK 5:
     * Start the PWM component, now that you've added it, configured its settings, and assigned its output pin.
     */
    
    ///////////////// PUT YOUR CODE HERE
    
    // Send an initial message over the UART / USB com port
    UART_for_USB_PutString("\r\n\r\nPWM on. Use one of the following commands:\r\n");
    UART_for_USB_PutString("Set the duty cycle, as number of clock ticks, by typing d : then the number. \r\n");
    UART_for_USB_PutString("Set the period by typing p : then a new period. \r\n");
    UART_for_USB_PutString("For example, p : 2000 sets the period to 2000. \r\n");
    UART_for_USB_PutString("Or, x stops the PWM, and e re-enables the PWM. \r\n\r\n");
    
    for(;;)
    {
        // Nothing to do - all interrupt driven!
    }
}

/* [] END OF FILE */
