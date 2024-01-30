/*
 * File:   iP5306.c
 * Comments: PIC32MK-MCJ Harmony
 * 
 * iP5306　リチウムイオンバッテリ充電、保護、5Vブースト出力、スイッチ　　I2C
 * 
 * メインスイッチ、スリープ
 * 
 * BatV - 3.3V convert  I2C
 * 
 * Revision history: 
 * 2024.01.03
 * 
 */

#include "header.h"
#include "iP5306.h"


#define IP5306_SLAVE_ID     0x75    //iP5306 Li battery charger & booster
//register
//system
#define REG_SYS_CTL0        0x00
#define REG_SYS_CTL1        0x01
#define REG_SYS_CTL2        0x02
//charge
#define REG_CHG_CTL0        0x20
#define REG_CHG_CTL1        0x21
#define REG_CHG_CTL2        0x22
#define REG_CHG_CTL3        0x23
//charge current
#define REG_CHG_DIG_CTL0    0x24
//status
#define REG_READ0           0x70
#define REG_READ1           0x71
#define REG_READ2           0x72
#define REG_READ3           0x77
#define REG_READ4           0x78


//メインスイッチ押し状態
#define     MAIN_SW_PUSH()    !MAIN_SW_Get()

//Global
power_saving_mask_t sleepStat = POWERSAVING_NORMAL;


//***** iP5306 *****************************************************************
bool ip5306_Init(void){
    //I2C iP5306 init
#define DEBUG5306_no
     
    uint8_t ip5306TxData[4];
    uint8_t ip5306RxData[3];
    
    //REG_SYS_CTL0                  //default 0x35 -> 0x31
    uint8_t boost = 0b1;            //[bit5]ブースト出力     1:する, 0:しない 
    uint8_t charge = 0b1;           //[bit4]充電機能        1:有効, 0:無効,
    uint8_t autoOn = 0b0;           //[bit2]オートオン      1:する, 0:しない       ----*
    uint8_t alwaysBoost = 0b0;      //[bit1]常時ブースト     0:しない, 1:オン
    uint8_t pushSwOff = 0b1;        //[bit0]プッシュSWオフ   1:可, 0:不可
    //REG_SYS_CTL1                  //default 0x1D -> 0xD9
    uint8_t boostLongPush = 0b1;    //[bit7]ブーストオフ     0:ダブル, 1:長押し     ----*
    uint8_t leddoubleclick = 0b1;   //[bit6]LEDライト       0:長押し, 1:ダブル押し ----* 
    uint8_t shortPushOff = 0b0;     //[bit5]短押しオフ       0:しない, 1:する
    uint8_t usbCutBoost = 0b0;      //[bit2]USB抜時ブースト出力 1:オン, 0:オフ　 (注.瞬停してしまう)
    uint8_t lowVoltage = 0b1;       //[bit0]3V以下オフ       1:する　0:そのまま 
    //REG_SYS_CTL2                  //default 0x64 -> 0x74
    uint8_t longPushTime = 0b1;     //[bit4]長押し時間 　　　　0:2sec, 1:3sec      ----*
    uint8_t autoOffTime = 0b01;     //[bit3:2]オートオフ時間  01:32sec, 00:8s, 10:16s, 11:64s
   

    //デフォルト値の読み出し
    printf("iP5306 init ");
    if (i2c1_ReadDataBlock(IP5306_SLAVE_ID, REG_SYS_CTL0, ip5306RxData, 3)){
        printf("error!\n");
        return ERROR;
    }
#ifdef DEBUG5306
    printf("\n(I2C 0x%02X reg 0x%02X\n", IP5306_SLAVE_ID, REG_SYS_CTL0);
    printf("read %02X %02X %02X) ", ip5306RxData[0], ip5306RxData[1], ip5306RxData[2]);
#endif

    //iP5306 initialize
    ip5306TxData[0] = (boost << 5) + (charge << 4) + (autoOn << 2) + (alwaysBoost << 1) + pushSwOff;   
    ip5306TxData[1] = (boostLongPush << 7) + (leddoubleclick << 6) + (shortPushOff << 5) + (usbCutBoost << 2) + lowVoltage;
    ip5306TxData[2] = (longPushTime << 4) + (autoOffTime << 2);
                                        
    if (i2c1_WriteDataBlock(IP5306_SLAVE_ID, REG_SYS_CTL0, &ip5306TxData[0], 3)){
        printf("error!\n");
        return ERROR;
    }
    printf("OK\n");
    return OK;
}


bool ip5306_ReadStatus(uint8_t* data){
    //I2C read
    //ret  0:OK, 1:ERROR
    // data[0]  [bit2:0]:Batlevel, [bit3]:u3.2V, [bit4]:u3.0V, [bit5]USBin, [bit6]chargeFull
#define DEBUG5306_2_no  
    uint8_t i2cRxData [3];
    uint8_t chargeEnable;
    uint8_t chargeStatus;
    uint8_t batLevel;
    uint8_t batPercent = 100;   //%
    bool    usbinFlag = 0;

    if (i2c1_ReadDataBlock(IP5306_SLAVE_ID, REG_READ0, i2cRxData, 2)){
        printf("error!\n");
        return ERROR;
    }
    chargeEnable = i2cRxData[0];
    chargeStatus = i2cRxData[1];
    
    if (i2c1_ReadDataBlock(IP5306_SLAVE_ID, REG_READ4, i2cRxData, 1)){
        printf("error!\n");
        return ERROR;
    }
    batLevel = i2cRxData[0];

    //BAT level 
    if (batLevel & 0b10000000){
        batPercent = 75;
    }
    if (batLevel & 0b01000000){
        batPercent = 50;
    }
    if (batLevel & 0b00100000){
        batPercent = 25;
    }
#ifdef DEBUG5306_2
    printf("(%3d%%)\n", batPercent);
#endif
    data[0] = (uint8_t)(batPercent / 25);  //[bit2:0]
    
#if DEBUG5306_3
    printf("cE %02X ", chargeEnable);
    printf("cS %02X ", chargeStatus);
    printf("bV %02X ", batLevel);
    printf("\n");
#endif
    
    if (batLevel & 0b00010000){
        printf("batV < 3.2V \n");
        data[0] |= 0b00001000;      //[bit3]
    }
    if (batLevel & 0b00000010){
        printf("batV < 3.0V \n");
        data[0] |= 0b00010000;      //[bit4] 
    }

    //Charge status change
    if (chargeEnable & 0b00001000){
        //USB IN
        data[0] |= 0b00100000;  //[bit5] 
        if (usbinFlag == 0){
#ifdef DEBUG5306_2
            printf("USB IN-");
#endif            
            usbinFlag = 1;
        }
    }else{
        CHARGE_LED_RED_Clear();
        if (usbinFlag == 1){
#ifdef DEBUG5306_2
            printf("USB Out\n");
#endif
        }
        usbinFlag = 0;
    }

    if (usbinFlag == 1){
        //USB接続
        if (chargeStatus & 0b00001000){
            //充電完了 -> ディープスリープ
#ifdef DEBUG5306_2
            printf("Full\n");
#endif
            data[0] |= 0b01000000;  //[bit6] 
            CHARGE_LED_RED_Clear();
            if (sleepStat == POWERSAVING_SLEEP){
                sleepStat = POWERSAVING_DEEPSLEEP;
                deepSleep();
                //------------- DEEP SLEEP -------------------------------------
            
                //awake
            }
        }else{
            //充電中
#ifdef DEBUG5306_2
            printf("CHARGE\n");
#endif
            CHARGE_LED_RED_Set();
        }
    }
    return OK;
}


//**** battery *****************************************************************

float batteryVolt(bool init){
    //バッテリ電圧計測する
    //init  1:初回　平均値用にサンプル数回読込
    //      0:通常
    
//分圧
#define     VREFP       2.493       //V
#define     RA          46.9        //kohm
#define     RB          9.93        //kohm
//
#define     SAMPLES     8           //平均サンプル数 max 16  12bit x 16 
    
    static uint16_t     batVdata[SAMPLES];      //平均値計算用
    uint16_t            batVSum;
    float               batV;
    static uint8_t      ringCount = 0;       
    uint8_t             i;
    
    //init
    if (init == true){
        //平均用データ配列を埋める
        for(i = 0; i < SAMPLES; i++){
            batVdata[i] = batteryAdcGet();  
        }
        return 0;
    }
    
    //normal
    batVdata[ringCount] = batteryAdcGet();    
    //printf("%1d:%03x ", ring_cnt, batv[ring_cnt]);

    ringCount ++;
    if (ringCount >= SAMPLES){
        ringCount = 0;
    }
    batVSum = 0;
    for(i = 0; i < SAMPLES; i++){
        batVSum += batVdata[i];
    }
    
    batV = (float)batVSum / SAMPLES * VREFP / 4096 * (RA + RB) / RB;    //12bit
     
    return batV;
}


uint16_t batteryAdcGet(void){
    //バッテリー電圧　12ビットAD変換値を読む
#define     ADCH_VBAT   ADCHS_CH19  //ピン4 チャンネル19 

    ADCHS_ChannelConversionStart(ADCH_VBAT);
    while(!ADCHS_ChannelResultIsReady(ADCH_VBAT)){
        //wait
    }
    return ADCHS_ChannelResultGet(ADCH_VBAT); 
}


//***** main switch ************************************************************

void mainSwPush(void){
    //メインスイッチが押された時
    uint8_t     sleep_sw_timer;

    CORETIMER_DelayMs(50);      //チャタリング対策
    if(MAIN_SW_PUSH()){
        printf("mainSW ON\n");
        if (POWERSAVING_NORMAL == sleepStat){
            //スリープ中でない時は長押し判定
            //充電中の長押しでみせかけのスリープ状態にする
            sleep_sw_timer = 0;
            while(MAIN_SW_PUSH()){
                //長押し中
                CORETIMER_DelayMs(100);
                printf(".");
                sleep_sw_timer++;
                if (sleep_sw_timer > 30){   //3秒
                    espSleep();             //PIC以外オフの擬似スリープ状態へ
                    break;
                }
            }
            printf("\n");
            CORETIMER_DelayMs(100);   
            
        }else{
            //スリープ、ディープスリープ中ならリセットし再起動
            resetRestart();
        }
    }
}


//--- SLEEP -----

void espSleep(void){
    //PICの充電チェック以外をオフ
    sleepStat = POWERSAVING_SLEEP;
    BME280_Sleep();                     //BME280スリープ
    ledLightOff(LED_BLUE | LED_YELLOW | LED_PINK);  //正面LED消灯
    printf("\n---ESP32 SLEEP-----\n");  //充電完了待ち状態へ節電
    ESP32slave_SleepCommand();          //ESP32スリープ　(電源を切るとI2Cバスに影響が出るため)
    ANALOG_POWER_Clear();               //Analog 3.3V LDOオフ
    printf("wait to FullCharge \n");
    printf("\n");
}

    
void deepSleep(void){
    //充電完了時のほとんどオフ状態
    sleepStat = POWERSAVING_DEEPSLEEP;
    LED_BLUE_Clear();
    printf("RTCC Interrupt off\n");
    RTCC_InterruptDisable(RTCC_INT_ALARM);  //RTCC 割り込み停止
    printf("ESP32 off\n");
    ESP_POWER_Clear();                      //ESP32 5V LoadSwitchオフ
    printf("---DEEP SLEEP-----\n");
    CORETIMER_DelayMs(500);
    POWER_LowPowerModeEnter(LOW_POWER_SLEEP_MODE);
    
    ///////////////////////// DEEP SLEEP //////////////////////////////////////////////////
    
    Nop();
    Nop();
}


//--- RESET -----
void resetRestart(void){
    ESP_POWER_Clear();          //ESP32 5V LoadSwitchオフ espSleep中の場合、起きないので一度電源を切る
    CORETIMER_DelayMs(200);
    printf("\n");
    printf("***** ReSTART! *****\n");
    CORETIMER_DelayMs(500);
    RCON_SoftwareReset();       //ソフトウエアリセット
    
}


