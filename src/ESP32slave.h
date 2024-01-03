/*
 * File:   ESP32slave.h
 * 
 * 2024.01.03
 * 
 */

#ifndef _ESP32SLAVE_H    /* Guard against multiple inclusion */
#define _ESP32SLAVE

//Global


bool    ESP32slave_Init(void);
bool    ESP32slave_WriteBatData(bool);
bool    ESP32slave_WriteTempData(uint32_t);

#endif //ESP32SLAVE


/* *****************************************************************************
 End of File
 */
