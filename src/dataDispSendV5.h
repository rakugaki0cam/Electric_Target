/*
 * dataDispSendV5.h
 * 
 * 測定値を表示、ESP タマモニ有線　送信
 * 
 * measure_V5.hより分離
 *  2024.1.8
 *  
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef DATA_DISPSEND_V5_H
#define	DATA_DISPSEND_V5_H


//画面表示モード
typedef enum {
    NONE,                   //出力なし
    SINGLE_LINE,            //画面表示:1行/1発
    MEAS_CALC,              //測定値と計算結果まとめ
    FULL_DEBUG,             //画面表示:デバッグ用計算経過もフル表示
    CSV_DATA,               //CSVデータ出しー表計算用
    DEBUGGER_MODE_SOURCE_NUM
} debugger_mode_sour_t;


    
void    serialPrintResult(uint16_t, uint8_t, uint8_t);
void    dataSendToTAMAMONI(void);
void    dataSendToESP32(void);

//debugger
void    printSingleLine(uint16_t, meas_stat_sor_t);
void    printMeasCalc(uint16_t, meas_stat_sor_t);
void    printFullDebug(uint16_t, meas_stat_sor_t);
void    printDataCSVtitle(void);
void    printDataCSV(uint16_t);



#endif	//DATA_DISPSEND_V5_H