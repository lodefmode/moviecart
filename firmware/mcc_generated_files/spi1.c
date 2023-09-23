
/**
  SPI1 Generated Driver API Source File

  Company:
    Microchip Technology Inc.

  File Name:
    spi1.c

  @Summary
    This is the generated source file for the SPI1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides APIs for driver for SPI1.
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

#include "spi1.h"

/**
 Section: File specific functions
*/

/**
  SPI1 Transfer Mode Enumeration

  @Summary
    Defines the Transfer Mode enumeration for SPI1.

  @Description
    This defines the Transfer Mode enumeration for SPI1.
 */
typedef enum {
    SPI1_TRANSFER_MODE_32BIT  = 2,
    SPI1_TRANSFER_MODE_16BIT = 1,
    SPI1_TRANSFER_MODE_8BIT = 0
}SPI1_TRANSFER_MODE;

inline __attribute__((__always_inline__)) SPI1_TRANSFER_MODE SPI1_TransferModeGet(void);
void SPI1_Exchange( uint8_t *pTransmitData, uint8_t *pReceiveData );
uint16_t SPI1_ExchangeBuffer(uint8_t *pTransmitData, uint16_t byteCount, uint8_t *pReceiveData);

/**
 Section: Driver Interface Function Definitions
*/


void SPI1_Initialize (void)
{
    // AUDEN disabled; FRMEN disabled; AUDMOD I2S; FRMSYPW One clock wide; AUDMONO stereo; FRMCNT 0; MSSEN disabled; FRMPOL disabled; IGNROV disabled; SPISGNEXT not sign-extended; FRMSYNC disabled; URDTEN disabled; IGNTUR disabled; 
    SPI1CON1H = 0x00;
    // WLENGTH 0; 
    SPI1CON2L = 0x00;
    // SPIROV disabled; FRMERR disabled; 
    SPI1STATL = 0x00;

#if 0
	// SPI1BRGL 9; 
	SPI1BRGL = 0x09;	// 5MHZ
#endif

    // SPI1BRGL 199; 
    SPI1BRGL = 0xC7;	// 250 KHz


    // SPITBFEN disabled; SPITUREN disabled; FRMERREN disabled; SRMTEN disabled; SPIRBEN disabled; BUSYEN disabled; SPITBEN disabled; SPIROVEN disabled; SPIRBFEN disabled; 
    SPI1IMSKL = 0x00;
    // RXMSK 0; TXWIEN disabled; TXMSK 0; RXWIEN disabled; 
    SPI1IMSKH = 0x00;
    // SPI1URDTL 0; 
    SPI1URDTL = 0x00;
    // SPI1URDTH 0; 
    SPI1URDTH = 0x00;
    // SPIEN enabled; DISSDO disabled; MCLKEN FOSC/2; CKP Idle:Low, Active:High; SSEN disabled; MSTEN Master; MODE16 disabled; SMP Middle; DISSCK disabled; SPIFE Frame Sync pulse precedes; CKE Active to Idle; MODE32 disabled; SPISIDL disabled; ENHBUF enabled; DISSDI disabled; 
    SPI1CON1L = 0x8121;

}

void SPI1_HighSpeed (void)
{
	// SPI1BRGL 9; 
	SPI1BRGL = 0x09;	// 5MHZ
}

void SPI1_Exchange( uint8_t *pTransmitData, uint8_t *pReceiveData )
{

    while( SPI1STATLbits.SPITBF == true )
    {

    }
        
    SPI1BUFL = *((uint8_t*)pTransmitData);

    while ( SPI1STATLbits.SPIRBE == true)
    {
    
    }

    *((uint8_t*)pReceiveData) = SPI1BUFL;
}

uint16_t SPI1_ExchangeBuffer(uint8_t *pTransmitData, uint16_t byteCount, uint8_t *pReceiveData)
{

    uint16_t dataSentCount = 0;
    uint16_t dataReceivedCount = 0;
    uint16_t dummyDataReceived = 0;
    uint16_t dummyDataTransmit = SPI1_DUMMY_DATA;

    uint8_t  *pSend, *pReceived;
    uint16_t addressIncrement;
    uint16_t receiveAddressIncrement, sendAddressIncrement;

    addressIncrement = 1;

    // set the pointers and increment delta 
    // for transmit and receive operations
    if (pTransmitData == NULL)
    {
        sendAddressIncrement = 0;
        pSend = (uint8_t*)&dummyDataTransmit;
    }
    else
    {
        sendAddressIncrement = addressIncrement;
        pSend = (uint8_t*)pTransmitData;
    }
        
    if (pReceiveData == NULL)
    {
       receiveAddressIncrement = 0;
       pReceived = (uint8_t*)&dummyDataReceived;
    }
    else
    {
       receiveAddressIncrement = addressIncrement;        
       pReceived = (uint8_t*)pReceiveData;
    }


    while( SPI1STATLbits.SPITBF == true )
    {

    }

    while (dataSentCount < byteCount)
    {
        if ( SPI1STATLbits.SPITBF != true )
        {

            SPI1BUFL = *pSend;

            pSend += sendAddressIncrement;
            dataSentCount++;

        }

        if (SPI1STATLbits.SPIRBE == false)
        {

            *pReceived = SPI1BUFL;

            pReceived += receiveAddressIncrement;
            dataReceivedCount++;
        }

    }
    while (dataReceivedCount < byteCount)
    {
        if (SPI1STATLbits.SPIRBE == false)
        {

            *pReceived = SPI1BUFL;

            pReceived += receiveAddressIncrement;
            dataReceivedCount++;
        }
    }

    return dataSentCount;
}

uint8_t SPI1_Exchange8bit( uint8_t data )
{
    uint8_t receiveData;
    
    SPI1_Exchange(&data, &receiveData);

    return (receiveData);
}


uint16_t SPI1_Exchange8bitBuffer(uint8_t *dataTransmitted, uint16_t byteCount, uint8_t *dataReceived)
{
    return (SPI1_ExchangeBuffer(dataTransmitted, byteCount, dataReceived));
}

inline __attribute__((__always_inline__)) SPI1_TRANSFER_MODE SPI1_TransferModeGet(void)
{
    if (SPI1CON1Lbits.MODE32 == 1)
        return SPI1_TRANSFER_MODE_32BIT;
    else if (SPI1CON1Lbits.MODE16 == 1)
        return SPI1_TRANSFER_MODE_16BIT;
    else
        return SPI1_TRANSFER_MODE_8BIT;
}

SPI1_STATUS SPI1_StatusGet()
{
    return(SPI1STATL);
}

/**
 End of File
*/
