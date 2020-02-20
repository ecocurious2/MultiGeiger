#ifndef LORAWAN_H
#define LORAWAN_H
#include "userdefines.h"

#if SEND2LORA
/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/
// Ensure the setting is correct in the LMIC config file located in
// LMIC library/project_config/lmic_project_config.h
// These two defines have no more effect than creating an error if the library setting
// is not correct
//#define CFG_eu868 1
//#define CFG_sx1276_radio 1
#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include "hal/heltecv2.h"
#include <SPI.h>
#include "loraWan.h"
#include "log.h"

//
// For normal use, we require that you edit the sketch to replace FILLMEIN
// with values assigned by the TTN console. However, for regression tests,
// we want to be able to compile these scripts. The regression tests define
// COMPILE_REGRESSION_TEST, and in that case we define FILLMEIN to a non-
// working but innocuous value.
//
#ifdef COMPILE_REGRESSION_TEST
# define FILLMEIN 0
#else
# warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
# define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif

// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
// static const u1_t PROGMEM APPEUI[8]={ 0x45, 0xB6, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70};
static const u1_t PROGMEM APPEUI[8]= { 0x98, 0x7A, 0x01, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
//static const u1_t PROGMEM DEVEUI[8]={ 0x00, 0x02, 0x25, 0xB0, 0x03, 0xCC, 0x10, 0x03 };
static const u1_t PROGMEM DEVEUI[8]= { 0xE8, 0x7C, 0x19, 0xC3, 0x00, 0xC0, 0xD0, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
//static const u1_t PROGMEM APPKEY[16] = { 0x16, 0x77, 0xD0, 0x1A, 0xEB, 0x57, 0xE5, 0x28, 0x84, 0x23, 0x19, 0x12, 0x70, 0xEC, 0xEF, 0xF0 };
static const u1_t PROGMEM APPKEY[16] = { 0x6F, 0xA9, 0x57, 0x78, 0xF0, 0x9E, 0xC1, 0xC5, 0xB6, 0xAF, 0x88, 0x74, 0x35, 0x58, 0xF1, 0x87 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = LORA_CS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST,
    .dio = {LORA_IRQ, LORA_IO1, LORA_IO2 },
};

static volatile transmissionStatus_t txStatus;
static uint8_t * __rxPort;
static uint8_t * __rxBuffer;
static uint8_t * __rxSz;

void event_log(const char *event){
  log(DEBUG, "%09d: %s", os_getTime(), event);
}

void onEvent(ev_t ev) {
    switch (ev) {
        case EV_SCAN_TIMEOUT:
            txStatus = TX_STATUS_ENDING_ERROR;
            event_log("EV_SCAN_TIMEOUT");
            break;
        case EV_BEACON_FOUND:
            txStatus = TX_STATUS_UNKNOWN;
            event_log("EV_BEACON_FOUND");
            break;
        case EV_BEACON_MISSED:
            txStatus = TX_STATUS_UNKNOWN;
            event_log("EV_BEACON_MISSED");
            break;
        case EV_BEACON_TRACKED:
            txStatus = TX_STATUS_UNKNOWN;
            event_log("EV_BEACON_TRACKED");
            break;
        case EV_JOINING:
            txStatus= TX_STATUS_JOINING;
            event_log("EV_JOINING");
            break;
        case EV_JOINED:
            txStatus= TX_STATUS_JOINED;
            event_log("EV_JOINED");
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              String ak, nk;
              for (int i = 0; i < sizeof(artKey); ++i) {
                ak += String(artKey[i], 16);
              }
              for (int i = 0; i < sizeof(nwkKey); ++i) {
                nk += String(nwkKey[i], 16);
              }
              log(DEBUG, "netid: %d devaddr: %x artKey: %s nwkKey: %s", netid, devaddr, ak.c_str(), nk.c_str());
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
            // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     event_log("EV_RFU1");
        ||     break;
        */
        case EV_JOIN_FAILED:
            txStatus = TX_STATUS_ENDING_ERROR;
            event_log("EV_JOIN_FAILED");
            break;
        case EV_REJOIN_FAILED:
            txStatus = TX_STATUS_ENDING_ERROR;
            event_log("EV_REJOIN_FAILED");
            break;
        case EV_TXCOMPLETE:
            event_log("EV_TXCOMPLETE (includes waiting for RX windows)");
            txStatus =   TX_STATUS_UPLINK_SUCCESS;
            if (LMIC.txrxFlags & TXRX_ACK) {
              txStatus = TX_STATUS_UPLINK_ACKED;
              event_log("Received ack");
            }
            if (LMIC.dataLen) {
              log(DEBUG, "Received %d bytes of payload", LMIC.dataLen);
              if ( __rxPort != NULL) *__rxPort = LMIC.frame[LMIC.dataBeg-1];
              if ( __rxSz != NULL ) *__rxSz = LMIC.dataLen;
              if ( __rxBuffer != NULL ) memcpy(__rxBuffer,&LMIC.frame[LMIC.dataBeg],LMIC.dataLen);
              txStatus = TX_STATUS_UPLINK_ACKED_WITHDOWNLINK;
            }
            // Schedule next transmission
            //os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            txStatus = TX_STATUS_ENDING_ERROR;
            event_log("EV_LOST_TSYNC");
            break;
        case EV_RESET:
            txStatus = TX_STATUS_ENDING_ERROR;
            event_log("EV_RESET");
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            txStatus = TX_STATUS_UNKNOWN;
            event_log("EV_RXCOMPLETE");
            break;
        case EV_LINK_DEAD:
            txStatus = TX_STATUS_ENDING_ERROR;
            event_log("EV_LINK_DEAD");
            break;
        case EV_LINK_ALIVE:
            txStatus = TX_STATUS_UNKNOWN;
            event_log("EV_LINK_ALIVE");
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    event_log("EV_SCAN_FOUND");
        ||    break;
        */
        case EV_TXSTART:
            txStatus = TX_STATUS_UNKNOWN;
            event_log("EV_TXSTART");
            break;
        default:
            txStatus = TX_STATUS_UNKNOWN;
            log(DEBUG, "Unknown event: %u", (unsigned int) ev);
            break;
    }
}


/**
 * Setup the LoRaWan stack for TTN Europe
 */
void setup_lorawan() {
    txStatus = TX_STATUS_UNKNOWN;
    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();    
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    LMIC_setLinkCheckMode(0);
    LMIC.dn2Dr = SF9;
    LMIC_setDrTxpow(DR_SF7,14);
}

/**
 * Send LoRaWan frame with ack or not
 * - txPort : port to transmit 
 * - txBuffer : message to transmit
 * - txSz : size of the message to transmit
 * - ack : true for message ack & downlink / false for pure uplink
 *   When Ack is false, the downlink buffer can be set to NULL as rxSz and rPort
 * - rxPort : where to write the port where downlink has been received
 * - rxBuffer : where the downlinked data will be stored
 * - rxSz : size of received data
 */
transmissionStatus_t lorawan_send(uint8_t txPort, uint8_t * txBuffer, uint8_t txSz, bool ack, uint8_t * rxPort, uint8_t * rxBuffer, uint8_t * rxSz){
  // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        log(DEBUG, "OP_TXRXPEND, not sending");
        return TX_STATUS_ENDING_ERROR;
    } else {
        txStatus = TX_STATUS_UNKNOWN;
        __rxPort = rxPort;
        __rxBuffer = rxBuffer;
        __rxSz = rxSz;
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(txPort, txBuffer, txSz, ((ack)?1:0));
        // wait for completion
        uint64_t start = millis();
        while ( true ) {
          switch ( txStatus ) {
            case TX_STATUS_UNKNOWN:
            case TX_STATUS_JOINING:
            case TX_STATUS_JOINED:
                os_runloop_once();
                break;
            case TX_STATUS_UPLINK_SUCCESS:
            case TX_STATUS_UPLINK_ACKED:
            case TX_STATUS_UPLINK_ACKED_WITHDOWNLINK:
            case TX_STATUS_UPLINK_ACKED_WITHDOWNLINK_PENDING:
                return txStatus;
            case TX_STATUS_ENDING_ERROR:
            case TX_STATUS_TIMEOUT:
                break;
          }
          if ( millis() - start > LORA_TIMEOUT_MS ) {
            lorawan_setup();
            return TX_STATUS_TIMEOUT;
          }
        }        
    }
}
#endif
#endif
