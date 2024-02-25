/*
 * measureTimeV5.c
 * 
 * 電子ターゲット
 * 測定と表示
 * 
 *  2022.04.16 measure.c
 * V2_edition
 *  2022.05.11  1-3番目に入力したセンサから計算する
 * V3_edition
 *  2023.01.20  wifi追加
 * V5_edition
 *  2024.01.5   センサ5個
 *  
 */

#include "header.h"
#include "measureTimeV5.h"

//DEBUG
#define COMP_DELAY_TIME_OFF //計算の検算中はコンパレータディレイタイムの合算は無し
//DEBUG (global)
//#define DEBUG_MEAS_no       //デバッグprintf表示(エラー系)   


//GLOBAL
const float   delay_a = (5.0 / 300);        //コンパレータオンの遅れ時間の計算係数 usec / mm
const float   delay_b = 10;                 //usec
sensor_data_t   sensor5Measure[NUM_SENSOR]= {
    {SENSOR1, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //左下
    {SENSOR2, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //右下
    {SENSOR3, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //左上
    {SENSOR4, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //右上
    {SENSOR5, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //右中
};


//LOCAL
typedef enum {
    INPUT_ORDER_STATUS_OK,
    INPUT_ORDER_STATUS_NO_INPUT,
    INPUT_ORDER_STATUS_FIXED,       
    INPUT_ORDER_STATUS_ERROR, 
    INPUT_ORDER_STATUS_SOURCE_NUM
} input_order_stat_sor_t;


void measureInit(void){
    //初期設定
    float       center_offset_x = 0.0;     //LCDセンターとセンサー配置のズレ
    float       center_offset_y = 0.0;
    
    correctSensorOffset(center_offset_x, center_offset_y);
    //log_title();  ////////////////////////////////////////////////////////////////////////
    
    //ターゲットLCDクリアコマンド送信
    ESP32slave_ClearCommand();
    
}


void    correctSensorOffset(float center_dx, float center_dy){
    //マト板のセンターラインとセンサー位置のズレを補正
    //センサーの物理位置のズレ補正
    float   sensor1_offset_x = 0;
    float   sensor1_offset_y = 0;

    float   sensor2_offset_x = 0;
    float   sensor2_offset_y = 0;

    float   sensor3_offset_x = 0;
    float   sensor3_offset_y = 0;

    float   sensor4_offset_x = 0;
    float   sensor4_offset_y = 0;
    
    float   sensor5_offset_x = 0;//ここをいじると一直線にならないので計算式が三角法のままになる。
    float   sensor5_offset_y = 0;
    
    float   sensor_offset_z = 0;    //z位置は同一平面状として計算式を作っているので、センサーごとの設定は不可。

    //x
    sensor5Measure[0].sensor_x_mm = -SENSOR_HORIZONTAL_SPACING + sensor1_offset_x + center_dx;
    sensor5Measure[1].sensor_x_mm =  SENSOR_HORIZONTAL_SPACING + sensor2_offset_x + center_dx;
    sensor5Measure[2].sensor_x_mm = -SENSOR_HORIZONTAL_SPACING + sensor3_offset_x + center_dx;
    sensor5Measure[3].sensor_x_mm =  SENSOR_HORIZONTAL_SPACING + sensor4_offset_x + center_dx;
    sensor5Measure[4].sensor_x_mm =  SENSOR_HORIZONTAL_SPACING + sensor5_offset_x + center_dx;  //三角配置の時eをゼロにしないためにはここに+0.1する。計算結果はは少し変わる。
    //y        
    sensor5Measure[0].sensor_y_mm = -SENSOR_VERTICAL_SPACING + sensor1_offset_y + center_dy;
    sensor5Measure[1].sensor_y_mm = -SENSOR_VERTICAL_SPACING + sensor2_offset_y + center_dy;
    sensor5Measure[2].sensor_y_mm =  SENSOR_VERTICAL_SPACING + sensor3_offset_y + center_dy;
    sensor5Measure[3].sensor_y_mm =  SENSOR_VERTICAL_SPACING + sensor4_offset_y + center_dy;
    sensor5Measure[4].sensor_y_mm =  SENSOR_CENTER_SPACING   + sensor5_offset_y + center_dy;
    //z
    sensor5Measure[0].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor_offset_z;
    sensor5Measure[1].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor_offset_z;
    sensor5Measure[2].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor_offset_z;
    sensor5Measure[3].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor_offset_z;
    sensor5Measure[4].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor_offset_z;

}



uint8_t measureMain(uint16_t shotCnt){
    //測定メイン～計算
    //shotCnt:ショット番号
    meas_stat_sor_t     measStat = MEASURE_STATUS_OK;
    calc_stat_sor_t     calcStat = CALC_STATUS_OK;
    
    //測定データ順を確認修正
    checkInputOrder();
    //測定データを計算、代入
    if (assignMeasureData() < 3){
        //測定数が足りない時(計算には3個以上のデータが必要)
        calcStat = CALC_STATUS_NOT_ENOUGH;
        resultError999(NUM_CAL, calcStat);
        
        measStat = MEASURE_STATUS_NOT_ENOUGH;
        ledLightOn(LED_PINK);
        return measStat;
    }

    //座標の計算
    calcStat = computeEpicenter();
    //printf("calc status:%d", ans);    
    
    if (CALC_STATUS_OK == calcStat){       
        //測定OK
        ledLightOn(LED_BLUE);
        
    }else{
        //エラーの時
        measStat = MEASURE_STATUS_ERROR;
        ledLightOn(LED_PINK);

    }
    
    return measStat;
}


//
uint8_t checkInputOrder(void){
    //入力順序の再確認修正
    //入力時間が近い場合に割込処理順が入れ替わることがある...複数の保留割込がある場合、処理順はIRQ#順になるため
    input_order_stat_sor_t status = INPUT_ORDER_STATUS_OK;
    uint8_t     i;
    uint8_t     sensor_number;             
    uint8_t     input_order[NUM_SENSOR];      //入力順計算結果
    
    for (sensor_number = 0; sensor_number < NUM_SENSOR; sensor_number++){
        input_order[sensor_number] = 0;
        
        for (i = 0; i < NUM_SENSOR; i++){
            if (sensor5Measure[sensor_number].timer_cnt > sensor5Measure[i].timer_cnt){
                //自分の値より小さい値があった時カウントする
                input_order[sensor_number]++;
                //同じ値があった時は同着ができてしまう......未処理////////////////////
            }
        }
        
        if (sensor5Measure[sensor_number].input_order == 0xff){
            //未検出センサは未検出のまま
#ifdef DEBUG_MEAS
            printf("Sensor%d is NO input!\n", (sensor_number + 1));   
#endif
            status = INPUT_ORDER_STATUS_NO_INPUT;
            ledLightOn(LED_CAUTION);
            
        }else if (sensor5Measure[sensor_number].input_order != input_order[sensor_number]){
            //修正があった場合
#ifdef DEBUG_MEAS
            printf("(S%d) input Order (%d -> %d) is changed correctly!\n", sensor_number, sensor5Measure[sensor_number].input_order, input_order[sensor_number]);
#endif
            status = INPUT_ORDER_STATUS_FIXED;
            ledLightOn(LED_CAUTION);
            sensor5Measure[sensor_number].input_order = input_order[sensor_number];
        }
        
    }
    return status;
}


uint8_t assignMeasureData(void){
    //測定データを代入
    //出力: 有効測定センサー数
    meas_stat_sor_t   sensorStat = SENSOR_STATUS_OK;
    uint8_t     SensNum;                //センサ番号 0-4
    uint8_t     firstSensor;            //最初にオンしたセンサーの番号
    uint8_t     validSensorCount = 0;   //有効測定センサー数のカウント
    
    for (SensNum = 0; SensNum < NUM_SENSOR; SensNum++){
        switch(sensor5Measure[SensNum].input_order){
            case 0:
                //最初にオンしたセンサー番号
                firstSensor = SensNum;
                
            case 1 ... (NUM_SENSOR - 1):
                validSensorCount ++;
                sensorStat = SENSOR_STATUS_OK;
                break;
                
            case 0xff:
                sensorStat = SENSOR_STATUS_INVALID;
                ledLightOn(LED_CAUTION);
                break;
                
            default:
                sensorStat = SENSOR_STATUS_ERROR;
                ledLightOn(LED_CAUTION);
               break;
        }
        sensor5Measure[SensNum].status = sensorStat;
    }
    
    //タイマカウント差、時間差、距離の計算と代入
    for (SensNum = 0; SensNum < NUM_SENSOR; SensNum++){
        if (SENSOR_STATUS_OK == sensor5Measure[SensNum].status){
            //ステータスOKのときだけ計算、代入
            sensor5Measure[SensNum].delay_cnt       = sensor5Measure[SensNum].timer_cnt - sensor5Measure[firstSensor].timer_cnt;                         //カウント差
            sensor5Measure[SensNum].delay_time_usec = delay_time_usec(sensor5Measure[SensNum].delay_cnt);                                               //カウント値→時間
            sensor5Measure[SensNum].distance_mm     = dist_delay_mm(sensor5Measure[SensNum].delay_time_usec - sensor5Measure[SensNum].comp_delay_usec); //時間→距離
            sensor5Measure[SensNum].comp_delay_usec = delay_comparator_usec(sensor5Measure[SensNum].delay_time_usec);                                   //コンパレータ応答遅れ簡易計算        
        }
    }

    return validSensorCount;
}


//
void clearData(void){
    //測定データをクリア
    uint8_t i;
    
    sensorCnt = 0;
    
    //測定値クリア
    for (i = 0; i < NUM_SENSOR; i++){
        sensor5Measure[i].input_order = 0xff;      //未入力判定用に0ではなくて0xff
        sensor5Measure[i].timer_cnt = 0xffffffff;  //タイム順を見る時のために最大の値にしておく
        sensor5Measure[i].delay_cnt = 0;
        sensor5Measure[i].delay_time_usec = 0;
        sensor5Measure[i].comp_delay_usec = 0;
        sensor5Measure[i].d_time_corr_usec = 0;
        sensor5Measure[i].distance_mm = 0;
        sensor5Measure[i].status = 0;
    }
   
    ICAP1_Enable();
    ICAP2_Enable();
    ICAP3_Enable();
    ICAP4_Enable();
    ICAP5_Enable();
    
    TMR2 = 0;
    TMR2_Start(); 
}



//*****time *******

float   dist_delay_mm(float time_usec){
    //時間を音速から距離に計算
    return  time_usec * v_air_mps() / 1000;
}


float   v_air_mps(void){
    //音速m/secを求める 
    return 331.5 + 0.61 * air_temp_degree_c;
}


float   delay_time_usec(uint32_t timer_count){
    //タイマーカウント値を実時間usecに変換
    return (float)timer_count / (TMR2_FrequencyGet() / 1000000);
}


float   delay_comparator_usec(float delay_time){
    //コンパレータ応答遅れ時間　ー　距離によって音が小さくなり遅れる
    //センサーが音を拾ってからコンパレータがオンするまでの遅れ時間usec
    //delay_time:着弾～センサオンまでの時間で代用。簡略化
    //本来はセンサ～着弾点の距離によるけれど、それやるには距離計算後にもう一度補正して距離を計算し直さないといけない
    
    float   correct_time;
    
//DEBUG   
#ifndef COMP_DELAY_TIME_OFF
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



//***** sensor interrupt ********************************************************

void detectSensor1(uintptr_t context){
    //センサ1割込
    impact_PT4_On();                                               //判定不要：最初のセンサオンでセットされるので2つ目以降のセンサで再セットされてもとくに影響は無い
    sensor5Measure[SENSOR1].timer_cnt = ICAP1_CaptureBufferRead();   //読み出しをしないと割込フラグをクリアできず、再割込が入ってしまう
    ICAP1_Disable();
    sensor5Measure[SENSOR1].input_order = sensorCnt;
    sensorCnt++;   
}


void detectSensor2(uintptr_t context){
    //センサ2割込
    impact_PT4_On();
    sensor5Measure[SENSOR2].timer_cnt = ICAP2_CaptureBufferRead();
    ICAP2_Disable();
    sensor5Measure[SENSOR2].input_order = sensorCnt;
    sensorCnt++;
}


void detectSensor3(uintptr_t context){
    //センサ3割込
    impact_PT4_On();
    sensor5Measure[SENSOR3].timer_cnt = ICAP3_CaptureBufferRead();
    ICAP3_Disable();
    sensor5Measure[SENSOR3].input_order = sensorCnt;
    sensorCnt++;
}


void detectSensor4(uintptr_t context){
    //センサ4割込
    impact_PT4_On();
    sensor5Measure[SENSOR4].timer_cnt = ICAP4_CaptureBufferRead();
    ICAP4_Disable();
    sensor5Measure[SENSOR4].input_order = sensorCnt;
    sensorCnt++;
}


void detectSensor5(uintptr_t context){
    //センサ5割込
    impact_PT4_On();
    sensor5Measure[SENSOR5].timer_cnt = ICAP5_CaptureBufferRead();
    ICAP5_Disable();
    sensor5Measure[SENSOR5].input_order = sensorCnt;
    sensorCnt++;
}



/*******************************************************************************
 End of File
*/