#include <xc.h>
#include "spi2.h"

inline __attribute__((__always_inline__)) SPI2_TRANSFER_MODE SPI2_TransferModeGet(void);
void SPI2_Exchange( uint8_t *pTransmitData, uint8_t *pReceiveData );
uint16_t SPI2_ExchangeBuffer(uint8_t *pTransmitData, uint16_t byteCount, uint8_t *pReceiveData);

void SPI2_Initialize (void) {
    // MSTEN Master; DISSDO disabled; PPRE 1:1; SPRE 2:1; MODE16 disabled; SMP Middle; DISSCK disabled; CKP Idle:Low, Active:High; CKE Active to Idle; SSEN disabled; 
    SPI2CON1 = 0x013B;
    // SPIFSD disabled; SPIBEN enabled; FRMPOL disabled; FRMDLY disabled; FRMEN disabled; 
    SPI2CON2 = 0x0001;
    // SISEL SPI_INT_SPIRBF; SPIROV disabled; SPIEN enabled; SPISIDL disabled; 
    SPI2STAT = 0x800C;
}

void SPI2_Exchange( uint8_t *pTransmitData, uint8_t *pReceiveData ) {

    while( SPI2STATbits.SPITBF == true ) {

    }

    if (SPI2_TransferModeGet() == SPI2_DRIVER_TRANSFER_MODE_16BIT)
        SPI2BUF = *((uint16_t*)pTransmitData);
    else
        SPI2BUF = *((uint8_t*)pTransmitData);

    while ( SPI2STATbits.SRXMPT == true);

    if (SPI2_TransferModeGet() == SPI2_DRIVER_TRANSFER_MODE_16BIT)
        *((uint16_t*)pReceiveData) = SPI2BUF;
    else
        *((uint8_t*)pReceiveData) = SPI2BUF;

}

uint16_t SPI2_ExchangeBuffer(uint8_t *pTransmitData, uint16_t byteCount, uint8_t *pReceiveData) {

    uint16_t dataSentCount = 0;
    uint16_t count = 0;
    uint16_t dummyDataReceived = 0;
    uint16_t dummyDataTransmit = SPI2_DUMMY_DATA;

    uint8_t  *pSend, *pReceived;
    uint16_t addressIncrement;
    uint16_t receiveAddressIncrement, sendAddressIncrement;

    SPI2_TRANSFER_MODE spiModeStatus;

    spiModeStatus = SPI2_TransferModeGet();
    // set up the address increment variable
    if (spiModeStatus == SPI2_DRIVER_TRANSFER_MODE_16BIT)
    {
        addressIncrement = 2;
        byteCount >>= 1;
    }        
    else
    {
        addressIncrement = 1;
    }

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


    while( SPI2STATbits.SPITBF == true )
    {

    }

    while (dataSentCount < byteCount)
    {
        if ((count < SPI2_FIFO_FILL_LIMIT))
        {
            if (spiModeStatus == SPI2_DRIVER_TRANSFER_MODE_16BIT)
                SPI2BUF = *((uint16_t*)pSend);
            else
                SPI2BUF = *pSend;
            pSend += sendAddressIncrement;
            dataSentCount++;
            count++;
        }

        if (SPI2STATbits.SRXMPT == false)
        {
            if (spiModeStatus == SPI2_DRIVER_TRANSFER_MODE_16BIT)
                *((uint16_t*)pReceived) = SPI2BUF;
            else
                *pReceived = SPI2BUF;
            pReceived += receiveAddressIncrement;
            count--;
        }

    }
    while (count)
    {
        if (SPI2STATbits.SRXMPT == false)
        {
            if (spiModeStatus == SPI2_DRIVER_TRANSFER_MODE_16BIT)
                *((uint16_t*)pReceived) = SPI2BUF;
            else
                *pReceived = SPI2BUF;
            pReceived += receiveAddressIncrement;
            count--;
        }
    }

    return dataSentCount;
}



uint8_t SPI2_Exchange8bit( uint8_t data )
{
    uint8_t receiveData;
    
    SPI2_Exchange(&data, &receiveData);

    return (receiveData);
}

uint16_t SPI2_Exchange8bitBuffer(uint8_t *dataTransmitted, uint16_t byteCount, uint8_t *dataReceived)
{
    return (SPI2_ExchangeBuffer(dataTransmitted, byteCount, dataReceived));
}

/**

    The module's transfer mode affects the operation
    of the exchange functions. The table below shows
    the effect on data sent or received:
    |=======================================================================|
    | Transfer Mode  |     Exchange Function      |        Comments         |
    |=======================================================================|
    |                | SPIx_Exchange8bitBuffer()  |                         |
    |                |----------------------------|  OK                     |
    |                | SPIx_Exchange8bit()        |                         |
    |     8 bits     |----------------------------|-------------------------|
    |                | SPIx_Exchange16bitBuffer() | Do not use. Only the    |
    |                |----------------------------| lower byte of the 16-bit|
    |                | SPIx_Exchange16bit()       | data will be sent or    |
    |                |                            | received.               |
    |----------------|----------------------------|-------------------------|
    |                | SPIx_Exchange8bitBuffer()  | Do not use. Additional  |
    |                |----------------------------| data byte will be       |
    |                | SPIx_Exchange8bit()        | inserted for each       |
    |                |                            | 8-bit data.             |
    |     16 bits    |----------------------------|-------------------------|
    |                | SPIx_Exchange16bitBuffer() |                         |
    |                |----------------------------|  OK                     |
    |                | SPIx_Exchange16bit()       |                         |
    |----------------|----------------------------|-------------------------|
*/
inline __attribute__((__always_inline__)) SPI2_TRANSFER_MODE SPI2_TransferModeGet(void)
{
	if (SPI2CON1bits.MODE16 == 0)
        return SPI2_DRIVER_TRANSFER_MODE_8BIT;
    else
        return SPI2_DRIVER_TRANSFER_MODE_16BIT;
}


SPI2_STATUS SPI2_StatusGet()
{
    return(SPI2STAT);
}
