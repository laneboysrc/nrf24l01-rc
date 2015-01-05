;
; D52 configuration file for 3xs.bin
; Generated by D52 V3.4.1 on 2014/12/13 15:54
;
c 0000-0a0d	; Code space

b 0a0e-0a15 ; bit masks
! 0a15 bit masks

c 0a16-0cf9	; Code space

b 0cfa-0d43 ; ram_initialization_data


c 0d44-1478	; Code space


; ******************************************************
; ******************************************************
; ******************************************************


! 0000 Vector: Reset
! 0003 Vector: Interrupt from pin GP INT0, GP INT1 or GP INT2
! 000b Vector: Timer 0 overflow
! 0013 Vector: Power Failure
! 001b Vector: Timer 1 overflow
! 0023 Vector: Serial channel
! 002b Vector: Timer 2 overflow or External reload
! 0043 Vector: RF SPI ready
! 004b Vector: RF Interrupt
;l 0053 Vector: SPI, I2C

; ******************************************************
; ******************************************************
; ******************************************************


;f 88 TCON
k 88 TCON_it0
k 89 TCON_ie0
k 8a TCON_it1
k 8b TCON_ie1
k 8c TCON_tr0
k 8d TCON_tf0
k 8e TCON_tr1
k 8f TCON_tf1

f 93 P0DIR
f 94 P1DIR

f 98 S0CON
k 98 S0CON_ri0
k 99 S0CON_ti0
k 9a S0CON_rb80
k 9b S0CON_tb80
k 9c S0CON_ren0
k 9d S0CON_sm20
k 9e S0CON_sm0
k 9f S0CON_sm1

f 99 S0BUF
f 9e P0CON
f 9f P1CON

f a1 PWMDC0
f a2 PWMDC1
f a3 CLKCTRL
f a4 PWRDWN
f a5 WUCON
f a7 MEMCON

f a8 IEN0
k a8 IEN0_ifp
k a9 IEN0_tf0
k aa IEN0_pwrfail
k ab IEN0_tf1
k ac IEN0_serial
k ad IEN0_tf2
k ae IEN0_6
k af IEN0_all

f a9 IP0
f aa S0RELL
f ab RTC2CPT01
f ac RTC2CPT10
f ad CLKLFCTRL
f ae OPMCON
f af WDSW

f b1 RSTREAS
f b2 PWMCON
f b3 RTC2CON
f b4 RTC2CMP0
f b5 RTC2CMP1
f b6 RTC2CPT00

f b8 IEN1
k b8 IEN1_rfready
k b9 IEN1_rfirq
k ba IEN1_spi_iic
k bb IEN1_wakeup
k bc IEN1_misc
k bd IEN1_tick
k be IEN1_6
k bf IEN1_exf2

f b9 IP1
f ba S0RELH
f bc SPISCON0
f be SPISSTAT
f bf SPISDAT

f c0 IRCON
k c0 IRCON_rfready
k c1 IRCON_rfiry
k c2 IRCON_spi_iic
k c3 IRCON_wakeup
k c4 IRCON_misc
k c5 IRCON_tick
k c6 IRCON_tf2
k c7 IRCON_exf2

f c1 CCEN
f c2 CCL1
f c3 CCH1
f c4 CCL2
f c5 CCH2
f c6 CCL3
f c7 CCH3

;f c8 T2CON
k c8 T2CON_t2i0
k c9 T2CON_t2i1
k ca T2CON_t2cm
k cb T2CON_t2r0
k cc T2CON_t2r1
k cd T2CON_i2fr
k ce T2CON_i3fr
k cf T2CON_t2ps

f c9 MPAGE
f ca CRCL
f cb CRCH
f ce WUOPC1
f cf WUOPC0

;f d0 PSW
k d1 PSW_f1

f d1 ADCCON3
f d2 ADCCON2
f d3 ADCCON1
f d4 ADCDATH
f d5 ADCDATL
f d6 RNGCTL
f d7 RNGDAT

f d8 SERCON
k df SERCON_bd

f d9 W2SADR
f da W2DAT
f db COMPCON
f dd CCPDATIA
f de CCPDATIB
f df CCPDATO

f e1 W2CON1
f e2 W2CON0
f e4 SPIRCON0
f e5 SPIRCON1
f e6 SPIRSTAT
f e7 SPIRDAT

f e8 RFCON
k e8 RFCON_rfce
k e9 RFCON_rfcsn
k ea RFCON_rfcken

f e9 MD0
f ea MD1
f eb MD2
f ec MD3
f ed MD4
f ee MD5
f ef ARCON

f f8 FSR
k f8 FSR_0
k f9 FSR_1
k fa FSR_rdismb
k fb FSR_infen
k fc FSR_rdyn
k fd FSR_wen
k fe FSR_stp
k ff FSR_endebug

f f9 FPCR
f fa FCR
f fc SPIMCON0
f fd SPIMCON1
f fe SPIMSTAT
f ff SPIMDAT


; ******************************************************
; ******************************************************
; ******************************************************


k 93 PORT_SCL       ; p1.3
k 91 PORT_SDA       ; p1.1
k 94 PORT_TX_OFF    ; p1.4


; ******************************************************
; ******************************************************
; ******************************************************

r 1a model_count
r 05d f_model_no
r 05e f_servo_data


; ******************************************************
; ******************************************************
; ******************************************************

l 000e i2c_read_byte_from_device
# 000e ***************************************************************************


l 004e init
# 004e ************************************************************************
# 004e Init
# 004e ************************************************************************

l 00ab main
# 00ab ***************************************************************************
# 00ab MAIN LOOP
# 00ab ***************************************************************************

l 00e2 process_stick_data
# 00e2 ***************************************************************************
# 00e2 Converts the stick data received from the UART into the data
# 00e2 sent across the air.
# 00e2
# 00e2 The received data is 12 bit, with center (1500us) being 0x465
# 00e2 The data sent across the air is the timer value that has to be set
# 00e2 to give the desired time until the timer overflows. The timer tick is
# 00e2 750ns.
# 00e2
# 00e2 The algorithm to calculate the timer value is as follows:
# 00e2
# 00e2   timer_value = (uart_data * 14 / 10) + 0xf200
# 00e2
# 00e2 Three channels are calculated that way
# 00e2 ***************************************************************************


l 0218 check_if_failsafe_data
# 0218 ***************************************************************************
# 021c Check if 2nd payload byte is 0bbh, which indicates failsafe

l 0277 uart_data_processed
# 0277 ***************************************************************************

l 027a check_model_cmd_received
# 027a ***************************************************************************
l 0281 process_new_model
# 0281 ***************************************************************************
! 0284 payload+1: contains the model number!

l 0290 change_model
# 0290 ***************************************************************************

l 02b1 init_eeprom_bind_data
# 02b1 ***************************************************************************

l 033b format_eeprom
# 033b ***************************************************************************

l 042f rf_interrupt_handler
# 042f ************************************************************************
# 042f RF Interrupt Handler
# 042f
# 042f Timer0 fires off a transmit sequence every 5ms (or 4ms?)
# 042f Timer0 sends the stick/fs data immediately
# 042f After the packet is finished, the interrupt sends the same packet again
# 042f Next is one of the bind packets on channel 51h, in a round-robin fashion
# 042f Next the next hop frequency is set, and then we wait for the next timer0
# 042f ************************************************************************

l 0531 timer0_handler
# 0531 ************************************************************************
# 0531 Timer0 Interrupt Handler
# 0531 ************************************************************************


l 0620 make_bind_packets
# 0620 ************************************************************************

l 0701 uart_handler
# 0701 ************************************************************************
# 0701 UART Interrupt Handler
# 0701 ************************************************************************

l 0714 uart_receive

l 07fa get_indirect_dptr_plus_r1_r2
# 07fa ***************************************************************************
# 07fa get_indirect_dptr_plus_r1_r2
# 07fa r3: flag; 0 = return @dptr+r1, 1 = return @dptr+r2:r1
# 07fa ***************************************************************************


l 0827 set_indirect_r1_r2
# 0827 ***************************************************************************
# 0827 set_indirect_r1_r2
# 0827 r3: flag; 0 = store at @r1, 1 = store at @r2:r1
# 0827 ***************************************************************************


l 0839 mul_16
# 0839 ***************************************************************************

l 084b div_16
# 084b ***************************************************************************


l 08a0 ic2_write_address
# 08a0 ***************************************************************************

l 08a8 i2c_write_byte
# 08a8 ***************************************************************************

l 0fe9 i2c_start
# 0fe9 ***************************************************************************

l 093d rf_enable_auto_acknowledge
# 093d ***************************************************************************


l 09cf reset_ram
# 09cf ************************************************************************
# 09cf Reset
# 09cf ************************************************************************


l 00b2 servo_data_received
# 00b2 ***************************************************************************

l 0acd rf_modify_config_bit
# 0acd ***************************************************************************
# 0acd rf_modify_config_bit
# 0acd
# 0acd r7: bit number in the CONFIG reg.  r5: new bit value
# 0acd ***************************************************************************

l 0b38 read_current_model_bind_data_from_eeprom
# 0b38 ************************************************************************

l 0b9c i2c_read_byte
# 0b9c ***************************************************************************

l 0c00 rf_enable_data_pipes
# 0c00 ***************************************************************************


l 0c5a write_bytes_to_flash
# 0c5a ***************************************************************************
# 0c5a r1:r2 (r3): pointer to source memory
# 0c5a r6:r7: pointer to flash memory
# 0c5a 32h:33h: count
# 0c5a ***************************************************************************

l 0caa i2c_has_write_finished
# 0caa ***************************************************************************

l 0cfa ram_initialization_data
# 0cfa ************************************************************************

l 0d44 rf_set_crc
# 0d44 ***************************************************************************
# 0d44 In: R7: Number of bytes of CRC to use, 0 to turn it off
# 0d44 ***************************************************************************

l 0dca init_rf
# 0dca ***************************************************************************

l 0e09 rf_get_or_set_address
# 0e09 ***************************************************************************

l 0e46  rf_set_data_rate
# 0e46 ***************************************************************************

l 0e83 init_serial
# 0e83 ***************************************************************************

l 0ebc rf_get_no_bytes_in_data_pipe
# 0ebc ***************************************************************************


l 0ef3 spi_write_stream
# 0ef3 ***************************************************************************


l 0f8f make_hop_data
# 0f8f ***************************************************************************


l 10b5 generate_bind_data
# 10b5 ***************************************************************************

l 10fe rf_set_power_up
# 10fe ***************************************************************************

l 1140 rf_set_power
# 1140 ***************************************************************************

l 1197 read_byte_from_eeprom
# 1197 ***************************************************************************
# 1197 In: r6:r7 address to read from
# 1197 ***************************************************************************

l 1121 write_byte_to_eeprom
# 1121 ***************************************************************************
# 1121 In: r6:r7 address   r5: data
# 1121 ***************************************************************************

l 115f i2c_stop
# 115f ***************************************************************************

l 11c8 init_gpio
# 11c8 ***************************************************************************

l 11df spi_write_register
# 11df ***************************************************************************
# 11df In: R7: register number, R5: value
# 11df ***************************************************************************

l 117d write_byte_to_flash
# 117d ***************************************************************************

l 11f6 flash_erase_page
# 11f6 ***************************************************************************

l 120c i2c_write_byte_to_device
# 120c ***************************************************************************

l 1222 rf_setup_address_width
# 1222 ************************************************************************

l 1237 spi_set_rf_channel
# 1237 ***************************************************************************
# 1237 In: R7: channel number
# 1237 ***************************************************************************

l 129b rng_is_result_ready
# 129b ***************************************************************************

l 12e5 rf_get_lost_packet_count
# 12e5 ***************************************************************************

l 12f2 rf_get_tx_fifo_status
# 12f2 ***************************************************************************

l 12ff rf_get_tx_reuse
# 12ff ***************************************************************************

l 130c spi_read_register
# 130c ***************************************************************************
# 130c In: A: register    Out: R7: read value
# 130c ***************************************************************************

l 1319 read_current_model_no_from_eeprom
# 1319 ************************************************************************

l 1325 rf_get_and_clear_interrupts
# 1325 ***************************************************************************

l 1331 rf_is_rx_fifo_full
# 1331 ***************************************************************************

l 133d rf_is_tx_fifo_full
# 133d ***************************************************************************

l 1349 rf_enable_dynamic_payload
# 1349 ***************************************************************************

l 1379 get_osc_status
# 1379 ************************************************************************
# 1379 Returns 2 if the XTAL OSC is ready, otherwise 1 if running from the RC OSC
# 1379 ************************************************************************

l 1384 save_current_model
# 1384 ***************************************************************************

l 13c5 spi_write
# 13c5 ***************************************************************************

l 13a5 rng_power_on_off
# 13a5 ***************************************************************************

l 13b0 rng_bias_corrector_on_off
# 13b0 ***************************************************************************

l 13cf rf_get_number_of_address_bytes
! 13cf SETUP_AW
# 13cf ***************************************************************************
# 13cf Out: R7: Number of address bytes
# 13cf ***************************************************************************

l 13bb init_timer0
# 13bb ************************************************************************

l 13f7 rf_get_power_detected
# 13f7 ***************************************************************************

l 1401 rf_write_ack_payload
# 1401 ***************************************************************************

l 140b rf_get_data_pipe_ready
# 140b ***************************************************************************

l 1415 rf_reuse_tx_payload
# 1415 ***************************************************************************

l 141f rf_flush_rx
# 141f ***************************************************************************

l 145f rf_get_fifo_status
# 145f ***************************************************************************

l 1429 rf_flush_tx
# 1429 ***************************************************************************

l 1433 rf_get_status
# 1433 ***************************************************************************

l 144b rf_write_tx_payload
# 144b ***************************************************************************

l 1477 rng_get_random_byte
# 1477 ***************************************************************************