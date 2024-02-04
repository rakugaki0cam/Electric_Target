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
#ifndef CONPUTE_EPICENTER_V5_H
#define	CONPUTE_EPICENTER_V5_H


//GLOBAL
//計算パターン数
#define     NUM_CAL         10      //センサー5個から3個選ぶパターンの数 10    5C3 = 5x4x3 / 3x2x1
#define     NUM_GROUP       5       //センサー4個でのばらつきを見るグループ数　


//センサーステータス
typedef enum {
    SENSOR_STATUS_OK,       //OK
    SENSOR_STATUS_INVALID,  //データ無効     
    SENSOR_STATUS_ERROR,    //エラー  
    SENSOR_STAT_SOURCE_NUM
} sensor_stat_sor_t;


//測定ステータス
typedef enum {
    MEASURE_STATUS_OK,
    MEASURE_STATUS_NOT_ENOUGH,       
    MEASURE_STATUS_ERROR,
    MEASURE_STATUS_SOURCE_NUM
} meas_stat_sor_t;


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

extern impact_result_t  calcValue[];    //座標の計算
extern impact_result_t  calcResult;     //座標の計算の最終結果


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

extern result_ave_t     vari5Groupe[];    //結果判定用ばらつきの計算





//
void            clearResult(uint8_t);
void            resultError999(uint8_t, calc_stat_sor_t);
//
calc_stat_sor_t computeEpicenter(void);
calc_stat_sor_t computeXY(uint8_t);
//
uint8_t         select3sensor(uint8_t, sensor_data_t*);
uint16_t        sensorOrderPattern(uint8_t);
uint16_t        sensorGroupePattern(uint8_t);
uint8_t         firstSensor(void);
uint8_t         check_deviation(void);
//calculation
uint8_t         apollonius3circleXYR(uint8_t, sensor_data_t*);


#endif	//CONPUTE_EPICENTER_V5_H

