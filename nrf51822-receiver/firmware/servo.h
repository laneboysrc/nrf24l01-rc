#pragma once

void SERVO_init(void);
void SERVO_process(void);
void SERVO_set(uint8_t index, uint32_t pulse_ns);
void SERVO_start(void);
