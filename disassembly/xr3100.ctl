
# 0000
# 0000 ==============================================================================
# 0000 XR3100 receiver firmware
# 0000
# 0000 Also fits the HKR3000 with the following changes:
# 0000
# 0000 - Ports are different
# 0000 - HKR writes hop data at offset 5 in the EEPROM, while XR3100 at offset 10 in the EEPROM
# 0000 ==============================================================================


;=========================================================

; XR3100 ports
k 80 LED_GREEN      ; p0.0
k 81 LED_RED        ; p0.1
k 83 BIND_BUTTON    ; p0.3
k 85 PORT_CH1       ; p0.5
k 87 PORT_CH2       ; p0.7

k 90 PORT_CH3       ; p1.0
k 91 PORT_CH4       ; p1.1
k 93 SCL            ; p1.3
k 94 SDA            ; p1.4


; HKR3000 ports
;k 92 LED_GREEN      ; p1.2
;k 93 LED_RED        ; p1.3
;k 86 BIND_BUTTON    ; p0.6
;k 85 PORT_CH1       ; p0.5
;k 87 PORT_CH2       ; p0.7
;k 90 PORT_CH3       ; p1.0
;k 91 PORT_CH4       ; p1.1
;k 84 SCL            ; p0.4
;k 83 SDA            ; p0.3

;=========================================================

p 0000 ;Uncomment the line below for HKR3100
p 0000 ;Leave it commented out for XR3100
p 0000
p 0000 ;IS_HKR3000 equ 1
p 0000
p 0000
p 0000 IFDEF IS_HKR3000
p 0000 LED_GREEN_HKR3000 equ p1.2
p 0000 LED_RED_HKR3000 equ p1.3
p 0000 BIND_BUTTON_HKR3000 equ p0.6
p 0000 SCL_HKR3000 equ p0.4
p 0000 SDA_HKR3000 equ p0.3
p 0000 ENDIF

p 0059     IFDEF IS_HKR3000
p 0059         clr     a
p 0059         mov     P1DIR,a
p 0059         anl     P0DIR,#0dfh
p 0059         mov     P0CON,#56h
p 0059         anl     P0DIR,#5fh
p 0059         anl     P1DIR,#0fch
p 0059     ELSE
p 0065     ENDIF

p 00d8     IFDEF IS_HKR3000
p 00d8         clr     LED_GREEN_HKR3000
p 00d8         setb    LED_RED_HKR3000
p 00d8         jb      BIND_BUTTON_HKR3000,X00e2
p 00d8     ELSE
p 00df     ENDIF

p 0a8c     IFDEF IS_HKR3000
p 0a8c         jb      BIND_BUTTON_HKR3000,X0ac3
p 0a8c     ELSE
p 0a8f     ENDIF

p 0103     IFDEF IS_HKR3000
p 0103         setb    LED_GREEN_HKR3000
p 0103         setb    LED_RED_HKR3000
p 0103     ELSE
p 0107     ENDIF

p 0109     IFDEF IS_HKR3000
p 0109 X0109:  clr     LED_GREEN_HKR3000
p 0109         clr     LED_RED_HKR3000
p 0109     ELSE
p 010d     ENDIF

p 0115     IFDEF IS_HKR3000
p 0115         setb    LED_RED_HKR3000
p 0115         clr     LED_GREEN_HKR3000
p 0115     ELSE
p 0119     ENDIF

p 02f0     IFDEF IS_HKR3000
p 02f0         setb    LED_GREEN_HKR3000
p 02f0         clr     LED_RED_HKR3000
p 02f0     ELSE
p 02f4     ENDIF

p 02f7     IFDEF IS_HKR3000
p 02f7 output_failsafe:
p 02f7         clr     LED_GREEN_HKR3000
p 02f7         setb    LED_RED_HKR3000
p 02f7     ELSE
p 02fb     ENDIF

p 050b     IFDEF IS_HKR3000
p 050b         xrl     p1,#8
p 050b     ELSE
p 050e     ENDIF

p 0534     IFDEF IS_HKR3000
p 0534         setb    LED_RED_HKR3000
p 0534         clr     LED_GREEN_HKR3000
p 0534     ELSE
p 0538     ENDIF

p 05ae     IFDEF IS_HKR3000
p 05ae         clr     LED_RED_HKR3000
p 05ae         setb    LED_GREEN_HKR3000
p 05ae     ELSE
p 05b2     ENDIF

p 056c     IFDEF IS_HKR3000
p 056c         add     a,#5
p 056c     ELSE
p 056e     ENDIF

p 071d     IFDEF IS_HKR3000
p 071d         orl     P0DIR,#8
p 071d     ELSE
p 0720     ENDIF

p 0722     IFDEF IS_HKR3000
p 0722 X0722:  anl     P0DIR,#0f7h
p 0722     ELSE
p 0725     ENDIF

p 0738     IFDEF IS_HKR3000
p 0738         setb    SCL_HKR3000
p 0738     ELSE
p 073a     ENDIF

p 0749     IFDEF IS_HKR3000
p 0749         clr     SCL_HKR3000
p 0749     ELSE
p 074b     ENDIF

p 075e     IFDEF IS_HKR3000
p 075e         orl     P0DIR,#8
p 075e     ELSE
p 0761     ENDIF

p 0770     IFDEF IS_HKR3000
p 0770         setb    SCL_HKR3000
p 0770     ELSE
p 0772     ENDIF

p 0781     IFDEF IS_HKR3000
p 0781         jnb     SDA_HKR3000,X0798
p 0781         clr     SCL_HKR3000
p 0781     ELSE
p 0786     ENDIF

p 0798     IFDEF IS_HKR3000
p 0798 X0798:  clr     SCL_HKR3000
p 0798     ELSE
p 079a     ENDIF

p 0a20     IFDEF IS_HKR3000
p 0a20         orl     P0DIR,#8
p 0a20     ELSE
p 0a23     ENDIF

p 0a33     IFDEF IS_HKR3000
p 0a33         clr     SCL_HKR3000
p 0a33     ELSE
p 0a35     ENDIF

p 0a44     IFDEF IS_HKR3000
p 0a44         setb    SCL_HKR3000
p 0a44     ELSE
p 0a46     ENDIF

p 0a59     IFDEF IS_HKR3000
p 0a59         jnb     SDA_HKR3000,X0a5d
p 0a59     ELSE
p 0a5c     ENDIF

p 0a70     IFDEF IS_HKR3000
p 0a70         clr     SCL_HKR3000
p 0a70     ELSE
p 0a72     ENDIF

p 0be3     IFDEF IS_HKR3000
p 0be3         clr     SDA_HKR3000
p 0be3         orl     P0DIR,#8
p 0be3     ELSE
p 0be8     ENDIF

p 0bf1     IFDEF IS_HKR3000
p 0bf1         orl     P0DIR,#8
p 0bf1     ELSE
p 0bf4     ENDIF

p 0bf6     IFDEF IS_HKR3000
p 0bf6 X0bf6:  anl     P0DIR,#0f7h
p 0bf6     ELSE
p 0bf9     ENDIF

p 0bfd     IFDEF IS_HKR3000
p 0bfd         setb    SCL_HKR3000
p 0bfd     ELSE
p 0bff     ENDIF

p 0c04     IFDEF IS_HKR3000
p 0c04         clr     SCL_HKR3000
p 0c04     ELSE
p 0c06     ENDIF

p 0c0f     IFDEF IS_HKR3000
p 0c0f         orl     P0DIR,#8
p 0c0f     ELSE
p 0c12     ENDIF

p 0c1a     IFDEF IS_HKR3000
p 0c1a         setb    SCL_HKR3000
p 0c1a     ELSE
p 0c1c     ENDIF

p 0c24     IFDEF IS_HKR3000
p 0c24         jnb     SDA_HKR3000,X0c2c
p 0c24         clr     SCL_HKR3000
p 0c24     ELSE
p 0c29     ENDIF

p 0ca6     IFDEF IS_HKR3000
p 0ca6         add     a,#5
p 0ca6     ELSE
p 0ca8     ENDIF


p 0c2c     IFDEF IS_HKR3000
p 0c2c X0c2c:  clr     SCL_HKR3000
p 0c2c     ELSE
p 0c2e     ENDIF

p 0ec5     IFDEF IS_HKR3000
p 0ec5 i2c_start:
p 0ec5         orl     P0DIR,#8
p 0ec5         nop
p 0ec5         nop
p 0ec5         setb    SCL_HKR3000
p 0ec5     ELSE
p 0ecc     ENDIF

p 0ed6     IFDEF IS_HKR3000
p 0ed6         anl     P0DIR,#0f7h
p 0ed6     ELSE
p 0ed9     ENDIF

p 0ee3     IFDEF IS_HKR3000
p 0ee3         clr     SCL_HKR3000
p 0ee3     ELSE
p 0ee5     ENDIF

p 1039     IFDEF IS_HKR3000
p 1039 i2c_stop:
p 1039         anl     P0DIR,#0f7h
p 1039         nop
p 1039         setb    SCL_HKR3000
p 1039     ELSE
p 103f     ENDIF

p 1049     IFDEF IS_HKR3000
p 1049         orl     P0DIR,#8
p 1049     ELSE
p 104c     ENDIF

p 1088     IFDEF IS_HKR3000
p 1088 init_ports:
p 1088         mov     P0CON,#53h
p 1088         mov     P0CON,#54h
p 1088         clr     SCL_HKR3000
p 1088         clr     SDA_HKR3000
p 1088         anl     P0DIR,#0efh
p 1088         orl     P0DIR,#8
p 1088     ELSE
p 1098     ENDIF

;=========================================================
;=========================================================
;=========================================================



! 0003 Vector: Interrupt from pin GP INT0, GP INT1 or GP INT2
! 000b Vector: Timer 0 overflow
! 0013 Vector: Power Failure
! 001b Vector: Timer 1 overflow
! 0023 Vector: Serial channel
! 002b Vector: Timer 2 overflow or External reload
! 0043 Vector: RF SPI ready
! 004b Vector: RF Interrupt
! 0053 Vector: SPI, I2C


c 0000-047 ; Code space

b 0048-004a ; Padding

c 004b-087c ; Code space

b 087d-0884 ; Bit masks
l 087d bit_masks

c 0885-0b3d ; Code space

b 0b3e-0b90 ; Initialization values
l 0b3e ram_initialization_data

c 0b91-1249	; Code space

i 124a-1ffe	; ignore data





;=========================================================

; X0000 Does not seem to be used, but is initialized?

; Is incremented every 16ms by Timer 0 until it reaches 32h (50; takes 800ms)
x 01a9 fs_timer  ; X0001
x 02e9 fs_timer
x 0539 fs_timer
x 0bc3 fs_timer
x 0bd1 fs_timer

x 02e3 fs_timer+1 ; X0002
x 0bbd fs_timer+1

x 05c7 servo_output_state ; X0003
x 05db servo_output_state
x 05f6 servo_output_state
x 0602 servo_output_state
x 0617 servo_output_state
x 0638 servo_output_state
x 0bb2 servo_output_state

x 00b1 hop_index ; X0004
x 0186 hop_index
x 08fc hop_index


; X0005 Does not seem to be used, but is initialized?

; X0006 is incremented every hop until 20 in Timer 2 interrupt
x 01a3 inc_every_hop_to_20  ; X0006
x 01af inc_every_hop_to_20
x 0529 inc_every_hop_to_20
x 05a3 inc_every_hop_to_20
x 0919 inc_every_hop_to_20

x 02bb failsafe_flag    ; X0007
x 02fc failsafe_flag

x 00ad rcv_state     ; X0008
x 014d rcv_state
x 015b rcv_state
x 0165 rcv_state
x 01bc rcv_state

x 01d8 stick_data       ; X0009
x 01e5 stick_data
x 01f4 stick_data
x 0272 stick_data

x 01ee stick_data+1     ; X000a

x 0200 stick_data+2     ; X000b
x 020d stick_data+2
x 021c stick_data+2
x 0281 stick_data+2

x 0216 stick_data+3     ; X000c

x 0228 stick_data+4     ; X000d
x 0235 stick_data+4
x 0244 stick_data+4

x 023e stick_data+5     ; X000e

x 024d stick_data+6     ; X000f
x 025a stick_data+6
x 0269 stick_data+6
x 029f stick_data+6

x 0263 stick_data+7     ; X0010


x 031a fs_ch1l      ; X0011
x 02db fs_ch1l

x 02d3 fs_ch1h    ; X0012

x 02cb fs_ch2l    ; X0013
x 0304 fs_ch2l

x 00f0 softtmr_1ms
x 0a83 softtmr_1ms    ; X0014
x 0ba0 softtmr_1ms

x 02c3 fs_ch2h    ; X0015
x 030b fs_ch2h


x 01e9 payload      ; X0016
x 02bf payload
x 03a3 payload
x 0405 payload
x 044e payload
x 0498 payload
x 01d3 payload+1    ; X0017
x 02c7 payload+1
x 0211 payload+2    ; X0018
x 02cf payload+2
x 01fb payload+3    ; X0019
x 02d7 payload+3
x 0239 payload+4    ; X001a
x 0223 payload+5    ; X001b
x 025e payload+6    ; X001c
x 01c7 payload+7    ; X001d
x 02b2 payload+7

; X001E ???
; X001F ???

; 16 bit timer, 1ms resolution
x 0a97 bind_btn_timer   ; X0020
x 0aa5 bind_btn_timer
x 0ab2 bind_btn_timer
x 0acb bind_btn_timer
x 0ada bind_btn_timer

x 0a91 bind_btn_timer+1 ; X0021
x 0aac bind_btn_timer+1
x 0ac5 bind_btn_timer+1


x 0279 ch1_value      ; X0022
x 0313 ch1_value
x 0332 ch1_value
x 05ce ch1_value

x 0288 ch2_value      ; X0024
x 0328 ch2_value
x 05e9 ch2_value

x 0297 ch3_value      ; X0026
x 060a ch3_value

x 02a6 ch4_value      ; X0028
x 062b ch4_value


; X0029 Does not seem to be used

x 0153 hop_data     ; X002A Comes from the EEPROM bind data, 20 bytes
! 0caf offset in hop_data
! 00b5 offset in hop_data
! 090a offset in hop_data

; X002B ???
; X002C ???
; X002D ???
; X002E ???
; X002F ???

; X0030 ???
; X0031 ???
; X0032 ???
; X0033 ???
; X0034 ???
; X0035 ???
; X0036 ???
; X0037 ???
; X0038 ???
; X0039 ???
; X003A ???
; X003B ???
; X003C ???
; X003D ???


x 0c8c bind_data    ; X003E Comes from the EEPROM bind data 5 bytes
x 009e bind_data
; x bind_data+1     ; X003F
; x bind_data+2     ; X0040
; x bind_data+3     ; X0041
; x bind_data+4     ; X0042

;=========================================================

r 0b rf_status
r 0d rf_detected
r 0c got_rf_payload
r 0e save_r7
r 0f save_r5

r 10 count_l
r 11 count_h

r 14 adr_0
r 15 adr_1
r 16 adr_2
r 17 adr_3
r 18 adr_4

r 1b pipe_no

r 1c adr_flag
r 1d adr_h
r 1e adr_l

r 24 rf_int_fired
r 25 div_16ms
r 2b factory
r 2c div_5ms
r 2d rf_data_avail

r 19 bind_state
r 1a bind_timeout
r 12 bind_blink_h
r 13 bind_blink_l




;=========================================================

# 0003 ***************************************************************************
l 0003 rf_get_irq_flags

# 000e ***************************************************************************
l 000e rf_get_lost_packet_count
! 000e OBSERVE_TX

# 001e ***************************************************************************
l 001e rf_get_tx_fifo_status
! 001e FIFO_STATUS
! 0024 Masks out TX_FULL and TX_EMPTY

# 002e ***************************************************************************
l 002e i2c_eeprom_read_one_byte


# 004e ***************************************************************************
# 004e Initialization
# 004e ***************************************************************************
l 004e init

! 00c3 Enable the receiver

x 00a5 0ah

! 00d3 Enable Timer0 and Timer2 interrupts

# 00eb
# 00eb
# 00eb ***************************************************************************
# 00eb MAIN LOOP
# 00eb ***************************************************************************
l 00eb main

! 00dc Bind switch checked at reset
! 00e6 Set channel 70 in factory mode?

l 010f not_factory
l 0121 no_bind_button
l 012c read_fifo_loop

l 0140 payload_read
! 0149 Enable Timer 1 interrupt (servo pulses)
l 014c no_rf_data_pending

l 0152 rcv_state_0
l 016c rcv_state_1
l 018f rcv_state_2
l 01ae rcv_state_end

# 0170 Start hopping
# 0191 Receiver locked

! 017e T2 clock = f/12, Reload Mode 0
! 019c T2 clock = f/12, Reload Mode 0

# 01bf More than 16 hops done: resync on first hop frequency

l 01d2 stick_data_handler
# 01d2 ***************************************************************************
# 01d2 This code below sets the servo pulse durations 0x55
# 01d2
# 01d2 It copies 8 bytes starting at payload to CH1..CH4,
# 01d2 swapping high/low byte along the way.
# 01d2 Note that CH4H is always 55 because it is a marker for stick data
# 01d2
# 01d2 Maybe the fs was added to the 4-channel version, limiting
# 01d2 the number of channels?
# 01d2 ***************************************************************************

# 02b1 ***************************************************************************
# 02b1 This code below sets failsafe 0xaa
# 02b1 ***************************************************************************

l 02e1 check_if_failsafe
! 02e6 40; Trigger failsafe after 640ms

x 02e7 28h
l 02f7 output_failsafe

l 01c6 process_rf_data
l 02de rf_data_processed

l 0346 perform_bind_procedure
# 0346 ***************************************************************************
# 0346 The bind process works as follows:
# 0346
# 0346 The transmitter regularly sends data on the fixed channel 51h, with address
# 0346 12:23:23:45:78.
# 0346 This data is sent at a lower power. All transmitters do that all the time.
# 0346
# 0346 The transmitter cycles through 4 packets:
# 0346 ff aa 55 a1 a2 a3 a4 a5
# 0346 cc cc 00 ha hb hc hd he hf hg
# 0346 cc cc 01 hh hi hj hk hl hm hn
# 0346 cc cc 02 ho hp hq hr hs ht
# 0346
# 0346 ff aa 55 is the special marker for the first byte
# 0346 a1..a5 are the 5 address bytes
# 0346 cc cc is a 16 byte checksum of bytes a1..a5
# 0346 ha..ht are the 20 hop data bytes
# 0346
# 0346 a1..a5 are stored in 14h..18h
# 0346 ha..ht are stored in x002a..x003d
# 0346
# 0346 ***************************************************************************
! 0346 disable Timer1 interrupt (servo pulses)
! 0356 Set special address 12h 23h 23h 45h 78h
! 035b Set bind channel 51h
l 0380 bind_loop
l 039b binding_got_payload
l 03a2 bind_state_0
l 0404 bind_state_1
l 044d bind_state_2
l 0497 bind_state_3
# 03a2 Check if fist byte is 0ffh
l 03ac bind_1st_byte_is_ff
l 03b5 bind_2nd_byte_is_aa
l 03be bind_3rd_byte_is_55
! 03c6 payload+3
! 03d2 write 14h..18h
! 03ef loop 5 times until wrote 18h
! 03f1 save checksum in save_r5:save_r7

# 0407 Check if payload:payload+1 maches checksum
# 0412 payload+2 must be 0
! 041b payload+3
! 0427 write x002a..x0030
! 043b loop 7 times until wrote x0030h

# 0450 Check if payload:payload+1 maches checksum
# 045b payload+2 must be 1
! 0466 payload+3
! 0472 write x0031..x0037
! 0486 loop 7 times until wrote X0037

# 049a Check if payload:payload+1 maches checksum
# 04a6 payload+2 must be 2
! 04b0 payload+3
! 04bc write x0038..x003d
! 04d0 loop 6 times until wrote X003d

l 04d8 bind_check_timeout
l 04dd bind_timeout_delay_loop
! 050b blink the red LED

l 051a bind_failed
l 0541 save_bind_data

l 0546 save_bind_address_loop
l 056a save_hop_data_loop
! 0546 write 5 bytes address from 14h
l 0594 use_new_bind_data


l 05b3 servo_pulse_t1_handler
;s 0003 servo_out_state
# 05b3 ***************************************************************************
# 05b3 Timer 1 interrupt handler, generates the servo pulses
# 05b3
# 05b3 Outputs the servo pulses, 3 or 4(!) channels
# 05b3 RAM 0x03 contains a state machine. Timer 1 is used
# 05b3 for timing
# 05b3 Timing values are stored in RAM 0x22, 0x24, 0x26, 0x28
# 05b3 ***************************************************************************

l 05cd servo_pulse_ch1
l 05e4 servo_pulse_ch2
l 0601 servo_pulse_ch3
l 0626 servo_pulse_ch4
l 0647 all_servos_done

l 0662 get_indirect_r1_r2
# 0662 ***************************************************************************
# 0662 get_indirect_r1_r2
# 0662 r3: flag; 0 = return @r1, 1 = return @r2:r1
# 0662 ***************************************************************************

l 06a8 set_indirect_r1_r2
# 06a8 ***************************************************************************
# 06a8 set_indirect_r1_r2
# 06a8 r3: flag; 0 = store at @r1, 1 = store at @r2:r1
# 06a8 ***************************************************************************

l 083e reset_handler
# 083e ***************************************************************************
# 083e Program start
# 083e ***************************************************************************
l 0841 _clear_ram

l 0885 init_rodata
# 0885 ***************************************************************************
# 0885 Initalize RAM with default values stored in a ROM table
# 0885 ***************************************************************************
l 084a init_ljmp
l 08af init_rodata_loop
l 084d init_rodata_page0

! 088f init_count -> r7
# 08af At this point r2:r0 contains the address to store values, r6:r7 the count

! 0b42 x0020 = 0, x0021 = 0
! 0b46 x0005 = 0
! 0b4a x0004 = 0
! 0b51 26h = 12h, 27h = 23h, 28h = 23h, 29h = 45h, 2ah = 78h
! 0b5c x0022.. = f8h, 2fh, f8h, 2fh, f8h 2fh, f8h, 2fh (= 1500us timer value! servo pulse CH1..4)
! 0b67 x0009.. = f8h, 2fh, f8h, 2fh, f8h 2fh, f8h, 2fh (= 1500us timer value! stick data+2..10)
! 0b6b x0003 = 0
! 0b6f x0000 = 0
! 0b73 x0006 = 0
! 0b77 x0008 = 0
! 0b7b x0014 = 0
! 0b7e 24h = 0
! 0b83 x0001 = 0, x0002 = 64h
! 0b86 2dh = 0
! 0b89 25h = 0
! 0b8c 2ch = 0
! 0b8f 2bh = 0
! 0b90 end marker

l 07ac rf_enable_auto_acknowledge
# 07ac ***************************************************************************
! 07ae EN_RXADDR
! 07b5 EN_AA

! 0865 Offset to bit_masks! (from @a+pc instruction below)

l 08ca timer2_handler
# 08ca ***************************************************************************
# 08ca Timer 2 interrupt handler
# 08ca
# 08ca Triggers every 1ms
# 08ca Performs frequency hopping every 4ms
# 08ca ***************************************************************************

! 08fb hop_index = (hop_index + 1) % 20

l 0941 rf_read_fifo_data
# 0941 ***************************************************************************
# 0941 r2:r1: address to store the information
# 0941 r3: flag; 0 = store at @r1, 1 = store at @r2:r1
# 0941 r7:
# 0941 ***************************************************************************
l 095a rf_read_is_0_or_1_or_5
l 096a rf_read_is_8
l 0985 rf_read_is_not_null

! 097e READ_RX_PAYLOAD

l 09b3 rf_modify_config_bit
# 09b3 ***************************************************************************
# 09b3 rf_modify_config_bit
# 09b3
# 09b3 r7: bit number in the CONFIG reg.  r5: new bit value
# 09b3 ***************************************************************************
! 09b8 CONFIG
l 09e3 rf_cfg_r7_is_5
l 09fc rf_cfg_r7_is_6
l 09d3 rf_cfg_r7_is_7
! 09c2 Jump if r7 is 5
! 09c5 Jump if r7 is 6
! 09c9 Jump if r7 is NOT 8 (jump if 7?)
! 0a1a CONFIG
l 0a14 rf_modify_execute

! 0a94 1388 => 5000ms

l 0ae4 rf_init_data_pipes
# 0ae4 ***************************************************************************
! 0ae8 EN_RXADDR
! 0aef EN_AA

l 0a82 is_bind_button_pressed
# 0a82 ***************************************************************************


! 0b2d Disable ‘Auto Acknowledgment’
! 0b28 Enable ‘Auto Acknowledgment’ on all pipes
! 0b32 EN_RXADDR
! 0b39 EN_AA


l 0b91 timer0_handler
# 0b91 ***************************************************************************
# 0b91 Timer 0 interrupt handler
# 0b91
# 0b91 Sets the servo output timer 1 every 16ms
# 0b91 ***************************************************************************

l 0c31 write_bytes_to_flash
# 0c31 ***************************************************************************
# 0c31 r1:r2 (r3): pointer to source memory
# 0c31 r6:r7: pointer to flash memory
# 0c31 33h:34h: count
# 0c31 ***************************************************************************

l 0c81 read_bind_data
# 0c81 ***************************************************************************


l 0cc6 rf_set_crc
# 0cc6 ***************************************************************************
# 0cc6 rf_set_crc
# 0cc6 In: R7: Number of bytes of CRC to use, 0 to turn it off
# 0cc6 ***************************************************************************

! 0cc9 CONFIG
! 0cde clear EN_CRC (CRC off)
l 0ce4 set_crc_1byte
# 0ce4 ***************************************************************************
! 0ce7 set EN_CRC
! 0cee clear CRCO (1 bytes CRC)
l 0cf4 set_crc_2bytes
# 0cf4 ***************************************************************************
! 0cf7 set EN_CRC
! 0cfe set CRCO (2 bytes CRC)
l 0d02 set_crc_write_config
! 0d05 CONFIG
! 0d0b EN_RXADDR
! 0d12 EN_AA
! 0d40 EN_RXADDR
! 0d47 EN_AA

# 0d4c ***************************************************************************
l 0d4c rf_set_receive_address_for_pipe
! 0d6a W_REGISTER + RX_ADDR_P0 + pipe_no

# 0d76 ***************************************************************************
l 0d76 rf_set_receive_address_for_pipe_1_to_5
! 0d78 RX_ADDR_P0 + pipe_no

# 0d89 ***************************************************************************
l 0d89 rf_set_data_rate
! 0d8b RF_SETUP
! 0dc1 RF_SETUP
! 0dda RX_PW_P0
! 0dde RX_PW_P1
! 0de2 RX_PW_P2
! 0de6 RX_PW_P3
! 0dea RX_PW_P4
! 0dee RX_PW_P5

l 0dfd rf_command_register_sequence
# 0dfd ***************************************************************************
# 0dfd Write command given in r7 with parameters:
# 0dfd Count in reg23h; Values at r2:r1,
# 0dfd ***************************************************************************



# 070f ***************************************************************************
l 070f ic2_write_address
# 0ec5 ***************************************************************************
l 0ec5 i2c_start
# 1039 ***************************************************************************
l 1039 i2c_stop
# 0a1e ***************************************************************************
l 0a1e i2c_read_byte
# 0717 ***************************************************************************
l 0717 i2c_write_byte
# 10e2 ***************************************************************************
l 10e2 i2c_read_byte_from_eeprom
# 0be1 ***************************************************************************
l 0be1 i2c_has_write_finished
# 10b6 ***************************************************************************
l 10b6 i2c_write_byte_to_eeprom

# 0dc6 ***************************************************************************
l 0dc6 rf_get_payload_count_in_pipe

# 0e34 ***************************************************************************
l 0e34 rf_get_address
! 0e56 RX_ADDR_P0 + r7


# 0e69 ***************************************************************************
l 0e69 rf_configure_auto_retransmission
! 0e94 SETUP_RETR


# 0e99 ***************************************************************************
! 0e9b RF_SETUP
! 0ec0 RF_SETUP

# 0ef0 ***************************************************************************
l 0ef0 rf_configure_dynamic_payload
! 0ef2 FEATURE
! 0f15 FEATURE
! 0f1c RF_SETUP
! 0f3e RF_SETUP
! 0f45 FEATURE
! 0f66 FEATURE
! 0f6f FEATURE
! 0f8c FEATURE

# 0fb5 ***************************************************************************
l 0fb5 rf_enable_rx
! 0fb8 CONFIG
! 0fd4 CONFIG

# 0f91 ***************************************************************************
l 0f91 copy_block

# 0fd8 ***************************************************************************
l 0fd8 rf_set_power_up
! 0fdb CONFIG
! 0ff7 CONFIG



l 0ffb write_bind_data_byte
# 0ffb ***************************************************************************

l 101a rf_set_tx_power
# 101a ***************************************************************************
! 101c RF_SETUP
! 1034 RF_SETUP

l 1057 write_byte_to_flash
# 1057 ***************************************************************************

# 1071 ***************************************************************************
# 1071 delay
# 1071
# 1071 Delay value in R6/R7
# 1071 ***************************************************************************
l 1071 delay
l 107c delay_loop

# 1088 ***************************************************************************
# 1088 init_ports
# 1088 ***************************************************************************
l 1088 init_ports

# 109f ***************************************************************************
# 109f In: R7: register number, R5: value
# 109f ***************************************************************************
l 109f rf_write_register

# 10f7 ***************************************************************************
l 10f7 rf_setup_address_width
! 1107 SETUP_AW


# 10cc ***************************************************************************
l 10cc flash_erase_page

# 111f ***************************************************************************
l 111f rf_clear_irq
! 112b STATUS

# 1185 ***************************************************************************
l 1242 init_timer1
# 1242 ***************************************************************************
l 1185 init_timer0

# 118f ***************************************************************************
l 118f spi_write

l 1191 _spi_write_loop



l 113d rf_read_register
# 113d ***************************************************************************
# 113d In: A: register    Out: R7: read value
# 113d ***************************************************************************

# 114a ***************************************************************************
# 114a Reads the RF status register. Clears the IRQ flags. Output in R7
# 114a ***************************************************************************
l 114a rf_read_status
l 114c STATUS

! 1185 Set Timer0 as 16 bit timer
! 118b Enable Timer0 interrupt

# 1199 ***************************************************************************
# 1199 Out: R7: Number of address bytes in the receiver
# 1199 ***************************************************************************
l 1199 rf_get_number_of_address_bytes
x 119a 03h
! 1199 SETUP_AW

# 110c ***************************************************************************
# 110c In: R7: channel number
# 110c ***************************************************************************
l 110c rf_set_channel
! 111a RF_CH

# 1130 ***************************************************************************
l 1130 rf_get_tx_reuse

# 1156 ***************************************************************************
l 1156 rf_is_rx_fifo_full
! 1156 FIFO_STATUS

# 1162 ***************************************************************************
l 1162 rf_is_tx_fifo_full
! 1162 FIFO_STATUS

# 116e ***************************************************************************
l 116e rf_enable_dynamic_payload_length
! 1175 DYNPD

# 117a ***************************************************************************
l 117a rf_is_tx_fifo_emtpy
! 117a FIFO_STATUS

# 11a3 ***************************************************************************
# 11a3 R7 is 1 if the RF RX FIFO is empty, 0 if it has data pending
# 11a3 ***************************************************************************
l 11a3 rf_is_rx_fifo_empty
! 11a3 FIFO_STATUS

# 11ad ***************************************************************************
l 11ad rf_get_rx_fifo_status
! 11ad FIFO_STATUS

# 11b7 ***************************************************************************
l 11b7 rf_get_retransmit_count
! 11b7 OBSERVE_TX

# 11c1 ***************************************************************************
# 11c1 Returns the Received Power Detector (Carrier Detect) flag
# 11c1 ***************************************************************************
l 11c1 is_rf_power_detected
! 11c1 RPD

# 11cb ***************************************************************************
l 11cb rf_write_ack_payload

# 11d5 ***************************************************************************
l 11d5 rf_read_rx_status

# 11fd ***************************************************************************
l 11fd rf_get_status

# 11df ***************************************************************************
l 11df rf_set_reuse_tx_payload

# 11e9 ***************************************************************************
l 11e9 rf_flush_rx

# 11f3 ***************************************************************************
l 11f3 rf_flush_tx

# 1207 ***************************************************************************
l 1207 enable_timer2

# 120e ***************************************************************************
# 120e rf_set_payload_bytes
# 120e
# 120e r7: data pipe number  r5: Number of payload bytes
# 120e ***************************************************************************
l 120e rf_set_payload_bytes
! 120f RX_PW_P0 + pipe_no

l 1215 rf_write_tx_payload
# 1215 ***************************************************************************
! 1217 W_TX_PAYLOAD

l 121c rf_write_tx_payload_no_ack
# 121c ***************************************************************************
! 121e W_TX_PAYLOAD_NOACK

l 1223 get_indirect_r7_r6
# 1223 ***************************************************************************

l 122a rf_get_observe_tx
# 122a ***************************************************************************
! 122a OBSERVE_TX

l 1230 rf_get_fifo_status
# 1230 ***************************************************************************
! 1230 FIFO_STATUS

l 1236 rf_read_rx_fifo_payload_width
# 1236 ***************************************************************************
# 1236 Returns the number of bytes of the top payload in the RX FIFO
# 1236 ***************************************************************************
! 1236 R_RX_PL_WID


l 123c rf_read_fifo
# 123c ***************************************************************************
# 123c Read data from the RF FIFO
# 123c  r3,#1,  r2,#0  r1,#16h
# 123c r2:r1: address to store the information
# 123c r3: flag; 0 = store at @r1, 1 = store at @r2:r1
# 123c  sets r7,#8 then calls X0941
# 123c ***************************************************************************

! 1242 Set Timer1 as 16 bit timer

l 1246 rf_handler
# 1246 ****************************************************
# 1246 RF interrupt handler
# 1246 ****************************************************

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






