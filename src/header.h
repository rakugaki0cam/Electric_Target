/*
 * Electric Target #2  V5 - PIC32MK MCJ
 * 
 *  header.h
 * 
 * 
 *  2023.12.22
 * 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef _HEADER_H    /* Guard against multiple inclusion */
#define _HEADER_H

//????
#include <xc.h> 
#include <stdint.h>
#include <string.h>
//

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

#include "MyI2C1.h"
#include "BME280_v3.h"
#include "iP5306.h"
#include "ESP32slave.h"



//return value
#define OK      0
#define ERROR   1


//Global
extern bool    usb_in_flag;

typedef enum
{
    POWERSAVING_NORMAL = 0,
    POWERSAVING_SLEEP  = 1,
    POWERSAVING_DEEPSLEEP = 2,
} power_saving_mask_t;

extern power_saving_mask_t sleep_flag;



//
void deep_sleep(void);

//callback
void main_sw_on_callback(EXTERNAL_INT_PIN, uintptr_t);
void timer_1sec(uintptr_t);


#endif //_HEADER_H

/* *****************************************************************************
 End of File
*/
