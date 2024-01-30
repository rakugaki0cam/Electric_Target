/*
 * File:   MyI2C1.h
 * Comments: PIC32MK-MCJ Harmony
 * 
 * 2024.01.02 
 * 
 */

#ifndef _I2C1_USER_H    /* Guard against multiple inclusion */
#define _I2C1_USER_H


typedef enum
{
    I2C1ERR_CHECK,
    I2C1ERR_REPORT,
    I2C1ERR_CLEAR,
} i2c_error_report_t;



void MyI2CCallback(uintptr_t);

bool    i2c1_Write1byteRegister(uint8_t, uint8_t, uint8_t);
bool    i2c1_WriteDataBlock(uint8_t, uint8_t, uint8_t*, uint8_t);
bool    i2c1_Read1byteRegister(uint8_t, uint8_t,uint8_t*);
bool    i2c1_ReadDataBlock(uint8_t, uint8_t, uint8_t*, uint8_t);

//PCF8574
//レジスタは一つのみなのでレジスタアドレスがない
bool    i2c1_WriteRegister(uint8_t, uint8_t);
bool    i2ci_ReadRegister(uint8_t, uint8_t*);

//ESP32slave
bool    i2c1_ESP32ReadDataBlock(uint8_t, uint8_t, uint8_t*, uint8_t);

//
bool    i2c1_BusCheck(void);
bool    i2c1_Wait(void);
bool    i2c1_Error(i2c_error_report_t);

#endif //_I2C1_USERE_H

/* *****************************************************************************
 End of File
 */
