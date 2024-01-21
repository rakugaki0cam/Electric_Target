/*
 * File:   PCF8574.c
 * 
 * PCF8574 I2C I/O Expander 
 * 
 * 3.3V I2C 8ch
 * 
 * Revision history: 
 * 2024.01.14   
 * 
 * 
 */
#include "header.h"
#include "PCF8574.h"


#define     PCF8574_ID      0x20        //0x20~0x27

//レジスタは1つだけなので、アドレスは無い

#define     IO_SETTING      0b00000000  //入力ポート:1を書込(100uAのプルアップが有効になる)

//local
static  uint8_t portRegister = IO_SETTING;



bool    PCF8574_Init(void){
    //initialize
    //ret value: 1:error, 0:OK
    
    //input port set
    printf("PCF8574 init ");

    if (i2c1_WriteRegister(PCF8574_ID, IO_SETTING)){
        printf("error!\n");
        return ERROR;
    }
    printf("OK\n");
    
    return OK;
}
    

//
void    ledLightOn(port_name_t color){
    //LEDをオン
    //複数一度にやりたい時はorで並べる
    uint8_t tmp;
    if (NO_OUTPUT == color){
        //何もオンしない時
        return;
    }
    
    tmp = portRegister | color;
    i2c1_WriteRegister(PCF8574_ID, tmp);
    
}


void    ledLightOff(port_name_t color){
    //LEDをオフ
    uint8_t tmp;
    if (NO_OUTPUT == color){
        //何もオフしない時
        return;
    }
    
    tmp = portRegister & (!color);
    i2c1_WriteRegister(PCF8574_ID, tmp);
    
}


//****** sub ************************************************


