/*
 * computeEpicenterv5.h
 * 
 * 座標の計算
 * 
 *  2022.03.24　calc_locate.hより
 * 
 *  2024.01.05 V5 センサー5個
 *  
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef CALC_LOCATE_V5_H
#define	CALC_LOCATE_V5_H

//??#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>



//GLOBAL
//計算パターン数
#define     NUM_CAL         10      //センサー5個から3個選ぶパターンの数 10    5C3 = 5x4x3 / 3x2x1
#define     NUM_GROUP       5       //センサー4個でのばらつきを見るグループ数　


//計算ステータス
typedef enum {
    //
    CALC_STATUS_OK,
    CALC_STATUS_NOT_ENOUGH,     //測定値数不足
    CALC_STATUS_ERROR,          //ここまでmeas_stat_sor_tと同じ
    //        
    CALC_STATUS_CAL_ERROR,      //計算エラーあり
    CALC_STATUS_E_ZERO,         //分母e=0
    CALC_STATUS_QUAD_F,         //解の公式の条件エラー
    CALC_STATUS_R0_UNDER0,      //r0が負
    CALC_STATUS_X0_INVALID,     //x0が範囲外
    //       
    CALC_STATUS_X0_ERR,         //解x0が範囲外
    CALC_STATUS_Y0_ERR,         //解y0が範囲外
    CALC_STATUS_R0_ERR,         //解r0が範囲外
    //        
    CALC_STATUS_AVERAGE_ERR,    //平均値が不可
    CALC_STATUS_DEV_OVER,       //偏差が大
    //
    CALC_STATUS_SOURCE_NUM
} calc_stat_sor_t;


//着弾位置計算結果
typedef struct {
    uint16_t        pattern;            //センサ選択パターン 十進　各桁がセンサ番号
    float           impact_pos_x_mm;    //着弾座標
    float           impact_pos_y_mm;
    float           radius0_mm;         //着弾点から最初のセンサまでの距離
    float           delay_time0_msec;   //着弾点から最初のセンサオンまでの時間
    calc_stat_sor_t status;             //状態
} impact_result_t;

extern impact_result_t  calcValue[];    //計算数値
extern impact_result_t  calcResult;     //計算結果





//
void    clearResult(uint8_t);
void    resultError999(uint8_t, calc_stat_sor_t);

//
calc_stat_sor_t    computeEpicenter(void);
calc_stat_sor_t    computeXY(uint8_t);
//
uint8_t     select3sensor(uint8_t);

uint16_t    sensorOrderPattern(uint8_t);
uint16_t    sensorGroupePattern(uint8_t);
uint8_t     firstSensor(void);

uint8_t     check_deviation(void);


//calculation
uint8_t     apollonius3circleXYR(uint8_t);


#endif	//CALC_LOCATE_V5_H

