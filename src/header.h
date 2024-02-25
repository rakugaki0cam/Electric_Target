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


//
#include <xc.h>
#include <stdint.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <string.h>
#include <math.h>

//
#include "BME280v3.h"
#include "computeEpicenterV5.h"
#include "dataDispSendV5.h"
#include "ESP32slave.h"
#include "iP5306.h"
#include "log.h"
#include "measureTimeV5.h"
#include "MyI2C1.h"
#include "PCF8574.h"


//return value
#define OK      0
#define ERROR   1

//DEBUG
//calculation error -> LED Yellow   //DEBUG 計算時の軽微なエラーの時黄色LEDを点ける
#define DEBUG_LED_no
#ifdef DEBUG_LED
    #define LED_CAUTION   LED_YELLOW
#else
    #define LED_CAUTION   NO_OUTPUT
#endif


//着弾タイミングのフォールエッジをESP32とタマモニに送る
#define     impact_PT4_On()     PT4_Clear()     //着弾センサ信号出力
#define     impact_PT4_Off()    PT4_Set()       //着弾センサ信号クリア



//Global
extern uint8_t      sensorCnt;                  //センサ入力順番のカウント
extern uint16_t     ringPos;                    //ログデータポインタ
extern debugger_mode_sour_t    debuggerMode;    //DEBUGger表示モード


//callback
void mainSwOn_callback(EXTERNAL_INT_PIN, uintptr_t);
void timer1sec_callback(uintptr_t);
//sub
void debuggerComand(void);


#endif //_HEADER_H

/* *****************************************************************************
 End of File
*/
