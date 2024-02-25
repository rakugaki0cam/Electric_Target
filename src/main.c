/*******************************************************************************
 * Electric TARGET #2 V5
 *      PIC32MK0512MCJ064  3.3V  120MHz
 *          Xtal 8MHz       2nd Xtal 32.768kHz
 * 
 * V5   2号機
 *      マイク5個
 *      Comparator: 5 used
 *      InputCapture: 5 used
 *      データはI2Cマスターとして、ESP32S3(7"LCD Yellow board)　slaveへ送信
 * 
 *      5つのセンサから、3つのセンサを選んで座標を計算 -> 10とおりの結果
 *      センサ1を抜いた時の結果は4とおり -> 4点のばらつきを計算。
 *      同様にセンサ2を抜いた時のばらつき、
 *      。。。センサ5を抜いた時のばらつき、
 *      の5とおりのうちの一番ばらつきが小さいものを採用する。
 * 
 * DEBUGGer: 9600bps　-> 115200bps (2024.01.17 ~)
 * タマモニ有線のため　また9600bpsへ (2024.02.25 ~)
 * 
 * 
 * Main.c
 * 
 * 2023.12.17
 *  
 * 2023.12.20   v.0.10  USB充電時の長押しオフに見えるようにスリープモードを設定
 * 2023.12.24   v.0.12  I2Cの修正、バッテリーと充電時スリープ機能の整理
 * 2023.12.24   v.0.13  ESP32からの読み出し用関数
 * 2024.01.03   v.0.14  BMP280気温気圧センサ追加
 *                      Compiler optimization level -0  for DEBUG
 * 2024.01.06   v.0.20  電源まわり完了　(espスリープ)
 * 2024.01.08   v.0.21  コンパレータ入力　測定部の修正
 * 2024.01.12   v.0.22  分散の計算。ばらつき最小グループを採用。
 * 2024.01.14   v.0.23  正面LEDインジケータのためI2C I/Oエキスパンダ追加。
 * 2024.01.17   v.0.24  マトLCDへの表示成功。UARTデバッガ出力を9600->115200bpsへ
 * 2024.01.19   v.0.25  Harmony -> MCCへ
 * 2024.01.20   v.0.30  計算間違い修正　z[1]のところz[0]と間違えていたためccが900ほど違っていて計算座標がずれていた。(1号機_V4も修正済み)
 * 2024.01.21   v.0.31  フルデバッグモード..計算経過を追加
 * 2024.01.21   v.0.40  センサ3個が一直線に並んでいる時(e=0)の計算式を追加。
 * 2024.01.27   v.0.41  センサ一直線の時x0が2つ求まるので、判定式を追加。センサ1の球r#1+r0の長さがほぼ(x0,y0)と(x#1,y#1)の距離に等しい、ことから差が小さい方を選択する。
 * 2024.01.30   v.0.50  整理整頓
 * 2024.02.25   v.0.51  有線部分の作成:
 *                      ブランチv051-wired-connect
 * 2024.02.25   v.0.52  有線接続の時、ターゲットオフセット分をタマモニへのデータに加算しなくてはいけない。(オフセットデータを持っていない)
 *                      有線接続の時、タマモニからのターゲットコマンド(UART)を解釈してESPへI2Cで送る
 * 
 * 
 */

#include "header.h"


//Global
uint16_t    ringPos = 0;            //ログデータポインタ
uint8_t     sensorCnt;              //センサ入力順番のカウント  ////////////////////////
float       targetY0Offset = 0;     //ターゲットYオフセット

//debug
debugger_mode_sour_t  debuggerMode = NONE;
//debugger_mode_sour_t    debuggerMode = FULL_DEBUG;


//local
const uint8_t fw_ver[] = "0.52";    //firmware version
bool        mainSwFlag = 0;         //メインスイッチ割込
bool        timer1secFlag = 0;      //RTCC 1秒割込
uint32_t    uartBaudrate = 9600;    //RS485ボーレート:通常　(タマモニ有線可)
//uint32_t    uartBaudrate = 115200;  //              :たくさんデバッグする時(タマモニ有線不可)


//--- callback ----------------------------------------------------------

void mainSwOn_callback(EXTERNAL_INT_PIN extIntPin, uintptr_t context)
{
    if (extIntPin == EXTERNAL_INT_4){
        mainSwFlag = 1;
    }
}


void timer1sec_callback(uintptr_t context){
    timer1secFlag = 1;
}


//--- main ---------------------------------------------------------------

int main ( void ){
    uint8_t cnt;
    meas_stat_sor_t measStat;
    uint16_t        shotCnt = 0;     //ショットカウントは1から。0は入力無し
    uint8_t         dispTimer = 0;
    uint8_t         ledTimer = 0;       //ステータスLEDを消すまでのタイマー
    //UART_SERIAL_SETUP rs485set;

    
    //Initialize all modules
    SYS_Initialize ( NULL );
    
    //power on
    ESP_POWER_Set();        //ESP32 5V LoadSwitch
    ANALOG_POWER_Set();     //Analog 3.3V LDO
    
    //UART
    //rs485set.baudRate = uartBaudrate;
    //rs485set.parity = UART_PARITY_NONE;
    //rs485set.dataWidth = UART_DATA_8_BIT;
    //rs485set.stopBits = UART_STOP_1_BIT;
    //UART1_SerialSetup(&rs485set, CPU_CLOCK_FREQUENCY >> 1);  //PBCLK2:60MHz
    
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
    printf("*  #2  PIC32MK     *\n");
    printf("*        ver.%s  *\n", fw_ver);
    printf("********************\n");
    printf(">full debug mode\n");    /////////デバッグ中
    printf("mode change .... R\n");
    printf("target clear ... C\n");
    printf("--- INIT -----------\n");

    //Pin Interrupt
    ICAP1_CallbackRegister(detectSensor1, 0);
    ICAP2_CallbackRegister(detectSensor2, 0);
    ICAP3_CallbackRegister(detectSensor3, 0);
    ICAP4_CallbackRegister(detectSensor4, 0);
    ICAP5_CallbackRegister(detectSensor5, 0);

     //Main SW interrupt INT4
    EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_4, mainSwOn_callback, 0);
    EVIC_ExternalInterruptEnable(EXTERNAL_INT_4);
    
    //RTCC1秒ごと割込
    RTCC_CallbackRegister(timer1sec_callback, 0);
    
    //I2C Init
    I2C1_CallbackRegister(MyI2CCallback, 0);    //NULL);
    
    //I2Cセンサ類初期設定
    ip5306_Init();      //Li-ion Battery charger&booster
    BME280_Init();      //BMP280 temp&pressure
    ESP32slave_Init();  //LCD&WiFi
    PCF8574_Init();     //IOexpander LED
    
    //comparator DAC
    uint16_t    compVth = 100;//mV  CDAconverter/////////////////////////////////////////
    uint16_t    val_12bit;
    val_12bit = 4096 / 3300 * compVth;
    printf("Vth=%5dmV:%03x(12bit)\n", compVth, val_12bit);
    CDAC2_DataWrite(val_12bit);
    
    //batV init
    batteryVolt(1);    //initialize
    
   //comparator ON
    CMP_1_CompareEnable();
    CMP_2_CompareEnable();
    CMP_3_CompareEnable();
    CMP_4_CompareEnable();
    CMP_5_CompareEnable();  
    
    printf("--------------------\n");
    printf("\n");
    //hardware init complete
    
    //uart buffer clear
    while(UART1_ReceiverIsReady()) {
        UART1_ReadByte();
    }
    
    //
    measureInit();
    clearData(); 
    
    
//**** MAIN LOOP ********************************************** 
    while ( true )
    {
        //Maintain state machines of all polled MPLAB Harmony modules.
        SYS_Tasks ( );
        
        //
        if (sensorCnt != 0){
            //センサー入力あり = 測定中
            LED_BLUE_Set();
            
            cnt = 80;
            while(cnt > 0){
                CORETIMER_DelayUs(10);        //つづいての入力信号を待つ時間 10us x 80 = 800usec
                cnt--;
                if (sensorCnt >= NUM_SENSOR){
                    break;
                }
            }
            
            ICAP1_Disable();
            ICAP2_Disable();
            ICAP3_Disable();
            ICAP4_Disable();
            ICAP5_Disable();                //入力がなかった時もあるはずなので止める
            TMR2_Stop();
            impact_PT4_Off();               //着弾センサ出力オフ->タマモニへいく信号
            
            shotCnt++;
            ringPos++;
            if (ringPos > LOG_MEM){
                //ログメモリポインタ制限
                ringPos = 0;
            }
            
            measStat = measureMain(shotCnt);
            serialPrintResult(shotCnt, measStat, debuggerMode);
            //log_data_make(shot_count);    //////////////////////////////////////////////////////////////////////
            
            //CORETIMER_DelayMs(100);     //printf表示処理が残っていて再トリガしてしまうので待つ////////////フルオート対応できない//////////////////////
            clearData();
            ledTimer = 0;
            
            LED_BLUE_Clear();
            
        }else{       
            //(sensorCnt == 0)測定中ではない時
            
            debuggerComand();
            
            //
            if (timer1secFlag ){
                //1秒ごと処理
                TMR2 = 0;       //1秒ごとタイマクリア。タイマは2秒までカウント。測定中はクリアされない。
                TMR2_Start();
                //
                timer1secFlag = 0;
                dispTimer++;
                ledTimer++;

                if ((dispTimer % 8) == 0){     //interval　8sec
                    LED_BLUE_Set();
                    ESP32slave_SendBatData();
                    LED_BLUE_Clear();
                }else if(POWERSAVING_NORMAL == sleepStat){
                    BME280_ReadoutSM();
                }
                
#define LED_INDICATE_TIME   6
                if (ledTimer >= LED_INDICATE_TIME){
                    //正面LEDを消灯
                    ledLightOff(LED_BLUE | LED_YELLOW | LED_PINK);
                }
            }
            
            //
            if (mainSwFlag){
                //INT4　電源スイッチの処理
                mainSwPush();
                mainSwFlag = 0;
            }
        }
  
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


//----- sub --------------------------------------------------------------------

void debuggerComand(void){
    //キー入力で表示モードを切り替え
#define     BUF_NUM     250     //UARTデータ読込バッファ数
    uint8_t buf[BUF_NUM];       //UARTデータ読込バッファ
    uint8_t i;
    
    if (!UART1_ReceiverIsReady()){
        return;
    }
    
    //シリアル(タマモニ、デバッガー)から受信あり
    for(i = 0; i < 5; i++){
        buf[i] = 0;
    }
    i = 0;
    while(UART1_ReceiverIsReady()) {
        buf[i] = UART1_ReadByte();
        if ((buf[i] == ',') || (buf[i] == 0)){ //end mark
            i++;
            buf[i] = 0;
            printf("buf='%s'\n", buf);
            CORETIMER_DelayMs(50);
            break;
        }
        i++;
        if (i > BUF_NUM){
            printf("uart command too long!\n");
            //buffer clear
            return;
        }
        CORETIMER_DelayUs(90);          //受信待ち 115200bps 1バイトデータは約78us
        if (uartBaudrate != 115200){
            CORETIMER_DelayUs(1000);    //受信待ち 9600bps 1バイトデータは約938us
        }
    }
    
    
    //DEBUGger出力モードの切り替え
    if (((buf[0] == 'R') | (buf[0] == 'r')) && ((buf[1] + buf[2] + buf[3] + buf[4]) == 0)){
        switch(debuggerMode){
            case NONE:
                debuggerMode = SINGLE_LINE;
                printf(">single line mode\n");
                break;
            case SINGLE_LINE:
                debuggerMode = MEAS_CALC;
                printf(">result mode\n");
                break;
            case MEAS_CALC:
                debuggerMode = FULL_DEBUG;
                printf(">full debug mode\n");
                break;
            case FULL_DEBUG:
                debuggerMode = CSV_DATA;
                printf(">csv output mode\n");
                printDataCSVtitle();
                break;
            case CSV_DATA:
                debuggerMode = NONE;
                printf(">no output mode\n");
                break;
            default:
                debuggerMode = NONE;    //failsafe
                break;
        }
    }else if (((buf[0] == 'C') | (buf[0] == 'c')) && ((buf[1] + buf[2] + buf[3] + buf[4]) == 0)){
        //ターゲットクリア
        printf(">target clear\n");
        ESP32slave_ClearCommand();
    }else{
        //コマンド解析
        
        
        tamamoniCommandCheck(buf);

        
        
        
        
        
        
        //printf(">invalid command!\n");
    }
    
    
    UART1_ErrorGet();
    //buffer clear
    for(i = 0; i < BUF_NUM; i++){
        buf[i] = 0;
    }
    
}


// Tamamoni TARGET Command ------------------------------------------------------------------------------------------
void tamamoniCommandCheck(uint8_t* tmp_str) {
    //タマモニからのコマンドを確認し実行
    char command[10] = { 0 };  //9文字まで
    const char clear[] = "CLEAR";
    const char reset[] = "RESET";
    const char defaultSet[] = "DEFAULT";
    const char offset[] = "OFFSET";
    const char aimpoint[] = "AIMPOINT";
    const char brightness[] = "BRIGHT";
    float val = 0;
    uint8_t   num;

    num = sscanf((char*)tmp_str, "TARGET_%s %f END", command, &val);   //valのところの数字は無くても正常に動くみたい
    if (num == 0) {
        //型が合わなかったとき
        printf("'%s' is not command!\n", tmp_str);
        return;
    }                                                                     
    //コマンド
    printf("Detect tamamoni command(%d) :%s   %f --- ", num, command, val);
    if (strcmp(clear, command) == 0) {
        printf("> target clear\n");
        ESP32slave_ClearCommand();

    } else if (strcmp(reset, command) == 0) {
        printf("** RESET ESP32 in 3 second **\n");
        //LCD
        CORETIMER_DelayMs(3000);
        ESP32slave_ResetCommand();  ////////////////////// RESET

    } else if (strcmp(defaultSet, command) == 0) {
        printf("> target default set \n");
        ESP32slave_DefaultSetCommand();

    } else if (strcmp(offset, command) == 0) {
        printf("> target Y offset %5.1f \n", val);
        ESP32slave_OffSetCommand(val);

    } else if (strcmp(aimpoint, command) == 0) {
        printf("> aimpoint set %5.1f \n", val);
        ESP32slave_AimpointCommand(val);

    } else if (strcmp(brightness, command) == 0) {
        printf("> LCD backlight brightness set %5.1f \n", val);
        ESP32slave_BrightnessCommand(val);
    } else {
        //
        printf(" invalid command!\n");
    }

}




/*******************************************************************************
 End of File
*/

