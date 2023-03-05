/*
 * measure_v3.c
 * 
 * 電子ターゲット
 * 測定と表示
 * 
 *  2022.04.16
 * V2_edition
 *  2022.05.11  1-3番目に入力したセンサから計算する
 * V3_edition
 *  2023.01.20  wifi追加
 * 
 *  
 */

#include "measure_v3.h"

//GLOBAL


//LOCAL
typedef enum {
    INPUT_ORDER_STATUS_OK    = 0x0,
    INPUT_ORDER_STATUS_FIXED = 0x1,       
    INPUT_ORDER_STATUS_ERROR = 0x2,        
} input_order_status_source_t;


void measure_init(void){
    //初期設定
    float       center_offset_x = -0.1;     //マト板センターラインとセンサーのズレ///////////////////////////
    float       center_offset_y = +1.0;
    
    sensor_offset_calc(center_offset_x, center_offset_y);
    log_title();
    
    //ターゲットLCDクリアコマンド送信
    uint8_t clear_command[] = "TARGET_CLEAR END ,";
    while(!UART2_TransmitterIsReady());
    UART2_Write(clear_command, strlen((char*)clear_command));
    
}


uint8_t measure_main(void){
    //測定メイン～計算
    measure_status_source_t     meas_stat = MEASURE_STATUS_OK;
    calc_status_source_t        ans;
    //測定データ順を確認修正
    input_order_check();
    //測定データを計算、代入
    if (measure_data_assign() < 3){
        //測定数が足りない時(計算には3個以上のデータが必要)
        meas_stat = MEASURE_STATUS_NOT_ENOUGH;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.delay_time0_msec = 0;        //タマモニでのエラー判定に使用
        return meas_stat;
    }
    //座標の計算
    ans = calc_locate_xy();
    //printf("calc status:%d", ans);
    if (CALC_STATUS_OK != ans){       
        //エラーの時
        meas_stat = MEASURE_STATUS_ERROR;
        
    }
    
    return meas_stat;
}


//
uint8_t input_order_check(void){
    //入力順序の再確認修正
    //入力時間が近い場合に割込処理順が入れ替わることがある...複数の保留割込がある場合、処理順はIRQ#順になるため
    input_order_status_source_t status = INPUT_ORDER_STATUS_OK;
    uint8_t     i;
    uint8_t     sensor_number;             
    uint8_t     input_order[NUM_SENSOR];      //入力順計算結果
    
    for (sensor_number = 0; sensor_number < NUM_SENSOR; sensor_number++){
        input_order[sensor_number] = 0;
        
        for (i = 0; i < NUM_SENSOR; i++){
            if (sensor_4mic[sensor_number].timer_cnt > sensor_4mic[i].timer_cnt){
                //自分の値より小さい値があった時カウントする
                input_order[sensor_number]++;
                //同じ値があった時は同着ができてしまう......未処理////////////////////
            }
        }
        
        if (sensor_4mic[sensor_number].input_order != 0xff){
            //未検出センサは未検出のまま
            if (sensor_4mic[sensor_number].input_order != input_order[sensor_number]){
                //修正があった場合
                sensor_4mic[sensor_number].input_order = input_order[sensor_number];
                status = INPUT_ORDER_STATUS_FIXED;
            }
        }
    }
    return status;
}


uint8_t measure_data_assign(void){
    //測定データを代入
    //出力: 有効測定センサー数
    measure_status_source_t   sensor_status = SENSOR_STATUS_OK;
    uint8_t     sensor_number;
    uint8_t     zero;                       //最初にオンしたセンサーの番号
    uint8_t     valid_sensor_count = 0;     //有効測定センサー数のカウント
    
    for (sensor_number = 0; sensor_number < NUM_SENSOR; sensor_number++){
        switch(sensor_4mic[sensor_number].input_order){
            case 0:
                //最初にオンしたセンサー番号
                zero = sensor_number;
                
            case 1 ... (NUM_SENSOR - 1):
                valid_sensor_count ++;
                sensor_status = SENSOR_STATUS_OK;
                break;
                
            case 0xff:
                sensor_status = SENSOR_STATUS_INVALID;
                break;
                
            default:
                sensor_status = SENSOR_STATUS_ERROR;
                break;
        }
        sensor_4mic[sensor_number].status = sensor_status;
    }
    
    //タイマカウント差、時間差、距離の計算と代入
    for (sensor_number = 0; sensor_number < NUM_SENSOR; sensor_number++){
        if (SENSOR_STATUS_OK == sensor_4mic[sensor_number].status){
            //ステータスOKのときだけ計算、代入
            sensor_4mic[sensor_number].delay_cnt 
                = sensor_4mic[sensor_number].timer_cnt - sensor_4mic[zero].timer_cnt;   //カウント差
            sensor_4mic[sensor_number].delay_time_usec
                = delay_time_usec(sensor_4mic[sensor_number].delay_cnt);                //カウント値→時間
            sensor_4mic[sensor_number].distance_mm
                = dist_delay_mm(sensor_4mic[sensor_number].delay_time_usec
                - sensor_4mic[sensor_number].sensor_delay_usec);   //時間→距離
        }
        
    }

    return valid_sensor_count;
}


//座標データをデバッガとLCD&WiFiに出力
void result_disp(uint16_t shot_count, measure_status_source_t meas_stat, uint8_t mode){ 
    //測定結果、計算結果の表示
    
    esp32wifi_data_send();  //着弾表示を早くする
    while(!UART2_TransmitComplete());
    
    CORETIMER_DelayMs(100); //RS485へ送信の前に間を開けるタマモニからデバッガへprintfしているかもしれないため
    tamamoni_data_send();   //LAN
    
    CORETIMER_DelayMs(30);  //データ送信の間をつくる
    switch(mode){
        case NONE:
            break;
        case SINGLE_LINE:
            single_line(shot_count, meas_stat);
            break;
        case FULL_DEBUG:
            full_debug(shot_count, meas_stat);
            break;
        case CSV_DATA:
            csv_data(shot_count);
            break;
        default:
            //error
            break;
    }    
}


//
void tamamoni_data_send(void){
    //タマモニへ座標データ他を送信
    printf("BINX0Y0dT %8.3f %8.3f %8.4f END ,", result.impact_pos_x_mm, result.impact_pos_y_mm, result.delay_time0_msec);
}


void esp32wifi_data_send(void){
    //ESP32へ座標データ他を送信
    //WiFi & target LCD
    char    buf[255];

    sprintf(buf, "BINX0Y0dT %8.3f %8.3f %8.4f END ,", result.impact_pos_x_mm, result.impact_pos_y_mm, result.delay_time0_msec);
    while(!UART2_TransmitterIsReady());
    UART2_Write(buf, strlen(buf));
}


//DEBUGger
void single_line(uint16_t shot_count, measure_status_source_t meas_stat){
    //座標のみ　1行表示
    
    if (meas_stat != MEASURE_STATUS_OK){
        printf("%02d:measurement error\n", shot_count);
        return;
    }else {
        printf("%02d:%6.1fx %6.1fy\n", shot_count, result.impact_pos_x_mm, result.impact_pos_y_mm);
    }
}


void full_debug(uint16_t shot_count, measure_status_source_t meas_stat){
    //デバッグ用フル表示
    sensor_status_source_t    sens_stat;
    calc_status_source_t    calc_stat;
    uint8_t i, n;
    
    printf("\n>shot%03d ***%5.1f%cC\n", shot_count, temp_ave_degree_c, 0xdf);
    
#ifndef  SENSOR_DATA_DEBUG_TIMER_VALUE
    //入力順タイマーカウント表示 
    for (i = 0; i < NUM_SENSOR; i++){
        for (n = 0; n < NUM_SENSOR; n++){
            if (sensor_4mic[n].input_order == i){
            //入力順番号が一致するまで送る
                break;
            }
        }
        printf("t%1d %08x (", i + 1, sensor_4mic[n].timer_cnt);  //入力順タイマ値 

        if (n == 4){
            printf("--)\n");    //入力なし

        }else {
            printf("s%1d)\n", n + 1);  //センサ#
        }
    }
#endif         
    for (i = 0; i < NUM_SENSOR; i++){
        sens_stat = sensor_4mic[i].status;
        if (SENSOR_STATUS_OK == sens_stat){
            printf("s%1d %7.2fus %5.1fmm\n", i + 1, sensor_4mic[i].delay_time_usec, sensor_4mic[i].distance_mm);
        }else{
            //検出していない時
            printf("s%1d  ---.--us\n", i + 1);
        }
    }
    if (meas_stat != MEASURE_STATUS_OK){
        printf(">measurement error\n");
        return;
    }
    
    //使用センサー番号
    printf("adopted sensor#%3x\n",result.pattern); 
    //計算結果
    printf("   x0     y0     r0 \n");
    printf("%6.1f %6.1f %6.1f\n", result.impact_pos_x_mm, result.impact_pos_y_mm, result.radius0_mm);
    //エラー
    calc_stat = result.status;
    switch(calc_stat){
        case CALC_STATUS_X0_ERR:
            printf(" x error\n");
            break;
        case CALC_STATUS_Y0_ERR:
            printf(" y error\n");
            break;
        case CALC_STATUS_R0_ERR:
            printf(" r error\n");
            break;
        default:
            break;
    }
}


void    csv_title(void){
    //表計算用のCSV　タイトル部
    uint8_t i;
    
    //1行目
    printf("                                         ,");   //タマモニデータ長分
    printf("shot#,x      ,y      ,r      ,status,sens");
    for (i = 1; i <= NUM_SENSOR; i++){
        printf(",dt sen%1d,sen%1d_d ,sen%1d_r", i, i, i);
    }
    printf(",temp\n");
    //2行目
    printf("                                         ,");
    printf("     ,mm     ,mm     ,mm     ,      ,    ");
    for (i = 1; i <= NUM_SENSOR; i++){
        printf(",usec   ,usec   ,mm    ");
    }
    printf(",deg  \n");
}


void    csv_data(uint16_t shot_count){
    //表計算用のCSVデータ　コンマ区切り
    uint8_t i;
    uint8_t stat;
    
    //番号
    printf("  %03d", shot_count);
    //結果x,y,r
    printf(",%7.2f,%7.2f,%7.2f", result.impact_pos_x_mm, result.impact_pos_y_mm, result.radius0_mm);
    //ステータス
    stat = result.status;
     if ((stat == CALC_STATUS_OK) || (stat == CALC_STATUS_AVERAGE_FIX)){
        printf(",OK    ");
    }else{
        printf(",error ");
    }
    //使用センサー番号
    printf(", %3x",result.pattern);

    //センサー順　t,dt,dr
    for (i = 0; i < NUM_SENSOR; i++){
        printf(",%7.2f,%7.2f,%6.1f", sensor_4mic[i].delay_time_usec, sensor_4mic[i].sensor_delay_usec, sensor_4mic[i].distance_mm);
    }
    //温度
    printf(",%5.1f", temp_ave_degree_c);
    
    printf("\n");
}


//
void data_clear(void){
    //測定データをクリア
    uint8_t i;
    
    LED_RED_Clear();
    sensor_count = 0;
    
    //測定値クリア
    for (i = 0; i < NUM_SENSOR; i++){
        sensor_4mic[i].input_order = 0xff;      //未入力判定用に0ではなくて0xff
        sensor_4mic[i].timer_cnt = 0xffffffff;  //タイム順を見る時のために最大の値にしておく
        sensor_4mic[i].delay_cnt = 0;
        sensor_4mic[i].delay_time_usec = 0;
        sensor_4mic[i].sensor_delay_usec = 0;
        sensor_4mic[i].d_time_corr_usec = 0;
        sensor_4mic[i].distance_mm = 0;
        sensor_4mic[i].status = 0;
    }
   
    ICAP1_Enable();
    ICAP2_Enable();
    ICAP3_Enable();
    ICAP4_Enable();
    
    TMR2 = 0;
    TMR2_Start(); 
}


void    sensor_offset_calc(float dx, float dy){
    //マト板のセンターラインとセンサー位置のズレを補正
    //センサーの物理位置のズレ補正
    float   sensor1_offset_x = -0.1;
    float   sensor1_offset_y = 0;
    float   sensor1_offset_z = 0;

    float   sensor2_offset_x = -0.1;
    float   sensor2_offset_y = 0;
    float   sensor2_offset_z = 0;

    float   sensor3_offset_x = 0;
    float   sensor3_offset_y = 0;
    float   sensor3_offset_z = 0;

    float   sensor4_offset_x = 0;
    float   sensor4_offset_y = 0;
    float   sensor4_offset_z = 0;

    //x
    sensor_4mic[0].sensor_x_mm = -SENSOR_HORIZONTAL_SPACING + sensor1_offset_x + dx;
    sensor_4mic[1].sensor_x_mm =  SENSOR_HORIZONTAL_SPACING + sensor2_offset_x + dx;
    sensor_4mic[2].sensor_x_mm = -SENSOR_HORIZONTAL_SPACING + sensor3_offset_x + dx;
    sensor_4mic[3].sensor_x_mm =  SENSOR_HORIZONTAL_SPACING + sensor4_offset_x + dx;
    //y        
    sensor_4mic[0].sensor_y_mm = -SENSOR_VERTICAL_SPACING + sensor1_offset_y + dy;
    sensor_4mic[1].sensor_y_mm = -SENSOR_VERTICAL_SPACING + sensor2_offset_y + dy;
    sensor_4mic[2].sensor_y_mm =  SENSOR_VERTICAL_SPACING + sensor3_offset_y + dy;
    sensor_4mic[3].sensor_y_mm =  SENSOR_VERTICAL_SPACING + sensor4_offset_y + dy;
    //z
    sensor_4mic[0].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor1_offset_z;
    sensor_4mic[1].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor2_offset_z;
    sensor_4mic[2].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor3_offset_z;
    sensor_4mic[3].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor4_offset_z;

}
