/*
 * File:   ESP32slave.c
 * Comments: PIC32MK-MCJ Harmony
 * 
 * ESP32S3 Yellow Board LCD　　I2C
 * 
 * 3.3V I2C
 * 
 * Revision history: 
 * 2024.01.03
 * 
 */


#include "header.h"
#include "ESP32slave.h"



bool ESP32slave_Init(void){
    //ESP32 I2C slave check
    
#define DEBUG_no
    
#define ESP_SLAVE_ID        0x25        //ESP32S3 Yellow Board LCD&WiFi
    uint8_t i2cRxData [4] = {0,0,0,0};  //エンドマークを含む
    
    printf("ESP32S3 init ");
    if(i2c1_ESP32ReadDataBlock(ESP_SLAVE_ID, 0x01, i2cRxData, 3)){
        printf("error!\n");
        return ERROR;
    }
#ifdef  DEBUG
    printf("\n(detect '%s').", i2cRxData);   
#endif
    printf("OK\n"); 
    return OK;
}


bool ESP32slave_WriteBatData(bool init) {
    //バッテリ電圧計測しESP32へ送る
    //init=1:初回
    
#define REG_BAT_STATUS  0x70    
#define     DEBUG_no
    
//#define     ADCH_VBAT   ADCHS_CH1
#define     ADCH_VBAT   ADCHS_CH19
#define     VREFP       2.493       //V
#define     RA          46.9        //kohm
#define     RB          9.93        //kohm
#define     SAMPLES     8           //max 16  12bit x 16 
    
    float               bat_v;
    static uint16_t     batv[SAMPLES];      //data arry
    static uint8_t      ring_cnt = 0;       
    uint16_t            sum_batv;
    uint8_t             dataToEsp[4];
    uint16_t            batToEsp;           //batV*1000[V]
    uint8_t             i;
        
    //平均用データ配列を埋める
    if (init == true){
        for(i = 0; i < SAMPLES; i++){
            ADCHS_ChannelConversionStart(ADCH_VBAT);
            while(!ADCHS_ChannelResultIsReady(ADCH_VBAT));
            batv[i] = ADCHS_ChannelResultGet(ADCH_VBAT);  
        }
        //return OK;
    }
    
    //normal
    ADCHS_ChannelConversionStart(ADCH_VBAT);
    while(!ADCHS_ChannelResultIsReady(ADCH_VBAT));
    batv[ring_cnt] = ADCHS_ChannelResultGet(ADCH_VBAT);    
    //printf("%1d:%03x ", ring_cnt, batv[ring_cnt]);

    ring_cnt ++;
    if (ring_cnt >= SAMPLES){
        ring_cnt = 0;
    }
    
    sum_batv = 0;
    for(i = 0; i < SAMPLES; i++){
        sum_batv += batv[i];
    }
    
    bat_v = (float)sum_batv / SAMPLES * VREFP / 4096 * (RA + RB) / RB;    //12bit
    printf("\nBatV:%4.2fV", bat_v);
     
    ip5306_ReadStatus(dataToEsp);
    if (sleep_flag != POWERSAVING_NORMAL ){
        return OK;
    }
    
    //write to ESP32slave
    batToEsp = bat_v * 1000;            //1000倍値
    //dataToEsp[0]-> bat status data ip5306_read_statusで取得
    dataToEsp[1] = batToEsp >> 8;
    dataToEsp[2] = batToEsp & 0xff;

    if (i2c1_WriteDataBlock(ESP_SLAVE_ID, REG_BAT_STATUS, &dataToEsp[0], 3)){
        printf("ESPslave error!\n");
        return ERROR;
    }
#ifdef DEBUG
    printf("to ESP(%02X)- %02X %02X %02X\n", REG_BAT_STATUS, dataToEsp[0], dataToEsp[1], dataToEsp[2]);
#endif
    return OK;
}


bool ESP32slave_WriteTempData(uint32_t temp){
    //気温データをESP32へ送る
    //temp:温度データ
#define REG_TEMP_STATUS  0x60  
    
    uint8_t     dataToEsp[2];

    dataToEsp[0] = (uint8_t)(temp >> 8) & 0xff;
    dataToEsp[1] = (uint8_t)temp & 0xff;

    if (i2c1_WriteDataBlock(ESP_SLAVE_ID, REG_TEMP_STATUS, &dataToEsp[0], 2)){
        printf("ESPslave error!\n");
        return ERROR;
    }
    return OK;
}
    
    