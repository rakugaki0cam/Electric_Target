/*******************************************************************************
  Comparator (CMP) Peripheral Library (PLIB)

  Company:
    Microchip Technology Inc.

  File Name:
    plib_cmp.c

  Summary:
    CMP Source File

  Description:
    None

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#include "plib_cmp.h"
#include "interrupts.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data
// *****************************************************************************
// *****************************************************************************

volatile static CMP_OBJECT cmp1Obj;
volatile static CMP_OBJECT cmp2Obj;
volatile static CMP_OBJECT cmp3Obj;
volatile static CMP_OBJECT cmp4Obj;
volatile static CMP_OBJECT cmp5Obj;
// *****************************************************************************

// *****************************************************************************
// Section: CMP Implementation
// *****************************************************************************
// *****************************************************************************

void CMP_Initialize (void)
{
    /*  Setup CMSTAT    */
    /* SIDL = false */
    CMSTAT = 0x0;

    /*  Setup CM1CON    */
    /*  CCH     = 1    */
    /*  CREF    = 1    */
    /*  EVPOL   = 2    */
    /*  OPLPWR  = false  */
    /*  ENPGA   = false  */
    /*  HYSPOL  = 1  */
    /*  HYSSEL  = Set highest hysteresis level (Typical 45 mV)  */
    /*  CFSEL   = SYSCLK Clock  */
    /*  CFLTREN = false  */
    /*  CFDIV   = 1:1 Clock Divide  */
    /*  CPOL    = 1    */
    /*  COE     = true     */
    IEC1SET = _IEC1_CMP1IE_MASK;
    /*  OPAON   = false    */
    /*  ENPGA   = false    */
    CM1CON = 0x13006091;

    /* Value loaded into CM1MSKCON is formed by combining configuration selected via MHC */
    CM1MSKCON = 0x0;

    /*  Setup CM2CON    */
    /*  CCH     = 3    */
    /*  CREF    = 1    */
    /*  EVPOL   = 2    */
    /*  OPLPWR  = false  */
    /*  ENPGA   = false  */
    /*  HYSPOL  = 1  */
    /*  HYSSEL  = Set highest hysteresis level (Typical 45 mV)  */
    /*  CFSEL   = SYSCLK Clock  */
    /*  CFLTREN = false  */
    /*  CFDIV   = 1:1 Clock Divide  */
    /*  CPOL    = 1    */
    /*  COE     = true     */
    IEC1SET = _IEC1_CMP2IE_MASK;
    /*  OPAON   = false    */
    /*  ENPGA   = false    */
    CM2CON = 0x13006093;

    /* Value loaded into CM2MSKCON is formed by combining configuration selected via MHC */
    CM2MSKCON = 0x0;

    /*  Setup CM3CON    */
    /*  CCH     = 3    */
    /*  CREF    = 1    */
    /*  EVPOL   = 2    */
    /*  OPLPWR  = false  */
    /*  ENPGA   = false  */
    /*  HYSPOL  = 1  */
    /*  HYSSEL  = Set highest hysteresis level (Typical 45 mV)  */
    /*  CFSEL   = SYSCLK Clock  */
    /*  CFLTREN = false  */
    /*  CFDIV   = 1:1 Clock Divide  */
    /*  CPOL    = 1    */
    /*  COE     = true     */
    IEC5SET = _IEC5_CMP3IE_MASK;
    /*  OPAON   = false    */
    /*  ENPGA   = false    */
    CM3CON = 0x13006093;

    /* Value loaded into CM3MSKCON is formed by combining configuration selected via MHC */
    CM3MSKCON = 0x0;

    /*  Setup CM4CON    */
    /*  CCH     = 0    */
    /*  CREF    = 1    */
    /*  EVPOL   = 2    */
    /*  HYSPOL  = 1  */
    /*  HYSSEL  = Set highest hysteresis level (Typical 45 mV)  */
    /*  CFSEL   = SYSCLK Clock  */
    /*  CFLTREN = false  */
    /*  CFDIV   = 1:1 Clock Divide  */
    /*  CPOL    = 1    */
    /*  COE     = true     */
    IEC5SET = _IEC5_CMP4IE_MASK;
    CM4CON = 0x13006090;

    /* Value loaded into CM4MSKCON is formed by combining configuration selected via MHC */
    CM4MSKCON = 0x0;

    /*  Setup CM5CON    */
    /*  CCH     = 1    */
    /*  CREF    = 1    */
    /*  EVPOL   = 2    */
    /*  OPLPWR  = false  */
    /*  ENPGA   = false  */
    /*  HYSPOL  = 1  */
    /*  HYSSEL  = Set highest hysteresis level (Typical 45 mV)  */
    /*  CFSEL   = SYSCLK Clock  */
    /*  CFLTREN = false  */
    /*  CFDIV   = 1:1 Clock Divide  */
    /*  CPOL    = 1    */
    /*  COE     = true     */
    IEC5SET = _IEC5_CMP5IE_MASK;
    /*  OPAON   = false    */
    /*  ENPGA   = false    */
    CM5CON = 0x13006091;

    /* Value loaded into CM5MSKCON is formed by combining configuration selected via MHC */
    CM5MSKCON = 0x0;

}

void CMP_1_CompareEnable (void)
{
    CM1CONSET = _CM1CON_ON_MASK;
}
void CMP_2_CompareEnable (void)
{
    CM2CONSET = _CM2CON_ON_MASK;
}
void CMP_3_CompareEnable (void)
{
    CM3CONSET = _CM3CON_ON_MASK;
}
void CMP_4_CompareEnable (void)
{
    CM4CONSET = _CM4CON_ON_MASK;
}
void CMP_5_CompareEnable (void)
{
    CM5CONSET = _CM5CON_ON_MASK;
}

void CMP_1_CompareDisable (void)
{
    CM1CONCLR = _CM1CON_ON_MASK;
}
void CMP_2_CompareDisable (void)
{
    CM2CONCLR = _CM2CON_ON_MASK;
}
void CMP_3_CompareDisable (void)
{
    CM3CONCLR = _CM3CON_ON_MASK;
}
void CMP_4_CompareDisable (void)
{
    CM4CONCLR = _CM4CON_ON_MASK;
}
void CMP_5_CompareDisable (void)
{
    CM5CONCLR = _CM5CON_ON_MASK;
}

bool CMP_StatusGet (CMP_STATUS_SOURCE ch_status)
{
    return ((CMSTAT & ch_status) != 0U);
}


void CMP_1_CallbackRegister(CMP_CALLBACK callback, uintptr_t context)
{
    cmp1Obj.callback = callback;

    cmp1Obj.context = context;
}

void __attribute__((used)) COMPARATOR_1_InterruptHandler(void)
{
    IFS1CLR = _IFS1_CMP1IF_MASK; //Clear IRQ flag

    if(cmp1Obj.callback != NULL)
    {
        uintptr_t context = cmp1Obj.context;

        cmp1Obj.callback(context);
    }
}

void CMP_2_CallbackRegister(CMP_CALLBACK callback, uintptr_t context)
{
    cmp2Obj.callback = callback;

    cmp2Obj.context = context;
}

void __attribute__((used)) COMPARATOR_2_InterruptHandler(void)
{
    IFS1CLR = _IFS1_CMP2IF_MASK; //Clear IRQ flag

    if(cmp2Obj.callback != NULL)
    {
        uintptr_t context = cmp2Obj.context;

        cmp2Obj.callback(context);
    }
}

void CMP_3_CallbackRegister(CMP_CALLBACK callback, uintptr_t context)
{
    cmp3Obj.callback = callback;

    cmp3Obj.context = context;
}

void __attribute__((used)) COMPARATOR_3_InterruptHandler(void)
{
    IFS5CLR = _IFS5_CMP3IF_MASK; //Clear IRQ flag

    if(cmp3Obj.callback != NULL)
    {
        uintptr_t context = cmp3Obj.context;

        cmp3Obj.callback(context);
    }
}

void CMP_4_CallbackRegister(CMP_CALLBACK callback, uintptr_t context)
{
    cmp4Obj.callback = callback;

    cmp4Obj.context = context;
}

void __attribute__((used)) COMPARATOR_4_InterruptHandler(void)
{
    IFS5CLR = _IFS5_CMP4IF_MASK; //Clear IRQ flag

    if(cmp4Obj.callback != NULL)
    {
        uintptr_t context = cmp4Obj.context;

        cmp4Obj.callback(context);
    }
}

void CMP_5_CallbackRegister(CMP_CALLBACK callback, uintptr_t context)
{
    cmp5Obj.callback = callback;

    cmp5Obj.context = context;
}

void __attribute__((used)) COMPARATOR_5_InterruptHandler(void)
{
    IFS5CLR = _IFS5_CMP5IF_MASK; //Clear IRQ flag

    if(cmp5Obj.callback != NULL)
    {
        uintptr_t context = cmp5Obj.context;

        cmp5Obj.callback(context);
    }
}
