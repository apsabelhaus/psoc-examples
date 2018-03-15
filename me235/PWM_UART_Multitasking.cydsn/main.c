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
    // Start the interrupt for the UART
    CyGlobalIntEnable;
    Interrupt_UART_Receive_StartEx( Interrupt_Handler_UART_Receive );
    
    // Start the UART itself
    UART_for_USB_Start();
    
    // Start the PWM component
    PWM_Servo_Start();
    
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
