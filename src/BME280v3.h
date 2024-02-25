/* 
 * File:   BME280v3.h
 * 
 * 2024.01.02
 * 
 */

#ifndef BME280V3_H
#define	BME280V3_H

////#include <stdint.h>
//#include <stdbool.h>
//#include <stddef.h>

//Global
extern float    air_temp_degree_c;      //‹C‰·


bool        BME280_Reset(void);
bool        BME280_Init(void);
bool        BME280_Sleep(void);
bool        BME280_TrimRead(void);
uint8_t     BME280_ReadoutSM(void);
//
int32_t     BME280_CompensateT(int32_t);
uint32_t    BME280_CompensateP(int32_t);
uint32_t    BME280_CompensateH(int32_t);


#endif //BME280V3_H