/**
 * Copyright 2017 Decawave Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*! ----------------------------------------------------------------------------
 * @file    dw1000_spi.c
 * @brief   SPI access functions
 *
 */

#include "dw1000_spi.h"
#include "dw1000_device_api.h"

#ifdef OT_CLI_NCP_APP
#include "nrf_gpio.h"
#else
#include "port.h"
#endif

int writetospi_serial( uint16 headerLength,
                    const uint8 *headerBuffer,
                    uint32 bodylength,
                    const uint8 *bodyBuffer
                  );

int readfromspi_serial( uint16  headerLength,
                     const uint8 *headerBuffer,
                     uint32 readlength,
                     uint8 *readBuffer );


#ifdef OT_CLI_NCP_APP
int spi_init(nrf_drv_spi_frequency_t frequency)
{
   int ret;
        /*SPI initialization*/
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG; //default mode 0, frequency 4MHz
    spi_config.frequency = frequency;
    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;

   ret =  nrf_drv_spi_init(&spi, &spi_config,0,NULL);

   return ret;
   /*SPI initialization*/
}
#else
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(/*SPI_TypeDef* SPIx*/)
{
    // done by port.c, default SPI used is SPI1

    return 0;

} // end openspi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void)
{
    while (port_SPIx_busy_sending()); //wait for tx buffer to empty

    port_SPIx_disable();

    return 0;

} // end closespi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
#pragma GCC optimize ("O3")
int writetospi_serial
(
    uint16       headerLength,
    const uint8 *headerBuffer,
    uint32       bodyLength,
    const uint8 *bodyBuffer
)
{

    int i=0;

    decaIrqStatus_t  stat ;

    stat = decamutexon() ;

    SPIx_CS_GPIO->BRR = SPIx_CS;

    for(i=0; i<headerLength; i++)
    {
        SPIx->DR = headerBuffer[i];

        while ((SPIx->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);

        SPIx->DR ;
    }

    for(i=0; i<bodyLength; i++)
    {
        SPIx->DR = bodyBuffer[i];

        while((SPIx->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);

        SPIx->DR ;
    }

    SPIx_CS_GPIO->BSRR = SPIx_CS;

    decamutexoff(stat) ;

    return 0;
} // end writetospi()


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
#pragma GCC optimize ("O3")
int readfromspi_serial
(
    uint16       headerLength,
    const uint8 *headerBuffer,
    uint32       readlength,
    uint8       *readBuffer
)
{

    int i=0;

    decaIrqStatus_t  stat ;

    stat = decamutexon() ;

    /* Wait for SPIx Tx buffer empty */
    //while (port_SPIx_busy_sending());

    SPIx_CS_GPIO->BRR = SPIx_CS;

    for(i=0; i<headerLength; i++)
    {
        SPIx->DR = headerBuffer[i];

        while((SPIx->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);

        readBuffer[0] = SPIx->DR ; // Dummy read as we write the header
    }

    for(i=0; i<readlength; i++)
    {
        SPIx->DR = 0;  // Dummy write as we read the message body

        while((SPIx->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);

        readBuffer[i] = SPIx->DR ;//port_SPIx_receive_data(); //this clears RXNE bit
    }

    SPIx_CS_GPIO->BSRR = SPIx_CS;

    decamutexoff(stat) ;

    return 0;
} // end readfromspi()

#if (EVB1000_LCD_SUPPORT == 1)

void writetoLCD
(
    uint32       bodylength,
    uint8        rs_enable,
    const uint8 *bodyBuffer
)
{

    int i = 0;
    int sleep = 0;
    //int j = 10000;

    if(rs_enable)
    {
        port_LCD_RS_set();
    }
    else
    {
        if(bodylength == 1)
        {
            if(bodyBuffer[0] & 0x3) //if this is command = 1 or 2 - exsecution time is > 1ms
                sleep = 1 ;
        }
        port_LCD_RS_clear();
    }

    port_SPIy_clear_chip_select();  //CS low


    //while(j--); //delay

    for(i=0; i<bodylength; i++)
    {
        port_SPIy_send_data(bodyBuffer[i]); //send data on the SPI

        while (port_SPIy_no_data()); //wait for rx buffer to fill

        port_SPIy_receive_data(); //this clears RXNE bit
    }

    //j = 10000;

    port_LCD_RS_clear();

    //while(j--); //delay

    port_SPIy_set_chip_select();  //CS high

    if(sleep)
        Sleep(2);
} // end writetoLCD()
#endif

#endif
