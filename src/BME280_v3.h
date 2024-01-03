/* 
 * File:   BME280_v3.h
 * 
 * 2024.01.02
 * 
 */

#ifndef BME280_V3_H
#define	BME280_V3_H


bool        BME280_Init(void);
bool        BME280_TrimRead(void);
uint8_t     BME280_Readout(void);
int32_t     BME280_CompensateT(int32_t);
uint32_t    BME280_CompensateP(int32_t);
uint32_t    BME280_CompensateH(int32_t);


#endif //BME280__V3H