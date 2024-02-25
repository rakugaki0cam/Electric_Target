/*
 * computeEpicenterV5.c
 * 
 * 電子ターゲット
 * 座標の計算
 * 
 *  2022.03.24　calc_locate.cより
 * V2_edition
 *  2022.05.11  1-3番目に入力したセンサから計算する
 * 
 * V4＿edition
 *  2023.08.26  4通りの計算値からエラーを検出
 *              1つのセンサー値が異常の場合、4つの座標がバラバラになる。
 *              → 正しい座標は異常値を抜いた他の3つのセンサから求める1つだけの座標。どれが正しい座標なのかの判定が難しい。
 * 
 * V5＿edition
 *  2024.01.05  5つのセンサから10とおりの計算値。センサを1つ抜いた4つの値のばらつきが一番小さいセンサ組み合わせを選択する
 *  2024.01.20  計算間違い修正　z[1]のところz[0]と間違えていたためccが900ほど違っていて計算座標がずれていた。
 *  2024.01.21  フルデバッグモード..計算経過を追加
 *  2024.01.21  センサ3個が一直線に並んでいる時(e=0)の計算式を追加。
 *  2024.01.27  一直線の時のx0の2つの解の判定
 * 
 * 
 */

#include "header.h"
#include "computeEpicenterV5.h"

//DEBUG (global)
//#define DEBUG_APO_no        //座標検算時のデバッグprintf表示(検算用)
//#define DEBUG_APO2_no       //座標検算時のデバッグprintf表示(エラー系)


//GLOBAL
impact_result_t calcValue[NUM_CAL];     //座標の計算
impact_result_t calcResult;             //座標の計算の最終結果
result_ave_t    vari5Groupe[NUM_GROUP];   //結果判定用ばらつきの計算


//LOCAL
//計算結果の数　10とおり
//#define     NUM_CAL      10   //GLOBAL
#define     CAL1         0      //計算結果の番号とプログラム中の添字
#define     CAL2         1
#define     CAL3         2       
#define     CAL4         3       
#define     CAL5         4
#define     CAL6         5       
#define     CAL7         6       
#define     CAL8         7       
#define     CAL9         8       
#define     CAL10        9

//5つのセンサから3つのセンサを選択する10のパターン
uint8_t sensor3outOf5[NUM_CAL][3] = {    
    { SENSOR1, SENSOR2, SENSOR3},   //CAL1ー計算結果1
    { SENSOR1, SENSOR2, SENSOR4},   //CAL2
    { SENSOR1, SENSOR2, SENSOR5},   //CAL3
    { SENSOR1, SENSOR3, SENSOR4},   //CAL4
    { SENSOR1, SENSOR3, SENSOR5},   //CAL5
    { SENSOR1, SENSOR4, SENSOR5},   //CAL6
    { SENSOR2, SENSOR3, SENSOR4},   //CAL7
    { SENSOR2, SENSOR3, SENSOR5},   //CAL8
    { SENSOR2, SENSOR4, SENSOR5},   //CAL9
    { SENSOR3, SENSOR4, SENSOR5},   //CAL10
};


//1つセンサを抜かす5グループに分類     
//#define     NUM_GROUP       5
#define     GROUP1          0
#define     GROUP2          1
#define     GROUP3          2
#define     GROUP4          3
#define     GROUP5          4
//4つのセンサからの計算結果4とおり
uint8_t calcValue4[NUM_GROUP][4] = {
    { CAL7, CAL8, CAL9, CAL10},     //GROUP1  SENSOR2,3,4,5　ー　センサ1抜かし
    { CAL4, CAL5, CAL6, CAL10},     //GROUP2  SENSOR1,3,4,5　ー　センサ2抜かし
    { CAL2, CAL3, CAL6, CAL9},      //GROUP3  SENSOR1,2,4,5　ー　センサ3抜かし
    { CAL1, CAL3, CAL5, CAL8},      //GROUP4  SENSOR1,2,3,5　ー　センサ4抜かし
    { CAL1, CAL2, CAL4, CAL7},      //GROUP5  SENSOR1,2,3,4　ー　センサ5抜かし
};


//
void    clearResult(uint8_t n){
    //計算結果をクリア
    calcValue[n].pattern = sensorOrderPattern(n);
    calcValue[n].impact_pos_x_mm = 0;
    calcValue[n].impact_pos_y_mm = 0;
    calcValue[n].radius0_mm = 0;
    calcValue[n].delay_time0_msec = 0;
    calcValue[n].status = 0;
}


void    resultError999(uint8_t n, calc_stat_sor_t status){
    //エラーの時、resultにエラー識別数値を書き込む
    //calcValue[n].impact_pos_x_mm = 999.99;/////////////////////////////////////////検算のためオフ
    //calcValue[n].impact_pos_y_mm = 999.99;
    //calcValue[n].radius0_mm = 999.99;
    calcValue[n].delay_time0_msec = 0;        //タマモニでのエラー判定に使用
    calcValue[n].status = status;
}


//
calc_stat_sor_t computeEpicenter(void){
    //着弾座標の計算
    //出力　result[]:計算結果座標x,y,r  グローバル
    //return value calc_stat_sor_tによる
    
#define     DEV_XY      3.0     //偏差 判定値　xy
#define     NUM_RES     4       //1グループ中の偏差計算値の数
    
    calc_stat_sor_t  calcStat = CALC_STATUS_OK;
    uint8_t     i;
    uint8_t     calcNum;
    uint8_t     grNum;
    uint8_t     cnt;                //平均値サンプル数のカウント,順位のカウント
    float       tmpX;
    float       tmpY;
    float       tmpR;
    float       dist2[NUM_RES];     //距離の2乗
    
    
    //計算結果クリア
    for (i = 0; i < NUM_CAL; i++){
        clearResult(i);
    }
    calcResult.pattern = 0xffff;    //平均値識別マーク

    
    //5つのセンサから3つを選んで計算する　10とおり
    for (i = 0; i < NUM_CAL; i++){
        calcStat = computeXY(i);    //座標を計算
    }
    
    //------resultに計算結果が入るーーーーーーーーーーーーーーーーーーーーーーーーーーーーーー
        
    //***** V5_edition　****************************************************
    //VERIANCE CHECK　分散チェック
    //センサー4個選択の5グループについて　4つの計算結果数値のばらつきを計算
    
    for (grNum = 0; grNum < NUM_GROUP; grNum++){
        calcStat = CALC_STATUS_OK;                          //error clear
        vari5Groupe[grNum].pattern = sensorGroupePattern(grNum);
        
        //4つの計算結果の平均
        cnt = 0;
        tmpX = 0;
        tmpY = 0;
        tmpR = 0;
        for(i = 0; i < NUM_RES; i++){
            calcNum = calcValue4[grNum][i];   //計算ナンバー
            if (CALC_STATUS_OK == calcValue[calcNum].status){
                tmpX += calcValue[calcNum].impact_pos_x_mm;
                tmpY += calcValue[calcNum].impact_pos_y_mm; 
                tmpR += calcValue[calcNum].radius0_mm;
                cnt ++;
            }else {
                //計算値が不可だった場合平均計算に含めない
                calcStat = CALC_STATUS_AVERAGE_ERR;
                ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
                printf("group%1d.d%1d: ave sample error!\n", (grNum + 1), (i + 1));
#endif
            }
        } // for-i loop 
        
        vari5Groupe[grNum].sample_n = cnt;       //平均値のサンプル数:正常なら4
        if (cnt == 0){
            //サンプル数0の時
            vari5Groupe[grNum].average_pos_x_mm   = 999.99;
            vari5Groupe[grNum].average_pos_y_mm   = 999.99; 
            vari5Groupe[grNum].average_radius0_mm = 999.99;
            calcStat = CALC_STATUS_AVERAGE_ERR;
            ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2            
            printf("group%1d No sample!\n", (grNum + 1));
#endif
        }else{
            //平均値
            vari5Groupe[grNum].average_pos_x_mm   = tmpX / cnt;
            vari5Groupe[grNum].average_pos_y_mm   = tmpY / cnt; 
            vari5Groupe[grNum].average_radius0_mm = tmpR / cnt; 
        }
        
        //4つ計算結果の分散
        for(i = 0; i < NUM_RES; i++){
            calcNum = calcValue4[grNum][i];
            if (CALC_STATUS_OK == calcValue[calcNum].status){
                //計算OK
                tmpX = calcValue[calcNum].impact_pos_x_mm - vari5Groupe[grNum].average_pos_x_mm;
                tmpY = calcValue[calcNum].impact_pos_y_mm - vari5Groupe[grNum].average_pos_y_mm;
                dist2[i] = tmpX * tmpX + tmpY * tmpY;   //距離(偏差)の2乗

                if (dist2[i] > (DEV_XY * DEV_XY)){      //ばらつきが範囲を超えている
                    calcStat = CALC_STATUS_DEV_OVER;
                    ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
                    printf("group%1d.d%1d: dev over!\n", (grNum + 1), (i + 1));
#endif
                }
                vari5Groupe[grNum].status = calcStat;
            }else{
                calcStat = CALC_STATUS_CAL_ERROR;
                ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
                printf("group%1d.d%1d: dev calc error!\n", (grNum + 1), (i + 1));
#endif
                dist2[i] = 99.999;
            }
            
        } // for-i loop 
        
        vari5Groupe[grNum].dist1_mm2 = dist2[0];          //距離(偏差)の2乗を収納
        vari5Groupe[grNum].dist2_mm2 = dist2[1];
        vari5Groupe[grNum].dist3_mm2 = dist2[2];
        vari5Groupe[grNum].dist4_mm2 = dist2[3];
        
        tmpR = 0;
        for (i = 0; i < NUM_RES; i++){
            tmpR += dist2[i];
        }
        vari5Groupe[grNum].variance = tmpR / NUM_RES;     //分散
        vari5Groupe[grNum].status = calcStat;
        
        
    } //for-groupe loop
    //----------結果判定用の分散がgroupeVariに入る------------------
    
    
    //*************************ばらつきの順位の計算**********************************
    for (grNum = 0; grNum < NUM_GROUP; grNum++){
        cnt = 0;
        for (i = 0; i < NUM_GROUP; i++){
            if (vari5Groupe[grNum].sample_n < 4){
                //サンプル数が少ない時
                cnt = 8;                ////順位 -> 9の表示が出るように8にする。
                continue;
            }
            if (vari5Groupe[grNum].variance > vari5Groupe[i].variance){
                //自分の値より小さい値があった時カウントする
                cnt++;
                //同じ値があった時は同着ができてしまう......未処理////////////////////
            }
        }
        vari5Groupe[grNum].order = cnt;
        if (cnt == 0){
            calcNum = grNum;        //calcNum:ばらつきが最小のグループ番号  ->　採用値
        }
    }

    //結果の収納
    calcResult.impact_pos_x_mm  = vari5Groupe[calcNum].average_pos_x_mm;
    calcResult.impact_pos_y_mm  = vari5Groupe[calcNum].average_pos_y_mm;
    calcResult.radius0_mm       = vari5Groupe[calcNum].average_radius0_mm;
    calcResult.delay_time0_msec = impact_time_msec(calcResult.radius0_mm);    //着弾からセンサオンまでの遅れ時間(着弾時刻計算用)
    calcResult.status           = calcStat;
    
    return calcStat;
}


calc_stat_sor_t computeXY(uint8_t calNum){
    //3センサデータから座標値を計算
    //Input  calNum:3つのセンサを選択するパターン番号  
    //Output result:計算値
    //       calc_stat:状態　0-OK,
    
    sensor_data_t   threeSensorFactor[3];  //センサデータ受け渡し用センサ3つで1つの計算結果
    calc_stat_sor_t calcStat = CALC_STATUS_OK;
    
    //tmp3sensorに計算用データ代入
    if (select3sensor(calNum, threeSensorFactor)){
        //代入するデータがダメな時
        calcStat = CALC_STATUS_NOT_ENOUGH;
        resultError999(calNum, calcStat);
        ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
        printf("sensor data select error!\n");
#endif
        return calcStat;
    }

    //使用センサーの番号を数字化
    calcValue[calNum].pattern = sensorOrderPattern(calNum);
    
    //座標の計算
    if (apollonius3circleXYR(calNum, threeSensorFactor) != 0){
        //計算がダメなとき
        calcStat = CALC_STATUS_CAL_ERROR;
        resultError999(calNum, calcStat);
        ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
        printf("- calculation error!\n");
#endif
        return calcStat;
    }
    
    //計算結果の判定
    if (calcValue[calNum].radius0_mm > R_MAX){
        //rが大きすぎる
        calcStat = CALC_STATUS_R0_ERR;
        resultError999(calNum, calcStat);
        ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
        printf(" r0:%f ", calcValue[calNum].radius0_mm);        
        printf("CAL%1d:r0 is too large!\n", (calNum + 1));
#endif
        return calcStat;
    }
    if ((calcValue[calNum].impact_pos_x_mm < -TARGET_WIDTH_HALF) || (calcValue[calNum].impact_pos_x_mm > TARGET_WIDTH_HALF)){
        //xが大きすぎる
        calcStat = CALC_STATUS_X0_ERR;
        resultError999(calNum, calcStat);
        ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
        printf(" x0:%f ", calcValue[calNum].impact_pos_x_mm);        
        printf("CAL%1d:x0 is too large!\n", (calNum + 1));
#endif
        return calcStat;
    }
    if ((calcValue[calNum].impact_pos_y_mm < -TARGET_HEIGHT_HALF) || (calcValue[calNum].impact_pos_y_mm > TARGET_HEIGHT_HALF)){
        //yが大きすぎる
        calcStat = CALC_STATUS_Y0_ERR;
        resultError999(calNum, calcStat);
        ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
        printf(" y0:%f ", calcValue[calNum].impact_pos_y_mm);        
        printf("CAL%1d:y0 is too large!\n", (calNum + 1));
#endif
        return calcStat;
    }
    
    calcValue[calNum].delay_time0_msec = impact_time_msec(calcValue[calNum].radius0_mm);    //着弾からセンサオンまでの遅れ時間(着弾時刻計算用)

    return calcStat;
}


uint8_t     select3sensor(uint8_t calNum, sensor_data_t* tmp){
    //センサ3ケを選択して代入
    //出力tmp3SensorFactor  -計算用の3組のデータ配列
    //stat 0:OK, 1:代入不可
    uint8_t i;
    calc_stat_sor_t  calcStat = CALC_STATUS_OK;
   
    //測定値代入
    for (i = 0; i < 3; i++){
        if (SENSOR_STATUS_OK == sensor5Measure[sensor3outOf5[calNum][i]].status){
            tmp[i] = sensor5Measure[sensor3outOf5[calNum][i]];      //一括代入 .sensor_x, .sensor_y, .sensor_z, .distance_mm
        } else{
            //データがダメな時
            calcStat = CALC_STATUS_NOT_ENOUGH;
            calcValue[calNum].status = calcStat;
            ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
            printf("sensor%2d data error!\n", (sensor3outOf5[calNum][i] + 1));
#endif
            return calcStat;
        }
    }
    
    return calcStat;
}


uint16_t    sensorOrderPattern(uint8_t calNum){
    //使用センサナンバーを3桁のBCD数字にして記録
    return (sensor3outOf5[calNum][0] + 1) * 0x100 +
           (sensor3outOf5[calNum][1] + 1) * 0x10 +
           (sensor3outOf5[calNum][2] + 1);
}


uint16_t    sensorGroupePattern(uint8_t calNum){
    //使用センサナンバーを4桁のBCD数字にして記録
    //123456789A 　A=10
    return (calcValue4[calNum][0] + 1) * 0x1000 +
           (calcValue4[calNum][1] + 1) * 0x100 + 
           (calcValue4[calNum][2] + 1) * 0x10 +
           (calcValue4[calNum][3] + 1);
}


uint8_t firstSensor(void){
    //最初の入力センサーの番号
    uint8_t i;
    for(i = SENSOR1; i < NUM_SENSOR; i++){
        if(sensor5Measure[i].input_order == 0){
            break;
        }
    }
    return i;
}



//*** calculation sub *****************

uint8_t apollonius3circleXYR(uint8_t numResult, sensor_data_t* tmp3Sensor){
    //座標の計算
    //リターン値　0:正常
    //          1:エラー
    //入力値 tmp3SensorFactor[3]: センサーデータ x、y、z、遅れ距離
    //出力値 calcResult[]: 座標x、yと距離r0の計算値
    //      numResult: 結果を代入する番号
    
    uint8_t i;
    calc_stat_sor_t  calc_stat = CALC_STATUS_OK;
    
    float   x[4], y[4], z[4], r[4];    //x0:x[0]...
    float   a[4], b[4], c[4], d[4];
    float   e, f[3], g[3];
    float   aa, bb, cc, q;
    float   x01, x02;
    float   hant, hant2;
    
#ifdef  DEBUG_APO   
    printf("\n");
    printf("calc #%2d ------------------------------\n", numResult + 1);
#endif  
    
    //データ代入  ---配列添字の0を使わない
    for (i = 0; i < 3; i++){
        x[i + 1] = tmp3Sensor[i].sensor_x_mm;
        y[i + 1] = tmp3Sensor[i].sensor_y_mm;
        z[i + 1] = tmp3Sensor[i].sensor_z_mm;
        r[i + 1] = tmp3Sensor[i].distance_mm;
#ifdef  DEBUG_APO
    printf("x%1d:%5.1f y%1d:%5.1f z%1d:%5.1f dr%1d:%8.4f\n", i + 1, x[i + 1], i + 1, y[i + 1], i + 1, z[i + 1], i + 1, r[i + 1]);
#endif
    }
    
    //定数の計算
    a[1] = x[2] - x[1];
    a[2] = x[3] - x[2];
    a[3] = x[1] - x[3];
    b[1] = y[2] - y[1];
    b[2] = y[3] - y[2];
    b[3] = y[1] - y[3];
    c[1] = r[2] - r[1];   //rは測定値なのでその都度違う
    c[2] = r[3] - r[2];
    c[3] = r[1] - r[3];
    
    e = a[1] * b[2] - a[2] * b[1];
    
#ifdef  DEBUG_APO
    
    printf("a1:%10.3f a2:%10.3f a3:%10.3f \n", a[1], a[2], a[3]);
    printf("b1:%10.3f b2:%10.3f b3:%10.3f \n", b[1], b[2], b[3]);
    printf("c1:%10.3f c2:%10.3f c3:%10.3f \n", c[1], c[2], c[3]);
    printf("e:%11.3f\n", e);
#endif
    if (e == 0){
        //センサが一直線に並ぶ時にe=0となる
        //分母がゼロとなり計算できない
#ifdef  DEBUG_APO_2
        printf("CAL%1d: straight line\n", (numResult + 1));
#endif        
        e = b[1] * c[2] - b[2] * c[1];      //別の公式で解く
        
#ifdef  DEBUG_APO
        printf("e(st):%11.3f\n", e);
#endif
        if (e == 0){
            //別公式でもダメな時
            calc_stat = CALC_STATUS_E_ZERO;
            calcValue[numResult].status = calc_stat;
            ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
            printf("CAL%1d: E(st) is also zero! ", numResult);
#endif
        }
        d[1] = (0 - y[1] * y[1] + y[2] * y[2] + r[1] * r[1] - r[2] * r[2]) / 2;
        d[2] = (0 - y[2] * y[2] + y[3] * y[3] + r[2] * r[2] - r[3] * r[3]) / 2;
        d[3] = (0 - y[3] * y[3] + y[1] * y[1] + r[3] * r[3] - r[1] * r[1]) / 2;
    
        r[0] = (b[1] * d[2] - b[2] * d[1]) / e;
        y[0] = (0 - c[1] * r[0] + d[1]) / b[1];
        
#ifdef  DEBUG_APO
        printf("d1:%10.3f d2:%10.3f d3:%10.3f \n", d[1], d[2], d[3]);
        printf("r0:%10.3f y0:%10.5f \n", r[0], y[0]);
#endif    
        aa = 1;
        bb = -2 * x[1];
        cc = x[1] * x[1] + (y[0] - y[1]) * (y[0] - y[1]) - (r[0] + r[1]) * (r[0] + r[1]) + z[1] * z[1];
        
        q = (bb * bb) - (4 * aa * cc);      //解の公式の条件

#ifdef  DEBUG_APO
        printf("A:%10.6f B:%10.5f C:%10.3f \n", aa, bb, cc);
        printf("Q:%10.3f \n", q);
#endif    
        if (q < 0){
            //解の公式の条件をみたさない
            calc_stat = CALC_STATUS_QUAD_F;
            calcValue[numResult].status = calc_stat;
            ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
            printf("CAL%1d: Qst is less than zero! ", (numResult + 1));
#endif
            return calc_stat;
        }

        //二次方程式を解いてx,y,rを求める計算
        x01 = (-bb - sqrt(q)) / (2 * aa);
        x02 = (-bb + sqrt(q)) / (2 * aa);

        hant  = (r[0] + sensor5Measure[SENSOR1].distance_mm)
              - sqrt( (x01 - sensor5Measure[SENSOR1].sensor_x_mm) * (x01 - sensor5Measure[SENSOR1].sensor_x_mm)
                    + (y[0] - sensor5Measure[SENSOR1].sensor_y_mm) * (y[0] - sensor5Measure[SENSOR1].sensor_y_mm) );
             
        hant2 = (r[0] + sensor5Measure[SENSOR1].distance_mm)
              - sqrt( (x02 - sensor5Measure[SENSOR1].sensor_x_mm) * (x02 - sensor5Measure[SENSOR1].sensor_x_mm)
                    + (y[0] - sensor5Measure[SENSOR1].sensor_y_mm) * (y[0] - sensor5Measure[SENSOR1].sensor_y_mm) );
         
#ifdef  DEBUG_APO_2
        printf("x01:%10.5f - hant: %10.3f \n", x01, hant);
        printf("x02:%10.5f - hant2:%10.3f \n", x02, hant2);
#endif
        //絶対値にする
        if (hant < 0){
            hant = -hant;
        }
        if (hant2 < 0){
            hant2 = -hant2;
        }
        if (hant < hant2){
            //小さい方を選択
            x[0] = x01;
        }else{
            x[0] = x02;
        }
#ifdef  DEBUG_APO
#endif
        
    }else { 
        //3つのセンサーが一直線ではなく三角形配置の時
        
        d[1] = (0 - x[1] * x[1] + x[2] * x[2] - y[1] * y[1] + y[2] * y[2] + r[1] * r[1] - r[2] * r[2]) / 2;
        d[2] = (0 - x[2] * x[2] + x[3] * x[3] - y[2] * y[2] + y[3] * y[3] + r[2] * r[2] - r[3] * r[3]) / 2;
        d[3] = (0 - x[3] * x[3] + x[1] * x[1] - y[3] * y[3] + y[1] * y[1] + r[3] * r[3] - r[1] * r[1]) / 2;

        f[1] = (b[1] * c[2] - b[2] * c[1]) / e;
        f[2] = (a[2] * c[1] - a[1] * c[2]) / e;
        g[1] = (b[2] * d[1] - b[1] * d[2]) / e;
        g[2] = (a[1] * d[2] - a[2] * d[1]) / e;

#ifdef  DEBUG_APO
        printf("d1:%10.3f d2:%10.3f d3:%10.3f \n", d[1], d[2], d[3]);
        printf("f1:%10.6f f2:%10.6f \n", f[1], f[2]);
        printf("g1:%10.6f g2:%10.6f \n", g[1], g[2]);
#endif
        
        //解の公式
        aa = (f[1] * f[1]) + (f[2] * f[2]) - 1;
        bb = (2 * f[1]) * (g[1] - x[1]) + (2 * f[2]) * (g[2] - y[1]) - (2 * r[1]);
        cc = (g[1] - x[1]) * (g[1] - x[1]) + (g[2] - y[1]) * (g[2] - y[1]) + (z[1] * z[1]) - (r[1] * r[1]);

        q = (bb * bb) - (4 * aa * cc);      //解の公式の条件

#ifdef  DEBUG_APO
        printf("A:%10.6f B:%10.5f C:%10.3f \n", aa, bb, cc);
        printf("Q:%10.3f \n", q);
#endif    

        if (q < 0){
            //解の公式の条件をみたさない
            calc_stat = CALC_STATUS_QUAD_F;
            calcValue[numResult].status = calc_stat;
            ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
            printf("CAL%1d: Q is less than zero! ", (numResult + 1));
#endif
            return calc_stat;
        }

        //二次方程式を解いてx,y,rを求める計算
        r[0] = (-bb - sqrt(q)) / (2 * aa);

#ifdef  DEBUG_APO
        printf("r0:%10.3f ", r[0]);
#endif    
        if (r[0] < 0){
            //半径が負の方は不採用
            r[0] = (-bb + sqrt(q)) / (2 * aa);
#ifdef  DEBUG_APO
            printf("XXX  -> r0:%10.3f ", r[0]);
#endif  
            if (r[0] < 0){
                //2つともゼロ以下
                calc_stat = CALC_STATUS_R0_UNDER0;
                calcValue[numResult].status = calc_stat;
                ledLightOn(LED_CAUTION);
#ifdef  DEBUG_APO_2
                printf("CAL%1d: R0 is less than zero! ", numResult);
#endif
#ifdef  DEBUG_APO
                printf("XXX  -> r0 is no solution! \n");
#endif  
                return calc_stat;
            }
        }else{
#ifdef  DEBUG_APO
            printf("...OK\n");
#endif
        }

        
        x[0] = f[1] * r[0] + g[1];
        y[0] = f[2] * r[0] + g[2];
        
    }   //三角配置ここまで
    
    //共通　代入
    calcValue[numResult].radius0_mm = r[0];
    calcValue[numResult].impact_pos_x_mm = x[0];
    calcValue[numResult].impact_pos_y_mm = y[0];
    
#ifdef  DEBUG_APO
    printf("r0:%10.3f x0:%10.5f y0:%10.5f \n\n", r[0], calcValue[numResult].impact_pos_x_mm, calcValue[numResult].impact_pos_y_mm);
    //CORETIMER_DelayMs(2);
#endif
    
    return calc_stat;
}


