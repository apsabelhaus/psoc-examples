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
    UART_for_USB_PutString("PWM on. Please set the period: \r\n");
    
    for(;;)
    {
        /* Place your application code here. */
    }
}

/* [] END OF FILE */
