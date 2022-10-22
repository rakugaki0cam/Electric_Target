
/*
 * calc_locate.h
 * 
 * 座標の計算
 * 
 *  2022.03.24
 * 
 *  
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef CALC_LOCATE_H
#define	CALC_LOCATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "header.h"
#include <math.h>


//GLOBAL
#define     NUM_SENSOR                  4           //センサー数
#define     SENSOR1                     0           //センサ呼び番号とプログラム中の添字
#define     SENSOR2                     1
#define     SENSOR3                     2
#define     SENSOR4                     3
#define     NUM_PATTERN                 4           //センサー4個から3個選ぶパターンの数 4C3 = 4!-3! = 4x3x2x1 - 3x2x1 = 4
#define     SENSOR_HORIZONTAL_SPACING   90.3        //センサー横幅　180.6 (マイクφ9.7)　測定値
#define     SENSOR_VERTICAL_SPACING     131.9       //センサー縦幅　251.8(フレーム間隔)+5+5(フレーム～マイク上面)+1+1(マイク上面～振動板)　測定値
#define     TARGET_WIDTH_HALF           105         //マト板横幅の1/2
#define     TARGET_HEIGHT_HALF          120         //マト板縦高さの1/2
#define     R_MAX                       340         //ほぼ対角長
#define     SENSOR_DEPTH_OFFSET         6           //塩ビ板t2　音の発生源は塩ビ板の裏面??

//センサ情報と測定値
typedef struct {
    uint8_t     sensor_num;         //センサ番号
    uint8_t     input_order;        //センサデータ入力順序
    float       sensor_x_mm;        //センサ座標
    float       sensor_y_mm;
    float       sensor_z_mm;
    uint32_t    timer_cnt;          //タイマ測定値
    uint32_t    delay_cnt;          //タイマ差値
    float       delay_time_usec;    //到達時間差測定値
    float       sensor_delay_usec;  //センサ遅れ時間(推定)
    float       d_time_corr_usec;   //到達時間差補正後
    float       distance_mm;        //距離差
    uint8_t     status;             //状態
} sensor_data_t;

typedef enum {
    SENSOR_STATUS_OK        = 0x0,      //OK
    SENSOR_STATUS_INVALID   = 0x1,      //データ無効     
    SENSOR_STATUS_ERROR     = 0x2       //エラー   
} sensor_status_source_t;


//着弾位置計算結果
typedef struct {
    uint16_t    pattern;            //センサ選択パターン 十進　各桁がセンサ番号
    float       impact_pos_x_mm;    //着弾座標
    float       impact_pos_y_mm;
    float       radius0_mm;         //着弾点から最初のセンサまでの距離
    float       delay_time0_msec;   //着弾点から最初のセンサオンまでの時間
    uint8_t     status;             //状態
} impact_data_t;

typedef enum {
    CALC_STATUS_OK          = 0x00,
    CALC_STATUS_NOT_ENOUGH  = 0x01,     //測定値数不足
    CALC_STATUS_TOO_MANY    = 0x02,     //測定値が多すぎ
    CALC_STATUS_CAL_ERROR   = 0x11,     //計算エラーあり
    CALC_STATUS_E_0_1       = 0x12,     //分母e=0
    CALC_STATUS_E_0_2       = 0x13,     //分母e=0
    CALC_STATUS_E_0_3       = 0x14,     //分母e=0
    CALC_STATUS_E_0_4       = 0x15,     //分母e=0
    CALC_STATUS_QUAD_F      = 0x16,     //解の公式の条件エラー
    CALC_STATUS_X0_ERR      = 0x21,     //解X0が範囲外
    CALC_STATUS_Y0_ERR      = 0x22,     //解Y0が範囲外
    CALC_STATUS_R0_ERR      = 0x23,     //解R0が範囲外
    CALC_STATUS_X0_DEV_ERR  = 0x29,     //解X0が偏差大
    CALC_STATUS_Y0_DEV_ERR  = 0x2a,     //解Y0が偏差大
    CALC_STATUS_R0_DEV_ERR  = 0x2b,     //解R0が偏差大
    CALC_STATUS_AVERAGE_FIX = 0x30,     //平均値修正
    CALC_STATUS_AVERAGE_ERR = 0x31      //平均値不可
} calc_status_source_t;



//GLOBAL
extern sensor_data_t    sensor_4mic[];  //センサーデータ、測定値
extern impact_data_t    result;         //計算結果
extern const float      delay_a;        //センサー遅れ時間の計算係数
extern const float      delay_b;


//
uint8_t     calc_locate_xy(void);

//
uint8_t     calc_of_3sensor(void);
uint8_t     asign_3sensor(void);
void        calc_sensor_pattern(void);
//calculation
//locate
uint8_t     apollonius_3circle_xyr(void);
float       dist_delay_mm(float);
//time
float       v_air_mps(void);
float       delay_time_usec(uint32_t);
float       delay_sensor_usec(float);
float       impact_time_msec(float);
    

#endif	//CALC_LOCATE_H

