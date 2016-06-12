#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <libopencm3/stm32/flash.h>


#include <config.h>
#include <systick.h>


static struct {
    bool active;
    uint32_t *version_src;
    uint32_t *version_dst;
    uint32_t *src;
    uint32_t *dst;
    size_t words_remaining;
} store_config = {.active = false};


config_t config;


#define FLASH_PAGE_SIZE 1024


// ****************************************************************************
static const config_t config_failsafe = {
    .version = 0xffffffff,
    // FIXME: add some useful defaults
    .model = {
        .name = "CONFIG CORRUPTED",
        .protocol_hk310 = {
            .hop_channels = {20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39},
            .address = {0x0d, 0x0e, 0x0a, 0x0d, 0x00}
        }
    }
};


// ****************************************************************************
static const config_t config_flash = {
    .version = CONFIG_VERSION,

    .tx = {
        .transmitter_inputs = {
            {.input = 1, .type = ANALOG_WITH_CENTER,        // Ailerons
             .calibration = {510, 1962, 3380}},

            {.input = 2, .type = ANALOG_WITH_CENTER,        // Elevator
             .calibration = {590, 1943, 3240}},

            {.input = 3, .type = ANALOG_WITH_CENTER,        // Throttle
             .calibration = {670, ADC_VALUE_HALF, 3370}},

            {.input = 4, .type = ANALOG_NO_CENTER,          // Rudder
             .calibration = {580, 1874, 3410}},
        },
        .logical_inputs = {
            {.type = ANALOG, .inputs = {1}, .labels = {AIL}},
            {.type = ANALOG, .inputs = {2}, .labels = {ELE}},
            {.type = ANALOG, .inputs = {3}, .labels = {THR, TH}},
            {.type = ANALOG, .inputs = {4}, .labels = {RUD, ST}}
        },
        .led_pwm_percent = 30,
        .bind_timeout_ms = 10 * 1000
    },

    .model = {
        .name = "HK Mini DLG",
        .mixer_units = {
            {
                .src = AIL,
                .dest = CH1,
                .curve = {
                    .type = CURVE_NONE,
                    .points = {50, 50}
                },
                .scalar = 100,
                .offset = 0
            },
            {
                .src = ELE,
                .dest = CH2,
                .curve = {
                    .type = CURVE_NONE,
                },
                .scalar = 100,
                .offset = 0
            },
            {
                .src = THR,
                .dest = CH3,
                .curve = {
                    .type = CURVE_NONE,
                },
                .scalar = 100
            },
            {
                .src = RUD,
                .dest = CH4,
                .curve = {
                    .type = CURVE_NONE,
                },
                .scalar = 100
            },
            {
                .src = 0
            }
        },

        .limits =  {
            {
                .ep_l = PERCENT_TO_CHANNEL(-35), .ep_h = PERCENT_TO_CHANNEL(30), .subtrim = -1600,
                .limit_l = -150000, .limit_h = 150000,
                .failsafe = PERCENT_TO_CHANNEL(8)
            },
            {
                .ep_l = PERCENT_TO_CHANNEL(-30), .ep_h = PERCENT_TO_CHANNEL(35), .subtrim = -2200,
                .limit_l = -150000, .limit_h = 150000,
                .failsafe = PERCENT_TO_CHANNEL(-5),
                .invert = 1
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
            {
                .ep_l = CHANNEL_N100_PERCENT, .ep_h = CHANNEL_100_PERCENT, .subtrim = 0,
                .limit_l = -150000, .limit_h = 150000
            },
        },

        .protocol_hk310 = {
            .hop_channels = {41, 21, 16, 66, 38, 33, 23, 32, 48, 37, 30, 54, 1, 12, 34, 19, 59, 17, 53, 49},
            .address = {0xab, 0x22, 0x08, 0x97, 0x45}
        }
    }
};


// static void test_flash(void)
// {
//     uint32_t loc2 = 0x0800e000;
//     volatile uint32_t loc = (uint32_t)&config_flash;


//     printf("Testing flash write...\n");

//     if (loc == loc2) {
//         printf("Address match!\n");
//     }

//     printf("Unlocking flash...\n");
//     flash_unlock();

//     printf("%p before erasing: 0x%lx (%ld)\n", (void *)loc, *(uint32_t *)loc, *(uint32_t *)loc);

//     printf("Erasing page...\n");
//     flash_erase_page((uint32_t)loc);
//     printf("%p after erasing: 0x%lx (%ld)\n", (void *)loc, *(uint32_t *)loc, *(uint32_t *)loc);

//     printf("Writing uint16...\n");
//     flash_clear_status_flags();
//     flash_program_word((uint32_t)loc, 0x4711);
//     printf("%p after writing: 0x%lx (%ld)\n", (void *)loc, *(uint32_t *)loc, *(uint32_t *)loc);

//     printf("Locking flash...\n");
//     flash_lock();
// }


// ****************************************************************************
void CONFIG_background_flash_write(void)
{
    static bool logged = false;

    if (!logged) {
        logged = true;
        printf("config.version=%lx\n", config.version);
    }


    if (!store_config.active) {
        return;
    }

    if (((uint32_t)store_config.dst % FLASH_PAGE_SIZE) == 0) {
        flash_erase_page((uint32_t)store_config.dst);
    }

    flash_program_word((uint32_t) store_config.dst, *store_config.src);

    ++store_config.src;
    ++store_config.dst;
    --store_config.words_remaining;

    if (store_config.words_remaining == 0) {
        store_config.active = false;
        flash_program_word((uint32_t)store_config.version_dst, *store_config.version_src);
        flash_lock();
        printf("CONFIG: done; config saved to flash.\n");
    }
}


// ****************************************************************************
void CONFIG_save(void)
{
    // WARNING: because config_flash is defined as const, the compiler
    // optimizes all kind of statements away. Don't put config_flash elements
    // in printf as it will not print the actual value in the flash, but
    // rather the value it was at compile time!

    if (store_config.active) {
        return;
    }

    store_config.active = true;
    store_config.version_src = (uint32_t *)(&config);
    store_config.version_dst = (uint32_t *)(&config_flash);
    store_config.src = store_config.version_src + 1;
    store_config.dst = store_config.version_dst + 1;
    store_config.words_remaining = sizeof(config) / sizeof(uint32_t) - 1;

    printf("CONFIG: saving config to flash ...\n");
    flash_unlock();
    flash_erase_page((uint32_t)store_config.version_dst);
}


// ****************************************************************************
void CONFIG_load(void)
{
    // Copy the settings stored in the flash (config_flash) into the
    // working-copy in RAM (config)
    memcpy(&config, &config_flash, sizeof(config_t));

    if (config.version != CONFIG_VERSION) {
        memcpy(&config, &config_failsafe, sizeof(config_t));
    }
}


// ****************************************************************************
void CONFIG_init(void)
{
    CONFIG_load();

    // SYSTICK_set_callback(CONFIG_save, 600);
    // SYSTICK_set_callback(test_flash, 3600);
}
