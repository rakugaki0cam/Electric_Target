/*
 * mcp9700a_temp.c
 * 
 * 温度センサー
 * 
 *   0℃  0.5V
 * 100℃  1.5V
 * -40～125℃ ±2℃
 * 
 * 
 *  2022.04.08
 * 
 */

#include "mcp9700a_temp.h"

#define     NUM_TEMP_DATA_BUFFER    4   //バッファ数
#define     NUM_SAMPLE_2_x          2   //4 = 2 ^ 2←

//GLOBAL
float       temp_ave_degree_c;          //気温　℃
        
//平均を取るためのデータバッファ
uint16_t    temp_data[NUM_TEMP_DATA_BUFFER];



void    init_temp(void){
    //温度センサ初期化
    //初期データを埋めておく
    uint8_t     i;
    
    for(i = 0; i < NUM_TEMP_DATA_BUFFER; i++){
        get_temp_adc();
        CORETIMER_DelayMs(100);
    }
    set_temp_degree_c();
}


float   set_temp_degree_c(void){
    //温度の計測
    //結果はグローバル変数に代入
#define     AVDD    3.3
#define     ADC_BIT 1024    //10bit
    float   v;              //センサー電圧
    uint16_t adc_count;
    
    adc_count = average_temp_adc();
    v = (float)AVDD * adc_count / ADC_BIT;
    
    temp_ave_degree_c = calc_temp(v);
    
    return temp_ave_degree_c;
}


float   calc_temp(float adc_v){
    //電圧から温度への変換 & 誤差補正計算
    //in  電圧
    //out 温度
#define     T_MAX       125
#define     T_MIN       -40    
#define     EC2         (float)(-244 * 0.000001)
#define     EC1         (float)(2 * 0.000000000001)
#define     ERROR_T_MIN 2
    
#define     OFFSET_TEMP (float)-1.7         //基板の熱の影響がある     /////////////////////////
    
    float   temp_sensor_c;
    float   temp_compensated;

    //電圧→度への変換
    temp_sensor_c = (adc_v - 0.5 ) * 100;

    //誤差補正計算
    temp_compensated = temp_sensor_c
                     + EC2 * (T_MAX - temp_sensor_c) * (temp_sensor_c - T_MIN)
                     + EC1 * (temp_sensor_c - T_MIN)
                     + ERROR_T_MIN
                     + OFFSET_TEMP;
    return temp_compensated;
}


void     get_temp_adc(void){
    //温度のADカウント値を収納
    static uint8_t cnt;     //バッファの位置
    //AD変換
    ADC_SamplingStart();
    while(!ADC_ResultIsReady()){
    }
    temp_data[cnt] = ADC_ResultGet(ADC_RESULT_BUFFER_0);
    
    cnt++;
    if (cnt > NUM_TEMP_DATA_BUFFER){
        cnt = 0;
    } 
}


uint16_t    average_temp_adc(void){
    //温度データADカウント値の平均値
    uint8_t     i;
    uint32_t    sum = 0;
    
    for(i = 0; i < NUM_TEMP_DATA_BUFFER; i++){
        sum += temp_data[i];
    }
    sum = sum >> NUM_SAMPLE_2_x;         //サンプル数　2のべき乗
    
    return (uint16_t)sum;
}

