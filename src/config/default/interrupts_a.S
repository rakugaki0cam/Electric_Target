/*******************************************************************************
  System Interrupts File

  Company:
    Microchip Technology Inc.

  File Name:
    interrupt_a.S

  Summary:
    Raw ISR definitions.

  Description:
    This file maps all the interrupt vectors to their corresponding
    implementations. If a particular module interrupt is used, then its ISR
    definition can be found in corresponding PLIB source file. If a module
    interrupt is not used, then its ISR implementation is mapped to dummy
    handler.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
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
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <xc.h>
#include "ISR_Support.h"

    .extern  INPUT_CAPTURE_1_Handler

    .section   .vector_6,code, keep
    .equ     __vector_dispatch_6, IntVectorINPUT_CAPTURE_1_Handler
    .global  __vector_dispatch_6
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorINPUT_CAPTURE_1_Handler

IntVectorINPUT_CAPTURE_1_Handler:
    portSAVE_CONTEXT
    la    s6,  INPUT_CAPTURE_1_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorINPUT_CAPTURE_1_Handler
    .extern  TIMER_2_Handler

    .section   .vector_9,code, keep
    .equ     __vector_dispatch_9, IntVectorTIMER_2_Handler
    .global  __vector_dispatch_9
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorTIMER_2_Handler

IntVectorTIMER_2_Handler:
    portSAVE_CONTEXT
    la    s6,  TIMER_2_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorTIMER_2_Handler
    .extern  INPUT_CAPTURE_2_Handler

    .section   .vector_11,code, keep
    .equ     __vector_dispatch_11, IntVectorINPUT_CAPTURE_2_Handler
    .global  __vector_dispatch_11
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorINPUT_CAPTURE_2_Handler

IntVectorINPUT_CAPTURE_2_Handler:
    portSAVE_CONTEXT
    la    s6,  INPUT_CAPTURE_2_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorINPUT_CAPTURE_2_Handler
    .extern  TIMER_3_Handler

    .section   .vector_14,code, keep
    .equ     __vector_dispatch_14, IntVectorTIMER_3_Handler
    .global  __vector_dispatch_14
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorTIMER_3_Handler

IntVectorTIMER_3_Handler:
    portSAVE_CONTEXT
    la    s6,  TIMER_3_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorTIMER_3_Handler
    .extern  INPUT_CAPTURE_3_Handler

    .section   .vector_16,code, keep
    .equ     __vector_dispatch_16, IntVectorINPUT_CAPTURE_3_Handler
    .global  __vector_dispatch_16
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorINPUT_CAPTURE_3_Handler

IntVectorINPUT_CAPTURE_3_Handler:
    portSAVE_CONTEXT
    la    s6,  INPUT_CAPTURE_3_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorINPUT_CAPTURE_3_Handler
    .extern  INPUT_CAPTURE_4_Handler

    .section   .vector_21,code, keep
    .equ     __vector_dispatch_21, IntVectorINPUT_CAPTURE_4_Handler
    .global  __vector_dispatch_21
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorINPUT_CAPTURE_4_Handler

IntVectorINPUT_CAPTURE_4_Handler:
    portSAVE_CONTEXT
    la    s6,  INPUT_CAPTURE_4_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorINPUT_CAPTURE_4_Handler
    .extern  INPUT_CAPTURE_5_Handler

    .section   .vector_26,code, keep
    .equ     __vector_dispatch_26, IntVectorINPUT_CAPTURE_5_Handler
    .global  __vector_dispatch_26
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorINPUT_CAPTURE_5_Handler

IntVectorINPUT_CAPTURE_5_Handler:
    portSAVE_CONTEXT
    la    s6,  INPUT_CAPTURE_5_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorINPUT_CAPTURE_5_Handler
    .extern  COMPARATOR_1_Handler

    .section   .vector_32,code, keep
    .equ     __vector_dispatch_32, IntVectorCOMPARATOR_1_Handler
    .global  __vector_dispatch_32
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorCOMPARATOR_1_Handler

IntVectorCOMPARATOR_1_Handler:
    portSAVE_CONTEXT
    la    s6,  COMPARATOR_1_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorCOMPARATOR_1_Handler
    .extern  COMPARATOR_2_Handler

    .section   .vector_33,code, keep
    .equ     __vector_dispatch_33, IntVectorCOMPARATOR_2_Handler
    .global  __vector_dispatch_33
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorCOMPARATOR_2_Handler

IntVectorCOMPARATOR_2_Handler:
    portSAVE_CONTEXT
    la    s6,  COMPARATOR_2_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorCOMPARATOR_2_Handler
    .extern  I2C1_BUS_Handler

    .section   .vector_41,code, keep
    .equ     __vector_dispatch_41, IntVectorI2C1_BUS_Handler
    .global  __vector_dispatch_41
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorI2C1_BUS_Handler

IntVectorI2C1_BUS_Handler:
    portSAVE_CONTEXT
    la    s6,  I2C1_BUS_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorI2C1_BUS_Handler
    .extern  I2C1_MASTER_Handler

    .section   .vector_43,code, keep
    .equ     __vector_dispatch_43, IntVectorI2C1_MASTER_Handler
    .global  __vector_dispatch_43
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorI2C1_MASTER_Handler

IntVectorI2C1_MASTER_Handler:
    portSAVE_CONTEXT
    la    s6,  I2C1_MASTER_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorI2C1_MASTER_Handler
    .extern  I2C2_BUS_Handler

    .section   .vector_59,code, keep
    .equ     __vector_dispatch_59, IntVectorI2C2_BUS_Handler
    .global  __vector_dispatch_59
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorI2C2_BUS_Handler

IntVectorI2C2_BUS_Handler:
    portSAVE_CONTEXT
    la    s6,  I2C2_BUS_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorI2C2_BUS_Handler
    .extern  I2C2_MASTER_Handler

    .section   .vector_61,code, keep
    .equ     __vector_dispatch_61, IntVectorI2C2_MASTER_Handler
    .global  __vector_dispatch_61
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorI2C2_MASTER_Handler

IntVectorI2C2_MASTER_Handler:
    portSAVE_CONTEXT
    la    s6,  I2C2_MASTER_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorI2C2_MASTER_Handler
    .extern  COMPARATOR_3_Handler

    .section   .vector_160,code, keep
    .equ     __vector_dispatch_160, IntVectorCOMPARATOR_3_Handler
    .global  __vector_dispatch_160
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorCOMPARATOR_3_Handler

IntVectorCOMPARATOR_3_Handler:
    portSAVE_CONTEXT
    la    s6,  COMPARATOR_3_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorCOMPARATOR_3_Handler
    .extern  COMPARATOR_4_Handler

    .section   .vector_161,code, keep
    .equ     __vector_dispatch_161, IntVectorCOMPARATOR_4_Handler
    .global  __vector_dispatch_161
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorCOMPARATOR_4_Handler

IntVectorCOMPARATOR_4_Handler:
    portSAVE_CONTEXT
    la    s6,  COMPARATOR_4_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorCOMPARATOR_4_Handler
    .extern  COMPARATOR_5_Handler

    .section   .vector_162,code, keep
    .equ     __vector_dispatch_162, IntVectorCOMPARATOR_5_Handler
    .global  __vector_dispatch_162
    .set     nomicromips
    .set     noreorder
    .set     nomips16
    .set     noat
    .ent  IntVectorCOMPARATOR_5_Handler

IntVectorCOMPARATOR_5_Handler:
    portSAVE_CONTEXT
    la    s6,  COMPARATOR_5_Handler
    jalr  s6
    nop
    portRESTORE_CONTEXT
    .end   IntVectorCOMPARATOR_5_Handler

