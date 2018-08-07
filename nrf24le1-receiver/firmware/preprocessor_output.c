#include <stdint.h>
#include <stdbool.h>

#include <platform.h>
#include <uart0.h>
#include <preprocessor_output.h>

#ifdef ENABLE_PREPROCESSOR_OUTPUT



#define SLAVE_MAGIC_BYTE 0x87
#define CH3_HYSTERESIS 5

#define SERVO_PULSE_MIN 600
#define SERVO_PULSE_MAX 2500
#define SERVO_PULSE_CLAMP_LOW 800
#define SERVO_PULSE_CLAMP_HIGH 2300

#ifdef EXTENDED_PREPROCESSOR_OUTPUT
    #define TX_DATA_SIZE 8
    extern __xdata uint16_t raw_data[2];
#else
    #define TX_DATA_SIZE 4
#endif

#define NUMBER_OF_STARTUP_PACKETS 20

extern bool systick;
extern __xdata uint16_t channels[NUMBER_OF_CHANNELS];
extern bool successful_stick_data;

static bool initialized = false;
static __xdata uint8_t tx_data[TX_DATA_SIZE];
static uint8_t next_tx_index = 0xff;
bool ch3_2pos = false;
uint16_t ch3_raw;

typedef struct {
    uint16_t raw_data;
    int16_t normalized;
    uint16_t centre;
    uint16_t left;
    uint16_t right;
} CHANNEL_T;

__xdata CHANNEL_T servo[2];

static void normalize_channel(CHANNEL_T *c)
{
    if (c->raw_data < SERVO_PULSE_MIN  ||  c->raw_data > SERVO_PULSE_MAX) {
        c->normalized = 0;
        return;
    }

    if (c->raw_data < SERVO_PULSE_CLAMP_LOW) {
        c->raw_data = SERVO_PULSE_CLAMP_LOW;
    }

    if (c->raw_data > SERVO_PULSE_CLAMP_HIGH) {
        c->raw_data = SERVO_PULSE_CLAMP_HIGH;
    }

    if (c->raw_data == c->centre) {
        c->normalized = 0;
    }
    else if (c->raw_data < c->centre) {
        if (c->raw_data < c->left) {
            c->left = c->raw_data;
        }
        // In order to acheive a stable 100% value we actually calculate the
        // percentage up to 101%, and then clamp to 100%.
        c->normalized = (c->centre - c->raw_data) * 101 /
            (c->centre - c->left);
        if (c->normalized > 100) {
            c->normalized = 100;
        }
        c->normalized = -c->normalized;
    }
    else {
        if (c->raw_data > c->right) {
            c->right = c->raw_data;
        }
        c->normalized = (c->raw_data - c->centre) * 101 /
            (c->right - c->centre);
        if (c->normalized > 100) {
            c->normalized = 100;
        }
    }
}


// ****************************************************************************
// Convert the received timer value into microseconds of servo pulse duration
static uint16_t stickdata2ms(uint16_t stickdata)
{
    uint16_t ms;

    ms = (0xffff - stickdata) * 3 / 4;
    return ms;
}


// ****************************************************************************
void output_preprocessor(void)
{
    static uint8_t startup_count = 0;

    if (systick) {
        if (successful_stick_data && startup_count >= NUMBER_OF_STARTUP_PACKETS) {
            servo[0].raw_data = stickdata2ms(channels[0]);
            servo[1].raw_data = stickdata2ms(channels[1]);
            ch3_raw = stickdata2ms(channels[2]);

            if (!initialized) {
                initialized = true;
                servo[0].centre = servo[0].raw_data;
                servo[1].centre = servo[1].raw_data;

                servo[0].left = servo[0].centre - INITIAL_ENDPOINT_DELTA;
                servo[0].right = servo[0].centre + INITIAL_ENDPOINT_DELTA;
                servo[1].left = servo[1].centre - INITIAL_ENDPOINT_DELTA;
                servo[1].right = servo[1].centre + INITIAL_ENDPOINT_DELTA;
            }

            normalize_channel(&servo[0]);
            normalize_channel(&servo[1]);

            tx_data[1] = servo[0].normalized;
            tx_data[2] = servo[1].normalized;

            if (ch3_2pos) {
                if (ch3_raw < SERVO_PULSE_CENTER - CH3_HYSTERESIS) {
                    ch3_2pos = false;
                }
            }
            else {
                if (ch3_raw > SERVO_PULSE_CENTER + CH3_HYSTERESIS) {
                    ch3_2pos = true;
                }
            }
            tx_data[3] = ch3_2pos ? 1 : 0;
        }
        else {
            if (startup_count < NUMBER_OF_STARTUP_PACKETS) {
                ++startup_count;
            }
            tx_data[1] = 0;
            tx_data[2] = 0;
            tx_data[3] = 0 + (1 << 4);          // CH3 + STARTUP_MODE flag
        }

#ifdef EXTENDED_PREPROCESSOR_OUTPUT
        tx_data[4] = (raw_data[0] >> 5) & 0x7f;
        tx_data[5] = ((raw_data[0] << 2) | (raw_data[1] >> 14))  & 0x7f;
        tx_data[6] = (raw_data[1] >> 7)  & 0x7f;
        tx_data[7] = raw_data[1] & 0x7f;
#endif

        tx_data[0] = SLAVE_MAGIC_BYTE;
        next_tx_index = 0;
    }

#ifdef NO_DEBUG
    if (next_tx_index < sizeof(tx_data)  &&  uart0_send_is_ready()) {
        uart0_send_char(tx_data[next_tx_index++]);
    }
#endif
}

#endif // PREPROCESSOR_OUTPUT
