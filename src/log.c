/*
 * log.c
 * 
 * ���O�f�[�^
 * 
 *  2022.04.16
 * 
 *  
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include "log.h"


//GLOBAL
log_fixed_t  log_sensor_data;   //�Z���T�[�f�[�^
log_calc_t   log_measure[256];  //���O����l
log_result_t log_result[256];   //���O�v�Z����



void log_title(void){
    //���O�@�Œ�l�@���
    //�Z���T�[�ʒu�f�[�^
    log_sensor_data.sensor_num = NUM_SENSOR;
    log_sensor_data.sensor_x1 = sensor_4mic[SENSOR1].sensor_x_mm;
    log_sensor_data.sensor_y1 = sensor_4mic[SENSOR1].sensor_y_mm;
    log_sensor_data.sensor_z1 = sensor_4mic[SENSOR1].sensor_z_mm;
    log_sensor_data.sensor_x2 = sensor_4mic[SENSOR2].sensor_x_mm;
    log_sensor_data.sensor_y2 = sensor_4mic[SENSOR2].sensor_y_mm;
    log_sensor_data.sensor_z2 = sensor_4mic[SENSOR2].sensor_z_mm;
    log_sensor_data.sensor_x3 = sensor_4mic[SENSOR3].sensor_x_mm;
    log_sensor_data.sensor_y3 = sensor_4mic[SENSOR3].sensor_y_mm;
    log_sensor_data.sensor_z3 = sensor_4mic[SENSOR3].sensor_z_mm;
    log_sensor_data.sensor_x4 = sensor_4mic[SENSOR4].sensor_x_mm;
    log_sensor_data.sensor_y4 = sensor_4mic[SENSOR4].sensor_y_mm;
    log_sensor_data.sensor_z4 = sensor_4mic[SENSOR4].sensor_z_mm;
    log_sensor_data.sensor_delay_a = delay_a;
    log_sensor_data.sensor_delay_b = delay_b;
    
}

void log_data_make(uint16_t shot_count){
    //���O�f�[�^�쐬(�ꔭ����)
    
    //����l�@�␳�l
    log_measure[ring_pos].shot_num = shot_count;
    log_measure[ring_pos].input_order = 0;
    log_measure[ring_pos].delay_time1 = sensor_4mic[SENSOR1].delay_time_usec;
    log_measure[ring_pos].delay_time2 = sensor_4mic[SENSOR2].delay_time_usec;
    log_measure[ring_pos].delay_time3 = sensor_4mic[SENSOR3].delay_time_usec;
    log_measure[ring_pos].delay_time4 = sensor_4mic[SENSOR4].delay_time_usec;
    log_measure[ring_pos].sensor_delay1 = sensor_4mic[SENSOR1].sensor_delay_usec;
    log_measure[ring_pos].sensor_delay2 = sensor_4mic[SENSOR2].sensor_delay_usec;
    log_measure[ring_pos].sensor_delay3 = sensor_4mic[SENSOR3].sensor_delay_usec;
    log_measure[ring_pos].sensor_delay4 = sensor_4mic[SENSOR4].sensor_delay_usec;
    log_measure[ring_pos].distance1 = sensor_4mic[SENSOR1].distance_mm;
    log_measure[ring_pos].distance2 = sensor_4mic[SENSOR2].distance_mm;
    log_measure[ring_pos].distance3 = sensor_4mic[SENSOR3].distance_mm;
    log_measure[ring_pos].distance4 = sensor_4mic[SENSOR4].distance_mm;
    log_measure[ring_pos].v_air = v_air_mps();

    //���� (���ϒl)
    log_result[ring_pos].shot_num = shot_count;
    log_result[ring_pos].impact_pos_x = result.impact_pos_x_mm;
    log_result[ring_pos].impact_pos_y = result.impact_pos_y_mm;
    log_result[ring_pos].radius0 = result.radius0_mm;
    log_result[ring_pos].temp = temp_ave_degree_c;

}


