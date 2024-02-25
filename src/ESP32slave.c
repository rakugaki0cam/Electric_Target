/*
 * File:   ESP32slave.c
 * Comments: PIC32MK-MCJ Harmony
 * 
 * ESP32S3 Yellow Board LCD　　I2C slave
 * 
 * 3.3V I2C
 * 
 * Revision history: 
 * 2024.01.03
 * 
 */

#include "header.h"
#include "ESP32slave.h"

//DEBUGger printf (Global...header.h)
//#define DEBUG_ESP_SLAVE_0_no    //Debug用printf(エラー関連)
//#define DEBUG_ESP_SLAVE_2_no    //Debug用printf(バッテリ電圧)
//#define DEBUG_ESP_SLAVE_3_no    //Debug用printf(送信データ)
    

#define ESP_SLAVE_ID      0x25  //I2C ID ESP32S3 Yellow Board LCD&WiFi
//register address
typedef enum  {
    REG_ID_CHECK        = 0x01, //ID確認　返答"ESP"
    REG_TARGET_CLEAR    = 0x20, //ターゲットクリアコマンド
    REG_TARGET_RESET    = 0x21, //ターゲットリセットコマンド
    REG_TARGET_DEFAULT  = 0x22, //ターゲットデフォルトセットコマンド
    REG_TARGET_OFFSET   = 0x23, //ターゲットYオフセットコマンド
    REG_TARGET_AIMPOINT = 0x24, //ターゲット狙点セットコマンド
    REG_TARGET_BRIGHT   = 0x25, //ターゲット輝度セットコマンド
    REG_DATA_IMPACT     = 0x30, //着弾データを送る
    REG_DATA_TEMP       = 0x60, //温度データを送る
    REG_DATA_BAT        = 0x70, //バッテリーデータを送る
    REG_TARGET_SLEEP    = 0x90, //ターゲットスリープコマンド
    DATA_SLEEP_KEY      = 0x99, //スリープのkey
} esp_register_t;


bool ESP32slave_Init(void){
    //ESP32 I2C slave ID check   
#define DEBUG32_1_no
    
    uint8_t i2cRxData [4] = {0,0,0,0};  //エンドマークを含む
    
    printf("ESP32S3 init ");
    if(i2c1_ESP32ReadDataBlock(ESP_SLAVE_ID, 0x01, i2cRxData, 3)){
        printf("error!\n");
        return ERROR;
    }
#ifdef  DEBUG32_1
    printf("\n(detect '%s').", i2cRxData);   
#endif
    printf("OK\n"); 
    return OK;
}

//**** send data **************************************************


bool ESP32slave_SendBatData(void) {
    //バッテリ電圧計測しESP32へ送る
    //init=1:初回
    uint8_t     dataToEsp[4];
    float       batV;
        
    batV = batteryVolt(0);
#ifdef DEBUG_ESP_SLAVE_2
    printf("\nBatV:%4.2fV", batV);
#endif
    batV *= 1000;            //1000倍値

    ip5306_ReadStatus(&dataToEsp[0]);
    
    //write to ESP32slave
    if (sleepStat != POWERSAVING_NORMAL ){
        return OK;
    }
    //dataToEsp[0]-> bat status data ip5306_read_statusで取得
    dataToEsp[1] = ((uint16_t)batV >> 8) & 0xff;
    dataToEsp[2] = (uint16_t)batV & 0xff;

    if (i2c1_WriteDataBlock(ESP_SLAVE_ID, REG_DATA_BAT, &dataToEsp[0], 3)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
#ifdef DEBUG_ESP_SLAVE_3
    printf("to ESP(%02X)- %02X %02X %02X\n", REG_DATA_BAT, dataToEsp[0], dataToEsp[1], dataToEsp[2]);
#endif
    return OK;
}


bool ESP32slave_SendTempData(uint32_t temp){
    //気温データをESP32へ送る
    //temp:温度データ
    uint8_t     dataToEsp[2];

    dataToEsp[0] = (uint8_t)(temp >> 8) & 0xff;
    dataToEsp[1] = (uint8_t)temp & 0xff;

    if (i2c1_WriteDataBlock(ESP_SLAVE_ID, REG_DATA_TEMP, &dataToEsp[0], 2)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;
}



bool ESP32slave_SendImpactData(uint8_t* dataToEsp, uint8_t len){
    //着弾データを送信
    //len:バイト数
    //送る前の変数の型は呼び出しの際(uint8_t*)でキャストすれば　なんでもOK。

    if (i2c1_WriteDataBlock(ESP_SLAVE_ID, REG_DATA_IMPACT, &dataToEsp[0], len)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;
}
    
/*  ポインタ変数を立てなくてもできるので、このやり方は無駄だけどポインタを理解するのには良いかも。
bool ESP32slave_SendImpactData(float* impactData, uint8_t len){
    //着弾データを送信
    //float 4バイト変数を1バイトごとの配列に変換
    uint8_t     dataToEsp[len];
    uint8_t*    pointerByte;    //ポインタ変数 - uint8_tなので1バイトずつ進む
    uint8_t     i;
    
    pointerByte = (uint8_t*)&impactData[0];
    for (i = 0; i < len; i++){
        dataToEsp[i] = pointerByte[i];
    }

    if (i2c1_WriteDataBlock(ESP_SLAVE_ID, REG_DATA_IMPACT, &dataToEsp[0], len)){
        printf("ESPslave error!\n");
        return ERROR;
    }
    return OK;
}
*/


//**** command **************************************************

bool ESP32slave_SleepCommand(void){  
    //ESP32へスリープ要請

    if (i2c1_Write1byteRegister(ESP_SLAVE_ID, REG_TARGET_SLEEP, DATA_SLEEP_KEY)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;
}


bool ESP32slave_ClearCommand(void){  
    //ESP32へターゲットクリア

    if (i2c1_Write1byteRegister(ESP_SLAVE_ID, REG_TARGET_CLEAR, 0)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;
}


bool ESP32slave_ResetCommand(void){  
    //ESP32へターゲットリセット

    if (i2c1_Write1byteRegister(ESP_SLAVE_ID, REG_TARGET_RESET, 0)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;
}


bool ESP32slave_DefaultSetCommand(void){  
    //ESP32へターゲットデフォルトセッティング

    if (i2c1_Write1byteRegister(ESP_SLAVE_ID, REG_TARGET_DEFAULT, 0)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;
}



bool ESP32slave_OffSetCommand(float offSet){  
    //ESP32へオフセットコマンド
    if ((offSet < -40) || (offSet > 35)){
        return ERROR;
    }
    if (i2c1_Write1byteRegister(ESP_SLAVE_ID, REG_TARGET_OFFSET, offSet)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;
}


bool ESP32slave_AimpointCommand(float aimPoint){  
    //ESP32へ狙点セットコマンド
    if ((aimPoint < 30) || (aimPoint > 120)){
        return ERROR;
    }
    if (i2c1_Write1byteRegister(ESP_SLAVE_ID, REG_TARGET_AIMPOINT, aimPoint)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;   
    
}


bool ESP32slave_BrightnessCommand(float brightness){  
    //ESP32へ輝度セットコマンド
    if ((brightness < 0) || (brightness > 250)){
        return ERROR;
    }
     if (i2c1_Write1byteRegister(ESP_SLAVE_ID, REG_TARGET_BRIGHT, brightness)){
#ifdef DEBUG_ESP_SLAVE_0
        printf("ESPslave error!\n");
#endif
        return ERROR;
    }
    return OK;   
    
    
}