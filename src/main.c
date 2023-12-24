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
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *******************************************************************************/



#include "header.h"


//Global
const float fw_version = 0.12;  //////////////////version
bool    i2c_flag = 0;
bool    main_sw_flag = 0;
bool    timer_1sec_flag = 0;
bool    usb_in_flag = 0;
uint8_t sleep_sw_timer = 0;
uint8_t disp_timer = 0;

typedef enum
{
    POWERSAVING_NORMAL = 0,
    POWERSAVING_SLEEP  = 1,
    POWERSAVING_DEEPSLEEP = 2,
} POWERSAVING_MASK;
POWERSAVING_MASK sleep_flag = POWERSAVING_NORMAL;



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

void timer_1sec(uintptr_t context){
    timer_1sec_flag = 1;
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
    
    printf("\n");
    printf("********************\n");
    printf("* electric TARGET  *\n");
    printf("*  #2    ver.%5.2f *\n", fw_version);
    printf("********************\n");
    printf("--- INIT -----------\n");

    
    //I2C Init
    I2C1_CallbackRegister(MyI2CCallback, 0);    //NULL);
    ip5306_init();
    
    //comparator
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
    battery_voltage_disp(1);    //initialize
    
    
    while ( true )
    {
        //Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks ( );
        
        if (timer_1sec_flag == 1){
            timer_1sec_flag = 0;
            disp_timer++;
            
            
            if ((disp_timer % 8) == 0){/////////////////////// interval
                LED_BLUE_Toggle();
                battery_voltage_disp(0);
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
                            printf("SLEEP\n");
                            printf("\n");
                            CORETIMER_DelayMs(1000);
                            deep_sleep();      //RTCC割り込みで周期的に起きる     
                            //---------------------- SLEEP ---------------------
                            //充電完了待ち
                            
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
        printf("I2C 0x%02X reg 0x%02X\n", IP5306_SLAVE_ADDR, i2cTxData[0]);
        printf("read %02X %02X %02X\n", i2cRxData[0], i2cRxData[1], i2cRxData[2]);
        printf("iP5306 OK!\n");
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


bool ip5306_read_status(uint8_t* data0x70){
    //I2C read
    uint8_t i2cTxData [1];
    uint8_t i2cRxData [9];
    uint8_t charge_enable;  //0x70
    uint8_t charge_status;  //0x71
    uint8_t bat_level;      //0x78
    uint8_t batZan = 100;   //%
    
    i2c_flag = 0;
    i2cTxData[0] = 0x70;    //register address
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
    data0x70[0] = (batZan / 25);        //[bit2:0]
    
#if DEBUG1
    printf("cE %02X ", charge_enable);
    printf("cS %02X ", charge_status);
    printf("bV %02X ", bat_level);
    printf("\n");
#endif
    
    if (bat_level & 0b00010000){
        printf("batV < 3.2V \n");
        data0x70[0] |= 0b00001000;      //[bit3]
    }
    if (bat_level & 0b00000010){
        printf("batV < 3.0V \n");
        data0x70[0] |= 0b00010000;      //[bit4] 
    }

    //Charge status change
    if (charge_enable & 0b00001000){
        //USB IN
        data0x70[0] |= 0b00100000;  //[bit5] 
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
            data0x70[0] |= 0b01000000;  //[bit6] 
            CHARGE_LED_RED_Clear();
            if (sleep_flag == POWERSAVING_SLEEP){
                sleep_flag = POWERSAVING_DEEPSLEEP;
                LED_BLUE_Clear();
                RTCC_InterruptDisable(RTCC_INT_ALARM);//RTCC 割り込み停止
                printf("RTCC Int off\n");
                printf("-- PIC DEEP SLEEP --\n");
                CORETIMER_DelayMs(500);
                deep_sleep();
                ///////////////////////// DEEP SLEEP ////////////////////////////////
            }
        }else{
            printf("CHARGE\n");
            CHARGE_LED_RED_Set();
        }
    }
    return 0;
}
   

bool battery_voltage_disp(bool init) {
    //
    //init=1:初回
    
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
    uint8_t             dataToEsp[4];
    uint16_t            batToEsp;           //batV*1000[V]
    uint8_t             i;
        
    //平均用データ配列を埋める
    if (init == 1){
        for(i = 0; i < SAMPLES; i++){
            ADCHS_ChannelConversionStart(ADCH_VBAT);
            while(!ADCHS_ChannelResultIsReady(ADCH_VBAT));
            batv[i] = ADCHS_ChannelResultGet(ADCH_VBAT);  
        }
        //return;
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
     
    
    ip5306_read_status(dataToEsp);
    if (sleep_flag != POWERSAVING_NORMAL ){
        return 0;
    }
    
    batToEsp = bat_v * 1000;
    dataToEsp[1] = dataToEsp[0];
    dataToEsp[0] = 0x70;    //register address
    dataToEsp[2] = batToEsp >> 8;
    dataToEsp[3] = batToEsp & 0xff;

    printf("to ESP %02X %02X %02X %02X\n", dataToEsp[0], dataToEsp[1], dataToEsp[2], dataToEsp[3]);

    
    //write to ESP32slave
    i2c_flag = 0;
    if (i2c1_bus_check()){
        return 1;
    }
    if(!I2C1_Write(ESP_SLAVE_ADDR, &dataToEsp[0], 4)){
        printf("I2C bus busy!\n");
        printf("\n");  
        return 1;
    }
    if (!i2c1_wait()){
        //printf("ESP slave OK\n");
    }else{
        printf("ESP slave Fault\n");
        printf("\n");  
        return 1;
    }

    printf("\n");  
    return 0;
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
    Nop();
}



/*******************************************************************************
 End of File
*/

