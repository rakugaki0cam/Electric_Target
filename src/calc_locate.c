/*
 * calc_locate.c
 * 
 * �d�q�^�[�Q�b�g
 * ���W�̌v�Z
 * 
 *  2022.03.24
 * V2_edition
 *  2022.05.11  1-3�Ԗڂɓ��͂����Z���T����v�Z����
 *  
 * 
 * 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

#include "calc_locate.h"


//GLOBAL
sensor_data_t   sensor_4mic[NUM_SENSOR]= {
    {SENSOR1, 0xff, -SENSOR_HORIZONTAL_SPACING, -SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //�Z���T�[1 ����
    {SENSOR2, 0xff,  SENSOR_HORIZONTAL_SPACING, -SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //�Z���T�[2 �E��
    {SENSOR3, 0xff, -SENSOR_HORIZONTAL_SPACING,  SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //�Z���T�[3 ����
    {SENSOR4, 0xff,  SENSOR_HORIZONTAL_SPACING,  SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //�Z���T�[4 �E��
};

impact_data_t   result;         //�v�Z����

const float   delay_a = (5.0 / 300);    //�Z���T�[�x�ꎞ�Ԃ̌v�Z�W��
const float   delay_b = 10;


//LOCAL
sensor_data_t   tmp_3[3];       //�Z���T�f�[�^�󂯓n���p


//
calc_status_source_t calc_locate_xy(void){
    //���e���W�̌v�Z
    //�o�́@result:�v�Z���ʍ��Wx,y,r  �O���[�o��
    //     stat:0-����I���@1-�G���[
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    uint8_t     i;
    
    //�Z���T�[�x�ꎞ�Ԃ̌v�Z
    for (i = 0; i < NUM_SENSOR; i++){
        sensor_4mic[i].sensor_delay_usec = delay_sensor_usec(sensor_4mic[i].delay_time_usec);
    }   
    
    //�v�Z���ʃN���A
    result.pattern = 0;
    result.impact_pos_x_mm = 0;
    result.impact_pos_y_mm = 0;
    result.radius0_mm = 0;
    result.delay_time0_msec = 0;
    result.status = 0;

    calc_stat = calc_of_3sensor();   //���W���v�Z
    if (CALC_STATUS_OK == calc_stat){
        //�v�ZOK
        //���e����Z���T�I���܂ł̒x�ꎞ��(���e�����v�Z�p)
        result.delay_time0_msec = impact_time_msec(result.radius0_mm); 
    }else {
        //�v�Z�l���s�������ꍇ
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.delay_time0_msec = 0;        //�^�}���j�ł̃G���[����Ɏg�p
        //calc_stat = CALC_STATUS_CAL_ERROR;
    }
            
    return calc_stat;
}


//
calc_status_source_t calc_of_3sensor(void){
    //tmp_3��3�Z���T�f�[�^������W�l���v�Z
    //Output result:�v�Z�l
    //       calc_stat:��ԁ@0-OK,
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    
    //tmp_3�Ɍv�Z�p�f�[�^���
    if (asign_3sensor() != 0){ 
        //�������f�[�^���_���Ȏ�
        calc_stat = CALC_STATUS_NOT_ENOUGH;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.status = calc_stat;
        return calc_stat;
    }

    //�g�p�Z���T�[�̔ԍ��𐔎���
    calc_sensor_pattern();
    
    //���W�̌v�Z
    if (apollonius_3circle_xyr() != 0){
        //�v�Z���_���ȂƂ�
        calc_stat = CALC_STATUS_CAL_ERROR;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.status = calc_stat;
        return calc_stat;
    }
    
    //�v�Z���ʂ̔���
    if (result.radius0_mm > R_MAX){
        //r���傫������
        calc_stat = CALC_STATUS_R0_ERR;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.status = calc_stat;
        return calc_stat;
    }
    if ((result.impact_pos_x_mm < -TARGET_WIDTH_HALF) || (result.impact_pos_x_mm > TARGET_WIDTH_HALF)){
        //x���傫������
        calc_stat = CALC_STATUS_X0_ERR;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.status = calc_stat;
        return calc_stat;
    }
    if ((result.impact_pos_y_mm < -TARGET_HEIGHT_HALF) || (result.impact_pos_y_mm > TARGET_HEIGHT_HALF)){
        //y���傫������
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        calc_stat = CALC_STATUS_Y0_ERR;
        result.status = calc_stat;
        return calc_stat;
    }
    return calc_stat;
}


uint8_t asign_3sensor(void){
    //�Z���T3�P��I������tmp_3�֑��
    //�o��tmp_3-�v�Z�p��3�g�̃f�[�^
    //stat 0:OK, 1:����s��
    uint8_t i;
    uint8_t n = 0;
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
   
    //����l���
    for (i = 0; i < NUM_SENSOR; i++){
        if (sensor_4mic[i].status == SENSOR_STATUS_OK){
            if (sensor_4mic[i].input_order < 3){
                tmp_3[n] = sensor_4mic[i];
                n++;
                if (n > 3){
                    calc_stat = CALC_STATUS_TOO_MANY;
                    result.status = calc_stat;
                    return calc_stat;
                }
            }
        //�ꊇ��� .sensor_x, .sensor_y, .sensor_z, .distance_mm
        } else{
            //�f�[�^���_��
            calc_stat = CALC_STATUS_NOT_ENOUGH;
            result.status = calc_stat;
            return calc_stat;
        }
    }
    if (n != 3){
        calc_stat = CALC_STATUS_NOT_ENOUGH;
        result.status = calc_stat;
    }
    return calc_stat;
}


void    calc_sensor_pattern(void){
    //�g�p�Z���T�i���o�[��3���̐����ɂ��ċL�^
    result.pattern = (tmp_3[0].sensor_num + 1) * 0x100
                   + (tmp_3[1].sensor_num + 1) * 0x10
                   + (tmp_3[2].sensor_num + 1);
}


//*** calculation sub *****************
//locate
uint8_t apollonius_3circle_xyr(void){
    //���W�̌v�Z
    //���^�[���l�@0:����
    //          1:�G���[
    //���͒l tmp_3[3]: �Z���T�[�f�[�^ x�Ay�Az�A�x�ꋗ��
    //�o�͒l result: ���Wx�Ay�Ƌ���r0�̌v�Z�l
    
    uint8_t i;
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    
    float   x[4], y[4], z[4], dr[4];
    float   a[4], b[4], c[4], d[4];
    float   e, f[3], g[3];
    float   aa, bb, cc;
    
    //�f�[�^���
    for (i = 0; i < 3; i++){
        x[i + 1] = tmp_3[i].sensor_x_mm;
        y[i + 1] = tmp_3[i].sensor_y_mm;
        z[i + 1] = tmp_3[i].sensor_z_mm;
        dr[i + 1] = tmp_3[i].distance_mm;
    }
    
    //�萔�̌v�Z
    a[1] = x[2] - x[1];
    a[2] = x[3] - x[2];
    a[3] = x[1] - x[3];
    b[1] = y[2] - y[1];
    b[2] = y[3] - y[2];
    b[3] = y[1] - y[3];
    c[1] = dr[2] - dr[1];
    c[2] = dr[3] - dr[2];
    c[3] = dr[1] - dr[3];
    d[1] = (0 - x[1] * x[1] + x[2] * x[2] - y[1] * y[1] + y[2] * y[2] + dr[1] * dr[1] - dr[2] * dr[2]) / 2;
    d[2] = (0 - x[2] * x[2] + x[3] * x[3] - y[2] * y[2] + y[3] * y[3] + dr[2] * dr[2] - dr[3] * dr[3]) / 2;
    d[3] = (0 - x[3] * x[3] + x[1] * x[1] - y[3] * y[3] + y[1] * y[1] + dr[3] * dr[3] - dr[1] * dr[1]) / 2;
    e = a[1] * b[2] - a[2] * b[1];
    if (e == 0){
        //error���ꂪ�[��
        //e = 0.00001;
        calc_stat = CALC_STATUS_E_0_1;
        result.status = calc_stat;
        return calc_stat;
    }
    f[1] = (b[1] * c[2] - b[2] * c[1]) / e;
    f[2] = (a[2] * c[1] - a[1] * c[2]) / e;
    g[1] = (b[2] * d[1] - b[1] * d[2]) / e;
    g[2] = (a[1] * d[2] - a[2] * d[1]) / e;

    //���̌���
    aa = f[1] * f[1] + f[2] * f[2] - 1;
    bb = 2 * f[1] * (g[1] - x[1]) + 2 * f[2] * (g[2] - y[1]) - 2 * dr[1];
    cc = (g[1] - x[1]) * (g[1] - x[1]) + (g[2] - y[1]) * (g[2] - y[1]) + z[0] * z[0] - dr[1] * dr[1];
    
    if ((bb * bb - 4 * aa * cc) < 0){
        //error�@���̌����̏���
        calc_stat = CALC_STATUS_QUAD_F;
        result.status = calc_stat;
        return calc_stat;
    }
    
    //�񎟕�������������x,y,r�����߂�v�Z
    result.radius0_mm = (-bb + sqrt(bb * bb - 4 * aa * cc)) / (2 * aa);
    if (result.radius0_mm < 0){
        //���a�����̕��͕s�̗p
        result.radius0_mm = (-bb - sqrt(bb * bb - 4 * aa * cc)) / (2 * aa);
    }
    
    result.impact_pos_x_mm = f[1] * result.radius0_mm + g[1];
    result.impact_pos_y_mm = f[2] * result.radius0_mm + g[2];
    
    return calc_stat;
}


float   dist_delay_mm(float time_usec){
    //�������狗���̌v�Z
    return  time_usec * v_air_mps() / 1000;
}


//time
float   v_air_mps(void){
    //����m/sec�����߂� 
    return 331.5 + 0.61 * temp_ave_degree_c;
}


float   delay_time_usec(uint32_t timer_count){
    //�^�C�}�[�J�E���g�l��������usec�ɕϊ�
    return (float)timer_count / (TMR2_FrequencyGet() / 1000000);
}


float   delay_sensor_usec(float delay_time){
    //�Z���T�[�������E���Ă���R���p���[�^���I������܂ł̒x�ꎞ��usec
    //delay_time:���e�`�Z���T�I���܂ł̎��Ԃő�p�B�ȗ���
    float   correct_time;

    correct_time = delay_a * delay_time + delay_b;  //�␳�W���@dt��10�`16���炢time��0�`300usec
    return correct_time;
}


float   impact_time_msec(float r0_mm){
    //r0���璅�e�����𐄒�v�Z
    //���e�����͍ŏ��̃Z���T�I�����O�̎���
    //���r��t2�̒���`��鎞��(�\�ɋʂ�������A���ʂɓ`��鎞��)
#define TARGET_PLATE_T_MM   2       //����r�̃}�g�̌��� mm
#define V_PLATE_MPS         2300    //����r���̉��� m/sec(= mm/msec)
    float   dt_t2_msec;
    
    dt_t2_msec = TARGET_PLATE_T_MM / V_PLATE_MPS;   //msec
    
    return  -r0_mm / v_air_mps() + dt_t2_msec;      //msec
}


