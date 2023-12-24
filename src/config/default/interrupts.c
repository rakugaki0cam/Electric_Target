/*******************************************************************************
 System Interrupts File

  Company:
    Microchip Technology Inc.

  File Name:
    interrupt.c

  Summary:
    Interrupt vectors mapping

  Description:
    This file maps all the interrupt vectors to their corresponding
    implementations. If a particular module interrupt is used, then its ISR
    definition can be found in corresponding PLIB source file. If a module
    interrupt is not used, then its ISR implementation is mapped to dummy
    handler.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "interrupts.h"
#include "definitions.h"



// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************


/* All the handlers are defined here.  Each will call its PLIB-specific function. */
// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector declarations
// *****************************************************************************
// *****************************************************************************
void INPUT_CAPTURE_1_Handler (void);
void TIMER_2_Handler (void);
void INPUT_CAPTURE_2_Handler (void);
void TIMER_3_Handler (void);
void INPUT_CAPTURE_3_Handler (void);
void INPUT_CAPTURE_4_Handler (void);
void EXTERNAL_4_Handler (void);
void INPUT_CAPTURE_5_Handler (void);
void RTCC_Handler (void);
void COMPARATOR_1_Handler (void);
void COMPARATOR_2_Handler (void);
void I2C1_BUS_Handler (void);
void I2C1_MASTER_Handler (void);
void I2C2_BUS_Handler (void);
void I2C2_SLAVE_Handler (void);
void COMPARATOR_3_Handler (void);
void COMPARATOR_4_Handler (void);
void COMPARATOR_5_Handler (void);


// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector definitions
// *****************************************************************************
// *****************************************************************************
void __ISR(_INPUT_CAPTURE_1_VECTOR, ipl1SRS) INPUT_CAPTURE_1_Handler (void)
{
    INPUT_CAPTURE_1_InterruptHandler();
}

void __ISR(_TIMER_2_VECTOR, ipl1SRS) TIMER_2_Handler (void)
{
    TIMER_2_InterruptHandler();
}

void __ISR(_INPUT_CAPTURE_2_VECTOR, ipl1SRS) INPUT_CAPTURE_2_Handler (void)
{
    INPUT_CAPTURE_2_InterruptHandler();
}

void __ISR(_TIMER_3_VECTOR, ipl1SRS) TIMER_3_Handler (void)
{
    TIMER_3_InterruptHandler();
}

void __ISR(_INPUT_CAPTURE_3_VECTOR, ipl1SRS) INPUT_CAPTURE_3_Handler (void)
{
    INPUT_CAPTURE_3_InterruptHandler();
}

void __ISR(_INPUT_CAPTURE_4_VECTOR, ipl1SRS) INPUT_CAPTURE_4_Handler (void)
{
    INPUT_CAPTURE_4_InterruptHandler();
}

void __ISR(_EXTERNAL_4_VECTOR, ipl1SRS) EXTERNAL_4_Handler (void)
{
    EXTERNAL_4_InterruptHandler();
}

void __ISR(_INPUT_CAPTURE_5_VECTOR, ipl1SRS) INPUT_CAPTURE_5_Handler (void)
{
    INPUT_CAPTURE_5_InterruptHandler();
}

void __ISR(_RTCC_VECTOR, ipl1SRS) RTCC_Handler (void)
{
    RTCC_InterruptHandler();
}

void __ISR(_COMPARATOR_1_VECTOR, ipl1SRS) COMPARATOR_1_Handler (void)
{
    COMPARATOR_1_InterruptHandler();
}

void __ISR(_COMPARATOR_2_VECTOR, ipl1SRS) COMPARATOR_2_Handler (void)
{
    COMPARATOR_2_InterruptHandler();
}

void __ISR(_I2C1_BUS_VECTOR, ipl1SRS) I2C1_BUS_Handler (void)
{
    I2C1_BUS_InterruptHandler();
}

void __ISR(_I2C1_MASTER_VECTOR, ipl1SRS) I2C1_MASTER_Handler (void)
{
    I2C1_MASTER_InterruptHandler();
}

void __ISR(_I2C2_BUS_VECTOR, ipl1SRS) I2C2_BUS_Handler (void)
{
    I2C2_BUS_InterruptHandler();
}

void __ISR(_I2C2_SLAVE_VECTOR, ipl1SRS) I2C2_SLAVE_Handler (void)
{
    I2C2_SLAVE_InterruptHandler();
}

void __ISR(_COMPARATOR_3_VECTOR, ipl1SRS) COMPARATOR_3_Handler (void)
{
    COMPARATOR_3_InterruptHandler();
}

void __ISR(_COMPARATOR_4_VECTOR, ipl1SRS) COMPARATOR_4_Handler (void)
{
    COMPARATOR_4_InterruptHandler();
}

void __ISR(_COMPARATOR_5_VECTOR, ipl1SRS) COMPARATOR_5_Handler (void)
{
    COMPARATOR_5_InterruptHandler();
}




/*******************************************************************************
 End of File
*/
