/*! ------------------------------------------------------------------------------------------------------------------
 * @file        dw1000_driver.c
 * @brief       This file defines the DW1000 Radio Driver Interface for
 *              OpenThread
 *
 * @attention
 *
 * Copyright 2017 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#include "dw1000_driver.h"
#include "dw1000_spi.h"

/* Enable LED debugging */
#define LED_DEBUG_ENABLE

/* promiscuous mode is disabled by default */
static bool m_promiscuous = false;

/* Extended address defined by upper layer*/
static uint8_t m_extended_addr[EXTENDED_ADDRESS_SIZE];

/*
*@fn dw1000_irq_init()
*
* This function configures DW1000 IRQ with GPIO Interrupt
*
*/
static void dw1000_irq_init(void)
{

    nrf_gpio_cfg_sense_input(GPIOTE_PIN,
                             NRF_GPIO_PIN_PULLDOWN,
                             NRF_GPIO_PIN_SENSE_HIGH);
    nrf_gpiote_int_enable(GPIOTE_INTENSET_PORT_Msk);
    NVIC_SetPriority(GPIOTE_IRQn, GPIOTE_CONFIG_IRQ_PRIORITY);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_EnableIRQ(GPIOTE_IRQn);

}

void Sleep(uint32_t ms)
{
    nrf_delay_ms(ms);
}

bool port_GetEXT_IRQStatus()
{
    return NVIC_GetPendingIRQ(GPIOTE_IRQn);
}

void port_DisableEXT_IRQ()
{
    NVIC_DisableIRQ(GPIOTE_IRQn);
}

void port_EnableEXT_IRQ()
{
   NVIC_EnableIRQ(GPIOTE_IRQn);
}

void dw1000RxEnable()
{
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

void dw1000RxDisable()
{
     /* TBD:Need to check whether we can disable receiver explicitly
      * instead of doing dwt_forcetrxoff();
      */
     dwt_forcetrxoff();
}

void dw1000Init()
{
    dwt_config_t config;
    uint8_t      status=0;
    int          ret;

    /* configure gpios for debugging error cases using LEDs*/
#ifdef LED_DEBUG_ENABLE
    nrf_gpio_cfg_output(LED1);
    nrf_gpio_cfg_output(LED2);
    nrf_gpio_cfg_output(LED3);
    nrf_gpio_cfg_output(LED4);

    nrf_gpio_pin_set(LED1);
    nrf_gpio_pin_set(LED2);
    nrf_gpio_pin_set(LED3);
    nrf_gpio_pin_set(LED4);
#endif

   /* Initialize SPI peripheral with 2MHz Clock Frequency*/
    ret = spi_init(NRF_DRV_SPI_FREQ_2M);
    if(ret != 0)
    {
#ifdef LED_DEBUG_ENABLE
       nrf_gpio_pin_clear(LED4);
#endif
    }
    status = dwt_initialise(DWT_LOADNONE);
    if (status != 0)
    {
#ifdef LED_DEBUG_ENABLE
        nrf_gpio_pin_clear(LED1);
#endif
    }

   /* Changing the frequency to 8MHz*/
    NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M8;

    if (DWT_DEVICE_ID != dwt_readdevid())
    {
#ifdef LED_DEBUG_ENABLE
        nrf_gpio_pin_clear(LED2);
#endif
    }

    dwt_setinterrupt(DWT_INT_RFCG | (DWT_INT_ARFE | DWT_INT_RFSL |
                     DWT_INT_SFDT | DWT_INT_RPHE | DWT_INT_RFCE |
                     DWT_INT_RFTO | DWT_INT_RXPTO) , 1);

    config.chan             = 5;
    config.prf              = DWT_PRF_16M;
    config.dataRate         = DWT_BR_6M8;
    config.txCode           =  3;
    config.rxCode           =  3;
    config.txPreambLength   = DWT_PLEN_128;
    config.rxPAC            = DWT_PAC8;
    config.nsSFD            = 0;
    config.sfdTO            = (129 + 8 - 8);
    config.phrMode          = DWT_PHRMODE_STD;

    status = dwt_configure(&config,0);
    if(status != 0)
    {
#ifdef LED_DEBUG_ENABLE
       nrf_gpio_pin_clear(LED3);
#endif
    }

    dw1000_irq_init();
}

void dw1000SetPanid(uint16_t p_pan_id)
{
    dwt_setpanid(p_pan_id);
}

uint16_t dw1000GetPanid(void)
{
     uint16_t send;

     send = dwt_read16bitoffsetreg(PANADR_ID,2);
     return send;
}

uint8_t dw1000GetChannel()
{
     uint8_t channel[1]={5};

     dwt_readfromdevice(CHAN_CTRL_ID,0,1,channel);

     return channel[0];
}

void dw1000SetChannel(uint8_t chan)
{
   /* This program is configured to openrate in channel 5
      so the channel will be default 5
   */
    (void) chan;
    uint8_t channel_set[1] = {5};
    uint8_t tx_channel = 5;         // channel;
    uint8_t rx_channel = (chan<<4) & CHAN_CTRL_RX_CHAN_MASK;

    channel_set[0] = tx_channel | rx_channel;
    dwt_writetodevice(CHAN_CTRL_ID,0,1,channel_set);
}

void dw1000SetExtAddress(const uint8_t *p_extended_address)
{
    memcpy(m_extended_addr, p_extended_address, EXTENDED_ADDRESS_SIZE);
}

void dw1000SetShortAddress(uint16_t p_short_address)
{
   dwt_setaddress16(p_short_address);
}

void dw1000GetEui(uint8_t *aIeeeEui64)
{
    dwt_geteui(aIeeeEui64);
}

/*
*@fn dw1000SetPromiscuous()
*
* This function is to set/clear the device promiscuous mode
* i.e., Disabling/Enabling the frame filter feature
*/
void dw1000SetPromiscuous(bool enabled)
{
    /* By default frame filtering is not enabled in dw1000
       So currently this mode is controlled by a variable
       m_promiscuous*/
     m_promiscuous = enabled;
}

bool dw1000GetPromiscuous(void)
{
    return m_promiscuous;
}

void dw1000_auto_pending_bit_set(bool enabled)
{
     (void)enabled;
}

bool dw1000_pending_bit_for_addr_set(const uint8_t *p_addr, bool extended)
{
     (void)p_addr;
     (void)extended;

     return 1;
}

bool dw1000_pending_bit_for_addr_clear(const uint8_t *p_addr, bool extended)
{
     (void)p_addr;
     (void)extended;

     return 1;
}

void dw1000_pending_bit_for_addr_reset(bool extended)
{
    (void)extended;
}
