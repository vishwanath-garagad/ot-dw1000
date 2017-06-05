/*! ----------------------------------------------------------------------------
 * @file    dw1000_driver.h
 * @brief   This file defines the DW1000 Radio Driver Interface for OpenThread
 *
 * @attention
 *
 * Copyright 2017 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 */

#ifndef DW1000_DRIVER_H_
#define DW1000_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include "device/nrf.h"
#include "cmsis/cmsis_gcc.h"

#include "dw1000_device_api.h"
#include "dw1000_regs.h"
#include "platform-nrf5.h"
#include "sdk_config.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_drv_spi.h"
#include "nrf_spi.h"

#define GPIOTE_PIN 30  /* GPIO Pin-30 is configured for External interrupt */

/* To configure LEDs on/off on NRF52840 Platform*/
#define LED1       13   /* GPIO 13 -> LED1 (LED is connected to GPIO Pin)*/
#define LED2       14   /* GPIO 14 -> LED2 */
#define LED3       15   /* GPIO 15 -> LED3 */
#define LED4       16   /* GPIO 16 -> LED4 */

#define SHORT_ADDRESS_SIZE    2    /* Size of Short Mac Address */
#define EXTENDED_ADDRESS_SIZE 8    /* Size of Extended Mac Address */

/* dw_status is used for printing debug logs */
uint8_t *dw_status;

/**
 * @brief Initialize dw1000 Radio
 *
 * This function is used to initialize dw1000
 *
 **/
void dw1000Init(void);

/**
 * @brief Read the PANID (Personal Area Network ID) of DW1000.
 *
 * This Fucntion returns the copy of the PAN Id.
 **/
uint16_t dw1000GetPanid(void);

/**
 * @section Setting addresses and Pan Id of this device.
 */

/**
 * @brief Set  @brief Set the PANID (Personal Area Network ID) of DW1000..
 *
 * @param[in]  p_pan_id  Pointer to PAN Id (2 bytes, little-endian).
 *
 * This function configures the PAN Id.
 */
void dw1000SetPanid(uint16_t p_pan_id);

/**
 * @brief Set Extended Address of this device.
 *
 * @param[in]  p_extended_address  Pointer to extended address
 *                                 (8 bytes, little-endian)
 *
 * This function makes copy of the address.
 */
void dw1000SetExtAddress(const uint8_t *p_extended_address);

/**
 * @brief Set Short Address of this device.
 *
 * @param[in]  p_short_address  Pointer to short address
 *                              (2 bytes, little-endian)
 *
 * This function configures the Short Address of this device.
 */
void dw1000SetShortAddress(uint16_t p_short_address);

/**
 * @brief Reads the configured radio channel
 *
 * This function is used to read dw1000 configured channel number
 *
 **/
uint8_t dw1000GetChannel(void);

/**
 * @brief Set the dw1000 radio channel
 *
 * This function is used to set the channel of dw1000 Radio
 *
 **/
void dw1000SetChannel(uint8_t channel);

/**
 * @brief Get the Eui64 value.
 *
 * This function is used to read the IEEE Extended Unique Identifier of dw1000
 * Radio
 *
 **/
void dw1000GetEui(uint8_t *aIeeeEui64);

/**
 * @section Promiscuous mode.
 */

/**
 * @brief Enable or disable promiscuous radio mode.
 *
 * In promiscuous mode driver notifies higher layer that it received any frame
 * (regardless frame type or destination address).
 * In normal mode (not promiscuous) higher layer is not notified about
 * ACK frames and frames with unknown type. Also frames with destination address
 * not matching this device address are ignored.
 *
 * @param[in]  enabled  If promiscuous mode should be enabled.
 */
void dw1000SetPromiscuous(bool enabled);

/**
 * @brief Check if radio is in promiscuous mode.
 *
 * @retval True   Radio is in promiscuous mode.
 * @retval False  Radio is not in promiscuous mode.
 */
bool dw1000GetPromiscuous(void);

/**
 * @brief Enable Radio Receiver
 *
 * This function is used to Enable dw1000 Receiver
 *
 **/
void dw1000RxEnable();

/**
 * @brief Disable Radio Receiver
 *
 * This function is used to Disable dw1000 Receiver by setting Transceiver off
 *
 **/
void dw1000RxDisable();

/**
 * @section Setting pending bit in automatically transmitted ACK frames.
 */

/**
 * @brief Enable or disable setting pending bit in automatically transmitted
 *        ACK frames.
 *
 * Radio driver automatically sends ACK frames in response to unicast frames
 * destined to this node.
 * Pending bit in ACK frame can be set or cleared
 * regarding data in pending buffer destined to ACK destination.
 *
 * If setting pending bit in ACK frames is disabled, pending bit in every
 * ACK frame is set.
 * If setting pending bit in ACK frames is enabled, radio driver checks
 * if there is data in pending buffer destined to ACK destination.
 * If there is no such data, pending bit is cleared.
 *
 * @note It is possible that if there is a lot of supported peers radio
 *       driver cannot verify
 *       if there is pending data before ACK is sent. In this case pending
 *       bit is set.
 *
 * @param[in]  enabled  If setting pending bit in ACK frames is enabled.
 */
void dw1000_auto_pending_bit_set(bool enabled);

/**
 * @brief Add address of peer node for which there is pending data in the buffer
 *
 * @note This function makes a copy of given address.
 *
 * @param[in]  p_addr    Array of bytes containing address of the node
 *                      (little-endian).
 * @param[in]  extended  If given address is Extended MAC Address or
 *             Short MAC Address.
 *
 * @retval True   Address successfully added to the list.
 * @retval False  There is not enough memory to store this address in the list.
 */
bool dw1000_pending_bit_for_addr_set(const uint8_t *p_addr, bool extended);

/**
 * @brief Remove address of peer node for which there is no more pending data in
 *        the buffer.
 *
 * @param[in]  p_addr    Array of bytes containing address of the node
 *                       (little-endian).
 * @param[in]  extended  If given address is Extended MAC Address or
 *                       Short MAC Address.
 *
 * @retval True   Address successfully removed from the list.
 * @retval False  There is no such address in the list.
 */
bool dw1000_pending_bit_for_addr_clear(const uint8_t *p_addr, bool extended);

/**
 * @brief Remove all addresses of given type from pending bit list.
 *
 * @param[in]  extended  If function should remove all Exnteded MAC Adresses of
 *                       all Short Addresses.
 */
void dw1000_pending_bit_for_addr_reset(bool extended);

/**
 * @brief Get External IRQ Status
 *
 * This function is used to get the external interrupt status, either enabled
 * or disabled.
 *
 **/
bool port_GetEXT_IRQStatus(void);

/**
 * @brief Disable External Interrupt
 *
 * This function is used to disable the External Interrupt from dw1000
 *
 **/
void port_DisableEXT_IRQ(void);

/**
 * @brief Enable External Interrupt
 *
 * This function is used to Enable External Interrupt from dw1000
 *
 **/
void port_EnableEXT_IRQ(void);

/**
 * @brief Sleep in milliseconds
 *
 * This function is used to introduce delay in milliseconds order
 *
 **/
void Sleep(uint32_t ms);

/**
 * @section Functions to request FSM transitions.
 *
 *          receive()       transmit()
 *          -------->       -------->
 *     Sleep         Receive         Transmit
 *          <-------- |  /|\<--------
 *           sleep()  |   |   receive() / transmitted() / busy_channel()
 *                    |   |
 * energy_detection() |   | energy_detected()
 *                   \|/  |
 *               Energy detection
 */


/**
 * @section Driver memory management
 */

/**
 * @brief Notify driver that buffer containing received frame is not used
 *        anymore.
 *
 * @note The buffer pointed by the @p p_data pointer may be modified by this
 *       function.
 *
 * @param[in]  p_data  A pointer to the buffer containing received data that is
 *                     no more needed by the higher layer.
 */
void dw1000_buffer_free(uint8_t * p_data);

#ifdef __cplusplus
}
#endif

#endif /* DW1000_RADIO_H_ */
