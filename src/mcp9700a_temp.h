/*
 * mcp9700a_temp.h
 * 
 * 温度センサー
 * 
 *  2022.04.08
 * 
 */
#ifndef _MCP9700A_TEMP_H    /* Guard against multiple inclusion */
#define _MCP9700A_TEMP_H

#include "header.h"
#include <math.h>


//GLOBAL
extern float    temp_ave_degree_c;



void        init_temp(void);
float       set_temp_degree_c(void);
float       calc_temp(float);
void        get_temp_adc(void);
uint16_t    average_temp_adc(void);


#endif //_MCP9700A_TEMP_H

/* *****************************************************************************
 End of File
 */
