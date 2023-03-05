/*
 * measure_v3.c
 * 
 * �d�q�^�[�Q�b�g
 * ����ƕ\��
 * 
 *  2022.04.16
 * V2_edition
 *  2022.05.11  1-3�Ԗڂɓ��͂����Z���T����v�Z����
 * V3_edition
 *  2023.01.20  wifi�ǉ�
 * 
 *  
 */

#include "measure_v3.h"

//GLOBAL


//LOCAL
typedef enum {
    INPUT_ORDER_STATUS_OK    = 0x0,
    INPUT_ORDER_STATUS_FIXED = 0x1,       
    INPUT_ORDER_STATUS_ERROR = 0x2,        
} input_order_status_source_t;


void measure_init(void){
    //�����ݒ�
    float       center_offset_x = -0.1;     //�}�g�Z���^�[���C���ƃZ���T�[�̃Y��///////////////////////////
    float       center_offset_y = +1.0;
    
    sensor_offset_calc(center_offset_x, center_offset_y);
    log_title();
    
    //�^�[�Q�b�gLCD�N���A�R�}���h���M
    uint8_t clear_command[] = "TARGET_CLEAR END ,";
    while(!UART2_TransmitterIsReady());
    UART2_Write(clear_command, strlen((char*)clear_command));
    
}


uint8_t measure_main(void){
    //���胁�C���`�v�Z
    measure_status_source_t     meas_stat = MEASURE_STATUS_OK;
    calc_status_source_t        ans;
    //����f�[�^�����m�F�C��
    input_order_check();
    //����f�[�^���v�Z�A���
    if (measure_data_assign() < 3){
        //���萔������Ȃ���(�v�Z�ɂ�3�ȏ�̃f�[�^���K�v)
        meas_stat = MEASURE_STATUS_NOT_ENOUGH;
        result.radius0_mm = 999.99;
        result.impact_pos_x_mm = 999.99;
        result.impact_pos_y_mm = 999.99;
        result.delay_time0_msec = 0;        //�^�}���j�ł̃G���[����Ɏg�p
        return meas_stat;
    }
    //���W�̌v�Z
    ans = calc_locate_xy();
    //printf("calc status:%d", ans);
    if (CALC_STATUS_OK != ans){       
        //�G���[�̎�
        meas_stat = MEASURE_STATUS_ERROR;
        
    }
    
    return meas_stat;
}


//
uint8_t input_order_check(void){
    //���͏����̍Ċm�F�C��
    //���͎��Ԃ��߂��ꍇ�Ɋ���������������ւ�邱�Ƃ�����...�����ۗ̕�����������ꍇ�A��������IRQ#���ɂȂ邽��
    input_order_status_source_t status = INPUT_ORDER_STATUS_OK;
    uint8_t     i;
    uint8_t     sensor_number;             
    uint8_t     input_order[NUM_SENSOR];      //���͏��v�Z����
    
    for (sensor_number = 0; sensor_number < NUM_SENSOR; sensor_number++){
        input_order[sensor_number] = 0;
        
        for (i = 0; i < NUM_SENSOR; i++){
            if (sensor_4mic[sensor_number].timer_cnt > sensor_4mic[i].timer_cnt){
                //�����̒l��菬�����l�����������J�E���g����
                input_order[sensor_number]++;
                //�����l�����������͓������ł��Ă��܂�......������////////////////////
            }
        }
        
        if (sensor_4mic[sensor_number].input_order != 0xff){
            //�����o�Z���T�͖����o�̂܂�
            if (sensor_4mic[sensor_number].input_order != input_order[sensor_number]){
                //�C�����������ꍇ
                sensor_4mic[sensor_number].input_order = input_order[sensor_number];
                status = INPUT_ORDER_STATUS_FIXED;
            }
        }
    }
    return status;
}


uint8_t measure_data_assign(void){
    //����f�[�^����
    //�o��: �L������Z���T�[��
    measure_status_source_t   sensor_status = SENSOR_STATUS_OK;
    uint8_t     sensor_number;
    uint8_t     zero;                       //�ŏ��ɃI�������Z���T�[�̔ԍ�
    uint8_t     valid_sensor_count = 0;     //�L������Z���T�[���̃J�E���g
    
    for (sensor_number = 0; sensor_number < NUM_SENSOR; sensor_number++){
        switch(sensor_4mic[sensor_number].input_order){
            case 0:
                //�ŏ��ɃI�������Z���T�[�ԍ�
                zero = sensor_number;
                
            case 1 ... (NUM_SENSOR - 1):
                valid_sensor_count ++;
                sensor_status = SENSOR_STATUS_OK;
                break;
                
            case 0xff:
                sensor_status = SENSOR_STATUS_INVALID;
                break;
                
            default:
                sensor_status = SENSOR_STATUS_ERROR;
                break;
        }
        sensor_4mic[sensor_number].status = sensor_status;
    }
    
    //�^�C�}�J�E���g���A���ԍ��A�����̌v�Z�Ƒ��
    for (sensor_number = 0; sensor_number < NUM_SENSOR; sensor_number++){
        if (SENSOR_STATUS_OK == sensor_4mic[sensor_number].status){
            //�X�e�[�^�XOK�̂Ƃ������v�Z�A���
            sensor_4mic[sensor_number].delay_cnt 
                = sensor_4mic[sensor_number].timer_cnt - sensor_4mic[zero].timer_cnt;   //�J�E���g��
            sensor_4mic[sensor_number].delay_time_usec
                = delay_time_usec(sensor_4mic[sensor_number].delay_cnt);                //�J�E���g�l������
            sensor_4mic[sensor_number].distance_mm
                = dist_delay_mm(sensor_4mic[sensor_number].delay_time_usec
                - sensor_4mic[sensor_number].sensor_delay_usec);   //���ԁ�����
        }
        
    }

    return valid_sensor_count;
}


//���W�f�[�^���f�o�b�K��LCD&WiFi�ɏo��
void result_disp(uint16_t shot_count, measure_status_source_t meas_stat, uint8_t mode){ 
    //���茋�ʁA�v�Z���ʂ̕\��
    
    esp32wifi_data_send();  //���e�\���𑁂�����
    while(!UART2_TransmitComplete());
    
    CORETIMER_DelayMs(100); //RS485�֑��M�̑O�ɊԂ��J����^�}���j����f�o�b�K��printf���Ă��邩������Ȃ�����
    tamamoni_data_send();   //LAN
    
    CORETIMER_DelayMs(30);  //�f�[�^���M�̊Ԃ�����
    switch(mode){
        case NONE:
            break;
        case SINGLE_LINE:
            single_line(shot_count, meas_stat);
            break;
        case FULL_DEBUG:
            full_debug(shot_count, meas_stat);
            break;
        case CSV_DATA:
            csv_data(shot_count);
            break;
        default:
            //error
            break;
    }    
}


//
void tamamoni_data_send(void){
    //�^�}���j�֍��W�f�[�^���𑗐M
    printf("BINX0Y0dT %8.3f %8.3f %8.4f END ,", result.impact_pos_x_mm, result.impact_pos_y_mm, result.delay_time0_msec);
}


void esp32wifi_data_send(void){
    //ESP32�֍��W�f�[�^���𑗐M
    //WiFi & target LCD
    char    buf[255];

    sprintf(buf, "BINX0Y0dT %8.3f %8.3f %8.4f END ,", result.impact_pos_x_mm, result.impact_pos_y_mm, result.delay_time0_msec);
    while(!UART2_TransmitterIsReady());
    UART2_Write(buf, strlen(buf));
}


//DEBUGger
void single_line(uint16_t shot_count, measure_status_source_t meas_stat){
    //���W�̂݁@1�s�\��
    
    if (meas_stat != MEASURE_STATUS_OK){
        printf("%02d:measurement error\n", shot_count);
        return;
    }else {
        printf("%02d:%6.1fx %6.1fy\n", shot_count, result.impact_pos_x_mm, result.impact_pos_y_mm);
    }
}


void full_debug(uint16_t shot_count, measure_status_source_t meas_stat){
    //�f�o�b�O�p�t���\��
    sensor_status_source_t    sens_stat;
    calc_status_source_t    calc_stat;
    uint8_t i, n;
    
    printf("\n>shot%03d ***%5.1f%cC\n", shot_count, temp_ave_degree_c, 0xdf);
    
#ifndef  SENSOR_DATA_DEBUG_TIMER_VALUE
    //���͏��^�C�}�[�J�E���g�\�� 
    for (i = 0; i < NUM_SENSOR; i++){
        for (n = 0; n < NUM_SENSOR; n++){
            if (sensor_4mic[n].input_order == i){
            //���͏��ԍ�����v����܂ő���
                break;
            }
        }
        printf("t%1d %08x (", i + 1, sensor_4mic[n].timer_cnt);  //���͏��^�C�}�l 

        if (n == 4){
            printf("--)\n");    //���͂Ȃ�

        }else {
            printf("s%1d)\n", n + 1);  //�Z���T#
        }
    }
#endif         
    for (i = 0; i < NUM_SENSOR; i++){
        sens_stat = sensor_4mic[i].status;
        if (SENSOR_STATUS_OK == sens_stat){
            printf("s%1d %7.2fus %5.1fmm\n", i + 1, sensor_4mic[i].delay_time_usec, sensor_4mic[i].distance_mm);
        }else{
            //���o���Ă��Ȃ���
            printf("s%1d  ---.--us\n", i + 1);
        }
    }
    if (meas_stat != MEASURE_STATUS_OK){
        printf(">measurement error\n");
        return;
    }
    
    //�g�p�Z���T�[�ԍ�
    printf("adopted sensor#%3x\n",result.pattern); 
    //�v�Z����
    printf("   x0     y0     r0 \n");
    printf("%6.1f %6.1f %6.1f\n", result.impact_pos_x_mm, result.impact_pos_y_mm, result.radius0_mm);
    //�G���[
    calc_stat = result.status;
    switch(calc_stat){
        case CALC_STATUS_X0_ERR:
            printf(" x error\n");
            break;
        case CALC_STATUS_Y0_ERR:
            printf(" y error\n");
            break;
        case CALC_STATUS_R0_ERR:
            printf(" r error\n");
            break;
        default:
            break;
    }
}


void    csv_title(void){
    //�\�v�Z�p��CSV�@�^�C�g����
    uint8_t i;
    
    //1�s��
    printf("                                         ,");   //�^�}���j�f�[�^����
    printf("shot#,x      ,y      ,r      ,status,sens");
    for (i = 1; i <= NUM_SENSOR; i++){
        printf(",dt sen%1d,sen%1d_d ,sen%1d_r", i, i, i);
    }
    printf(",temp\n");
    //2�s��
    printf("                                         ,");
    printf("     ,mm     ,mm     ,mm     ,      ,    ");
    for (i = 1; i <= NUM_SENSOR; i++){
        printf(",usec   ,usec   ,mm    ");
    }
    printf(",deg  \n");
}


void    csv_data(uint16_t shot_count){
    //�\�v�Z�p��CSV�f�[�^�@�R���}��؂�
    uint8_t i;
    uint8_t stat;
    
    //�ԍ�
    printf("  %03d", shot_count);
    //����x,y,r
    printf(",%7.2f,%7.2f,%7.2f", result.impact_pos_x_mm, result.impact_pos_y_mm, result.radius0_mm);
    //�X�e�[�^�X
    stat = result.status;
     if ((stat == CALC_STATUS_OK) || (stat == CALC_STATUS_AVERAGE_FIX)){
        printf(",OK    ");
    }else{
        printf(",error ");
    }
    //�g�p�Z���T�[�ԍ�
    printf(", %3x",result.pattern);

    //�Z���T�[���@t,dt,dr
    for (i = 0; i < NUM_SENSOR; i++){
        printf(",%7.2f,%7.2f,%6.1f", sensor_4mic[i].delay_time_usec, sensor_4mic[i].sensor_delay_usec, sensor_4mic[i].distance_mm);
    }
    //���x
    printf(",%5.1f", temp_ave_degree_c);
    
    printf("\n");
}


//
void data_clear(void){
    //����f�[�^���N���A
    uint8_t i;
    
    LED_RED_Clear();
    sensor_count = 0;
    
    //����l�N���A
    for (i = 0; i < NUM_SENSOR; i++){
        sensor_4mic[i].input_order = 0xff;      //�����͔���p��0�ł͂Ȃ���0xff
        sensor_4mic[i].timer_cnt = 0xffffffff;  //�^�C���������鎞�̂��߂ɍő�̒l�ɂ��Ă���
        sensor_4mic[i].delay_cnt = 0;
        sensor_4mic[i].delay_time_usec = 0;
        sensor_4mic[i].sensor_delay_usec = 0;
        sensor_4mic[i].d_time_corr_usec = 0;
        sensor_4mic[i].distance_mm = 0;
        sensor_4mic[i].status = 0;
    }
   
    ICAP1_Enable();
    ICAP2_Enable();
    ICAP3_Enable();
    ICAP4_Enable();
    
    TMR2 = 0;
    TMR2_Start(); 
}


void    sensor_offset_calc(float dx, float dy){
    //�}�g�̃Z���^�[���C���ƃZ���T�[�ʒu�̃Y����␳
    //�Z���T�[�̕����ʒu�̃Y���␳
    float   sensor1_offset_x = -0.1;
    float   sensor1_offset_y = 0;
    float   sensor1_offset_z = 0;

    float   sensor2_offset_x = -0.1;
    float   sensor2_offset_y = 0;
    float   sensor2_offset_z = 0;

    float   sensor3_offset_x = 0;
    float   sensor3_offset_y = 0;
    float   sensor3_offset_z = 0;

    float   sensor4_offset_x = 0;
    float   sensor4_offset_y = 0;
    float   sensor4_offset_z = 0;

    //x
    sensor_4mic[0].sensor_x_mm = -SENSOR_HORIZONTAL_SPACING + sensor1_offset_x + dx;
    sensor_4mic[1].sensor_x_mm =  SENSOR_HORIZONTAL_SPACING + sensor2_offset_x + dx;
    sensor_4mic[2].sensor_x_mm = -SENSOR_HORIZONTAL_SPACING + sensor3_offset_x + dx;
    sensor_4mic[3].sensor_x_mm =  SENSOR_HORIZONTAL_SPACING + sensor4_offset_x + dx;
    //y        
    sensor_4mic[0].sensor_y_mm = -SENSOR_VERTICAL_SPACING + sensor1_offset_y + dy;
    sensor_4mic[1].sensor_y_mm = -SENSOR_VERTICAL_SPACING + sensor2_offset_y + dy;
    sensor_4mic[2].sensor_y_mm =  SENSOR_VERTICAL_SPACING + sensor3_offset_y + dy;
    sensor_4mic[3].sensor_y_mm =  SENSOR_VERTICAL_SPACING + sensor4_offset_y + dy;
    //z
    sensor_4mic[0].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor1_offset_z;
    sensor_4mic[1].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor2_offset_z;
    sensor_4mic[2].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor3_offset_z;
    sensor_4mic[3].sensor_z_mm =  SENSOR_DEPTH_OFFSET + sensor4_offset_z;

}
