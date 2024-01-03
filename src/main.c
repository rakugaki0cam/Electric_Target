/*******************************************************************************
 * Electric TARGET #2 V5
 *      PIC32MK0512MCJ064  3.3V  120MHz
 *          Xtal 8MHz       2nd 32.768kHz
 *          Comparator: 5 used
 *          InputCapture: 5 used
 * 
 * Main.c
 * 
 * 2023.12.17
 *  
 * 2023.12.20   v.0.10  USB充電時の長押しオフに見えるようにスリープモードを設定
 * 2023.12.24   v.0.12  I2Cの修正、バッテリーと充電時スリープ機能の整理
 * 2023.12.24   v.0.13  ESP32からの読み出し用関数
 * 2024.01.02   v.0.14  BMP280気温気圧センサ追加
 * 
 * 
 * 
 * 
 * 
 * 
 *******************************************************************************/

#include "header.h"


//Global
bool    usb_in_flag = 0;
power_saving_mask_t sleep_flag = POWERSAVING_NORMAL;

//local
const uint8_t fw_ver[] = "0.14";                        //version
bool    main_sw_flag = 0;
bool    timer_1sec_flag = 0;
uint8_t sleep_sw_timer = 0;
uint8_t disp_timer = 0;



int main ( void )
{
    //Initialize all modules
    SYS_Initialize ( NULL );
    
    //power on
    ESP_POWER_Set();        //ESP32 5V LoadSwitch
    ANALOG_POWER_Set();     //Analog 3.3V LDO
    
    //start up
    CORETIMER_DelayMs(1000);
    LED_BLUE_Clear();
    CORETIMER_DelayMs(200);
    LED_BLUE_Set();
    CORETIMER_DelayMs(100);
    LED_BLUE_Clear();
    CORETIMER_DelayMs(200);
    LED_BLUE_Set();
    CORETIMER_DelayMs(100);
    LED_BLUE_Clear();
    
    printf("\n");
    printf("********************\n");
    printf("* electric TARGET  *\n");
    printf("*  #2     ver.%s *\n", fw_ver);
    printf("********************\n");
    printf("--- INIT -----------\n");

    
    //I2C Init
    I2C1_CallbackRegister(MyI2CCallback, 0);    //NULL);
    ip5306_Init();
    BME280_Init();
    ESP32slave_Init();
    
    //comparator DAC
    float       v_th = 0.200; //V CDAconverter
    uint16_t    val_12bit;
    val_12bit = (float)4096 / 3.3 * v_th;
    printf("Vth=%4.2fV:%d(12bit)\n", v_th, val_12bit);
    CDAC2_DataWrite(val_12bit);
    
    //Main SW interrupt INT4
    EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_4, main_sw_on_callback, 0);
    EVIC_ExternalInterruptEnable(EXTERNAL_INT_4);
    
    //RTCC
    RTCC_CallbackRegister(timer_1sec, 0);
    
    printf("--------------------\n");
    printf("\n");
    
    //batV init
    ESP32slave_WriteBatData(1);    //initialize
    
    
//**** MAIN LOOP *********    
    while ( true )
    {
        //Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks ( );
        
        
        if (timer_1sec_flag == 1){
            timer_1sec_flag = 0;
            disp_timer++;
            
            if ((disp_timer % 8) == 0){/////////////////////// interval
                LED_BLUE_Set();
                ESP32slave_WriteBatData(0);
                LED_BLUE_Clear();
            }else{
                BME280_Readout();
            }
        }
        
        if (main_sw_flag == 1){
            //INT4
            CORETIMER_DelayMs(50);
            if(MAIN_SW_Get() == 0){
                //スイッチ押されている
                printf("mainSW ON\n");
                if (sleep_flag != POWERSAVING_NORMAL){
                    //スリープ、ディープスリープ中ならリスタート
                    main_sw_flag = 0;                
                    if(MAIN_SW_Get() == 0){
                        printf("\n");
                        printf("***** ReSTART! *****\n");
                        CORETIMER_DelayMs(500);
                        RCON_SoftwareReset();       //リセット
                    }
                }else{
                    //スリープ中でない時は長押しスタート
                    sleep_sw_timer = 0;
                    while(!MAIN_SW_Get()){
                        //ボタンが押されている
                        CORETIMER_DelayMs(100);
                        printf(".");
                        sleep_sw_timer++;
                        if (sleep_sw_timer > 30){
                            //スリープへ
                            sleep_flag = POWERSAVING_SLEEP;
                            ESP_POWER_Clear();          //ESP32 5V LoadSwitch
                            ANALOG_POWER_Clear();       //Analog 3.3V LDO
                            printf("SLEEP\n");          //充電完了待ち
                            printf("\n");
                            break;
                        }
                    }
                    main_sw_flag = 0;
                    printf("\n");
                    CORETIMER_DelayMs(100);
                }
            }
        }
        
  
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


//***** sub ********************************************************************


void deep_sleep(void){
    //DEEP SLEEP
    SYSKEY = 0x0;           // Write invalid key to force lock
    SYSKEY = 0xAA996655;    // Write Key1 to SYSKEY
    SYSKEY = 0x556699AA;    // Write Key2 to SYSKEY
    OSCCONbits.SLPEN = 1;   // Set the power-saving mode to a sleep mode
    SYSKEY = 0x0;           // Write invalid key to force lock
    asm volatile ("wait");  // Put device in selected power-saving mode
                            // Code execution will resume here after wake and

    //////////// S L E E P //////////////////////////////////////////////////////////////////////////////////////////////
    Nop();
    Nop();
}


//***** callback ********************************************************
void main_sw_on_callback(EXTERNAL_INT_PIN extIntPin, uintptr_t context)
{
    if (extIntPin == EXTERNAL_INT_4){
        main_sw_flag = 1;
    }
}

void timer_1sec(uintptr_t context){
    timer_1sec_flag = 1;
}


/*******************************************************************************
 End of File
*/

