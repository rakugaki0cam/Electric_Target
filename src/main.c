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
 * 2023.12.20   v.0.12  I2Cの修正
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

//#include <stddef.h>                     // Defines NULL
//#include <stdbool.h>                    // Defines true
//#include <stdlib.h>                     // Defines EXIT_FAILURE
//#include "definitions.h"                // SYS function prototypes
#include "header.h"


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

//Global
const float fw_version = 0.12;  //////////////////version
bool    i2c_flag = 0;
bool    main_sw_flag = 0;
bool    usb_in_flag = 0;
bool    sleep_flag = 0;
uint8_t sleep_sw_timer = 0;
uint8_t disp_timer = 0;


//callback
void MyI2CCallback(uintptr_t context)
{
    //This function will be called when the transfer completes. Note
    //that this functioin executes in the context of the I2C interrupt.

    //printf("I2C complete! \n");
    i2c_flag = 1;
}

void main_sw_on_callback(EXTERNAL_INT_PIN extIntPin, uintptr_t context)
{
    if (extIntPin == EXTERNAL_INT_4){
        main_sw_flag = 1;
    }
}



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
    
    printf("********************\n");
    printf("* electric TARGET  *\n");
    printf("*  #2    ver.%5.2f *\n", fw_version);
    printf("*                  *\n");
    printf("********************\n");
    
    
    //I2C Init
    I2C1_CallbackRegister(MyI2CCallback, 0);    //NULL);
    ip5306_init();
    
    //batV init
    battery_voltage_disp(1);    //initialize
    
    //Main SW interrupt INT4
    EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_4, main_sw_on_callback, 0);
    EVIC_ExternalInterruptEnable(EXTERNAL_INT_4);
    
    
    while ( true )
    {
        //Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks ( );
        
        CORETIMER_DelayMs(100);
        disp_timer++;
        if (disp_timer >= 200){
            disp_timer = 0;
        }
              
        if (main_sw_flag == 1){
            //INT4
            CORETIMER_DelayMs(50);
            if(MAIN_SW_Get() == 0){
                //スイッチ押されている
                printf("mainSW ON\n");
                if (sleep_flag == 1){
                    //スリープ中ならリスタート
                    main_sw_flag = 0;                
                    if(MAIN_SW_Get() == 0){
                        printf("ReSTART!\n");
                        RCON_SoftwareReset();       //リセット
                    }
                }else{
                    //スリープ中でない時は長押しカウントスタート
                    main_sw_flag = 0;
                    sleep_sw_timer = 1;
                }
            }
        }
        
        if (sleep_sw_timer > 0){
            //USB接続中のときはスリープモードへ入る
            //USB接続していない時はiP5306がオフする
            if (MAIN_SW_Get() == 0){
                //長押しのチェック
                sleep_sw_timer++;
                printf(".");
                if (sleep_sw_timer > 30){
                    sleep_sw_timer = 0;       
                    ESP_POWER_Clear();          //ESP32 5V LoadSwitch
                    ANALOG_POWER_Clear();       //Analog 3.3V LDO
                    sleep_flag = 1;
                    printf(" SLEEP\n");
                    CORETIMER_DelayMs(3000);
                }
            }else{
                //途中でボタンを話した時
                sleep_sw_timer = 0;
                CORETIMER_DelayMs(100);
            }
            
        }
        
        if ((disp_timer % 20) == 0){///////////////////////20...2sec interval
            LED_BLUE_Toggle();
            battery_voltage_disp(0);
        }
        
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


// **** sub ********************************************************************

#define IP5306_SLAVE_ADDR   0x75    //iP5306 Li battery charger & booster
#define ESP_SLAVE_ADDR      0x25    //ESP32 LCD WiFi


void ip5306_init(void){
   //I2C iP5306 init
    
    uint8_t i2cTxData [1] = {0};
    uint8_t i2cRxData [3] = {0,0,0};

    //デフォルト読み出し
    i2c_flag = 0;
    i2cTxData[0] = 0x00;    //register address
    i2c1_bus_check();
    if(!I2C1_WriteRead(IP5306_SLAVE_ADDR, &i2cTxData[0], 1, i2cRxData, 3)){
        printf("I2C bus busy!\n");
        return;
    }
    if (!i2c1_wait()){
        printf("iP5306 OK!\n");
        printf("ID 0x%02X reg 0x%02X\n", IP5306_SLAVE_ADDR, i2cTxData[0]);
        printf("data %02X %02X %02X\n", i2cRxData[0], i2cRxData[1], i2cRxData[2]);
    }else{
        printf("iP5306 Fault\n");
        return;
    }
    
    //iP5306 initialize
    i2c_flag = 0;
    i2cTxData[0] = 0x00;    //先頭レジスタアドレス
    i2cTxData[1] = 0x31;    //0x00 bit5:ブーストオン, bit4:充電オン, bit2:オートオンしない, bit1常時ブーストしない, bit0プッシュSWオフ可
    i2cTxData[2] = 0xD9;    //0x01 bit7:ブーストオフ長押し, bit6:LEDライトダブル押し, bit5:短押しオフしない, bit2:USB抜いたらオフ, bit0:3V以下オフ
                            //USBを抜くとブースト5Vが出るけれど、タイムラグがあって瞬停するためPICはリセットしてしまう。-> USB抜いたらオフにしておく
    i2cTxData[3] = 0x74;    //0x02 bit4:長押し3秒, bit3-2:オートオフ32秒
                                               
    i2c1_bus_check();
    if(!I2C1_Write(IP5306_SLAVE_ADDR, &i2cTxData[0], 4)){
        printf("I2C bus busy!\n");
        return;
    }
    if (!i2c1_wait()){
        printf("\n");
    }else{
        printf("iP5306 Fault\n");
    }
}


bool ip5306_read_status(void){
    //I2C read
    uint8_t i2cTxData [11];
    uint8_t i2cRxData [10];
    uint8_t charge_enable;  //0x70
    uint8_t charge_status;  //0x71
    //uint8_t key_status;     //0x77
    uint8_t bat_level;      //0x78
    
    i2c_flag = 0;
    i2cTxData[0] = 0x70;    //register address
    while(I2C1_IsBusy( ));  //wait for the current transfer to complete
    if (i2c1_bus_check()){
        return 1;
    }
    if(!I2C1_WriteRead(IP5306_SLAVE_ADDR, &i2cTxData[0], 1, i2cRxData, 9)){
        printf("I2C bus busy!\n");
        return 1;
    }
    if (i2c1_wait()){
        printf("iP5306 Fault\n");
        return 1;
    }
    
    //
    charge_enable = i2cRxData[0];
    charge_status = i2cRxData[1];
    //key_status = i2cRxData[7];
    bat_level = i2cRxData[8];

    //BAT level
    uint8_t batZan = 100;
    if (bat_level & 0b10000000){
        batZan = 75;
    }
    if (bat_level & 0b01000000){
        batZan = 50;
    }
    if (bat_level & 0b00100000){
        batZan = 25;
    }
    printf("(%3d%%)\n", batZan);
    
#if DEBUG1
    printf("cE %02X ", charge_enable);
    printf("cS %02X ", charge_status);
    printf("bV %02X ", bat_level);
    printf("\n");
#endif
    
    if (bat_level & 0b00010000){
        //batV < 3.2V
        printf("batV < 3.2V \n");
    }
    if (bat_level & 0b00000010){
        //batV < 3.0V
        printf("batV < 3.0V \n");
    }

    //Charge status change
    if (charge_enable & 0b00001000){
        if (usb_in_flag == 0){
            printf("USB IN-");
            usb_in_flag = 1;
        }
    }else{
        CHARGE_LED_RED_Clear();
        if (usb_in_flag == 1){
            printf("USB Out\n");
        }
        usb_in_flag = 0;
    }

    if (usb_in_flag == 1){
        if (charge_status & 0b00001000){
            printf("Full\n");
            CHARGE_LED_RED_Clear();
            if (sleep_flag == 1){
                LED_BLUE_Clear();
                printf("PIC DEEP SLEEP\n");
                CORETIMER_DelayMs(500);
                deep_sleep();
            }

        }else{
            printf("CHARGE\n");
            CHARGE_LED_RED_Set();
        }
    }
    return 0;
}
   

void battery_voltage_disp(bool init) {
    //
    //init=1:initialize  data read -> array
    
//#define     ADCH_VBAT   ADCHS_CH1
#define     ADCH_VBAT   ADCHS_CH19
#define     VREFP       2.493       //V
#define     RA          46.9        //kohm
#define     RB          9.93        //kohm
#define     SAMPLES     8           //max 16  12bit x 16 
    
    float               bat_v;
    static uint16_t     batv[SAMPLES];      //data arry
    static uint8_t      ring_cnt = 0;       
    uint16_t            sum_batv;
    uint8_t             i;
        
    //initialize
    if (init == 1){
        for(i = 0; i < SAMPLES; i++){
            ADCHS_ChannelConversionStart(ADCH_VBAT);
            while(!ADCHS_ChannelResultIsReady(ADCH_VBAT));
            batv[i] = ADCHS_ChannelResultGet(ADCH_VBAT);  
        }
        return;
    }
    
    //normal
    ADCHS_ChannelConversionStart(ADCH_VBAT);
    while(!ADCHS_ChannelResultIsReady(ADCH_VBAT));
    batv[ring_cnt] = ADCHS_ChannelResultGet(ADCH_VBAT);    
    //printf("%1d:%03x ", ring_cnt, batv[ring_cnt]);

    ring_cnt ++;
    if (ring_cnt >= SAMPLES){
        ring_cnt = 0;
    }
    
    sum_batv = 0;
    for(i = 0; i < SAMPLES; i++){
        sum_batv += batv[i];
    }
    
    bat_v = (float)sum_batv / SAMPLES * VREFP / 4096 * (RA + RB) / RB;    //12bit
    printf("BatV:%4.2fV", bat_v);
     
    
    ip5306_read_status();
    
    /*
    //write to ESP32slave
    i2cTxData [0] = 0x70;    //register address(=iP5306)
    for (i = 1; i < 10; i++){
        i2cTxData [i] = i2cRxData[i - 1];
    }

    if(!I2C1_Write(ESP_SLAVE_ADDR, &i2cTxData[0], 10)){
        printf("I2C bus busy!\n");
    }
    if (!i2c1_wait()){
        //printf("esp slave OK\n");/////////////////////////
    }else{
        printf("ESP32 I2C Fault\n");
    }
    */

    printf("\n");  

}


bool i2c1_bus_check(void){
    //bus idle
#define COUNT2   100
    uint16_t i = 0;
    
    while(I2C1_IsBusy( )){
        //wait for the current transfer to complete
        i++;
        if (i > COUNT2){
            //bus error
            printf("bus error\n");
            I2C1_TransferAbort();       //必要???
            return 1;
        }
    }
    return 0;
}


bool i2c1_error(void){
    //error handling
    static uint16_t cnt1 = 0;
    static uint16_t cnt2 = 0;
    static uint32_t a = 0;
    
    a++;
    while(I2C1_IsBusy());  //wait for the current transfer to complete
    switch(I2C1_ErrorGet()){
        case I2C_ERROR_NONE:
            return 0;
            break;
        case I2C_ERROR_NACK:
            cnt1++;
            printf("I2C_ERROR_NACK\n");
            break;
        case I2C_ERROR_BUS_COLLISION:
            cnt2++;
            printf("I2C_ERROR_BUS_COLLISION\n");
            break;
    }
    I2C1_TransferAbort();       //必要みたい
    return 1;
}


bool i2c1_wait(void){
    //i2c1 処理終了待ち
    //返り値 0:OK, 1:error or time over
    
#define COUNT   20
    uint16_t    i = 0;
    
    while(!i2c_flag){
        i++;
        if (i > COUNT){
            printf("I2C timeout!\n");
            return 1;
        }
        printf(".");
    }
    return i2c1_error();
}


void deep_sleep(void){
    //DEEP SLEEP
    SYSKEY = 0x0;           // Write invalid key to force lock
    SYSKEY = 0xAA996655;    // Write Key1 to SYSKEY
    SYSKEY = 0x556699AA;    // Write Key2 to SYSKEY
    OSCCONbits.SLPEN = 1;   // Set the power-saving mode to a sleep mode
    SYSKEY = 0x0;           // Write invalid key to force lock
    asm volatile ("wait");  // Put device in selected power-saving mode // Code execution will resume here after wake and

    //////////// S L E E P //////////////////////////////////////////////////////////////////////////////////////////////
    Nop();
}



/*******************************************************************************
 End of File
*/

