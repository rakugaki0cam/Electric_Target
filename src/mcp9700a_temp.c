/*
 * mcp9700a_temp.c
 * 
 * ���x�Z���T�[
 * 
 *   0��  0.5V
 * 100��  1.5V
 * -40�`125�� �}2��
 * 
 * 
 *  2022.04.08
 * 
 */

#include "mcp9700a_temp.h"

#define     NUM_TEMP_DATA_BUFFER    4   //�o�b�t�@��
#define     NUM_SAMPLE_2_x          2   //4 = 2 ^ 2��

//GLOBAL
float       temp_ave_degree_c;          //�C���@��
        
//���ς���邽�߂̃f�[�^�o�b�t�@
uint16_t    temp_data[NUM_TEMP_DATA_BUFFER];



void    init_temp(void){
    //���x�Z���T������
    //�����f�[�^�𖄂߂Ă���
    uint8_t     i;
    
    for(i = 0; i < NUM_TEMP_DATA_BUFFER; i++){
        get_temp_adc();
        CORETIMER_DelayMs(100);
    }
    set_temp_degree_c();
}


float   set_temp_degree_c(void){
    //���x�̌v��
    //���ʂ̓O���[�o���ϐ��ɑ��
#define     AVDD    3.3
#define     ADC_BIT 1024    //10bit
    float   v;              //�Z���T�[�d��
    uint16_t adc_count;
    
    adc_count = average_temp_adc();
    v = (float)AVDD * adc_count / ADC_BIT;
    
    temp_ave_degree_c = calc_temp(v);
    
    return temp_ave_degree_c;
}


float   calc_temp(float adc_v){
    //�d�����牷�x�ւ̕ϊ� & �덷�␳�v�Z
    //in  �d��
    //out ���x
#define     T_MAX       125
#define     T_MIN       -40    
#define     EC2         (float)(-244 * 0.000001)
#define     EC1         (float)(2 * 0.000000000001)
#define     ERROR_T_MIN 2
    
#define     OFFSET_TEMP (float)-1.7         //��̔M�̉e��������     /////////////////////////
    
    float   temp_sensor_c;
    float   temp_compensated;

    //�d�����x�ւ̕ϊ�
    temp_sensor_c = (adc_v - 0.5 ) * 100;

    //�덷�␳�v�Z
    temp_compensated = temp_sensor_c
                     + EC2 * (T_MAX - temp_sensor_c) * (temp_sensor_c - T_MIN)
                     + EC1 * (temp_sensor_c - T_MIN)
                     + ERROR_T_MIN
                     + OFFSET_TEMP;
    return temp_compensated;
}


void     get_temp_adc(void){
    //���x��AD�J�E���g�l�����[
    static uint8_t cnt;     //�o�b�t�@�̈ʒu
    //AD�ϊ�
    ADC_SamplingStart();
    while(!ADC_ResultIsReady()){
    }
    temp_data[cnt] = ADC_ResultGet(ADC_RESULT_BUFFER_0);
    
    cnt++;
    if (cnt > NUM_TEMP_DATA_BUFFER){
        cnt = 0;
    } 
}


uint16_t    average_temp_adc(void){
    //���x�f�[�^AD�J�E���g�l�̕��ϒl
    uint8_t     i;
    uint32_t    sum = 0;
    
    for(i = 0; i < NUM_TEMP_DATA_BUFFER; i++){
        sum += temp_data[i];
    }
    sum = sum >> NUM_SAMPLE_2_x;         //�T���v�����@2�ׂ̂���
    
    return (uint16_t)sum;
}

