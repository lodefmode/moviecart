/**
  CLC1 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    clc1.h

  @Summary
    This is the generated header file for the CLC1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for CLC1.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  dsPIC33CK64MC105
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB             :  MPLAB X v5.45
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#ifndef _CLC1_H
#define _CLC1_H

/**
  Section: Included Files
*/

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: CLC1 APIs
*/

/**
  @Summary
    Initializes the CLC1

  @Description
    This routine configures the CLC1 specific control registers

  @Preconditions
    None

  @Returns
    None

  @Param
    None

  @Comment
    

  @Example
    <code>
    CLC1_Initialize();
    </code>
*/
void CLC1_Initialize(void);

/**
  @Summary
    Callback for CLC1 PositiveEdge.

  @Description
    This routine is callback for CLC1 PositiveEdge

  @Param
    None.

  @Returns
    None
 
  @Example 
	Refer to CLC1_Initialize(); for an example
*/
void CLC1_PositiveEdge_CallBack(void);

/**
  @Summary
    Callback for CLC1 NegativeEdge.

  @Description
    This routine is callback for CLC1 NegativeEdge

  @Param
    None.

  @Returns
    None
 
  @Example 
	Refer to CLC1_Initialize(); for an example
*/
void CLC1_NegativeEdge_CallBack(void);

/**
  @Summary
    Polled implementation

  @Description
    This routine is used to implement the tasks for polled implementations.
  
  @Preconditions
    CLC1_Initialize() function should have been 
    called before calling this function.
 
  @Returns 
    None
 
  @Param
    None
 
  @Example
    Refer to CLC1_Initialize(); for an example
    
*/
void CLC1_PositiveEdge_Tasks(void);

/**
  @Summary
    Polled implementation

  @Description
    This routine is used to implement the tasks for polled implementations.
  
  @Preconditions
    CLC1_Initialize() function should have been 
    called before calling this function.
 
  @Returns 
    None
 
  @Param
    None
 
  @Example
    Refer to CLC1_Initialize(); for an example
    
*/
void CLC1_NegativeEdge_Tasks(void);

/**
  @Summary
    Returns output pin status of the CLC module.

  @Description
    This routine returns output pin status of the CLC module.

  @Param
    None.

  @Returns
    Output pin status
 
  @Example 
    <code>
    bool outputStatus;
    outputStatus = CLC1_OutputStatusGet();
    </code>
*/

bool CLC1_OutputStatusGet(void);

/******************************************************************************
*                                                                             
*    Function:			CLC1_Enable
*    Description:       Enables the CLC module                                     
*      
*	 Parameters:		None                                       
*    Return Value:      None
******************************************************************************/
inline static void CLC1_Enable(void)
{
    CLC1CONLbits.LCEN = 1;
}

/******************************************************************************
*                                                                             
*    Function:			CLC1_Disable
*    Description:       Disables the CLC module                                     
*      
*	 Parameters:		None                                       
*    Return Value:      None
******************************************************************************/
inline static void CLC1_Disable(void)
{
    CLC1CONLbits.LCEN = 0;
}

#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif  // _CLC1_H
/**
 End of File
*/

