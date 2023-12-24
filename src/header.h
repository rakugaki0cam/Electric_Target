/***************************************************************************
 *  header.h
 * 
 * 
 ***************************************************************************
*/

#ifndef _HEADER_H    /* Guard against multiple inclusion */
#define _HEADER_H

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes


void ip5306_init(void);
bool ip5306_read_status(uint8_t*);

bool battery_voltage_disp(bool);
bool i2c1_bus_check(void);
bool i2c1_error(void);
bool i2c1_wait(void);

void deep_sleep(void);


#endif // _HEADER_H

/* *****************************************************************************
 End of File
 */
