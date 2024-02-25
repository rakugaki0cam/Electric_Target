/*
 * File:   ESP32slave.h
 * 
 * 2024.01.03
 * 
 */

#ifndef _ESP32SLAVE_H    /* Guard against multiple inclusion */
#define _ESP32SLAVE_H


//Global


bool    ESP32slave_Init(void);
//send data
bool    ESP32slave_SendBatData(void);
bool    ESP32slave_SendTempData(uint32_t);
bool    ESP32slave_SendImpactData(uint8_t*, uint8_t);
//command
bool    ESP32slave_SleepCommand(void);
bool    ESP32slave_ClearCommand(void);
bool    ESP32slave_ResetCommand(void);
bool    ESP32slave_DefaultSetCommand(void); 
bool    ESP32slave_OffSetCommand(float);
bool    ESP32slave_AimpointCommand(float);
bool    ESP32slave_BrightnessCommand(float);



#endif //ESP32SLAVE_H


/* *****************************************************************************
 End of File
 */
