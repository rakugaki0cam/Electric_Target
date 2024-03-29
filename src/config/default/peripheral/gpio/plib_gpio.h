/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
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

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


/*** Macros for TX_ESP pin ***/
#define TX_ESP_Get()               ((PORTB >> 0) & 0x1)
#define TX_ESP_PIN                  GPIO_PIN_RB0

/*** Macros for RX_ESP pin ***/
#define RX_ESP_Get()               ((PORTB >> 1) & 0x1)
#define RX_ESP_PIN                  GPIO_PIN_RB1

/*** Macros for TEMP_V pin ***/
#define TEMP_V_Get()               ((PORTB >> 2) & 0x1)
#define TEMP_V_PIN                  GPIO_PIN_RB2

/*** Macros for XTAL1 pin ***/
#define XTAL1_Get()               ((PORTA >> 2) & 0x1)
#define XTAL1_PIN                  GPIO_PIN_RA2

/*** Macros for XTAL2 pin ***/
#define XTAL2_Get()               ((PORTA >> 3) & 0x1)
#define XTAL2_PIN                  GPIO_PIN_RA3

/*** Macros for LED_RED pin ***/
#define LED_RED_Set()               (LATBSET = (1<<4))
#define LED_RED_Clear()             (LATBCLR = (1<<4))
#define LED_RED_Toggle()            (LATBINV= (1<<4))
#define LED_RED_OutputEnable()      (TRISBCLR = (1<<4))
#define LED_RED_InputEnable()       (TRISBSET = (1<<4))
#define LED_RED_Get()               ((PORTB >> 4) & 0x1)
#define LED_RED_PIN                  GPIO_PIN_RB4

/*** Macros for SENSOR1 pin ***/
#define SENSOR1_Get()               ((PORTA >> 4) & 0x1)
#define SENSOR1_PIN                  GPIO_PIN_RA4

/*** Macros for LED_BLUE pin ***/
#define LED_BLUE_Set()               (LATBSET = (1<<5))
#define LED_BLUE_Clear()             (LATBCLR = (1<<5))
#define LED_BLUE_Toggle()            (LATBINV= (1<<5))
#define LED_BLUE_OutputEnable()      (TRISBCLR = (1<<5))
#define LED_BLUE_InputEnable()       (TRISBSET = (1<<5))
#define LED_BLUE_Get()               ((PORTB >> 5) & 0x1)
#define LED_BLUE_PIN                  GPIO_PIN_RB5

/*** Macros for SENSOR4 pin ***/
#define SENSOR4_Get()               ((PORTB >> 7) & 0x1)
#define SENSOR4_PIN                  GPIO_PIN_RB7

/*** Macros for SENSOR3 pin ***/
#define SENSOR3_Get()               ((PORTB >> 8) & 0x1)
#define SENSOR3_PIN                  GPIO_PIN_RB8

/*** Macros for SENSOR2 pin ***/
#define SENSOR2_Get()               ((PORTB >> 9) & 0x1)
#define SENSOR2_PIN                  GPIO_PIN_RB9

/*** Macros for RTS pin ***/
#define RTS_Get()               ((PORTB >> 10) & 0x1)
#define RTS_PIN                  GPIO_PIN_RB10

/*** Macros for PT4 pin ***/
#define PT4_Set()               (LATBSET = (1<<11))
#define PT4_Clear()             (LATBCLR = (1<<11))
#define PT4_Toggle()            (LATBINV= (1<<11))
#define PT4_OutputEnable()      (TRISBCLR = (1<<11))
#define PT4_InputEnable()       (TRISBSET = (1<<11))
#define PT4_Get()               ((PORTB >> 11) & 0x1)
#define PT4_PIN                  GPIO_PIN_RB11

/*** Macros for DEBUG_RX pin ***/
#define DEBUG_RX_Get()               ((PORTB >> 13) & 0x1)
#define DEBUG_RX_PIN                  GPIO_PIN_RB13

/*** Macros for VTH pin ***/
#define VTH_Get()               ((PORTB >> 14) & 0x1)
#define VTH_PIN                  GPIO_PIN_RB14

/*** Macros for DEBUG_TX pin ***/
#define DEBUG_TX_Get()               ((PORTB >> 15) & 0x1)
#define DEBUG_TX_PIN                  GPIO_PIN_RB15


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/

typedef enum
{
    GPIO_PORT_A = 0,
    GPIO_PORT_B = 1,
} GPIO_PORT;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/

typedef enum
{
    GPIO_PIN_RA0 = 0,
    GPIO_PIN_RA1 = 1,
    GPIO_PIN_RA2 = 2,
    GPIO_PIN_RA3 = 3,
    GPIO_PIN_RA4 = 4,
    GPIO_PIN_RB0 = 16,
    GPIO_PIN_RB1 = 17,
    GPIO_PIN_RB2 = 18,
    GPIO_PIN_RB3 = 19,
    GPIO_PIN_RB4 = 20,
    GPIO_PIN_RB5 = 21,
    GPIO_PIN_RB7 = 23,
    GPIO_PIN_RB8 = 24,
    GPIO_PIN_RB9 = 25,
    GPIO_PIN_RB10 = 26,
    GPIO_PIN_RB11 = 27,
    GPIO_PIN_RB13 = 29,
    GPIO_PIN_RB14 = 30,
    GPIO_PIN_RB15 = 31,

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
    GPIO_PIN_NONE = -1

} GPIO_PIN;


void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
    GPIO_PortWrite((GPIO_PORT)(pin>>4), (uint32_t)(0x1) << (pin & 0xF), (uint32_t)(value) << (pin & 0xF));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return (bool)(((GPIO_PortRead((GPIO_PORT)(pin>>4))) >> (pin & 0xF)) & 0x1);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (bool)((GPIO_PortLatchRead((GPIO_PORT)(pin>>4)) >> (pin & 0xF)) & 0x1);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable((GPIO_PORT)(pin>>4), 0x1 << (pin & 0xF));
}


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
