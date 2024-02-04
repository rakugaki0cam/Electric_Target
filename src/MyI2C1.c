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


void    MyI2CCallback(uintptr_t context){
    //This function will be called when the transfer completes. Note
    //that this functioin executes in the context of the I2C interrupt.

    //printf("I2C complete! \n");
    i2c1Complete = true;
}


bool    i2c1_Write1byteRegister(uint8_t i2cId, uint8_t reg, uint8_t data){
    //レジスタにデータを1バイト書き込む
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //ret val: 0:OK,1:error
    
#define DEBUGI2C1_no
    
    uint8_t txData[2];
    
    txData[0] = reg;
    txData[1] = data;
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_Write(i2cId, &txData[0], 2)){
        printf("I2C write error!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        //printf("error!\n");
        return ERROR;
    }
#ifdef DEBUGI2C1
    printf("I2C 0x%02X (0x%02X)<-%02X\n", i2cId, txData[0], txData[1]);
#endif
    return OK;
}


bool    i2c1_WriteDataBlock(uint8_t i2cId, uint8_t reg, uint8_t* tmp, uint8_t len){
    //レジスタのデータをつづけて書き込む
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //tmp:  data arrey
    //len:  num of data < 256
    //ret val: 0:OK,1:error
    
#define DEBUGI2C2_No
    
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
        printf("I2C write error!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        //printf("error!\n");
        return ERROR;
    }
#ifdef DEBUGI2C2
    printf("I2C 0x%02X (0x%02X)-", i2cId, txData[0]);
    for (i = 1; i <= len; i++){
        printf(" %02x", txData[i]);
    }
    printf("\n");
#endif
    return OK;
}


bool    i2c1_Read1byteRegister(uint8_t i2cId, uint8_t reg, uint8_t* rxData){
    //レジスタのデータを1バイト読み出す
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //ret val: 0:OK,1:error
    
#define DEBUGI2C3_No
    
    uint8_t txData[1];

    txData[0] = reg;
    rxData[0] = 0;          ////////////////////////////////////////////////////////////clear
    
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_WriteRead(i2cId, &txData[0], 1, rxData, 1)){
        printf("I2C write read error!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        //printf("error!\n");
        return ERROR;
    }
#ifdef DEBUGI2C3
    printf("I2C 0x%02X (0x%02X)->%02X\n", i2cId, txData[0], rxData[0]);
#endif
    return OK;
}


bool    i2c1_ReadDataBlock(uint8_t i2cId, uint8_t reg, uint8_t* rxData, uint8_t len){
    //レジスタのデータをつづけて読み出す
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //rxData:  data
    //len:  num of data < 256
    //ret val: 0:OK,1:error
    
#define DEBUGI2C4_no
    
    uint8_t txData[1];
    uint8_t i;
    
    txData[0] = reg;
    for(i = 0; i < len; i++){/////////////////////////////////////////////////////////////clear
        rxData[i] = 0;
    }
    
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_WriteRead(i2cId, &txData[0], 1, rxData, len)){
        printf("I2C write read error!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        //printf("error!\n");
        return ERROR;
    }
#ifdef DEBUGI2C4
    printf("I2C 0x%02X (0x%02X)-", i2cId, txData[0]);
    for (i = 0; i < len; i++){
        printf(" %02x", rxData[i]);
    }
    printf("\n");
#endif
    return OK;
}


//PCF8574
//レジスタは一つのみなのでレジスタアドレスがない

bool    i2c1_WriteRegister(uint8_t i2cId, uint8_t portReg){
    //レジスタ書き込みなし
    //port Write
#define DEBUGPCF8574_1_no
    
    uint8_t txData[1];
    
    txData[0] = portReg;
        
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_Write(i2cId, txData, 1)){
        printf("I2C write error!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        //printf("error!\n");
        return ERROR;
    }
#ifdef DEBUGPCF8574_1
    printf("I2C 0x%02X - %02X\n", i2cId, txData[0]);
#endif
    return OK;
 
}


//レジスタ書き込みなし
bool    i2ci_ReadRegister(uint8_t i2cId, uint8_t* rxData){
    //レジスタ書き込みなし
    //port Read
    //ret   rxData
#define DEBUGPCF8574A_2_no
            
    rxData[0] = 0;                  //////////////////////////////////////////////clear
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if (!I2C1_Read(i2cId, rxData, 1)){
        printf("I2C write error!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        //printf("error!\n");
        return ERROR;
    }
#ifdef DEBUGPCF8574A_2
    printf("I2C 0x%02X - %02X\n", i2cId, rxData[0]);
#endif
    return OK;
 
}



//ESP32slave
bool    i2c1_ESP32ReadDataBlock(uint8_t i2cId, uint8_t reg, uint8_t* rxData, uint8_t len){
    //レジスタのデータをつづけて読み出す
    //**** ESP32スレーブ向け *****
    //     WriteReadだと最初のレジスタが取り込めないようなので
    //i2cId: I2C device ID (7bit)
    //reg:  register address
    //rxData:  data
    //len:  num of data < 256
    //ret val: 0:OK,1:error
    
#define DEBUGI2C5_no
    
    uint8_t txData[1];
    uint8_t i;
    
    for(i = 0; i < len; i++){               //////////////////////////////////////////clear
        rxData[i] = 0;
    }
    
    txData[0] = reg;
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if(!I2C1_Write(i2cId, &txData[0], 1)){
        printf("I2C write error!\n");
        return ERROR;
    }
    if (i2c1_Wait()){
        //printf("error!\n");
        return ERROR;
    }
    //一度終了。間にストップコンディションが入る。
    if (i2c1_BusCheck()){
        printf("I2C bus error!\n");
        return ERROR;
    }
    i2c1Complete = false;
    if(!I2C1_Read(i2cId, rxData, len)){
        printf("I2C read error!\n");
        return ERROR;
    }
    //
    if (i2c1_Wait()){
        //printf("error!\n");
        return ERROR;
    }
#ifdef DEBUGI2C5
    printf("I2C 0x%02X (0x%02X)-", i2cId, txData[0]);
    for (i = 0;i < len; i++){
        printf(" %02x", rxData[i]);
    }
    printf("\n");
#endif
    return OK;
    
}


//
bool    i2c1_BusCheck(void){
    //bus idle check
    uint16_t    cnt = 60000;
    
    while(I2C1_IsBusy( )){
        //wait for the current transfer to complete
        cnt--;
        if (cnt <= 0){
            //bus busy
            printf("bus busy\n");
            return ERROR;
        }
    }
    return OK;
}


bool    i2c1_Wait(void){
    //i2c1 ノンブロックのステートマシンの処理が終了するのを待つ
    // 処理完了でコールバックへ割り込みが入る -> i2cComplete がtrueになる
    //返り値 0:OK, 1:error or time over
#define DEBUGI2C6_no
    
    uint16_t    cnt = 40000;

    while(!i2c1Complete){
        cnt--;
        if (cnt <= 0){
            printf("I2C timeout!");
            return ERROR;
        }
#ifdef DEBUGI2C6
        printf(".");
#endif
    }
    //printf("I2C wait cnt:%d", cnt);
    return i2c1_Error(I2C1ERR_CHECK);
}


 bool   i2c1_Error(i2c_error_report_t report){
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


