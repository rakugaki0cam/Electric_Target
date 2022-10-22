/*
 * measure.h
 * 
 * ����ƕ\��
 * 
 *  2022.04.16
 * V2_editon
 *  2022.5.11
 * 
 *  
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MEASURE_H
#define	MEASURE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "header.h"
#include <math.h>

//GLOBAL
//����X�e�[�^�X
typedef enum {
    MEASURE_STATUS_OK         = 0x0,
    MEASURE_STATUS_NOT_ENOUGH = 0x1,       
    MEASURE_STATUS_ERROR      = 0x2        
} measure_status_source_t;

//��ʕ\�����[�h
typedef enum {
    SINGLE_LINE = 0,                        //��ʕ\��:1�s/1��
    FULL_DEBUG  = 1,                        //��ʕ\��:�f�o�b�O�p�t���\��
    CSV_DATA    = 2                         //CSV�f�[�^�o���[�\�v�Z�p
} display_mode_source_t;



void        measure_init(void);
uint8_t     measure_main(void);
//
uint8_t     input_order_check(void);
uint8_t     measure_data_assign(void);


//display
void    result_disp(uint16_t, measure_status_source_t, uint8_t);
void    tamamoni_data_send(void);
void    single_line(uint16_t, measure_status_source_t);
void    full_debug(uint16_t, measure_status_source_t);
void    csv_title(void);
void    csv_data(uint16_t);
void    log_title(void);
void    log_data_make(uint16_t);

//
void    data_clear(void);
void    sensor_offset_calc(float, float);


#endif	//MEASURE_H