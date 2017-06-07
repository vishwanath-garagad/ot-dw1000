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
 * @file    dw1000_spi.h
 * @brief   SPI access functions
 *
 */

#ifndef _DW1000_SPI_H_
#define _DW1000_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OT_CLI_NCP_APP
#include "nrf_drv_spi.h"
#include "nrf_spi.h"
#endif

#include "dw1000_types.h"

#define DECA_MAX_SPI_HEADER_LENGTH      (3)                     // max number of bytes in header (for formating & sizing)

#ifndef OT_CLI_NCP_APP
#define EVB1000_LCD_SUPPORT             (1)
#endif

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(void) ;

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void) ;

#ifdef OT_CLI_NCP_APP
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0);
int spi_init(nrf_drv_spi_frequency_t frequency);
#else
#if (EVB1000_LCD_SUPPORT == 1)
/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetoLCD()
 *
 * Low level abstract function to write data to the LCD display via SPI2 peripheral
 * Takes byte buffer and rs_enable signals
 * or returns -1 if there was an error
 */
void writetoLCD
(
    uint32       bodylength,
    uint8        rs_enable,
    const uint8 *bodyBuffer
);
#else
#define writetoLCD(x)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DW1000_SPI_H_ */



