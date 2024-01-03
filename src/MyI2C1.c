/*
 * File:   MyI2C1.c
 * Comments: PIC32MK-MCJ Harmony
 * 
 * Revision history: 
 * 2024.01.02 
 * 
 * HarmonyのI2Cはノンブロッキングなステートマシンによる処理なので
 * 処理完了まで待つブロッキング処理のサブルーチンとした。
 * 
 */


#include "header.h"
#include "MyI2C1.h"

//local
bool    i2c1Complete = false;


void MyI2CCallback(uintptr_t context){
    //This function will be called when the transfer completes. Note
    //that this functioin executes in the context of the I2C interrupt.

    //printf("I2C complete! \n");
    i2c1Complete = true;
}


bool i2c1_Write1byteRegister(uint8_t i2cId, uint8_t reg, uint8_t data){
    //レジスタにデータを1バイト書き込む
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //ret val: 0:OK,1:error
    
#define DEBUG1_no
    
    uint8_t txData[2];
    
    txData[0] = reg;
    txData[1] = data;
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_Write(i2cId, &txData[0], 2)){
        printf("I2C bus busy!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        printf("error!\n");
        return ERROR;
    }
#ifdef DEBUG1
    printf("I2C 0x%02X (0x%02X)<-%02X\n", i2cId, txData[0], txData[1]);
#endif
    return OK;
}


bool i2c1_WriteDataBlock(uint8_t i2cId, uint8_t reg, uint8_t* tmp, uint8_t len){
    //レジスタのデータをつづけて書き込む
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //tmp:  data arrey
    //len:  num of data < 256
    //ret val: 0:OK,1:error
    
#define DEBUG2_No
    
    uint8_t txData[len + 1];
    uint8_t i;
    
    txData[0] = reg;
    for (i = 0;i < len; i++){
        txData[i + 1] = tmp[i];
    }
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_Write(i2cId, &txData[0], (len + 1))){
        printf("I2C bus busy!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        printf("error!\n");
        return ERROR;
    }
#ifdef DEBUG2
    printf("I2C 0x%02X (0x%02X)-", i2cId, txData[0]);
    for (i = 1; i <= len; i++){
        printf(" %02x", txData[i]);
    }
    printf("\n");
#endif
    return OK;
}


bool i2c1_Read1byteRegister(uint8_t i2cId, uint8_t reg, uint8_t* rxData){
    //レジスタのデータを1バイト読み出す
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //ret val: 0:OK,1:error
    
#define DEBUG3_No
    
    uint8_t txData[1];

    txData[0] = reg;
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_WriteRead(i2cId, &txData[0], 1, rxData, 1)){
        printf("I2C bus busy!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        printf("error!\n");
        return ERROR;
    }
#ifdef DEBUG3
    printf("I2C 0x%02X (0x%02X)->%02X\n", i2cId, txData[0], rxData[0]);
#endif
    return OK;
}


bool i2c1_ReadDataBlock(uint8_t i2cId, uint8_t reg, uint8_t* rxData, uint8_t len){
    //レジスタのデータをつづけて読み出す
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //rxData:  data
    //len:  num of data < 256
    //ret val: 0:OK,1:error
    
#define DEBUG4_no
    
    uint8_t txData[1];
    
    txData[0] = reg;
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_WriteRead(i2cId, &txData[0], 1, rxData, len)){
        printf("I2C bus busy!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        printf("error!\n");
        return ERROR;
    }
#ifdef DEBUG4
    uint8_t i;
    printf("I2C 0x%02X (0x%02X)-", i2cId, txData[0]);
    for (i = 0;i < len; i++){
        printf(" %02x", rxData[i]);
    }
    printf("\n");
#endif
    return OK;
}


//ESP32slave
bool i2c1_ESP32ReadDataBlock(uint8_t i2cId, uint8_t reg, uint8_t* rxData, uint8_t len){
    //レジスタのデータをつづけて読み出す
    //**** ESP32スレーブ向け *****
    //     WriteReadだと最初のレジスタが取り込めないようなので
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //rxData:  data
    //len:  num of data < 256
    //ret val: 0:OK,1:error
    
#define DEBUG5_no
    
    uint8_t txData[1];
    
    txData[0] = reg;
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if(!I2C1_Write(i2cId, &txData[0], 1)){
        printf("error!\n");
        return ERROR;
    }
    if (i2c1_Wait()){
        printf("error!\n");
        return ERROR;
    }
    //一度終了
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if(!I2C1_Read(i2cId, rxData, len)){
        printf("error!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        printf("error!\n");
        return ERROR;
    }
#ifdef DEBUG5
    uint8_t i;
    printf("I2C 0x%02X (0x%02X)-", i2cId, txData[0]);
    for (i = 0;i < len; i++){
        printf(" %02x", rxData[i]);
    }
    printf("\n");
#endif
    return OK;
    
}


//
bool i2c1_BusCheck(void){
    //bus idle check
    
#define COUNT2   100
    uint16_t i = 0;
    
    while(I2C1_IsBusy( )){
        //wait for the current transfer to complete
        i++;
        if (i > COUNT2){
            //bus error
            //printf("bus error\n");
            I2C1_TransferAbort();       //たぶん必要
            return ERROR;
        }
    }
    return OK;
}


bool i2c1_Wait(void){
    //i2c1 ノンブロックのステートマシンの処理が終了するのを待つ
    // 処理完了でコールバックへ割り込みが入る -> i2cComplete がtrueになる
    //返り値 0:OK, 1:error or time over
    
#define TIMEOUT   20
    uint16_t    cnt = 0;

    while(!i2c1Complete){
        cnt++;
        if (cnt > TIMEOUT){
            printf("I2C timeout!");
            return ERROR;
        }
        printf(".");
    }
    return i2c1_Error(I2C1ERR_CHECK);
}


 bool i2c1_Error(i2c_error_report_t report){
    //error handling
    //report 0:normal error check
    //ret val 0:OK, 1:error
     
    static uint32_t i2cErrCnt = 0;      //エラー総数カウント
    static uint16_t nackErrCnt = 0;
    static uint16_t busErrCnt = 0;
    
    if (I2C1ERR_REPORT == report){
        printf("I2C err total %d\n", i2cErrCnt);
        printf("I2C NACK err  %d\n", nackErrCnt);
        printf("I2C bus err   %d\n", busErrCnt);
        return OK;
    }
    if (I2C1ERR_CLEAR == report){
        i2cErrCnt = 0;
        nackErrCnt = 0;
        busErrCnt = 0;
        printf("I2C err cnt CLEAR\n");
        return OK;
    }
    
    i2cErrCnt++;
    while(I2C1_IsBusy());  //wait for the current transfer to complete
    switch(I2C1_ErrorGet()){
        case I2C_ERROR_NONE:
            return OK;
            break;
        case I2C_ERROR_NACK:
            nackErrCnt++;
            printf("I2C_ERROR_NACK\n");
            break;
        case I2C_ERROR_BUS_COLLISION:
            busErrCnt++;
            printf("I2C_ERROR_BUS_COLLISION\n");
            break;
    }
    I2C1_TransferAbort();       //必要みたい
    return ERROR;
}


