/*
 *  Copyright (c) 2017, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the OpenThread platform abstraction for
 *   radio communication.
 *
 */

#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif
// NRF tools use #define PACKAGE - for other purposes
// ie: the physical package the chip comes in
// This conflicts with the GNU Autoconf "PACAKGE" define
#undef PACKAGE

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <common/logging.hpp>
#include <common/code_utils.hpp>
#include <platform-config.h>
#include <openthread/platform/logging.h>
#include <openthread/platform/radio.h>
#include <openthread/platform/diag.h>

#include <openthread-core-config.h>
#include <openthread-config.h>
#include <openthread/types.h>

#include "dw1000_device_api.h"
#include "dw1000_driver.h"
#include "dw1000_radio.h"

#define SHORT_ADDRESS_SIZE           2
#define EXTENDED_ADDRESS_SIZE        8

#define ACK_HEADER_WITH_PENDING      0x12  // First byte of ACK frame containing
                                           // pending bit
#define ACK_HEADER_WITHOUT_PENDING   0x02  // First byte of ACK frame without
                                           // pending bit

enum
{
    IEEE802154_MIN_LENGTH         = 5,
    IEEE802154_MAX_LENGTH         = 127,
    IEEE802154_ACK_LENGTH         = 5,

    IEEE802154_FRAME_TYPE_ACK     = 2 << 0,
    IEEE802154_FRAME_TYPE_MACCMD  = 3 << 0,
    IEEE802154_FRAME_TYPE_MASK    = 7 << 0,

    IEEE802154_SECURITY_ENABLED   = 1 << 3,
    IEEE802154_FRAME_PENDING      = 1 << 4,
    IEEE802154_ACK_REQUEST        = 1 << 5,

    IEEE802154_DST_ADDR_NONE      = 0 << 2,
    IEEE802154_DST_ADDR_SHORT     = 2 << 2,
    IEEE802154_DST_ADDR_EXT       = 3 << 2,
    IEEE802154_DST_ADDR_MASK      = 3 << 2,

    IEEE802154_SRC_ADDR_NONE      = 0 << 6,
    IEEE802154_SRC_ADDR_SHORT     = 2 << 6,
    IEEE802154_SRC_ADDR_EXT       = 3 << 6,
    IEEE802154_SRC_ADDR_MASK      = 3 << 6,

    IEEE802154_DSN_OFFSET         = 2,
    IEEE802154_DSTPAN_OFFSET      = 3,
    IEEE802154_DSTADDR_OFFSET     = 5,

};

static otRadioFrame sTransmitFrame;
static otRadioFrame sReceiveFrame;
static otError sTransmitError;
static otError sReceiveError;

static uint8_t sTransmitPsdu[IEEE802154_MAX_LENGTH];
static uint8_t sReceivePsdu[IEEE802154_MAX_LENGTH];
static uint8_t ack_psdu[] = {0x12 , 0x00, 0x00, 0x00, 0x00};
static uint8_t sChannel = 0;

bool  receiveDone=1;
bool  transmitDone=1;

static otRadioState sState = OT_RADIO_STATE_DISABLED;
static bool     sIsReceiverEnabled = false;
uint8_t         *dw_status;

void enableReceiver(void)
{
    if (!sIsReceiverEnabled)
    {
        otLogInfoPlat(sInstance, "Enabling receiver", NULL);

        /* TBD: FIFO related changes if required */
        dw1000RxEnable();
        sIsReceiverEnabled = true;
    }
}

void disableReceiver(void)
{
    if (sIsReceiverEnabled)
    {
        otLogInfoPlat(sInstance, "Disabling receiver", NULL);

        /* TBD: FIFO related changes if required
         * To add : Wait if radio is transmitting any frame
         * To check for whether Receiver is enabled or not
         */

        dw1000RxDisable();
        sIsReceiverEnabled = false;
    }
}

static void convertShortAddress(uint8_t *aTo, uint16_t aFrom)
{
    aTo[0] = (uint8_t) aFrom;
    aTo[1] = (uint8_t)(aFrom >> 8);
}

void setChannel(uint8_t achannel)
{
    if (sChannel != achannel)
    {
        bool enabled = false;

        if (sIsReceiverEnabled)
        {
            disableReceiver();
            enabled = true;
        }

        otLogInfoPlat(sInstance, "Channel=%d", achannel);
        dw1000SetChannel(achannel);
        sChannel = achannel;

        if (enabled)
        {
            enableReceiver();
        }
    }
}

void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64)
{
    (void)aInstance;

    dw1000GetEui(aIeeeEui64);
}

uint16_t otPlatformGetPanId(otInstance *aInstance)
{
    (void)aInstance;

    return dw1000GetPanid();
}

void otPlatRadioSetPanId(otInstance *aInstance, uint16_t aPanid)
{
    (void)aInstance;

    otLogInfoPlat(sInstance, "PANID=%X", aPanid);
    dw1000SetPanid(aPanid);
}

void otPlatRadioSetExtendedAddress(otInstance *aInstance, uint8_t *aAddress)
{
    (void)aInstance;

    otLogInfoPlat(sInstance, "ExtAddr=%X%X%X%X%X%X%X%X", aAddress[7],
                  aAddress[6], aAddress[5], aAddress[4], aAddress[3],
                  aAddress[2], aAddress[1], aAddress[0]);

    dw1000SetExtAddress(aAddress);
}

void otPlatRadioSetShortAddress(otInstance *aInstance, uint16_t aAddress)
{
    (void)aInstance;

    otLogInfoPlat(sInstance, "ShortAddr=%X", aAddress);

    dw1000SetShortAddress(aAddress);
}

void dw1000RadioInit(void)
{
    sTransmitFrame.mLength  = 0;
    sTransmitFrame.mPsdu    = sTransmitPsdu;
    sReceiveFrame.mLength   = 0;
    sReceiveFrame.mPsdu     = sReceivePsdu;

    dw1000Init();

    otLogInfoPlat(sInstance, "Initialized", NULL);
}

bool otPlatRadioIsEnabled(otInstance *aInstance)
{
    (void)aInstance;

    return (sState != OT_RADIO_STATE_DISABLED) ? true : false;
}

otError otPlatRadioEnable(otInstance *aInstance)
{
    if (!otPlatRadioIsEnabled(aInstance))
    {
        otLogDebgPlat(sInstance, "State=OT_RADIO_STATE_SLEEP", NULL);
        sState = OT_RADIO_STATE_SLEEP;
    }

    return OT_ERROR_NONE;
}

otError otPlatRadioDisable(otInstance *aInstance)
{
    if (otPlatRadioIsEnabled(aInstance))
    {
        otLogDebgPlat(sInstance, "State=OT_RADIO_STATE_DISABLED", NULL);

        sState = OT_RADIO_STATE_DISABLED;
    }

    return OT_ERROR_NONE;
}

otError otPlatRadioSleep(otInstance *aInstance)
{
    otError error = OT_ERROR_INVALID_STATE;
    (void)aInstance;

    if (sState == OT_RADIO_STATE_SLEEP || sState == OT_RADIO_STATE_RECEIVE)
    {
        otLogDebgPlat(sInstance, "State=OT_RADIO_STATE_SLEEP", NULL);

        error  = OT_ERROR_NONE;
        sState = OT_RADIO_STATE_SLEEP;
        disableReceiver();
    }

    return error;
}

otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel)
{
    otError error = OT_ERROR_INVALID_STATE;
    (void)aInstance;
    (void)aChannel;

    if (sState != OT_RADIO_STATE_DISABLED)
    {
        otLogDebgPlat(sInstance, "State=OT_RADIO_STATE_RECEIVE", NULL);

        error  = OT_ERROR_NONE;
        sState = OT_RADIO_STATE_RECEIVE;

        setChannel(aChannel);
        sReceiveFrame.mChannel = aChannel;
        enableReceiver();
    }

    return error;
}

otError otPlatRadioTransmit(otInstance *aInstance, otRadioFrame *aPacket)
  {

    otError error = OT_ERROR_INVALID_STATE;
    (void)aInstance;

    if (sState == OT_RADIO_STATE_RECEIVE)
    {
        error          = OT_ERROR_NONE;
        sState         = OT_RADIO_STATE_TRANSMIT;
        sTransmitError = OT_ERROR_NONE;

        dwt_forcetrxoff();
        dwt_writetxdata(aPacket->mLength, (uint8_t *)aPacket->mPsdu, 0);
        dwt_writetxfctrl(aPacket->mLength, 0);

        /* Start transmission. */
        setChannel(aPacket->mChannel);
        dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);

        /* Wait for transmit done*/
        while(dwt_read32bitoffsetreg(SYS_STATUS_ID,0) == SYS_STATUS_TXFRS){}
         transmitDone=1;
    }

    otLogDebgPlat(sInstance, "Transmitted %d bytes", aPacket->mLength);

    return error;
}

otRadioFrame *otPlatRadioGetTransmitBuffer(otInstance *aInstance)
{
    (void)aInstance;

    return &sTransmitFrame;
}

int8_t otPlatRadioGetRssi(otInstance *aInstance)
{
    (void)aInstance;

    return 0;
}

otRadioCaps otPlatRadioGetCaps(otInstance *aInstance)
{
    (void)aInstance;

    return OT_RADIO_CAPS_NONE;
}

bool otPlatRadioGetPromiscuous(otInstance *aInstance)
{
    (void)aInstance;

    return dw1000GetPromiscuous();
}

void otPlatRadioSetPromiscuous(otInstance *aInstance, bool aEnable)
{
    (void)aInstance;

    otLogInfoPlat(sInstance, "PromiscuousMode=%d", aEnable ? 1 : 0);

    dw1000SetPromiscuous(aEnable);
}

void dw1000RadioProcess(otInstance *aInstance)
{
    decaIrqStatus_t stat;

    stat = decamutexon();

    VerifyOrExit(sState == OT_RADIO_STATE_RECEIVE ||
                 sState == OT_RADIO_STATE_TRANSMIT, ;);

    if ((sState == OT_RADIO_STATE_RECEIVE && sReceiveFrame.mLength > 0) ||
        (sState == OT_RADIO_STATE_TRANSMIT &&
         sReceiveFrame.mLength > IEEE802154_ACK_LENGTH))
    {
#if OPENTHREAD_ENABLE_DIAG

        if (otPlatDiagModeGet())
        {
            otPlatDiagRadioReceiveDone(aInstance,
                                       &sReceiveFrame,
                                       sReceiveError);
        }
        else
#endif
        {
            // signal MAC layer for each received frame if promiscous is enabled
            // otherwise only signal MAC layer for non-ACK frame
            if ((dw1000GetPromiscuous() ||
                (sReceiveFrame.mLength > IEEE802154_ACK_LENGTH)) &&
                (receiveDone==1))
            {
                otLogDebgPlat(sInstance,
                              "Received %d bytes",
                              sReceiveFrame.mLength);
                otPlatRadioReceiveDone(aInstance,
                                       &sReceiveFrame,
                                       sReceiveError);
                receiveDone=0;
            }
        }
    }

    if (sState == OT_RADIO_STATE_TRANSMIT)
    {
        if (sTransmitError != OT_ERROR_NONE ||
            (sTransmitFrame.mPsdu[0] & IEEE802154_ACK_REQUEST) == 0)
        {
            if (sTransmitError != OT_ERROR_NONE)
            {
                otLogDebgPlat(sInstance,
                              "Transmit failed ErrorCode=%d",
                              sTransmitError);
            }

            sState = OT_RADIO_STATE_RECEIVE;

#if OPENTHREAD_ENABLE_DIAG
            if (otPlatDiagModeGet())
            {
                otPlatDiagRadioTransmitDone(aInstance,
                                            &sTransmitFrame,
                                            false,
                                            sTransmitError);
            }
            else
#endif
            {
                otPlatRadioTransmitDone(aInstance,
                                        &sTransmitFrame,
                                        false,
                                        sTransmitError);
            }

            transmitDone=0;
        }
        else if (sReceiveFrame.mLength == IEEE802154_ACK_LENGTH &&
                 (sReceiveFrame.mPsdu[0] & IEEE802154_FRAME_TYPE_MASK)
                                                == IEEE802154_FRAME_TYPE_ACK &&
                 (sReceiveFrame.mPsdu[IEEE802154_DSN_OFFSET]
                                == sTransmitFrame.mPsdu[IEEE802154_DSN_OFFSET]))
        {
            sState = OT_RADIO_STATE_RECEIVE;

#if OPENTHREAD_ENABLE_DIAG
            if (otPlatDiagModeGet())
            {
                otPlatDiagRadioTransmitDone(
                    aInstance,
                    &sTransmitFrame,
                    (sReceiveFrame.mPsdu[0] & IEEE802154_FRAME_PENDING) != 0,
                    sTransmitError);
            }
            else
#endif
            {
                otPlatRadioTransmitDone(
                    aInstance,
                    &sTransmitFrame,
                    (sReceiveFrame.mPsdu[0] & IEEE802154_FRAME_PENDING) != 0,
                    sTransmitError);
            }

            receiveDone=0;
        }
    }

    sReceiveFrame.mLength = 0;

exit:
    decamutexoff(stat);
return;
}

void otPlatRadioEnableSrcMatch(otInstance *aInstance, bool aEnable)
{
    (void) aInstance;

    dw1000_auto_pending_bit_set(aEnable);
}

otError otPlatRadioAddSrcMatchShortEntry(otInstance *aInstance,
                                             const uint16_t aShortAddress)
{
    (void) aInstance;
    otError error;
    uint8_t shortAddress[SHORT_ADDRESS_SIZE];

    convertShortAddress(shortAddress, aShortAddress);

    if (dw1000_pending_bit_for_addr_set(shortAddress, false))
    {
        error = OT_ERROR_NONE;
    }
    else
    {
        error = OT_ERROR_NO_BUFS;
    }

    return error;
}

otError otPlatRadioAddSrcMatchExtEntry(otInstance *aInstance,
                                           const uint8_t *aExtAddress)
{
    (void) aInstance;
    otError error;

    if (dw1000_pending_bit_for_addr_set(aExtAddress, true))
    {
        error = OT_ERROR_NONE;
    }
    else
    {
        error = OT_ERROR_NO_BUFS;
    }

    return error;
}

otError otPlatRadioClearSrcMatchShortEntry(otInstance *aInstance,
                                               const uint16_t aShortAddress)
{
    (void) aInstance;
    otError error;
    uint8_t shortAddress[SHORT_ADDRESS_SIZE];

    convertShortAddress(shortAddress, aShortAddress);

    if (dw1000_pending_bit_for_addr_clear(shortAddress, false))
    {
        error = OT_ERROR_NONE;
    }
    else
    {
        error = OT_ERROR_NO_ADDRESS;
    }

    return error;
}

otError otPlatRadioClearSrcMatchExtEntry(otInstance *aInstance,
                                             const uint8_t *aExtAddress)
{
    (void) aInstance;
    otError error;

    if (dw1000_pending_bit_for_addr_clear(aExtAddress, true))
    {
        error = OT_ERROR_NONE;
    }
    else
    {
        error = OT_ERROR_NO_ADDRESS;
    }

    return error;
}

void otPlatRadioClearSrcMatchShortEntries(otInstance *aInstance)
{

    (void) aInstance;

    dw1000_pending_bit_for_addr_reset(false);
}

void otPlatRadioClearSrcMatchExtEntries(otInstance *aInstance)
{

    (void) aInstance;

    dw1000_pending_bit_for_addr_reset(true);
}

/* TBD : This feature is not yet implemented */
otError otPlatRadioEnergyScan(otInstance *aInstance,
                              uint8_t aScanChannel,
                              uint16_t aScanDuration)
{

    (void)aInstance;
    (void)aScanChannel;
    (void)aScanDuration;

    return OT_ERROR_NOT_IMPLEMENTED;
}

/*
 *  GPIOTE_IRQ handler
 *
 *  This handler is used to interrupt cpu on Receive Good Frame and Transmit
 *  Frame Sent
 */
void  GPIOTE_IRQHandler(void)
{
    uint32_t sStatus = 0;

    sStatus    = dwt_read32bitoffsetreg(SYS_STATUS_ID,0);

    /* Check Whether interrupt is to indicate Received Frame is Good */
    if (sStatus & SYS_STATUS_RXFCG)
    {
        dwt_write32bitreg(
            SYS_STATUS_ID,
            SYS_STATUS_RXFCG | SYS_STATUS_RXPHD | SYS_STATUS_RXDFR);

        sReceiveFrame.mLength =
            (uint8_t) dwt_read16bitoffsetreg(RX_FINFO_ID,0) & 0x7f;

        dwt_readrxdata(sReceiveFrame.mPsdu, sReceiveFrame.mLength, 0);

         if (sReceiveFrame.mPsdu[0] & IEEE802154_ACK_REQUEST)
            {
               if((sReceiveFrame.mPsdu[1] & IEEE802154_SRC_ADDR_EXT) ||
                  (sReceiveFrame.mPsdu[1] & IEEE802154_SRC_ADDR_SHORT) ||
                  (sReceiveFrame.mPsdu[1] & IEEE802154_DST_ADDR_EXT) ||
                  (sReceiveFrame.mPsdu[1] & IEEE802154_DST_ADDR_SHORT))
                     ack_psdu[0]= 0x12;
               else
                     ack_psdu[0]= 0x02;

               ack_psdu[2] = sReceiveFrame.mPsdu[2];
               dwt_writetxdata(5, (uint8_t *)ack_psdu, 0);
               dwt_writetxfctrl(5, 0);
               dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
           }
         else
         {
              dw1000RxEnable();
         }

           receiveDone=1;
    }

   /*Check if any Receive Frame Error occured. If any error occured manually
     reset and enable the receiver*/
    else if (sStatus & (DWT_INT_ARFE | DWT_INT_RFSL | DWT_INT_SFDT |
             DWT_INT_RPHE| DWT_INT_RFCE | DWT_INT_RFTO | DWT_INT_RXPTO))
    {
         dwt_rxreset();
         dwt_write32bitreg(SYS_STATUS_ID, (DWT_INT_ARFE | DWT_INT_RFSL |
                            DWT_INT_SFDT | DWT_INT_RPHE | DWT_INT_RFCE |
                            DWT_INT_RFTO | DWT_INT_RXPTO));
         dw1000RxEnable();
    }
    else
    {
       /* Do Nothing */
    }

   /* Clear the Interrupt*/
    NRF_GPIOTE->EVENTS_PORT = 0;
}

/*This function is for printing logs for debug purpose.*/
uint8_t otPlatRadioPrintBuf(uint8_t *abuffer)
{
    memcpy(abuffer, dw_status, 4);

    /*Return the size. (Allowed value is 32 bytes)*/
     return 4;
}

/* This function sets the Default tx power */
void otPlatRadioSetDefaultTxPower(otInstance *aInstance, int8_t aPower)
{
    // TBD for Setting Default transmit power
    (void)aInstance;
    (void)aPower;
}

/*This function used to Receive the Receiver sensitivity */
int8_t otPlatRadioGetReceiveSensitivity(otInstance *aInstance)
{
    // TBD for Getting receiver sensitivity
    (void)aInstance;
    int8_t a=12;
    return a;
}
