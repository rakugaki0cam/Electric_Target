 /*
  * �d�q�^�[�Q�b�g
  * V2  1-3�Ԗڂɓ��͂����Z���T�[�݂̂Ōv�Z
  *     ����܂�悭�Ȃ�
  * 
  * 32�r�b�g��
  * 
  * PIC32MX270F256B
  * 
  * MAIN Generated Driver File
  * 
  * @file main.c
  * 
  * @defgroup main MAIN
  * 
  * @brief This is the generated driver implementation file for the MAIN driver.
  *
  * @version MAIN Driver Version 1.0.0
  * 
  * 2022.03.29
  * 
  * 2022.03.29  ver.1.0.1   PIC18F27Q84 8bit ����ڐA
  *                         Input Capture��4ch�g�p
  * 2022.04.02  ver.1.0.2   �o�O�t�B�N�X
  * 2022.04.03  ver.1.1.0   �v�Z�G���[�����ƕ\���𐮗�
  *                         DEBUGger��R�L�[�ŕ\�����[�h�؂�ւ��@DEBUG ���� 1LINE
  * 2022.04.06  ver.1.1.2   ICAP�����͔����Ԋu���߂��ꍇ�ɏ����Ɉڂ鏇�Ԃ��ς�邱�Ƃ�����悤�B
  *                         2�ȏ�̊������ۗ����̎��A�����D��x�ݒ肪�����ꍇ�AIRQ#�̏��Ɏ��s����邱�ƂƂȂ�B
  *                         ���͏��������߂鏈����ǉ�
  * 2022.04.09  ver.1.1.3   ���x����
  * 2022.04.10  ver.1.1.4   ���x��1�b�����ɑ��肵��16�l�̕���
  * 2022.04.11  ver.1.1.5   �Z���T�[�ʒu�C���@�I�t�Z�b�g�l��ǉ�
  * 2022.04.12  ver.1.1.6   �Z���T�I������R���p���[�^�I���܂ł̒x�ꎞ�Ԃ�␳�@10~16usec���炢�̒x�ꂪ����B�����ɂ��
  * 2022.04.13  ver.1.1.7   �Z���T�g�ݕt���A�܂ƔŒ葼�H��B�Z���T�ʒu�Y���␳�I�t�Z�b�g�ǉ��B���x�Z���T�M�΍�B
  * 2022.04.14  ver.1.1.8   ���r��t2���𔲂��āA���̔������͗��ʂƂ��Ă݂�BSENSOR_DEPTH_OFFSET(z) 8 �� 6
  * 2022.04.15  ver.1.1.9   ��x�v�Z���ċ��������߂Ă���A�Z���T�x�ꎞ�ԕ␳�l���v�Z���āA�{�v�Z����B->���܂������Ȃ��B�␳����Ȃ������B
  * 2022.04.16  ver.1.10    csv�����ƃ��O�f�[�^�ۊ�
  * 2022.04.16  ver.1.11    �Z���T�x��␳�v�Z�[���莞�ԍ�����̌v�Z�̕��������݂����B
  * 2022.04.17  ver.1.20    �ʐM��RS485��(���M�����RTS���g��)�A���e�^�C�~���OPT4�A�E�g�v�b�g�ǉ�
  * 2022.05.03  ver.1.21    �^�}���j�p�o�C�i���f�[�^�o��
  * 
  * V2_edition
  * 2022.05.11  ver.2.00    V1���v���W�F�N�g���R�s�[ (�t�@�C���ŃR�s�[������AMPLABX�ɂăv���W�F�N�g�l�[����ύX���AHarmony���N���Acode�쐬�B
  *                         �R�s�[�O�̂��̂ƃ����N����Ă���.c .h�t�@�C�����v���W�F�N�g���珜�O���ăR�s�[����.c .h�t�@�C����Add Existing Item�Œǉ�����B
  * 2022.05.14  ver.2.01    1�`3�Ԗڂɓ��͂����Z���T�[�݂̂Ōv�Z����B
  * 
  * 2022.05.16              ����l�����������ɂ΂�����Ƃ�����悤�ł���܂�悭�Ȃ��B
  * 2022.11.08  ver.2.02    �^�}���j�̃f�o�b�O�ł�printf�ŁhR�h�̕������o���Ƃ���Mode���ς���Ă��܂��B"R"1���������̂Ƃ��̂�mode�ύX�B
  * 2022.11.10  ver.2.04    UART�̎�MFIFO�o�b�t�@8���x�����I�[�o�[�t���[���ăG���[�B�\�t�g���Z�b�g���Ȃ��Ǝ�M�ł��Ȃ��B
  * 
  * 
*/

#include "header.h"


//�R���p���[�^臒l�ݒ�
#define     V_DD        3.3                     //�d���d��
#define     V_TH        0.200                   //�R���p���[�^臒l default:0.200V
#define     RA          1000                    //������R+��
#define     RB          1000                    //������R-��
#define     V_REF       V_DD * RB / (RA + RB)   //CVref+ ����Ra:(Ra+Rb)
//���e�^�C�~���O�̃t�H�[���G�b�W���^�}���j�ɑ���
#define     impact_PT4_set()    PT4_Clear()     //���e�Z���T�M���o��
#define     impact_PT4_reset()  PT4_Set()       //���e�Z���T�M���N���A
#define     BUF_NUM     64                      //UART�f�[�^�Ǎ��o�b�t�@��


//GLOBAL
uint16_t    ring_pos = 0;                   //���O�f�[�^�|�C���^

//LOCAL
uint8_t     version[] = "2.04";             //�o�[�W�����i���o�[
uint8_t     sensor_count;                   //�Z���T���͏��Ԃ̃J�E���g
bool        flag_1sec = 0;                  //1�b�^�C�}�[����
char        buf[BUF_NUM];                   //UART�f�[�^�Ǎ��o�b�t�@
uint8_t     i;

/*
    Main application
*/

int main(void){
    measure_status_source_t   meas_stat;
    display_mode_source_t     disp_mode = SINGLE_LINE;

    uint16_t    shot_count = 0;     //�V���b�g�J�E���g��1����B0�͓��͖���

    
    
#if TX_TEST
    uint16_t    data;
#endif
    
    //Initialize all modules
    SYS_Initialize ( NULL );

    //Pin Interrupt
    ICAP1_CallbackRegister(sensor1_detect, NULL);
    ICAP2_CallbackRegister(sensor2_detect, NULL);
    ICAP3_CallbackRegister(sensor3_detect, NULL);
    ICAP4_CallbackRegister(sensor4_detect, NULL);
    //TMR2 Interrupt
    TMR2_CallbackRegister(timer2_1sec_int, NULL);
    
    //�R���p���[�^�d���o��
    CVR_UpdateValue((uint8_t)(V_TH / (V_REF / 24) + 0.99));      //�؂�グ
    CVR_Start();
    
    //���x
    init_temp();                    //���σT���v����x0.1�b�قǂ�����@�@���݃T���v������4
    
    //LED
    CORETIMER_DelayMs(600);
    LED_RED_Clear();
    CORETIMER_DelayMs(300);
    LED_BLUE_Clear();
    
    //DEBUGger Title
    printf("\n\n");
    printf("********************\n");
    printf(" Target TEST 32MX   \n");
    printf("           ver.%s\n", version);
    printf("                    \n");
    printf(" R:disp mode change \n");
    printf("********************\n");
    printf(">single line mode\n");
    printf(">temp: %5.1f%cC\n", temp_ave_degree_c, 0xdf);
    printf("\n");
    
    measure_init();
    data_clear(); 
    
    
    while( true ){
        // Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks ( );
        
        if (sensor_count != 0){
            //�Z���T�[���͂���
            LED_BLUE_Set();
            CORETIMER_DelayUs(800);        //�Â��Ă̓��͂�҂���

            ICAP1_Disable();
            ICAP2_Disable();
            ICAP3_Disable();
            ICAP4_Disable();
            TMR2_Stop();
            impact_PT4_reset();
            
            shot_count++;
            ring_pos++;
            if (ring_pos > 255){
                //���O�������|�C���^����
                ring_pos = 0;
            }
            set_temp_degree_c();          
            meas_stat = measure_main();
            
            CORETIMER_DelayMs(100);     //�^�}���j�̃f�o�b�K�[�ւ̏o�͂��I���܂ŊԂ��J����
            result_disp(shot_count, meas_stat, disp_mode);
            log_data_make(shot_count);
            
            CORETIMER_DelayMs(100);     //1�s�\���̎��܂��]�C���c���Ă��čăg���K���Ă��܂��̂ő҂�
            data_clear();
            LED_BLUE_Clear();
        }
        
        //�L�[���͂ŕ\�����[�h��؂�ւ�
        if (UART1_ReceiverIsReady()){
            //��M����
            CORETIMER_DelayMs(1);       //��M�҂� 9600bps 1�f�[�^�͖�1ms
            i = 0;
            while(UART1_ReceiverIsReady()) {
                buf[i] = UART1_ReadByte();
                i++;
                if (i > BUF_NUM){
                    break;
                }
                CORETIMER_DelayMs(1);   //��M�҂� 9600bps 1�f�[�^�͖�1ms
            }
            
            if ((buf[0] == 'R') && (buf[1] == 0) && (buf[2] == 0) && (buf[3] == 0)){
                //�悤�́fR'��1�����������͂����Ƃ�
                switch(disp_mode){
                    case SINGLE_LINE:
                        disp_mode = FULL_DEBUG;
                        printf(">debug mode\n");
                        break;
                    case FULL_DEBUG:
                        disp_mode = CSV_DATA;
                        printf(">csv data mode\n");
                        csv_title();
                        break;
                    case CSV_DATA:
                        disp_mode = SINGLE_LINE;
                        printf(">single line mode\n");
                        break;
                    default:
                        disp_mode = SINGLE_LINE;    //failsafe
                        break;
                }
            }
            UART1_ErrorGet();
            //�o�b�t�@�N���A
            for(i = 0; i < BUF_NUM; i++){
                buf[i] = 0;
            }
        }
                
        if (flag_1sec == 1){
            //1�b������
            get_temp_adc();
            flag_1sec = 0;
#ifdef TEMP_TEST
            //���x�f�o�b�O�p1�b���\���X�V
            set_temp_degree_c();          
            printf(">temp %5.1f%cC\n", temp_ave_degree_c, 0xdf);
#endif
        }
        
    } //main loop
    
    // Execution should not come here during normal operation
    return ( EXIT_FAILURE );
}


//interrupt
void sensor1_detect(uintptr_t context){
    //�Z���T1����
    if (sensor_4mic[SENSOR1].input_order == 0xff){
        impact_PT4_set();      //����s�v�F�ŏ��̃Z���T�I���ŃZ�b�g�����̂ōăZ�b�g����Ă��Ƃ��ɉe���͖���
        sensor_4mic[SENSOR1].input_order = sensor_count;
        sensor_4mic[SENSOR1].timer_cnt = ICAP1_CaptureBufferRead();
        //�ǂݏo�������Ȃ��Ɗ����t���O���N���A�ł����A�Ċ����������Ă��܂�
        sensor_count++;   
    }else {
        ICAP1_CaptureBufferRead();
    }

}


void sensor2_detect(uintptr_t context){
    //�Z���T2����
    if (sensor_4mic[SENSOR2].input_order == 0xff){
        impact_PT4_set();
        sensor_4mic[SENSOR2].input_order = sensor_count;
        sensor_4mic[SENSOR2].timer_cnt = ICAP2_CaptureBufferRead();
        //�ǂݏo�������Ȃ��Ɗ����t���O���N���A�ł����A�Ċ����������Ă��܂�
        sensor_count++;
    }else {
        ICAP2_CaptureBufferRead();
    }

}

void sensor3_detect(uintptr_t context){
    //�Z���T3����
    if (sensor_4mic[SENSOR3].input_order == 0xff){
        impact_PT4_set();
        sensor_4mic[SENSOR3].input_order = sensor_count;
        sensor_4mic[SENSOR3].timer_cnt = ICAP3_CaptureBufferRead();
        //�ǂݏo�������Ȃ��Ɗ����t���O���N���A�ł����A�Ċ����������Ă��܂�
        sensor_count++;
    }else {
        ICAP3_CaptureBufferRead();
    }

}


void sensor4_detect(uintptr_t context){
    //�Z���T4����
    if (sensor_4mic[SENSOR4].input_order == 0xff){
        impact_PT4_set();
        sensor_4mic[SENSOR4].input_order = sensor_count;
        sensor_4mic[SENSOR4].timer_cnt = ICAP4_CaptureBufferRead();
        //�ǂݏo�������Ȃ��Ɗ����t���O���N���A�ł����A�Ċ����������Ă��܂�
        sensor_count++;
    }else {
        ICAP4_CaptureBufferRead();
    }

}
void timer2_1sec_int(uint32_t status, uintptr_t context){
    //�^�C�}�[2+3 32bit timer 1�b������
    //harmony�����ݒ��TMR3�̂ق��ɂ���
    flag_1sec = 1;
}


//
void speaker(void){
    //4kHz
    //IO_RC6_SetHigh();
    CORETIMER_DelayUs(125);
    //IO_RC6_SetLow();
    CORETIMER_DelayUs(125);
    
}


/*******************************************************************************
 End of File
*/
