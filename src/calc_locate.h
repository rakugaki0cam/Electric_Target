
/*
 * calc_locate.h
 * 
 * ���W�̌v�Z
 * 
 *  2022.03.24
 * 
 *  
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef CALC_LOCATE_H
#define	CALC_LOCATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "header.h"
#include <math.h>


//GLOBAL
#define     NUM_SENSOR                  4           //�Z���T�[��
#define     SENSOR1                     0           //�Z���T�Ăєԍ��ƃv���O�������̓Y��
#define     SENSOR2                     1
#define     SENSOR3                     2
#define     SENSOR4                     3
#define     NUM_PATTERN                 4           //�Z���T�[4����3�I�ԃp�^�[���̐� 4C3 = 4!-3! = 4x3x2x1 - 3x2x1 = 4
#define     SENSOR_HORIZONTAL_SPACING   90.3        //�Z���T�[�����@180.6 (�}�C�N��9.7)�@����l
#define     SENSOR_VERTICAL_SPACING     131.9       //�Z���T�[�c���@251.8(�t���[���Ԋu)+5+5(�t���[���`�}�C�N���)+1+1(�}�C�N��ʁ`�U����)�@����l
#define     TARGET_WIDTH_HALF           105         //�}�g������1/2
#define     TARGET_HEIGHT_HALF          120         //�}�g�c������1/2
#define     R_MAX                       340         //�قڑΊp��
#define     SENSOR_DEPTH_OFFSET         6           //���r��t2�@���̔������͉��r�̗���??

//�Z���T���Ƒ���l
typedef struct {
    uint8_t     sensor_num;         //�Z���T�ԍ�
    uint8_t     input_order;        //�Z���T�f�[�^���͏���
    float       sensor_x_mm;        //�Z���T���W
    float       sensor_y_mm;
    float       sensor_z_mm;
    uint32_t    timer_cnt;          //�^�C�}����l
    uint32_t    delay_cnt;          //�^�C�}���l
    float       delay_time_usec;    //���B���ԍ�����l
    float       sensor_delay_usec;  //�Z���T�x�ꎞ��(����)
    float       d_time_corr_usec;   //���B���ԍ��␳��
    float       distance_mm;        //������
    uint8_t     status;             //���
} sensor_data_t;

typedef enum {
    SENSOR_STATUS_OK        = 0x0,      //OK
    SENSOR_STATUS_INVALID   = 0x1,      //�f�[�^����     
    SENSOR_STATUS_ERROR     = 0x2       //�G���[   
} sensor_status_source_t;


//���e�ʒu�v�Z����
typedef struct {
    uint16_t    pattern;            //�Z���T�I���p�^�[�� �\�i�@�e�����Z���T�ԍ�
    float       impact_pos_x_mm;    //���e���W
    float       impact_pos_y_mm;
    float       radius0_mm;         //���e�_����ŏ��̃Z���T�܂ł̋���
    float       delay_time0_msec;   //���e�_����ŏ��̃Z���T�I���܂ł̎���
    uint8_t     status;             //���
} impact_data_t;

typedef enum {
    CALC_STATUS_OK          = 0x00,
    CALC_STATUS_NOT_ENOUGH  = 0x01,     //����l���s��
    CALC_STATUS_TOO_MANY    = 0x02,     //����l��������
    CALC_STATUS_CAL_ERROR   = 0x11,     //�v�Z�G���[����
    CALC_STATUS_E_0_1       = 0x12,     //����e=0
    CALC_STATUS_E_0_2       = 0x13,     //����e=0
    CALC_STATUS_E_0_3       = 0x14,     //����e=0
    CALC_STATUS_E_0_4       = 0x15,     //����e=0
    CALC_STATUS_QUAD_F      = 0x16,     //���̌����̏����G���[
    CALC_STATUS_X0_ERR      = 0x21,     //��X0���͈͊O
    CALC_STATUS_Y0_ERR      = 0x22,     //��Y0���͈͊O
    CALC_STATUS_R0_ERR      = 0x23,     //��R0���͈͊O
    CALC_STATUS_X0_DEV_ERR  = 0x29,     //��X0���΍���
    CALC_STATUS_Y0_DEV_ERR  = 0x2a,     //��Y0���΍���
    CALC_STATUS_R0_DEV_ERR  = 0x2b,     //��R0���΍���
    CALC_STATUS_AVERAGE_FIX = 0x30,     //���ϒl�C��
    CALC_STATUS_AVERAGE_ERR = 0x31      //���ϒl�s��
} calc_status_source_t;



//GLOBAL
extern sensor_data_t    sensor_4mic[];  //�Z���T�[�f�[�^�A����l
extern impact_data_t    result;         //�v�Z����
extern const float      delay_a;        //�Z���T�[�x�ꎞ�Ԃ̌v�Z�W��
extern const float      delay_b;


//
uint8_t     calc_locate_xy(void);

//
uint8_t     calc_of_3sensor(void);
uint8_t     asign_3sensor(void);
void        calc_sensor_pattern(void);
//calculation
//locate
uint8_t     apollonius_3circle_xyr(void);
float       dist_delay_mm(float);
//time
float       v_air_mps(void);
float       delay_time_usec(uint32_t);
float       delay_sensor_usec(float);
float       impact_time_msec(float);
    

#endif	//CALC_LOCATE_H

