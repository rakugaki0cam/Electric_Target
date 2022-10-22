/* 
 * 電子ターゲット
 * テスト
 * 
 * File:   header.h
 * 
 * Comments:
 * Revision history: 
 * 2022.03.19
 * 
 * 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef HEADER_H
#define	HEADER_H

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

//#include <xc.h> // include processor files - each processor file is guarded. 

#include "calc_locate.h"
#include "log.h"
#include "mcp9700a_temp.h"
#include "measure_v2.h"


//GLOBAL
extern uint8_t     sensor_count;            //センサ入力順番のカウント
extern uint16_t    ring_pos;                //ログデータポインタ


//interrupt
void        sensor1_detect(uintptr_t);
void        sensor2_detect(uintptr_t);
void        sensor3_detect(uintptr_t);
void        sensor4_detect(uintptr_t);
void        timer2_1sec_int(uint32_t, uintptr_t);
//
void        speaker(void);


#endif	//HEADER_H

