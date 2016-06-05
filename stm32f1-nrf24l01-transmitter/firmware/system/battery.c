#include <battery.h>
#include <inputs.h>
#include <led.h>
#include <music.h>
#include <systick.h>


typedef enum {
    BATTERY_OK,
    BATTERY_LOW,
    BATTERY_VERY_LOW,
    BATTERY_DANGEROUSLY_LOW
} battery_state_t;

#define BATTERY_LOW_LEVEL 3500
#define BATTERY_VERY_LOW_LEVEL 3430
#define BATTERY_DANGEROUSLY_LOW_LEVEL 3340

static battery_state_t battery_state = BATTERY_OK;


// ****************************************************************************
static void battery_alarm_callback(void)
{
    // Repeat the alarm in 1 minute, unless when dangerously low, then we
    // repeat it every 10 seconds
    SYSTICK_set_callback(battery_alarm_callback,
        (battery_state != BATTERY_DANGEROUSLY_LOW) ? 60 * 1000 : 10 * 1000);

    MUSIC_play((battery_state == BATTERY_LOW) ?
        &song_alarm_battery_low : &song_alarm_battery_very_low);
}


// ****************************************************************************
void BATTERY_check_level(void)
{
    uint32_t battery_voltage;

    battery_voltage = INPUTS_get_battery_voltage();

    switch (battery_state) {
        case BATTERY_OK:
            if (battery_voltage < BATTERY_LOW_LEVEL) {
                battery_state = BATTERY_LOW;
                LED_flashing();
                SYSTICK_set_callback(battery_alarm_callback, 1);
            }
            break;

        case BATTERY_LOW:
            if (battery_voltage < BATTERY_VERY_LOW_LEVEL) {
                battery_state = BATTERY_VERY_LOW;
            }
            break;

        case BATTERY_VERY_LOW:
            if (battery_voltage < BATTERY_DANGEROUSLY_LOW_LEVEL) {
                battery_state = BATTERY_DANGEROUSLY_LOW;
            }
            break;

        case BATTERY_DANGEROUSLY_LOW:
        default:
            break;
    }
}