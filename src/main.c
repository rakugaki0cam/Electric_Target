 /*
  * 電子ターゲット
  * V2  1-3番目に入力したセンサーのみで計算
  *     あんまりよくない
  * 
  * 32ビット版
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
  * 2022.03.29  ver.1.0.1   PIC18F27Q84 8bit から移植
  *                         Input Captureを4ch使用
  * 2022.04.02  ver.1.0.2   バグフィクス
  * 2022.04.03  ver.1.1.0   計算エラー処理と表示を整理
  *                         DEBUGgerのRキーで表示モード切り替え　DEBUG ←→ 1LINE
  * 2022.04.06  ver.1.1.2   ICAP割込は発生間隔が近い場合に処理に移る順番が変わることがあるよう。
  *                         2つ以上の割込が保留中の時、割込優先度設定が同じ場合、IRQ#の順に実行されることとなる。
  *                         入力順序を求める処理を追加
  * 2022.04.09  ver.1.1.3   温度入力
  * 2022.04.10  ver.1.1.4   温度は1秒おきに測定して16値の平均
  * 2022.04.11  ver.1.1.5   センサー位置修正　オフセット値を追加
  * 2022.04.12  ver.1.1.6   センサオンからコンパレータオンまでの遅れ時間を補正　10~16usecくらいの遅れがある。距離による
  * 2022.04.13  ver.1.1.7   センサ組み付け、まと板固定他工作。センサ位置ズレ補正オフセット追加。温度センサ熱対策。
  * 2022.04.14  ver.1.1.8   塩ビ板t2分を抜いて、音の発生源は裏面としてみる。SENSOR_DEPTH_OFFSET(z) 8 → 6
  * 2022.04.15  ver.1.1.9   一度計算して距離を求めてから、センサ遅れ時間補正値を計算して、本計算する。->うまくいかない。補正足りない感じ。
  * 2022.04.16  ver.1.10    csv整理とログデータ保管
  * 2022.04.16  ver.1.11    センサ遅れ補正計算ー測定時間差からの計算の方がいいみたい。
  * 2022.04.17  ver.1.20    通信をRS485へ(送信制御にRTSを使う)、着弾タイミングPT4アウトプット追加
  * 2022.05.03  ver.1.21    タマモニ用バイナリデータ出力
  * 
  * V2_edition
  * 2022.05.11  ver.2.00    V1よりプロジェクトをコピー (ファイルでコピーした後、MPLABXにてプロジェクトネームを変更し、Harmonyを起動、code作成。
  *                         コピー前のものとリンクされている.c .hファイルをプロジェクトから除外してコピーした.c .hファイルをAdd Existing Itemで追加する。
  * 2022.05.14  ver.2.01    1～3番目に入力したセンサーのみで計算する。
  * 
  * 2022.05.16              測定値があちこちにばらつくことがあるようであんまりよくない。
  * 2022.11.08  ver.2.02    タマモニのデバッグでのprintfで”R”の文字が出たときにModeが変わってしまう。"R"1文字だけのときのみmode変更。
  * 2022.11.10  ver.2.04    UARTの受信FIFOバッファ8レベルをオーバーフローしてエラー。ソフトリセットしないと受信できない。
  * 
  * 
*/

#include "header.h"


//コンパレータ閾値設定
#define     V_DD        3.3                     //電源電圧
#define     V_TH        0.200                   //コンパレータ閾値 default:0.200V
#define     RA          1000                    //分圧抵抗+側
#define     RB          1000                    //分圧抵抗-側
#define     V_REF       V_DD * RB / (RA + RB)   //CVref+ 分圧Ra:(Ra+Rb)
//着弾タイミングのフォールエッジをタマモニに送る
#define     impact_PT4_set()    PT4_Clear()     //着弾センサ信号出力
#define     impact_PT4_reset()  PT4_Set()       //着弾センサ信号クリア
#define     BUF_NUM     64                      //UARTデータ読込バッファ数


//GLOBAL
uint16_t    ring_pos = 0;                   //ログデータポインタ

//LOCAL
uint8_t     version[] = "2.04";             //バージョンナンバー
uint8_t     sensor_count;                   //センサ入力順番のカウント
bool        flag_1sec = 0;                  //1秒タイマー割込
char        buf[BUF_NUM];                   //UARTデータ読込バッファ
uint8_t     i;

/*
    Main application
*/

int main(void){
    measure_status_source_t   meas_stat;
    display_mode_source_t     disp_mode = SINGLE_LINE;

    uint16_t    shot_count = 0;     //ショットカウントは1から。0は入力無し

    
    
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
    
    //コンパレータ電圧出力
    CVR_UpdateValue((uint8_t)(V_TH / (V_REF / 24) + 0.99));      //切り上げ
    CVR_Start();
    
    //温度
    init_temp();                    //平均サンプル数x0.1秒ほどかかる　　現在サンプル数は4
    
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
            //センサー入力あり
            LED_BLUE_Set();
            CORETIMER_DelayUs(800);        //つづいての入力を待つ時間

            ICAP1_Disable();
            ICAP2_Disable();
            ICAP3_Disable();
            ICAP4_Disable();
            TMR2_Stop();
            impact_PT4_reset();
            
            shot_count++;
            ring_pos++;
            if (ring_pos > 255){
                //ログメモリポインタ制限
                ring_pos = 0;
            }
            set_temp_degree_c();          
            meas_stat = measure_main();
            
            CORETIMER_DelayMs(100);     //タマモニのデバッガーへの出力が終わるまで間を開ける
            result_disp(shot_count, meas_stat, disp_mode);
            log_data_make(shot_count);
            
            CORETIMER_DelayMs(100);     //1行表示の時まだ余韻が残っていて再トリガしてしまうので待つ
            data_clear();
            LED_BLUE_Clear();
        }
        
        //キー入力で表示モードを切り替え
        if (UART1_ReceiverIsReady()){
            //受信あり
            CORETIMER_DelayMs(1);       //受信待ち 9600bps 1データは約1ms
            i = 0;
            while(UART1_ReceiverIsReady()) {
                buf[i] = UART1_ReadByte();
                i++;
                if (i > BUF_NUM){
                    break;
                }
                CORETIMER_DelayMs(1);   //受信待ち 9600bps 1データは約1ms
            }
            
            if ((buf[0] == 'R') && (buf[1] == 0) && (buf[2] == 0) && (buf[3] == 0)){
                //ようは’R'が1文字だけ入力したとき
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
            //バッファクリア
            for(i = 0; i < BUF_NUM; i++){
                buf[i] = 0;
            }
        }
                
        if (flag_1sec == 1){
            //1秒毎処理
            get_temp_adc();
            flag_1sec = 0;
#ifdef TEMP_TEST
            //温度デバッグ用1秒毎表示更新
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
    //センサ1割込
    if (sensor_4mic[SENSOR1].input_order == 0xff){
        impact_PT4_set();      //判定不要：最初のセンサオンでセットされるので再セットされてもとくに影響は無い
        sensor_4mic[SENSOR1].input_order = sensor_count;
        sensor_4mic[SENSOR1].timer_cnt = ICAP1_CaptureBufferRead();
        //読み出しをしないと割込フラグをクリアできず、再割込が入ってしまう
        sensor_count++;   
    }else {
        ICAP1_CaptureBufferRead();
    }

}


void sensor2_detect(uintptr_t context){
    //センサ2割込
    if (sensor_4mic[SENSOR2].input_order == 0xff){
        impact_PT4_set();
        sensor_4mic[SENSOR2].input_order = sensor_count;
        sensor_4mic[SENSOR2].timer_cnt = ICAP2_CaptureBufferRead();
        //読み出しをしないと割込フラグをクリアできず、再割込が入ってしまう
        sensor_count++;
    }else {
        ICAP2_CaptureBufferRead();
    }

}

void sensor3_detect(uintptr_t context){
    //センサ3割込
    if (sensor_4mic[SENSOR3].input_order == 0xff){
        impact_PT4_set();
        sensor_4mic[SENSOR3].input_order = sensor_count;
        sensor_4mic[SENSOR3].timer_cnt = ICAP3_CaptureBufferRead();
        //読み出しをしないと割込フラグをクリアできず、再割込が入ってしまう
        sensor_count++;
    }else {
        ICAP3_CaptureBufferRead();
    }

}


void sensor4_detect(uintptr_t context){
    //センサ4割込
    if (sensor_4mic[SENSOR4].input_order == 0xff){
        impact_PT4_set();
        sensor_4mic[SENSOR4].input_order = sensor_count;
        sensor_4mic[SENSOR4].timer_cnt = ICAP4_CaptureBufferRead();
        //読み出しをしないと割込フラグをクリアできず、再割込が入ってしまう
        sensor_count++;
    }else {
        ICAP4_CaptureBufferRead();
    }

}
void timer2_1sec_int(uint32_t status, uintptr_t context){
    //タイマー2+3 32bit timer 1秒毎割込
    //harmony割込設定はTMR3のほうにある
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
