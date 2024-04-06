/**
  CLC1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    clc1.c

  @Summary
    This is the generated driver implementation file for the CLC1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides implementations for driver APIs for CLC1.
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

/**
  Section: Included Files
*/

#include "clc1.h"

/**
  Section: CLC1 APIs
*/

void CLC1_Initialize(void)
{
    // Set the CLC1 to the options selected in the User Interface

	CLC1CONL = 0x80A2 & ~(0x8000);

    CLC1CONH = 0x0C;

    CLC1SELL = 0x5000;

    CLC1GLSL = 0x802;

    CLC1GLSH = 0x00;

	
	CLC1_Enable();
}

void __attribute__ ((weak)) CLC1_PositiveEdge_CallBack(void)
{
    // Add your custom callback code here
}

void __attribute__ ((weak)) CLC1_NegativeEdge_CallBack(void)
{
    // Add your custom callback code here
}

void CLC1_PositiveEdge_Tasks ( void )
{
	if(IFS7bits.CLC1PIF)
	{
		// CLC1 PositiveEdge callback function 
		CLC1_PositiveEdge_CallBack();
		
		// clear the CLC1 interrupt flag
		IFS7bits.CLC1PIF = 0;
	}
}

void CLC1_NegativeEdge_Tasks ( void )
{
	if(IFS11bits.CLC1NIF)
	{
		// CLC1 NegativeEdge callback function 
		CLC1_NegativeEdge_CallBack();
		
		// clear the CLC1 interrupt flag
		IFS11bits.CLC1NIF = 0;
	}
}
bool CLC1_OutputStatusGet(void)
{
    return(CLC1CONLbits.LCOUT);

}
/**
 End of File
*/
