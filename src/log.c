/*
 * log.c
 * 
 * ログデータ
 * 
 *  2022.04.16
 * 
 *  V5_edition
 *  2024.01.08
 * 
 */

#include "header.h"
#include "log.h"


//GLOBAL
log_fixed_t  log_sensor_data;   //センサーデータ
log_calc_t   log_measure[LOG_MEM];  //ログ測定値
log_result_t log_result[LOG_MEM];   //ログ計算結果



void log_title(void){
    //ログ　固定値　代入
    //センサー位置データ
    log_sensor_data.sensor_num = NUM_SENSOR;
    log_sensor_data.sensor_x1 = sensor5Measure[SENSOR1].sensor_x_mm;
    log_sensor_data.sensor_y1 = sensor5Measure[SENSOR1].sensor_y_mm;
    log_sensor_data.sensor_z1 = sensor5Measure[SENSOR1].sensor_z_mm;
    log_sensor_data.sensor_x2 = sensor5Measure[SENSOR2].sensor_x_mm;
    log_sensor_data.sensor_y2 = sensor5Measure[SENSOR2].sensor_y_mm;
    log_sensor_data.sensor_z2 = sensor5Measure[SENSOR2].sensor_z_mm;
    log_sensor_data.sensor_x3 = sensor5Measure[SENSOR3].sensor_x_mm;
    log_sensor_data.sensor_y3 = sensor5Measure[SENSOR3].sensor_y_mm;
    log_sensor_data.sensor_z3 = sensor5Measure[SENSOR3].sensor_z_mm;
    log_sensor_data.sensor_x4 = sensor5Measure[SENSOR4].sensor_x_mm;
    log_sensor_data.sensor_y4 = sensor5Measure[SENSOR4].sensor_y_mm;
    log_sensor_data.sensor_z4 = sensor5Measure[SENSOR4].sensor_z_mm;
    log_sensor_data.sensor_x4 = sensor5Measure[SENSOR5].sensor_x_mm;
    log_sensor_data.sensor_y4 = sensor5Measure[SENSOR5].sensor_y_mm;
    log_sensor_data.sensor_z4 = sensor5Measure[SENSOR5].sensor_z_mm;
    log_sensor_data.sensor_delay_a = delay_a;
    log_sensor_data.sensor_delay_b = delay_b;
    
}

void log_data_make(uint16_t shot_count){
    //ログデータ作成(一発ごと)
    
    //測定値　補正値
    log_measure[ringPos].shot_num = shot_count;
    log_measure[ringPos].input_order = 0;
    log_measure[ringPos].delay_time1 = sensor5Measure[SENSOR1].delay_time_usec;
    log_measure[ringPos].delay_time2 = sensor5Measure[SENSOR2].delay_time_usec;
    log_measure[ringPos].delay_time3 = sensor5Measure[SENSOR3].delay_time_usec;
    log_measure[ringPos].delay_time4 = sensor5Measure[SENSOR4].delay_time_usec;
    log_measure[ringPos].delay_time5 = sensor5Measure[SENSOR5].delay_time_usec;
    log_measure[ringPos].sensor_delay1 = sensor5Measure[SENSOR1].comp_delay_usec;
    log_measure[ringPos].sensor_delay2 = sensor5Measure[SENSOR2].comp_delay_usec;
    log_measure[ringPos].sensor_delay3 = sensor5Measure[SENSOR3].comp_delay_usec;
    log_measure[ringPos].sensor_delay4 = sensor5Measure[SENSOR4].comp_delay_usec;
    log_measure[ringPos].sensor_delay5 = sensor5Measure[SENSOR5].comp_delay_usec;
    log_measure[ringPos].distance1 = sensor5Measure[SENSOR1].distance_mm;
    log_measure[ringPos].distance2 = sensor5Measure[SENSOR2].distance_mm;
    log_measure[ringPos].distance3 = sensor5Measure[SENSOR3].distance_mm;
    log_measure[ringPos].distance4 = sensor5Measure[SENSOR4].distance_mm;
    log_measure[ringPos].distance5 = sensor5Measure[SENSOR5].distance_mm;
    log_measure[ringPos].v_air = v_air_mps();

    //結果 (平均値)
    log_result[ringPos].shot_num = shot_count;
    log_result[ringPos].impact_pos_x = calcResult.impact_pos_x_mm;
    log_result[ringPos].impact_pos_y = calcResult.impact_pos_y_mm;
    log_result[ringPos].radius0      = calcResult.radius0_mm;
    log_result[ringPos].temp         = air_temp_degree_c;

}


