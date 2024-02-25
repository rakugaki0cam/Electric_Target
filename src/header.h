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
#include <xc.h>

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
extern uint8_t      sensorCnt;              //センサ入力順番のカウント
extern uint16_t     ringPos;                //ログデータポインタ
extern float        targetY0Offset;         //ターゲットYオフセット


extern debugger_mode_sour_t    debuggerMode;    //DEBUGger表示モード

//DEBUGger printf
//計算過程のデバッグ表示
#define DEBUG_MEAS_no       //デバッグprintf表示(エラー系)   
#define DEBUG_APO_no        //座標検算時のデバッグprintf表示(検算用)
#define DEBUG_APO2_no       //座標検算時のデバッグprintf表示(エラー系)
//ESP32スレーブへの送信関連デバッグ表示
#define DEBUG_ESP_SLAVE_0_no    //Debug用printf(エラー関連)
#define DEBUG_ESP_SLAVE_2_no    //Debug用printf(バッテリ電圧)
#define DEBUG_ESP_SLAVE_3_no    //Debug用printf(送信データ)








//callback
void mainSwOn_callback(EXTERNAL_INT_PIN, uintptr_t);
void timer1sec_callback(uintptr_t);
//sub
void debuggerComand(void);
void tamamoniCommandCheck(uint8_t*); 


#endif //_HEADER_H

/* *****************************************************************************
 End of File
*/
