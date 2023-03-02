/*
 * calc_locate.c
 * 
 * 電子ターゲット
 * 座標の計算
 * 
 *  2022.03.24
 * V2_edition
 *  2022.05.11  1-3番目に入力したセンサから計算する
 *  
 * 
 * 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include "calc_locate.h"


//GLOBAL
sensor_data_t   sensor_4mic[NUM_SENSOR]= {
    {SENSOR1, 0xff, -SENSOR_HORIZONTAL_SPACING, -SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //センサー1 左下
    {SENSOR2, 0xff,  SENSOR_HORIZONTAL_SPACING, -SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //センサー2 右下
    {SENSOR3, 0xff, -SENSOR_HORIZONTAL_SPACING,  SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //センサー3 左上
    {SENSOR4, 0xff,  SENSOR_HORIZONTAL_SPACING,  SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //センサー4 右上
};

impact_data_t   result;         //計算結果

const float   delay_a = (5.0 / 300);    //センサー遅れ時間の計算係数
const float   delay_b = 10;


//LOCAL
sensor_data_t   tmp_3[3];       //センサデータ受け渡し用


//
calc_status_source_t calc_locate_xy(void){
    //着弾座標の計算
    //出力　result:計算結果座標x,y,r  グローバル
    //     stat:0-正常終了　1-エラー
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    uint8_t     i;
    
    //センサー遅れ時間の計算
    for (i = 0; i < NUM_SENSOR; i++){
        sensor_4mic[i].sensor_delay_usec = delay_sensor_usec(sensor_4mic[i].delay_time_usec);
    }   
    
    //計算結果クリア
    result.pattern = 0;
    result.impact_pos_x_mm = 0;
    result.impact_pos_y_mm = 0;
    result.radius0_mm = 0;
    result.delay_time0_msec = 0;
    result.status = 0;

    calc_stat = calc_of_3sensor();   //座標を計算
    if (CALC_STATUS_OK == calc_stat){
        //計算OK
        //着弾からセンサオンまでの遅れ時間(着弾時刻計算用)
        result.delay_time0_msec = impact_time_msec(result.radius0_mm); 
    }else {
        //計算値が不可だった場合
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.delay_time0_msec = 0;        //タマモニでのエラー判定に使用
        //calc_stat = CALC_STATUS_CAL_ERROR;
    }
            
    return calc_stat;
}


//
calc_status_source_t calc_of_3sensor(void){
    //tmp_3の3センサデータから座標値を計算
    //Output result:計算値
    //       calc_stat:状態　0-OK,
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    
    //tmp_3に計算用データ代入
    if (asign_3sensor() != 0){ 
        //代入するデータがダメな時
        calc_stat = CALC_STATUS_NOT_ENOUGH;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.status = calc_stat;
        return calc_stat;
    }

    //使用センサーの番号を数字化
    calc_sensor_pattern();
    
    //座標の計算
    if (apollonius_3circle_xyr() != 0){
        //計算がダメなとき
        calc_stat = CALC_STATUS_CAL_ERROR;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.status = calc_stat;
        return calc_stat;
    }
    
    //計算結果の判定
    if (result.radius0_mm > R_MAX){
        //rが大きすぎる
        calc_stat = CALC_STATUS_R0_ERR;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.status = calc_stat;
        return calc_stat;
    }
    if ((result.impact_pos_x_mm < -TARGET_WIDTH_HALF) || (result.impact_pos_x_mm > TARGET_WIDTH_HALF)){
        //xが大きすぎる
        calc_stat = CALC_STATUS_X0_ERR;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.status = calc_stat;
        return calc_stat;
    }
    if ((result.impact_pos_y_mm < -TARGET_HEIGHT_HALF) || (result.impact_pos_y_mm > TARGET_HEIGHT_HALF)){
        //yが大きすぎる
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        calc_stat = CALC_STATUS_Y0_ERR;
        result.status = calc_stat;
        return calc_stat;
    }
    return calc_stat;
}


uint8_t asign_3sensor(void){
    //センサ3ケを選択してtmp_3へ代入
    //出力tmp_3-計算用の3組のデータ
    //stat 0:OK, 1:代入不可
    uint8_t i;
    uint8_t n = 0;
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
   
    //測定値代入
    for (i = 0; i < NUM_SENSOR; i++){
        if (sensor_4mic[i].status == SENSOR_STATUS_OK){
            if (sensor_4mic[i].input_order < 3){
                tmp_3[n] = sensor_4mic[i];
                n++;
                if (n > 3){
                    calc_stat = CALC_STATUS_TOO_MANY;
                    result.status = calc_stat;
                    return calc_stat;
                }
            }
        //一括代入 .sensor_x, .sensor_y, .sensor_z, .distance_mm
        } else{
            //データがダメ
            calc_stat = CALC_STATUS_NOT_ENOUGH;
            result.status = calc_stat;
            return calc_stat;
        }
    }
    if (n != 3){
        calc_stat = CALC_STATUS_NOT_ENOUGH;
        result.status = calc_stat;
    }
    return calc_stat;
}


void    calc_sensor_pattern(void){
    //使用センサナンバーを3桁の数字にして記録
    result.pattern = (tmp_3[0].sensor_num + 1) * 0x100
                   + (tmp_3[1].sensor_num + 1) * 0x10
                   + (tmp_3[2].sensor_num + 1);
}


//*** calculation sub *****************
//locate
uint8_t apollonius_3circle_xyr(void){
    //座標の計算
    //リターン値　0:正常
    //          1:エラー
    //入力値 tmp_3[3]: センサーデータ x、y、z、遅れ距離
    //出力値 result: 座標x、yと距離r0の計算値
    
    uint8_t i;
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    
    float   x[4], y[4], z[4], dr[4];
    float   a[4], b[4], c[4], d[4];
    float   e, f[3], g[3];
    float   aa, bb, cc;
    
    //データ代入
    for (i = 0; i < 3; i++){
        x[i + 1] = tmp_3[i].sensor_x_mm;
        y[i + 1] = tmp_3[i].sensor_y_mm;
        z[i + 1] = tmp_3[i].sensor_z_mm;
        dr[i + 1] = tmp_3[i].distance_mm;
    }
    
    //定数の計算
    a[1] = x[2] - x[1];
    a[2] = x[3] - x[2];
    a[3] = x[1] - x[3];
    b[1] = y[2] - y[1];
    b[2] = y[3] - y[2];
    b[3] = y[1] - y[3];
    c[1] = dr[2] - dr[1];
    c[2] = dr[3] - dr[2];
    c[3] = dr[1] - dr[3];
    d[1] = (0 - x[1] * x[1] + x[2] * x[2] - y[1] * y[1] + y[2] * y[2] + dr[1] * dr[1] - dr[2] * dr[2]) / 2;
    d[2] = (0 - x[2] * x[2] + x[3] * x[3] - y[2] * y[2] + y[3] * y[3] + dr[2] * dr[2] - dr[3] * dr[3]) / 2;
    d[3] = (0 - x[3] * x[3] + x[1] * x[1] - y[3] * y[3] + y[1] * y[1] + dr[3] * dr[3] - dr[1] * dr[1]) / 2;
    e = a[1] * b[2] - a[2] * b[1];
    if (e == 0){
        //error分母がゼロ
        //e = 0.00001;
        calc_stat = CALC_STATUS_E_0_1;
        result.status = calc_stat;
        return calc_stat;
    }
    f[1] = (b[1] * c[2] - b[2] * c[1]) / e;
    f[2] = (a[2] * c[1] - a[1] * c[2]) / e;
    g[1] = (b[2] * d[1] - b[1] * d[2]) / e;
    g[2] = (a[1] * d[2] - a[2] * d[1]) / e;

    //解の公式
    aa = f[1] * f[1] + f[2] * f[2] - 1;
    bb = 2 * f[1] * (g[1] - x[1]) + 2 * f[2] * (g[2] - y[1]) - 2 * dr[1];
    cc = (g[1] - x[1]) * (g[1] - x[1]) + (g[2] - y[1]) * (g[2] - y[1]) + z[0] * z[0] - dr[1] * dr[1];
    
    if ((bb * bb - 4 * aa * cc) < 0){
        //error　解の公式の条件
        calc_stat = CALC_STATUS_QUAD_F;
        result.status = calc_stat;
        return calc_stat;
    }
    
    //二次方程式を解いてx,y,rを求める計算
    result.radius0_mm = (-bb + sqrt(bb * bb - 4 * aa * cc)) / (2 * aa);
    if (result.radius0_mm < 0){
        //半径が負の方は不採用
        result.radius0_mm = (-bb - sqrt(bb * bb - 4 * aa * cc)) / (2 * aa);
    }
    
    result.impact_pos_x_mm = f[1] * result.radius0_mm + g[1];
    result.impact_pos_y_mm = f[2] * result.radius0_mm + g[2];
    
    return calc_stat;
}


float   dist_delay_mm(float time_usec){
    //音速から距離の計算
    return  time_usec * v_air_mps() / 1000;
}


//time
float   v_air_mps(void){
    //音速m/secを求める 
    return 331.5 + 0.61 * temp_ave_degree_c;
}


float   delay_time_usec(uint32_t timer_count){
    //タイマーカウント値を実時間usecに変換
    return (float)timer_count / (TMR2_FrequencyGet() / 1000000);
}


float   delay_sensor_usec(float delay_time){
    //センサーが音を拾ってからコンパレータがオンするまでの遅れ時間usec
    //delay_time:着弾～センサオンまでの時間で代用。簡略化
    float   correct_time;

    correct_time = delay_a * delay_time + delay_b;  //補正係数　dtは10～16くらいtimeは0～300usec
    return correct_time;
}


float   impact_time_msec(float r0_mm){
    //r0から着弾時刻を推定計算
    //着弾時刻は最初のセンサオンより前の時刻
    //塩ビ板t2の中を伝わる時間(表に玉が当たり、裏面に伝わる時間)
#define TARGET_PLATE_T_MM   2       //軟質塩ビのマト板の厚さ mm
#define V_PLATE_MPS         2300    //軟質塩ビ板中の音速 m/sec(= mm/msec)
    float   dt_t2_msec;
    
    dt_t2_msec = TARGET_PLATE_T_MM / V_PLATE_MPS;   //msec
    
    return  -r0_mm / v_air_mps() + dt_t2_msec;      //msec
}


