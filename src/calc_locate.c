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
 * V4�Qedition
 *  2023.08.26  4�ʂ�̌v�Z�l����G���[�����o
 *              1�̃Z���T�[�l���ُ�̏ꍇ�A4�̍��W���o���o���ɂȂ�B
 *              �� ���������W�ُ͈�l�𔲂�������3�̃Z���T���狁�߂�1�����̍��W�B�ǂꂪ���������W�Ȃ̂��̔��肪����B
 * 
 * 
 */

#include "calc_locate.h"


#define     CALC_TEST_no   //�v�Z�e�X�g = �f�B���C���v�Z���Ȃ�

//GLOBAL
//���Ƃ�measure_v3.c�ֈړ�
//������ɂ͏����I�Ȃ��̂͂Ȃ��Ƃ���
sensor_data_t   sensor_4mic[NUM_SENSOR]= {
    {SENSOR1, 0xff, -SENSOR_HORIZONTAL_SPACING, -SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //�Z���T�[1 ����
    {SENSOR2, 0xff,  SENSOR_HORIZONTAL_SPACING, -SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //�Z���T�[2 �E��
    {SENSOR3, 0xff, -SENSOR_HORIZONTAL_SPACING,  SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //�Z���T�[3 ����
    {SENSOR4, 0xff,  SENSOR_HORIZONTAL_SPACING,  SENSOR_VERTICAL_SPACING, SENSOR_DEPTH_OFFSET, 0, 0, 0, 0, 0, 0, 0},    //�Z���T�[4 �E��
};

impact_data_t   result[NUM_PATTERN + 1];    //�v�Z���� �Ō�̂Ƃ���ɕ��ς����[���邽��+1

const float   delay_a = (5.0 / 300);        //�Z���T�[�x�ꎞ�Ԃ̌v�Z�W��
const float   delay_b = 10;


//LOCAL
sensor_data_t   tmp_3[3];       //�Z���T�f�[�^�󂯓n���p

uint8_t sensor_order[NUM_PATTERN][3] = {        //4�Z���T����3�P��I������4�̃p�^�[��
    { SENSOR1, SENSOR2, SENSOR3},
    { SENSOR1, SENSOR2, SENSOR4},
    { SENSOR1, SENSOR3, SENSOR4},
    { SENSOR2, SENSOR3, SENSOR4}
};


//
calc_status_source_t calc_locate_xy(void){
    //���e���W�̌v�Z
    //�o�́@result:�v�Z���ʍ��Wx,y,r  �O���[�o��
    //     stat:0-����I���@1-�G���[
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    uint8_t     i, pattern;
    uint8_t     average_cnt = 0;        //���ϒl�T���v�����̃J�E���g
    
    //�Z���T�[�x�ꎞ�Ԃ̌v�Z
    for (pattern = 0; pattern < NUM_PATTERN; pattern++){
        sensor_4mic[pattern].sensor_delay_usec = delay_sensor_usec(sensor_4mic[pattern].delay_time_usec);
    }   
    
    //�v�Z���ʃN���A
    for (i = 0; i < (NUM_PATTERN + 1); i++){
        clear_result(i);
    }
    result[NUM_PATTERN].pattern = 0xffff;   //���ϒl���ʃ}�[�N

    //�p�^�[���񐔕����v�Z
    for (pattern = 0; pattern < NUM_PATTERN; pattern++){
        calc_stat = calc_of_3sensor(pattern);   //���W���v�Z
        if (CALC_STATUS_OK == calc_stat){
            //�v�ZOK
            //���ς̂��߂̐ώZ-[NUM_PATTERN]�ɑ��a������
            result[NUM_PATTERN].impact_pos_x_mm += result[pattern].impact_pos_x_mm;
            result[NUM_PATTERN].impact_pos_y_mm += result[pattern].impact_pos_y_mm; 
            result[NUM_PATTERN].radius0_mm      += result[pattern].radius0_mm; 
            average_cnt ++;
        }else {
            //�v�Z�l���s�������ꍇ
            result[pattern].radius0_mm = 999.99;
            result[pattern].impact_pos_x_mm = 999.99;
            result[pattern].impact_pos_y_mm = 999.99;
            result[pattern].delay_time0_msec = 0;        //�^�}���j�ł̃G���[����Ɏg�p
            //calc_stat = CALC_STATUS_CAL_ERROR;
            //���όv�Z�Ɋ܂߂Ȃ�
        }
    }
    
    //���ς̌v�Z result�z��̍Ō�ɕ��ϒl����
    if (average_cnt == 0){
        calc_stat = CALC_STATUS_AVERAGE_ERR;
        result[NUM_PATTERN].status = calc_stat;
        return calc_stat;
    }
    result[NUM_PATTERN].impact_pos_x_mm = result[NUM_PATTERN].impact_pos_x_mm / average_cnt;
    result[NUM_PATTERN].impact_pos_y_mm = result[NUM_PATTERN].impact_pos_y_mm / average_cnt;
    result[NUM_PATTERN].radius0_mm      = result[NUM_PATTERN].radius0_mm      / average_cnt;
    
    //�΍��̃`�F�b�N
    calc_stat = check_deviation();
    if ((calc_stat != CALC_STATUS_OK) && (calc_stat != CALC_STATUS_AVERAGE_FIX)){
        return calc_stat;
    }
    
    //���e����Z���T�I���܂ł̒x�ꎞ�� ���ϒl(���e�����v�Z�p)
    result[NUM_PATTERN].delay_time0_msec = impact_time_msec(result[NUM_PATTERN].radius0_mm);      
       
    return calc_stat;
}

void    clear_result(uint8_t n){
    //�v�Z���ʂ��N���A
    result[n].pattern = 0;
    result[n].impact_pos_x_mm = 0;
    result[n].impact_pos_y_mm = 0;
    result[n].radius0_mm = 0;
    result[n].delay_time0_msec = 0;
    result[n].status = 0;
}

//
calc_status_source_t calc_of_3sensor(uint8_t n){
    //tmp_3��3�Z���T�f�[�^������W�l���v�Z
    //Input  n:�p�^�[���ԍ�    
    //Output result:�v�Z�l
    //       calc_stat:��ԁ@0-OK,
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    
    //tmp_3�Ɍv�Z�p�f�[�^���
    if (asign_3sensor(n) != 0){ 
        //�������f�[�^���_���Ȏ�
        calc_stat = CALC_STATUS_NOT_ENOUGH;
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        result[n].status = calc_stat;
        return calc_stat;
    }

    //�g�p�Z���T�[�̔ԍ��𐔎���
    calc_sensor_pattern(n);
    
    //���W�̌v�Z
    if (apollonius_3circle_xyr(n) != 0){
        //�v�Z���_���ȂƂ�
        calc_stat = CALC_STATUS_CAL_ERROR;
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        result[n].status = calc_stat;
        return calc_stat;
    }
    
    //�v�Z���ʂ̔���
    if (result[n].radius0_mm > R_MAX){
        //r���傫������
        calc_stat = CALC_STATUS_R0_ERR;
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        result[n].status = calc_stat;
        return calc_stat;
    }
    if ((result[n].impact_pos_x_mm < -TARGET_WIDTH_HALF) || (result[n].impact_pos_x_mm > TARGET_WIDTH_HALF)){
        //x���傫������
        calc_stat = CALC_STATUS_X0_ERR;
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        result[n].status = calc_stat;
        return calc_stat;
    }
    if ((result[n].impact_pos_y_mm < -TARGET_HEIGHT_HALF) || (result[n].impact_pos_y_mm > TARGET_HEIGHT_HALF)){
        //y���傫������
        result[n].radius0_mm = 999.99;
        result[n].impact_pos_x_mm = 999.99;
        result[n].impact_pos_y_mm = 999.99;
        calc_stat = CALC_STATUS_Y0_ERR;
        result[n].status = calc_stat;
        return calc_stat;
    }
    return calc_stat;
}


uint8_t asign_3sensor(uint8_t n){
    //�Z���T3�P��I������tmp_3�֑��
    //�o��tmp_3-�v�Z�p��3�g�̃f�[�^
    //stat 0:OK, 1:����s��
    uint8_t i;
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
   
    //����l���
    for (i = 0; i < 3; i++){
        if (sensor_4mic[sensor_order[n][i]].status == SENSOR_STATUS_OK){
            tmp_3[i] = sensor_4mic[sensor_order[n][i]];
        //�ꊇ��� .sensor_x, .sensor_y, .sensor_z, .distance_mm
        } else{
            //�f�[�^���_��
            calc_stat = CALC_STATUS_NOT_ENOUGH;
            result[n].status = calc_stat;
            return calc_stat;
        }
    }
    
    return calc_stat;
}


void    calc_sensor_pattern(uint8_t n){
    //�g�p�Z���T�i���o�[��3���̐����ɂ��ċL�^
    result[n].pattern = (tmp_3[0].sensor_num + 1) * 0x100
                   + (tmp_3[1].sensor_num + 1) * 0x10
                   + (tmp_3[2].sensor_num + 1);
}

uint8_t check_deviation(void){
    //�΍��̃`�F�b�N ���ςƂ̍����傫�����̂�����
    //���ϒl���Čv�Z
    
#define TARGET_SIZE_A4
#ifdef  TARGET_SIZE_A4
    #define     DEVIATION_XY    2.0                 //�΍� xy
    #define     DEVIATION_R     3.0                 //�΍� r
#else   //TARGET_SIZE_A3 
    #define     DEVIATION_XY    100.0               ///////////////////////TEST
    #define     DEVIATION_R     200.0
#endif
    
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    uint8_t     pattern;
    uint8_t     average_cnt = 0;        //���ϒl�T���v�����̃J�E���g
    float       dx, dy, dr;             //�΍�
    
    for (pattern = 0; pattern < NUM_PATTERN; pattern++){
        calc_stat = result[pattern].status;
        if(calc_stat == CALC_STATUS_OK){
            //�΍��̌v�Z
            dx = result[pattern].impact_pos_x_mm - result[NUM_PATTERN].impact_pos_x_mm;
            dy = result[pattern].impact_pos_y_mm - result[NUM_PATTERN].impact_pos_y_mm;
            dr = result[pattern].radius0_mm      - result[NUM_PATTERN].radius0_mm;

            if((dx < -DEVIATION_XY) || (dx > DEVIATION_XY)){
                calc_stat = CALC_STATUS_X0_DEV_ERR;
            }
            if((dy < -DEVIATION_XY) || (dy > DEVIATION_XY)){
                calc_stat = CALC_STATUS_Y0_DEV_ERR;
            }
            if((dr < -DEVIATION_R) || (dr > DEVIATION_R)){
                calc_stat = CALC_STATUS_R0_DEV_ERR;
            }
            if (calc_stat != CALC_STATUS_OK){
                //�΍��G���[�����鎞
                result[pattern].status = calc_stat;
            }
        }
    }
    
    if (calc_stat != CALC_STATUS_OK){
        //�΍��G���[�����鎞�͕΍��f�[�^�����O���ĕ��ς���蒼��
        
        //���ϗp���������N���A
        clear_result(NUM_PATTERN);
        result[NUM_PATTERN].pattern = 0xffff;       //���ϒl���ʃ}�[�N
        
        
        //���ς̂��߂̐ώZ���Ȃ���
        for (pattern = 0; pattern < NUM_PATTERN; pattern++){
            if (result[pattern].status == CALC_STATUS_OK){
                result[NUM_PATTERN].impact_pos_x_mm += result[pattern].impact_pos_x_mm;
                result[NUM_PATTERN].impact_pos_y_mm += result[pattern].impact_pos_y_mm; 
                result[NUM_PATTERN].radius0_mm      += result[pattern].radius0_mm; 
                average_cnt ++;
            }
        }
        
        //�T���v�����`�F�b�N
        if (average_cnt == 0){
            calc_stat = CALC_STATUS_AVERAGE_ERR;
            result[NUM_PATTERN].status = calc_stat;
            return calc_stat;
        }
        
        result[NUM_PATTERN].impact_pos_x_mm = result[NUM_PATTERN].impact_pos_x_mm / average_cnt;
        result[NUM_PATTERN].impact_pos_y_mm = result[NUM_PATTERN].impact_pos_y_mm / average_cnt;
        result[NUM_PATTERN].radius0_mm      = result[NUM_PATTERN].radius0_mm      / average_cnt;
        
        calc_stat = CALC_STATUS_AVERAGE_FIX;        //���ϒl�̍Čv�Z���s��ꂽ
        result[NUM_PATTERN].status = calc_stat;
    }
    return calc_stat;
}


//*** calculation sub *****************
//locate
uint8_t apollonius_3circle_xyr(uint8_t num_result){
    //���W�̌v�Z
    //���^�[���l�@0:����
    //          1:�G���[
    //���͒l tmp_3[3]: �Z���T�[�f�[�^ x�Ay�Az�A�x�ꋗ��
    //�o�͒l result[]: ���Wx�Ay�Ƌ���r0�̌v�Z�l
    //      num_result: ���ʂ�������ԍ�

    uint8_t i;
    calc_status_source_t  calc_stat = CALC_STATUS_OK;
    
    float   x[4], y[4], z[4], dr[4];    //�Y����1,2,3���g�p
    float   a[4], b[4], c[4], d[4];
    float   e, f[3], g[3];
    float   aa, bb, cc;
    float   q, r0;
    
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
        result[num_result].status = calc_stat;
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
    
    q = bb * bb - 4 * aa * cc;
    if (q < 0){
        //error�@���̌����̏���
        calc_stat = CALC_STATUS_QUAD_F;
        result[num_result].status = calc_stat;
        return calc_stat;
    }
    
    //�񎟕�������������x,y,r�����߂�v�Z
    r0 = (-bb - sqrt(q)) / (2 * aa);
    if (r0 < 0){
        //���a�����̕��͕s�̗p
        r0 = (-bb + sqrt(q)) / (2 * aa);
    }
    result[num_result].radius0_mm = r0;
    result[num_result].impact_pos_x_mm = f[1] * r0 + g[1];
    result[num_result].impact_pos_y_mm = f[2] * r0 + g[2];
    
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
#ifndef CALC_TEST
    correct_time = delay_a * delay_time + delay_b;  //�␳�W���@dt��10�`16���炢time��0�`300usec
#else
    correct_time = 0;
#endif    
    
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


