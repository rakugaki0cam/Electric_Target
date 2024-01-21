/*
 * measureTimeV5.h
 * 
 * 測定と表示
 * 
 *  2022.04.16 measure.h
 * V5_edition
 *  2024.1.5
 *  
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MEASURE_H
#define	MEASURE_H

//??#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <math.h>


//sensor number
#define     NUM_SENSOR      5       //センサー数
#define     SENSOR1         0       //センサ呼び番号とプログラム中の添字
#define     SENSOR2         1
#define     SENSOR3         2
#define     SENSOR4         3
#define     SENSOR5         4

//sensor mic position     - センサーマイクとLCDを箱に一体化
#define SENSOR_HORIZONTAL_SPACING   69.0        //センサー横幅      138.0mm
#define SENSOR_VERTICAL_SPACING     87.0        //センサー縦幅      174.0mm
#define SENSOR_CENTER_SPACING       0.0         //センサー中心高さ    0.0mm
#define SENSOR_DEPTH_OFFSET         30//15.5        //塩ビ板t2　音の発生源は塩ビ板の裏面とする

//target sheet
#define     TARGET_WIDTH_HALF       150//105         //マト板横幅の1/2
#define     TARGET_HEIGHT_HALF      210//120         //マト板縦高さの1/2
#define     R_MAX                   260//340         //ほぼ対角長


//GLOBAL
extern const float      delay_a;        //センサー遅れ時間の計算係数
extern const float      delay_b;


//センサーステータス
typedef enum {
    SENSOR_STATUS_OK,       //OK
    SENSOR_STATUS_INVALID,  //データ無効     
    SENSOR_STATUS_ERROR,    //エラー  
    SENSOR_STAT_SOURCE_NUM
} sensor_stat_sor_t;


//センサ情報と測定値
typedef struct {
    uint8_t             sensor_num;         //センサ番号
    uint8_t             input_order;        //センサデータ入力順位 0~5
    float               sensor_x_mm;        //センサ座標
    float               sensor_y_mm;
    float               sensor_z_mm;
    uint32_t            timer_cnt;          //タイマ測定値
    uint32_t            delay_cnt;          //タイマ差値
    float               delay_time_usec;    //到達時間差測定値
    float               comp_delay_usec;    //コンパレータ応答遅れ時間(推定)
    float               d_time_corr_usec;   //到達時間差補正後
    float               distance_mm;        //距離差
    sensor_stat_sor_t   status;             //状態
} sensor_data_t;
extern sensor_data_t    sensor5Measure[];  //センサーデータ、測定値


//測定ステータス
typedef enum {
    MEASURE_STATUS_OK,
    MEASURE_STATUS_NOT_ENOUGH,       
    MEASURE_STATUS_ERROR,
    MEASURE_STATUS_SOURCE_NUM
} meas_stat_sor_t;

//グループ毎の分散の計算
typedef struct {
    uint16_t        pattern;            //センサ選択パターン 十進　各桁がセンサ番号
    uint8_t         sample_n;           //平均サンプル数
    float           average_pos_x_mm;
    float           average_pos_y_mm;
    float           average_radius0_mm;
    float           dist1_mm2;          //偏差 距離の2乗 dx^2+dy^2
    float           dist2_mm2;
    float           dist3_mm2;
    float           dist4_mm2;
    float           variance;           //偏差の総和の平均　=　分散(σ^2値)
    uint8_t         order;              //ばらつきの順位
    calc_stat_sor_t status;
} result_ave_t;

extern result_ave_t    groupVari[];




//
void    measureInit(void);
void    correctSensorOffset(float, float);
uint8_t measureMain(uint16_t);
//
uint8_t checkInputOrder(void);
uint8_t assignMeasureData(void);
//
void    clearData(void);

//time
float   dist_delay_mm(float);

float   v_air_mps(void);
float   delay_time_usec(uint32_t);
float   delay_comparator_usec(float);
float   impact_time_msec(float);
    
//sensor interrupt
void    detectSensor1(uintptr_t);
void    detectSensor2(uintptr_t);
void    detectSensor3(uintptr_t);
void    detectSensor4(uintptr_t);
void    detectSensor5(uintptr_t);


#endif	//MEASURE_H