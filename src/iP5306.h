/*
 * File:   iP5306.h
 * 
 * 2024.01.03
 * 
 */

#ifndef _IP5306_H    /* Guard against multiple inclusion */
#define _IP5306_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//Global
typedef enum
{
    POWERSAVING_NORMAL = 0,
    POWERSAVING_SLEEP  = 1,
    POWERSAVING_DEEPSLEEP = 2,
} power_saving_mask_t;

extern power_saving_mask_t sleepStat;



//iP5306
bool        ip5306_Init(void);
bool        ip5306_ReadStatus(uint8_t*);

//battery
float       batteryVolt(bool);
uint16_t    batteryAdcGet(void);
//main switch
void        mainSwPush(void);
//
void        espSleep(void);
void        deepSleep(void);
void        resetRestart(void);


#endif //_IP5306_H


/* *****************************************************************************
 End of File
 */
