#include <battery.h>
#include <inputs.h>
#include <led.h>
#include <music.h>
#include <systick.h>


typedef enum {
    BATTERY_OK,
    BATTERY_LOW,
    BATTERY_VERY_LOW
} battery_state_t;

static battery_state_t battery_state = BATTERY_OK;


// ****************************************************************************
static void battery_alarm_callback(void)
{
    // Repeat the alarm in 1 minute
    SYSTICK_set_callback(battery_alarm_callback, 60 * 1000);

    if (battery_state == BATTERY_LOW) {
        MUSIC_play(&song_alarm_battery_low);

    }
    else {
        MUSIC_play(&song_alarm_battery_very_low);
    }
}


// ****************************************************************************
void BATTERY_check_level(void)
{
    uint32_t battery_voltage;

    battery_voltage = INPUTS_get_battery_voltage();

    switch (battery_state) {
        case BATTERY_OK:
            if (battery_voltage < 3500) {
                battery_state = BATTERY_LOW;
                LED_flashing();
                SYSTICK_set_callback(battery_alarm_callback, 1);
            }
            break;

        case BATTERY_LOW:
            if (battery_voltage < 3450) {
                battery_state = BATTERY_VERY_LOW;
            }
            break;

        case BATTERY_VERY_LOW:
        default:
            break;
    }
}