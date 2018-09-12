#ifndef __UESB_ERROR_CODES_H__
#define __UESB_ERROR_CODES_H__

#define     UESB_SUCCESS                    0x0000

// State related errors
#define     UESB_ERROR_NOT_INITIALIZED      0x0101
#define     UESB_ERROR_ALREADY_INITIALIZED  0x0102
#define     UESB_ERROR_NOT_IDLE             0x0103
#define     UESB_ERROR_NOT_IN_RX_MODE       0x0104

// Invalid parameter errors
#define     UESB_ERROR_INVALID_PARAMETERS   0x0200
#define     UESB_ERROR_DYN_ACK_NOT_ENABLED  0x0201

// FIFO related errors
#define     UESB_ERROR_TX_FIFO_FULL         0x0301
#define     UESB_ERROR_TX_FIFO_EMPTY        0x0302
#define     UESB_ERROR_RX_FIFO_EMPTY        0x0303

#endif
