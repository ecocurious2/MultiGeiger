typedef enum {
  TX_STATUS_UNKNOWN = 0,
  TX_STATUS_JOINING,
  TX_STATUS_JOINED,
  TX_STATUS_ENDING_ERROR,
  TX_STATUS_TIMEOUT,
  TX_STATUS_UPLINK_SUCCESS,
  TX_STATUS_UPLINK_ACKED,
  TX_STATUS_UPLINK_ACKED_WITHDOWNLINK,
  TX_STATUS_UPLINK_ACKED_WITHDOWNLINK_PENDING
} transmitionStatus_t;

#define LORA_TIMEOUT_MS 30000L

void lorawan_setup();
transmitionStatus_t lorawan_send(uint8_t txPort, uint8_t * txBuffer, uint8_t txSz, bool ack, uint8_t * rxPort, uint8_t * rxBuffer, uint8_t * rxSz);
