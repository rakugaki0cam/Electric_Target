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
 * V4＿edition
 *  2023.08.26  4通りの計算値からエラーを検出
 *              1つのセンサー値が異常の場合、4つの座標がバラバラになる。
 *              → 正しい座標は異常値を抜いた他の3つのセンサから求める1つだけの座標。どれが正しい座標なのかの判定が難しい。
 * 
 * 
 */

#include "calc_locate.h"


#define     CALC_TEST_no   //計算テスト = ディレイを計算しない

//GLOBAL
//あとでmeasure_v3.cへ移動
//こちらには諸元的なものはなしとする
sensor_data_t   sensor_4mic[NUM_SENSOR]= {
    {SENSOR1, 0xff, -SENSOR_HORIZONTAL_SPACING, -SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //センサー1 左下
    {SENSOR2, 0xff,  SENSOR_HORIZONTAL_SPACING, -SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //センサー2 右下
    {SENSOR3, 0xff, -SENSOR_HORIZONTAL_SPACING,  SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //センサー3 左上
    {SENSOR4, 0xff,  SENSOR_HORIZONTAL_SPACING,  SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //センサー4 右上
};

impact_data_t   result[NUM_PATTERN + 1];    //計算結果 最後のところに平均を収納するため+1

const float   delay_a = (5.0 / 300);        //センサー遅れ時間の計算係数
const float   delay_b = 10;


//LOCAL
sensor_data_t   tmp_3[3];       //センサデータ受け渡し用

uint8_t sensor_order[NUM_PATTERN][3] = {        //4センサから3ケを選択する4つのパターン
    { SENSOR1, SENSOR2, SENSOR3},
    { SENSOR1, SENSOR2, SENSOR4},
    { SENSOR1, SENSOR3, SENSOR4},
    { SENSOR2, SENSOR3, SENSOR4}
};


//
calc_status_source_t calc_locate_xy(void){
    //着弾座標の計算
    //出力　result:計算結果座標x,y,r  グローバル
    //     stat:0-正常終了　1-エラー
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    uint8_t     i, pattern;
    uint8_t     average_cnt = 0;        //平均値サンプル数のカウント
    
    //センサー遅れ時間の計算
    for (pattern = 0; pattern < NUM_PATTERN; pattern++){
        sensor_4mic[pattern].sensor_delay_usec = delay_sensor_usec(sensor_4mic[pattern].delay_time_usec);
    }   
    
    //計算結果クリア
    for (i = 0; i < (NUM_PATTERN + 1); i++){
        clear_result(i);
    }
    result[NUM_PATTERN].pattern = 0xffff;   //平均値識別マーク

    //パターン回数分を計算
    for (pattern = 0; pattern < NUM_PATTERN; pattern++){
        calc_stat = calc_of_3sensor(pattern);   //座標を計算
        if (CALC_STATUS_OK == calc_stat){
            //計算OK
            //平均のための積算-[NUM_PATTERN]に総和を入れる
            result[NUM_PATTERN].impact_pos_x_mm += result[pattern].impact_pos_x_mm;
            result[NUM_PATTERN].impact_pos_y_mm += result[pattern].impact_pos_y_mm; 
            result[NUM_PATTERN].radius0_mm      += result[pattern].radius0_mm; 
            average_cnt ++;
        }else {
            //計算値が不可だった場合
            result[pattern].radius0_mm = 999.99;
            result[pattern].impact_pos_x_mm = 999.99;
            result[pattern].impact_pos_y_mm = 999.99;
            result[pattern].delay_time0_msec = 0;        //タマモニでのエラー判定に使用
            //calc_stat = CALC_STATUS_CAL_ERROR;
            //平均計算に含めない
        }
    }
    
    //平均の計算 result配列の最後に平均値を代入
    if (average_cnt == 0){
        calc_stat = CALC_STATUS_AVERAGE_ERR;
        result[NUM_PATTERN].status = calc_stat;
        return calc_stat;
    }
    result[NUM_PATTERN].impact_pos_x_mm = result[NUM_PATTERN].impact_pos_x_mm / average_cnt;
    result[NUM_PATTERN].impact_pos_y_mm = result[NUM_PATTERN].impact_pos_y_mm / average_cnt;
    result[NUM_PATTERN].radius0_mm      = result[NUM_PATTERN].radius0_mm      / average_cnt;
    
    //偏差のチェック
    calc_stat = check_deviation();
    if ((calc_stat != CALC_STATUS_OK) && (calc_stat != CALC_STATUS_AVERAGE_FIX)){
        return calc_stat;
    }
    
    //着弾からセンサオンまでの遅れ時間 平均値(着弾時刻計算用)
    result[NUM_PATTERN].delay_time0_msec = impact_time_msec(result[NUM_PATTERN].radius0_mm);      
       
    return calc_stat;
}

void    clear_result(uint8_t n){
    //計算結果をクリア
    result[n].pattern = 0;
    result[n].impact_pos_x_mm = 0;
    result[n].impact_pos_y_mm = 0;
    result[n].radius0_mm = 0;
    result[n].delay_time0_msec = 0;
    result[n].status = 0;
}

//
calc_status_source_t calc_of_3sensor(uint8_t n){
    //tmp_3の3センサデータから座標値を計算
    //Input  n:パターン番号    
    //Output result:計算値
    //       calc_stat:状態　0-OK,
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    
    //tmp_3に計算用データ代入
    if (asign_3sensor(n) != 0){ 
        //代入するデータがダメな時
        calc_stat = CALC_STATUS_NOT_ENOUGH;
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        result[n].status = calc_stat;
        return calc_stat;
    }

    //使用センサーの番号を数字化
    calc_sensor_pattern(n);
    
    //座標の計算
    if (apollonius_3circle_xyr(n) != 0){
        //計算がダメなとき
        calc_stat = CALC_STATUS_CAL_ERROR;
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        result[n].status = calc_stat;
        return calc_stat;
    }
    
    //計算結果の判定
    if (result[n].radius0_mm > R_MAX){
        //rが大きすぎる
        calc_stat = CALC_STATUS_R0_ERR;
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        result[n].status = calc_stat;
        return calc_stat;
    }
    if ((result[n].impact_pos_x_mm < -TARGET_WIDTH_HALF) || (result[n].impact_pos_x_mm > TARGET_WIDTH_HALF)){
        //xが大きすぎる
        calc_stat = CALC_STATUS_X0_ERR;
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        result[n].status = calc_stat;
        return calc_stat;
    }
    if ((result[n].impact_pos_y_mm < -TARGET_HEIGHT_HALF) || (result[n].impact_pos_y_mm > TARGET_HEIGHT_HALF)){
        //yが大きすぎる
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        calc_stat = CALC_STATUS_Y0_ERR;
        result[n].status = calc_stat;
        return calc_stat;
    }
    return calc_stat;
}


uint8_t asign_3sensor(uint8_t n){
    //センサ3ケを選択してtmp_3へ代入
    //出力tmp_3-計算用の3組のデータ
    //stat 0:OK, 1:代入不可
    uint8_t i;
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
   
    //測定値代入
    for (i = 0; i < 3; i++){
        if (sensor_4mic[sensor_order[n][i]].status == SENSOR_STATUS_OK){
            tmp_3[i] = sensor_4mic[sensor_order[n][i]];
        //一括代入 .sensor_x, .sensor_y, .sensor_z, .distance_mm
        } else{
            //データがダメ
            calc_stat = CALC_STATUS_NOT_ENOUGH;
            result[n].status = calc_stat;
            return calc_stat;
        }
    }
    
    return calc_stat;
}


void    calc_sensor_pattern(uint8_t n){
    //使用センサナンバーを3桁の数字にして記録
    result[n].pattern = (tmp_3[0].sensor_num + 1) * 0x100
                   + (tmp_3[1].sensor_num + 1) * 0x10
                   + (tmp_3[2].sensor_num + 1);
}

uint8_t check_deviation(void){
    //偏差のチェック 平均との差が大きいものを除く
    //平均値を再計算
    
#define TARGET_SIZE_A4
#ifdef  TARGET_SIZE_A4
    #define     DEVIATION_XY    2.0                 //偏差 xy
    #define     DEVIATION_R     3.0                 //偏差 r
#else   //TARGET_SIZE_A3 
    #define     DEVIATION_XY    100.0               ///////////////////////TEST
    #define     DEVIATION_R     200.0
#endif
    
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    uint8_t     pattern;
    uint8_t     average_cnt = 0;        //平均値サンプル数のカウント
    float       dx, dy, dr;             //偏差
    
    for (pattern = 0; pattern < NUM_PATTERN; pattern++){
        calc_stat = result[pattern].status;
        if(calc_stat == CALC_STATUS_OK){
            //偏差の計算
            dx = result[pattern].impact_pos_x_mm - result[NUM_PATTERN].impact_pos_x_mm;
            dy = result[pattern].impact_pos_y_mm - result[NUM_PATTERN].impact_pos_y_mm;
            dr = result[pattern].radius0_mm      - result[NUM_PATTERN].radius0_mm;

            if((dx < -DEVIATION_XY) || (dx > DEVIATION_XY)){
                calc_stat = CALC_STATUS_X0_DEV_ERR;
            }
            if((dy < -DEVIATION_XY) || (dy > DEVIATION_XY)){
                calc_stat = CALC_STATUS_Y0_DEV_ERR;
            }
            if((dr < -DEVIATION_R) || (dr > DEVIATION_R)){
                calc_stat = CALC_STATUS_R0_DEV_ERR;
            }
            if (calc_stat != CALC_STATUS_OK){
                //偏差エラーがある時
                result[pattern].status = calc_stat;
            }
        }
    }
    
    if (calc_stat != CALC_STATUS_OK){
        //偏差エラーがある時は偏差データを除外して平均を取り直し
        
        //平均用メモリをクリア
        clear_result(NUM_PATTERN);
        result[NUM_PATTERN].pattern = 0xffff;       //平均値識別マーク
        
        
        //平均のための積算やりなおし
        for (pattern = 0; pattern < NUM_PATTERN; pattern++){
            if (result[pattern].status == CALC_STATUS_OK){
                result[NUM_PATTERN].impact_pos_x_mm += result[pattern].impact_pos_x_mm;
                result[NUM_PATTERN].impact_pos_y_mm += result[pattern].impact_pos_y_mm; 
                result[NUM_PATTERN].radius0_mm      += result[pattern].radius0_mm; 
                average_cnt ++;
            }
        }
        
        //サンプル数チェック
        if (average_cnt == 0){
            calc_stat = CALC_STATUS_AVERAGE_ERR;
            result[NUM_PATTERN].status = calc_stat;
            return calc_stat;
        }
        
        result[NUM_PATTERN].impact_pos_x_mm = result[NUM_PATTERN].impact_pos_x_mm / average_cnt;
        result[NUM_PATTERN].impact_pos_y_mm = result[NUM_PATTERN].impact_pos_y_mm / average_cnt;
        result[NUM_PATTERN].radius0_mm      = result[NUM_PATTERN].radius0_mm      / average_cnt;
        
        calc_stat = CALC_STATUS_AVERAGE_FIX;        //平均値の再計算が行われた
        result[NUM_PATTERN].status = calc_stat;
    }
    return calc_stat;
}


//*** calculation sub *****************
//locate
uint8_t apollonius_3circle_xyr(uint8_t num_result){
    //座標の計算
    //リターン値　0:正常
    //          1:エラー
    //入力値 tmp_3[3]: センサーデータ x、y、z、遅れ距離
    //出力値 result[]: 座標x、yと距離r0の計算値
    //      num_result: 結果を代入する番号

    uint8_t i;
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    
    float   x[4], y[4], z[4], dr[4];    //添字は1,2,3を使用
    float   a[4], b[4], c[4], d[4];
    float   e, f[3], g[3];
    float   aa, bb, cc;
    float   q, r0;
    
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
        result[num_result].status = calc_stat;
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
    
    q = bb * bb - 4 * aa * cc;
    if (q < 0){
        //error　解の公式の条件
        calc_stat = CALC_STATUS_QUAD_F;
        result[num_result].status = calc_stat;
        return calc_stat;
    }
    
    //二次方程式を解いてx,y,rを求める計算
    r0 = (-bb - sqrt(q)) / (2 * aa);
    if (r0 < 0){
        //半径が負の方は不採用
        r0 = (-bb + sqrt(q)) / (2 * aa);
    }
    result[num_result].radius0_mm = r0;
    result[num_result].impact_pos_x_mm = f[1] * r0 + g[1];
    result[num_result].impact_pos_y_mm = f[2] * r0 + g[2];
    
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
#ifndef CALC_TEST
    correct_time = delay_a * delay_time + delay_b;  //補正係数　dtは10～16くらいtimeは0～300usec
#else
    correct_time = 0;
#endif    
    
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


