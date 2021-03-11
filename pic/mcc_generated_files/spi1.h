/**
  SPI1 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    spi1.h

  @Summary
    This is the generated header file for the SPI1 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides APIs for SPI1.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.65.2
        Device            :  PIC18F47K42
        Driver Version    :  2.01
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.45
        MPLAB 	          :  MPLAB X 4.15
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#ifndef _SPI1_H
#define _SPI1_H

/**
  Section: Included Files
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: Macro Declarations
*/

#define DUMMY_DATA 0x0

/**
  Section: SPI1 Module APIs
*/

/**
  @Summary
    Initializes the SPI1

  @Description
    This routine initializes the SPI1.
    This routine must be called before any other SPI1 routine is called.
    This routine should only be called once during system initialization.

  @Preconditions
    None

  @Param
    None

  @Returns
    None

  @Comment
    

  @Example
    <code>
    uint8_t     myWriteBuffer[MY_BUFFER_SIZE];
    uint8_t     myReadBuffer[MY_BUFFER_SIZE];
    uint8_t     writeData;
    uint8_t     readData;
    uint8_t     total;

    SPI1_Initialize();

    total = 0;
    do
    {
        total = SPI1_Exchange8bitBuffer(&myWriteBuffer[total], MY_BUFFER_SIZE - total, &myReadBuffer[total]);

        // Do something else...

    } while(total < MY_BUFFER_SIZE);

    readData = SPI1_Exchange8bit(writeData);
    </code>
 */
void SPI1_Initialize(void);

/**
  @Summary
    Exchanges a data byte over SPI1

  @Description
    This routine exchanges a data byte over SPI1 bus.
    This is a blocking routine.

  @Preconditions
    The SPI1_Initialize() routine should be called
    prior to use this routine.

  @Param
    data - data byte to be transmitted over SPI1 bus

  @Returns
    The received byte over SPI1 bus

  @Example
    <code>
    uint8_t     writeData;
    uint8_t     readData;
    uint8_t     readDummy;

    SPI1_Initialize();

    // for transmission over SPI bus
    readDummy = SPI1_Exchange8bit(writeData);

    // for reception over SPI bus
    readData = SPI1_Exchange8bit(DUMMY_DATA);
    </code>
 */
uint8_t SPI1_Exchange8bit(uint8_t data);

 /**
  @Summary
    Exchanges buffer of data over SPI1

  @Description
    This routine exchanges buffer of data (of size one byte) over SPI1 bus.
    This is a blocking routine.

  @Preconditions
    The SPI1_Initialize() routine should be called
    prior to use this routine.

  @Param
    dataIn  - Buffer of data to be transmitted over SPI1.
    bufLen  - Number of bytes to be exchanged.
    dataOut - Buffer of data to be received over SPI1.

  @Returns
    Number of bytes exchanged over SPI1.

  @Example
    <code>
    uint8_t     myWriteBuffer[MY_BUFFER_SIZE];
    uint8_t     myReadBuffer[MY_BUFFER_SIZE];
    uint8_t     total;

    SPI1_Initialize();

    total = 0;
    do
    {
        total = SPI1_Exchange8bitBuffer(&myWriteBuffer[total], MY_BUFFER_SIZE - total, &myReadBuffer[total]);

        // Do something else...

    } while(total < MY_BUFFER_SIZE);
    </code>
 */
uint8_t SPI1_Exchange8bitBuffer(uint8_t *dataIn, uint8_t bufLen, uint8_t *dataOut);

/**
  @Summary
    Gets the SPI1 buffer full status

  @Description
    This routine gets the SPI1 buffer full status

  @Preconditions
    The SPI1_Initialize() routine should be called
    prior to use this routine.

  @Param
    None

  @Returns
    true  - if the buffer is full
    false - if the buffer is not full.

  @Example
    Refer to SPI1_Initialize() for an example
 */


#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif // _SPI1_H
/**
 End of File
*/
