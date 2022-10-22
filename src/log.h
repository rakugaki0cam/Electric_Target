/*
 * log.h
 * 
 * ���O
 * 
 *  2022.04.16
 * 
 *  
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef LOG_H
#define	LOG_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "header.h"


//�Œ�l
typedef struct {
    uint8_t     sensor_num;      //�Z���T�� = 4
    float       sensor_x1;       //�Z���T���W
    float       sensor_y1;
    float       sensor_z1;
    float       sensor_x2;
    float       sensor_y2;
    float       sensor_z2;
    float       sensor_x3;
    float       sensor_y3;
    float       sensor_z3;
    float       sensor_x4;
    float       sensor_y4;
    float       sensor_z4;
    float       sensor_delay_a;
    float       sensor_delay_b;
} log_fixed_t;

//����&�␳
typedef struct {
    uint8_t     shot_num;           //�ʔԍ�
    uint8_t     input_order;        //�Z���T�f�[�^���͏���     
    float       delay_time1;        //���B���ԍ�����l
    float       delay_time2;
    float       delay_time3;
    float       delay_time4;
    float       sensor_delay1;      //�Z���T�I���x��␳
    float       sensor_delay2;
    float       sensor_delay3;
    float       sensor_delay4;
    float       distance1;          //������
    float       distance2;
    float       distance3;
    float       distance4; 
    float       v_air;              //����
} log_calc_t;

//����
typedef struct {
    uint8_t     shot_num;       //�ʔԍ�
    float       impact_pos_x;   //���e���W
    float       impact_pos_y;
    float       radius0;        //���e�_����ŏ��̃Z���T�܂ł̋���
    float       temp;           //���x
} log_result_t;


//GLOBAL
extern log_fixed_t  log_sensor[];   //�Z���T�[�f�[�^
extern log_calc_t   log_measure[];  //���O����l
extern log_result_t log_result[];   //���O�v�Z����



void        log_title(void);
void        log_data_make(uint16_t);


#endif	//LOG_H