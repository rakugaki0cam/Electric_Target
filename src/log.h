/*
 * log.h
 * 
 * ログ
 * 
 *  2022.04.16
 * 
 *  V5_edition
 *  2024.01.08
 * 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef LOG_H
#define	LOG_H

//#include <stdint.h>
//#include <stdbool.h>
//#include <stddef.h>


#define LOG_MEM 200             //ログメモリ確保

//固定値
typedef struct {
    uint8_t     sensor_num;      //センサ数 = 5
    float       sensor_x1;       //センサ座標
    float       sensor_y1;
    float       sensor_z1;
    float       sensor_x2;
    float       sensor_y2;
    float       sensor_z2;
    float       sensor_x3;
    float       sensor_y3;
    float       sensor_z3;
    float       sensor_x4;
    float       sensor_y4;
    float       sensor_z4;
    float       sensor_x5;
    float       sensor_y5;
    float       sensor_z5;
    float       sensor_delay_a;
    float       sensor_delay_b;
} log_fixed_t;

//測定&補正
typedef struct {
    uint8_t     shot_num;           //玉番号
    uint8_t     input_order;        //センサデータ入力順序     
    float       delay_time1;        //到達時間差測定値
    float       delay_time2;
    float       delay_time3;
    float       delay_time4;
    float       delay_time5;
    float       sensor_delay1;      //センサオン遅れ補正
    float       sensor_delay2;
    float       sensor_delay3;
    float       sensor_delay4;
    float       sensor_delay5;
    float       distance1;          //距離差
    float       distance2;
    float       distance3;
    float       distance4; 
    float       distance5; 
    float       v_air;              //音速
} log_calc_t;

//結果
typedef struct {
    uint8_t     shot_num;       //玉番号
    float       impact_pos_x;   //着弾座標
    float       impact_pos_y;
    float       radius0;        //着弾点から最初のセンサまでの距離
    float       temp;           //温度
} log_result_t;


//GLOBAL
extern log_fixed_t  log_sensor[];   //センサーデータ
extern log_calc_t   log_measure[];  //ログ測定値
extern log_result_t log_result[];   //ログ計算結果



void        log_title(void);
void        log_data_make(uint16_t);


#endif	//LOG_H