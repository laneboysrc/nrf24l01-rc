; nRF24LE1 SPI programmer hardware interface.  File: nrf_spi_if.asm
;
;
; Global Assembler directives
        list            c=132, n=80
        list            p=16F1825
        errorlevel      -302    ; Disable messages for register banking
        errorlevel      -306    ; Disable messages for page crossing

#include        p16f1825.inc

;
; Uncomment the #define USE_INT_CLOCK to use the internal oscillator instead of an
; external 8 MHz crystal.
;
#define	USE_INT_CLOCK	1

;
; This program provides the interface between software running on a PC and a target
; nRF24LE1 chip.  The PC communciates with this program via a serial interface and
; the program interfaces with the nRF24LE1 through the programming SPI port (and
; associated control signals).
;
; Serial communication occurs at 115.2 Kbaud (8N1).  The program interprets a simple ASCII-
; based command set with two command encoding forms (allowing use of a simple terminal).
; Each command generates a response.  Only one command may be issued at a time or bad
; things will happen.
;
;   Form 1: For use by programming software
;
;      'H'<HexCommand>[HexData]'T'<HexChecksum>
;
;     - The ASCII character 'H' is the packet start marker
;     - <HexCommand> is a two ASCII-character hex command
;     - HexData is the optional command data representing one or more bytes (up to 256)
;       comprised of two ASCII-character hex numbers for each data byte
;     - The ASCII character 'T' is the packet end marker.  It indicates this is Form 1
;       and requires a checksum byte to follow.
;     - <HexChecksum> is a two ASCII-character hex checksum (the SUM of all bytes 'H' to
;      'T' inclusive).
;     - No other characters should included in the packet for performance reasons.  The
;       firmware will ignore ASCII space characters and <CR> (carriage return) characters.
;
;   Form 2: For use via a terminal
;
;      'H' <HexCommand> [HexData][<CR>] <LF>
;
;     - The ASCII character 'H' is the packet start marker
;     - <HexCommand> is a two ASCII-character hex command
;     - HexData is the optional command data representing one or more bytes (up to 256)
;       comprised of two ASCII-character hex numbers for each data byte
;     - The ASCII character <LF> (linefeed - 0x0A - Ctrl-J) is the packet end marker.  It
;       indicates this is Form 2 and does not require a checksum to follow.
;     - ASCII space characters and <CR> (carriage return - 0x0D - Ctrl-L) characters are
;       ignored between 'H' and <LF>.
;
;   All hex values are expressed as two-character strings comprised of the ASCII characters
;   '0' - '9' and 'A' - 'F' or 'a' - 'f'.
;
;   All characters between the end of packet (or checksum for Form 1) and the next start of
;   packet character 'H' are ignored.
;
;   HexCommand values
;   -----------------
;
;    0x00 ("00"): Version. Firmware identifier and version.
;           - No data bytes
;           - Returns three bytes <FIRMWARE_TYPE><MAJOR VERSION><MINOR VERSION> with ACK
;
;    0x01 ("01"): Access Mode.  Controls entry and exit from program mode.  Entering access
;           mode causes the following actions to occur:
;             1. Assert the PROG signal to the nRF24LE1
;             2. Assert the nRESET signal to the nRF24LE1 for > 0.2 uSec
;             3. Enables the SPI interface to the nRF24LE1
;             4. Waits 1.5 mSec before returning a response
;           Exiting access mode causes the following actions to occur:
;             1. Disables (tri-states) the SPI interface to the nRF24LE1
;             2. De-asserts the PROG signal to the nRF24LE1
;
;           - One data byte: AccessMode
;              7:1: Reserved
;                0: Set Program Mode (1 = enter program mode, 0 = exit program mode)
;           - Returns zero response bytes with ACK
;
;    0x02 ("02"): SPI Command.
;           - 1 - 256 data bytes.  First data byte is the nRF24LE1 SPI Command.
;             Subsequent bytes are any required data for the command.
;           - Returns an equal number of response bytes, one for each data byte, with ACK.
;
;    0x03 ("03"): Set Address.  Set the starting address for subsequent Program or Read
;           Memory commands.
;           - 2 data bytes: <Address[15:8]><Address[7:0]>
;           - Returns zero response bytes with ACK
;
;    0x04 ("04"): Program. Program and verify a series of data bytes starting with the
;           current address.  Executes the following sequence of operations:
;             1. Execute the WREN SPI Command sequence.
;             2. Execute the PROGRAM SPI Command with data bytes
;             3. Poll FSR using RDSR SPI Command for write to finish
;             4. Verify data using the READ SPI Command
;             5. Update current address to point to next address location past data bytes
;           - 1-256 data bytes
;           - Returns one byte, Status, with ACK
;                0x00 "00": Success
;                0x01 "01": Verify failed
;
;    0x05 ("05"): Read Memory.  Read a series of data bytes starting with the current
;           address.
;           - 1 data byte indicating read length (256, 1-255)
;           - Returns 1-256 read data bytes with ACK
;           - Update current address to point to next address location past read length
;
;    0x06 ("06"): Reset.  Pulse the reset signal.  Designed to be used with the programmer
;           attached to a functioning device (not in program mode).
;           - No data bytes
;           - Returns zero response bytes with ACK
;
;
;    Responses
;    ---------
;
;    There are two different response forms.  The NACK and ACK response form operate
;    slightly differently depending if the original command was Form 1 or Form 2.
;
;       1. NACK response for commands that have failed.
;
;          'X'<HexNackCode>'T'<HexChecksum>                  (for Form 1)
;          'X' <HexNackCode><CR><LF>                         (for Form 2)
;
;          <HexNackCode> is one of the following strings:
;              "00": Unknown command
;              "01": Not in access mode
;              "02": Command packet checksum failure
;              "03": Malformed command packet
;              "04": Illegal number of arguments
;
;       2. ACK response for commands that have succeeded.
;
;          'R'[HexData]'T'<HexChecksum>                      (for Form 1)
;          'R' [HexData]<CR><LF>                             (for Form 2)
;
;             HexData bytes are separated by space characters for Form 2
;
;       <HexChecksum> is a two ASCII-character hex checksum (the SUM of all bytes ACK
;         or NACK code to 'T' inclusive).
;
;    Notes
;    -----
;      1. Access mode must be enabled before executing any commands that use the SPI
;         interface.  A NACK response is generated otherwise.
;      2. Configuring reads/programming writes to InfoPage must be configured through
;         a SPI Command to execute the WRSR SPI command.
;
; The SPI Interface runs at 2 MHz.
;
;
; Revision History
;   10/09/11 Revision 1.0  DJJ  Initial version.
;   02/02/12 Revision 2.0  DJJ  Revamped communication protocol to speed programming.
;   08/15/13 Revision 2.1  DJJ  Added WPU to RX input for circuit change when designing PCB.
;   08/18/13 Revision 2.2  DJJ  Added RESET command.  Added GPL V3 license.
;
;
; Copyright (c) 2011-2013 Dan Julio.  All rights reserved.
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
;
; ------------------------------------------------------------------------
; Processor Information:
;
; Target processor: PIC 16F1825
; Processor frequency: Internal 32 MHz (125 ns instruction cycle) from external 8 MHz crystal
;
; Memory Layout
;    Page 0: 0x0000 - 0x07FF    Main code, ISR and subroutines
;    Page 1: 0x0800 - 0x0FFF    Unused
;    Page 2: 0x1000 - 0x17FF    Unused
;    Page 3: 0x1800 - 0x1FFF    Unused
;
; Operating voltage : 3.3 volts
;
;   Pin     Signal                       Pol  I/O  default
;   --------------------------------------------------------
;   13 RA0  PROG (ICSP DATA)              H    O    0
;   12 RA1  nCSN (ICSP CLK)               L    O    1 (WPU when tri-stated)
;   11 RA2  nRESET                        L    O    1 WPU
;    4 RA3  MCLR / (VPP)                  -    -    - (external pull-up)
;    3 RA4  OSC2 / CLKOUT                 -    -    -
;    2 RA5  OSC1 / CLKIN                  -    -    -
;   10 RC0  SCLK                          H    O    0 (WPU when tri-stated)
;    9 RC1  SDI                           H    I    0 WPU
;    8 RC2  SDO                           H    O    0 (WPU when tri-stated)
;    7 RC3  nACTIVITY (LED indicator)     L    O    1
;    6 RC4  TX                            -    O    0
;    5 RC5  RX                            -    I    0 WPU
;
; PIC Timer Utilization
;   TIMER0 - not used
;   TIMER1 - PROG Sequence timer
;   TIMER2 - not used
;
; PIC Memory layout
;   Bank 0 - Operating Variables
;   Linear Addressed Memory
;      256 byte Command FIFO : 0x2100 - 0x21FF
;      256 byte Response FIFO : 0x2200 - 0x22FF
;   FSR utilization
;      FSR0 used by ISR and main code for accessing Command FIFO
;      FSR1 used by main code for accessing Response FIFO
;
;
; ------------------------------------------------------------------------
; File Layout:
;
;    1. Configuration Bits
;    2. Program Constants
;    3. Macro definitions
;    4. Main code
;    5. Main code subroutines
;
;
; ------------------------------------------------------------------------
; Set the device configuration bits
;  CONFIG1
;    FCMEN = 0 (Fail-safe clock monitor disabled)
;    IESO = 0 (Internal/External switchover mode disabled)
;    nCLKOUTEN = 1 (CLKOUT function disabled.  I/O function on I/O pin)
;    BOREN = 10 (Brown-out detection enabled during operation, disabled during sleep)
;    nCPD = 1 (Data memory code protection disabled)
;    nCP = 1 (Program memory code protection disabled)
;    MCLRE = 1 (MCLR/VPP pin is MCLR; digital IO disabled)
;    nPWRTE = 0 (Power-up timer enabled)
;    WDTE = 10 (Watchdog timer enabled while running, disabled while sleeping)
;    FOSC2:0 = 010 (HS oscillator)
;  CONFIG2
;    LVP = 0 (High-voltage on MCLR used for programming)
;    nDEBUG = 1 (ICSPCLK/ICSPDATA are general purpose IO)
;    BORV = 1 (Brown-out voltage = 1.9V)
;    STVREN = 0 (Stack overflow/underflow will not cause a reset)
;    PLLEN = 1 (4xPLL enabled)
;    WRT1:0 = 11 (Write protection off)
;
 ifdef USE_INT_CLOCK

        __config _CONFIG1, _FCMEN_OFF & _IESO_OFF & _CLKOUTEN_OFF & _BOREN_OFF & _CPD_OFF & _CP_OFF & _MCLRE_ON & _PWRTE_ON & _WDTE_NSLEEP & _FOSC_INTOSC

 else

        __config _CONFIG1, _FCMEN_OFF & _IESO_OFF & _CLKOUTEN_OFF & _BOREN_OFF & _CPD_OFF & _CP_OFF & _MCLRE_ON & _PWRTE_ON & _WDTE_NSLEEP & _FOSC_HS

 endif
        __config _CONFIG2, _LVP_OFF & _BORV_19 & _STVREN_OFF & _PLLEN_ON & _WRT_OFF


; ------------------------------------------------------------------------
; Program Constants
;

;
; External device bits (port number bits)
PROG		equ	.0
N_CSN		equ	.1
N_RESET		equ	.2
N_ACTIVITY	equ	.3
;
; CmdFlags variable bits (all active true)
CmdInProcess	equ	.0		; Set while processing a command
Form2Cmd	equ	.1		; Set with CmdInProcess when a Form 2 command is
					;   detected (clear for Form 1)
ChksumError	equ	.2		; Set with CmdInProcess when a checksum error is
					;   detected
BadCmdErr	equ	.3		; Set with CmdInProcess when a malformed command is
					;   detected (extraneous/illegal characters)
ReadInProcess	equ	.4		; Read Memory command in process
SpiEnabled	equ	.6		; Set when the SPI interface is enabled
IsHexChar	equ	.7		; Set by CheckIfHexChar subroutine for a legal
					;   hex character
;
;
; SifRxState variable bits (all active true and mutually exclusive)
RxIdleState	equ	.0		; RX decode idle
RxCmdStateH	equ	.1		; RX decode awaiting command byte
RxCmdStateL	equ	.2
RxDataStateH	equ	.3		; RX decode awaiting data bytes
RxDataStateL	equ	.4
RxChksumStateH	equ	.5		; RX decode awaiting checksum byte
RxChksumStateL	equ	.6
;
RX_IDLE_STATE	equ	0x01
RX_CMD_STATE_H	equ	0x02
RX_CMD_STATE_L	equ	0x04
RX_DATA_STATE_H	equ	0x08
RX_DATA_STATE_L	equ	0x10
RX_CHK_STATE_H	equ	0x20
RX_CHK_STATE_L	equ	0x40
;
;
; SifTxState variable bits (all active true and mutually exclusive; idle is no bits set)
TxAckState	equ	.0		; TX send ACK start-of-packet character
TxNackState	equ	.1		; TX send NACK start-of-packet character
TxData1State	equ	.2		; TX send <Space>
TxData2State	equ	.3		; TX send High Hex ASCII Data Byte
TxData3State	equ	.4		; TX send Low Hex ASCII Data Byte
TxTerm1State	equ	.5		; TX send 'T' or <CR>
TxTerm2State	equ	.6		; TX send High Checksum HEX ASCII Data Byte or <LF>
TxTerm3State	equ	.7		; TX send Low Checksum HEX ASCII Data Byte
;
TX_IDLE_STATE	equ	0x00
TX_ACK_STATE	equ	0x01
TX_NACK_STATE	equ	0x02
TX_DATA_1_STATE	equ	0x04
TX_DATA_2_STATE	equ	0x08
TX_DATA_3_STATE	equ	0x10
TX_TERM_1_STATE	equ	0x20
TX_TERM_2_STATE	equ	0x40
TX_TERM_3_STATE	equ	0x80
;
; Bank 0 Variable register equates
;
NumBytes	equ	0x20		; Loop counter for access routines
CmdByte		equ	0x21		; HexCommand byte
CmdCount	equ	0x22		; # of valid bytes in CMD FIFO (0-255 -> 256,1-255)
CmdAddrHi	equ	0x23
CmdAddrLo	equ	0x24
SifRxState	equ	0x25		; RX State used by ISR
SifRxChar	equ	0x26		; Incoming character from serial interface
					;   Commands with no data ignore this variable
SifTempData	equ	0x27		; Temporary byte used to build hex data bytes
SifRxChecksum	equ	0x28		; Sum of incoming bytes
SifCmdPushPtr	equ	0x29		; External command push pointer, set when a complete
					;   command has been pushed into the Cmd Fifo
SifCmdPopPtr	equ	0x2A		; Command pop pointer
;
; Registers located in common region for access from any bank
;
TempReg		equ	0x70
CmdFlags	equ	0x71
SifTxState	equ	0x72		; TX State used by response mechanism
SifTxChecksum	equ	0x73		; Sum of outgoing bytes
RspCount	equ	0x74		; # of valid bytes in RSP FIFO (0-255 -> 256,1-255)
SifRspPushPtr	equ	0x75		; Response push pointer
SifRspPopPtr	equ	0x76		; Response pop pointer

;
; High address bytes for the fifos: loaded into FSRH (specific pointer is loaded
; into FSRL)
CMD_FIFO_HI	equ	0x21
RSP_FIFO_HI	equ	0x22

;
; PIC Initialization constants
OSCCON_SETUP	equ	B'11110000'	; 7: SPLLEN = 1 (4x PLL enabled)
					; 6:3: IRCF3:0 = 1110 (8 MHz for 32 MHz operation)
					; 2: Unimplemented
					; 1:0: SCS1:0 = 00 (Use Config Word 1 FOSC bits)

OPTION_SETUP	equ	B'00000111'	; 7: WPUEN = 0 (enable pull-ups)
					; 6: INTEDG = 0 (Int on falling edge RB0/INT)
					; 5: T0CS = 0 (internal TMR0 clock)
					; 4: T0SE = 0 (rising TMR0 clock)
					; 3: PSA = 0 (prescaler to TMR0)
					; 2:0: prescaler = 111b (1:256) for every 32 uSec

INTCON_SETUP	equ	B'11000000'	; 7: GIE = 1 (interrupts enabled)
					; 6: PEIE = 1 (Peripheral ints enabled)
					; 5: T0IE = 0 (TMR0 ints disabled)
					; 4: INTE = 0 (INT ints disabled)
					; 3: IOCIE = 0 (Interrupt-on-change disabled)
					; 2: T0IF = 0
					; 1: INTF = 0
					; 0: RBIF = 0

T1CON_INIT	equ	B'00110001'	; 7:6: TMR1CS1:0 = 00 (Timer1 clock = Fosc/4)
					; 5:4: T1CKPS1:0 = 11 (1:8 prescale)
					; 3: T1OSCEN = 0 (Oscillator shut off)
					; 2: T1SYNC = 0 (Sync control unused)
					; 1: Unimplemented
					; 0: TMR1ON = 1 (Enable timer)
					; (Increments every 1 uSec)

T1GCON_INIT	equ	B'00000000'	; 7: TMR1GE = 0 (Timer1 always counts)
					; 6: T1GPOL = 0
					; 5: T1GTM = 0
					; 4: T1GSPM = 0
					; 3: T1GGO/DONE = 0
					; 2: T1GVAL = 0
					; 1:0: T1GSS1:0 = 00

PIE1_SETUP	equ	B'00100000'	; 7: TMR1GIE = 0 (Timer 1 Gate Enable)
					; 6: ADIE = 0 (A/D converter)
					; 5: RCIE = 1 (USART RX Enable)
					; 4: TXIE = 0 (USART TX Enable)
					; 3: SSPIE = 0 (Sync Serial Port)
					; 2: CCP1IE = 0 (CCPQ Int enable)
					; 1: TMR2IE = 0 (TMR2 to PR2 match)
					; 0: TMR1IE = 0 (TMR1 Overflow)

PORTA_INIT	equ	B'00000110'
PORTC_INIT	equ	B'00001000'
PORTA_OE_EN	equ	B'00000000'
PORTC_OE_EN	equ	B'00100010'
PORTA_OE_DIS	equ	B'00000010'	; Tri-state SPI IF when disabled
PORTC_OE_DIS	equ	B'00100111'

ANSELA_INIT	equ	B'00000000'	; All PORT bits are digital I/O
ANSELC_INIT	equ	B'00000000'

WPUA_INIT_EN	equ	B'00000100'	; Pull-up's when enabled
WPUC_INIT_EN	equ	B'00100010'
WPUA_INIT_DIS	equ	B'00000110'	; Pull-up's when disabled
WPUC_INIT_DIS	equ	B'00100111'

INLVLA_INIT	equ	B'00000000'	; TTL levels on all pins
INLVLC_INIT	equ	B'00000000'

TXSTA_INIT	equ	B'00100100'	; 7: CSRC = 0 (Clock Source Select)
					; 6: TX9 = 0 (9-bit transmit enable)
					; 5: TXEN = 1 (Transmit enable)
					; 4: SYNC = 0 (Mode select bit)
					; 3: SENDB = 0 (Send Break Character bit)
					; 2: BRGH = 1 (High Baud Rate Select)
					; 1: TRMT = 0 (Transmit Shift Register Status)
					; 0: TX9D = 0 (Ninth bit of transmit data)

RCSTA_INIT	equ	B'10010000'	; 7: SPEN = 1 (Serial Port Enable)
					; 6: RX9 = 0 (9-bit Receive Enable)
					; 5: SREN = 0 (Single Receive Enable)
					; 4: CREN = 1 (Continuous Receive Enable)
					; 3: ADDEN = 0 (Address Detect Enable)
					; 2: FERR = 0 (Framing Error)
					; 1: OERR = 0 (Overrun Error)
					; 0: RX9D = 0 (Ninth bit of Received Data)

BAUDCON_INIT	equ	B'00001000'	; 7: ABDOVF = 0 (Auto_baud Detect Overflow)
					; 6: RCIDL = 0 (Receive Idle Flag)
					; 5: Unimplemented
					; 4: SCKP = 0 (Sync Clock Polarity)
					; 3: BRG16 = 1 (16-bit baud rate generator)
					; 2: Unimplemented
					; 1: WUE = 0 (Wake-up Enable)
					; 0: ABDEN = 0 (Auto-Baud Detect mode)

SPBRG_INIT	equ	.68		; Async 115200 baud w/ BRG16=1, BRGH=1

SSP1CON1_INIT	equ	B'00000001'	; 7: WCOL = 0 (Write Collision)
					; 6: SSPOV = 0 (Recieve Overflow indicator)
					; 5: SSPEN = 0 (Synchronous Serial Port Disable)
					; 4: CKP = 0 (Clock Polarity bit)
					; 3:0: SSPM = 0001 (Synchronous Serial Port Mode)
					;      (Fosc/16 = 2 Mhz)

SSP1CON2_INIT	equ	B'00000000'	; 7: GCEN = 0 (General Call Enable)
					; 6: ACKSTAT = 0 (Acknowledge Status bit)
					; 5: ACKDT = 0 (Acknowledge Data bit)
					; 4: ACKEN = 0 (Acknowledge Sequence Enable bit)
					; 3: RCEN = 0 (Receive Enable bit)
					; 2: PEN = 0 (Stop Condition enable bit)
					; 1: RSEN = 0 (Repeated Start Condition Enabled bit)
					; 0: SEN = 0 (Start Condition enable bit)

SSP1CON3_INIT	equ	B'00000000'	; 7: ACKTIM = 0 (Acknowledge Time status bit)
					; 6: PCIE = 0 (Stop Condition Interrupt Enable)
					; 5: SCIE = 0 (Start Condition Interrupt Enable)
					; 4: BOEN = 0 (Buffer Overwrite Enable bit)
					; 3: SDAHT = 0 (SDAx HOlde Time Selection)
					; 2: SBCDE = 0 (Slave Mode Bus Collision Detect Enable)
					; 1: AHEN = 0 (Address Hold Enable)
					; 0: DHEN = 0 (Data Hold Enable)

SSP1STAT_INIT	equ	B'01000000'	; 7: SMP = 0 (SPI Data Input Sample)
					; 6: CKE = 1 (SPI Clock Edge Select)
					; 5: D/A = 0 (Data/nAddress)
					; 4: P = 0 (Stop bit)
					; 3: S = 0 (Start bit)
					; 2: R/W = 0 (Read/nWrite)
					; 1: UA = 0 (Update Address)
					; 0: BF = 0 (Buffer Full Status)

;
; PROG assertion timer setup
;
PROG_TIMEOUT	equ	0xF9F2		; 1550 uSec (0x10000 - 1550)

;
; Command characters
;
START_CMD_CHAR	equ	'H'
STOP_CMD_CHAR	equ	'T'
ACK_CHAR	equ	'R'
NACK_CHAR	equ	'X'
LF_CHAR		equ	0x0A
CR_CHAR		equ	0x0D
SPACE_CHAR	equ	' '
;
; Command bytes (should be contiguous starting with 0x00)
;
CMD_VERSION	equ	0x00
CMD_ACCESS_MODE	equ	0x01
CMD_SPI		equ	0x02
CMD_SET_ADDR	equ	0x03
CMD_PROG	equ	0x04
CMD_READ	equ	0x05
CMD_RESET	equ	0x06
NUM_CMD_BYTES	equ	.7
;
; Program Command response bytes
;
PROG_SUCCESS	equ	0x00
PROG_FAILED	equ	0x01
;
; NACK Response codes
;
NACK_UNKNOWN	equ	0x00
NACK_NOT_ACCESS	equ	0x01
NACK_CHECKSUM	equ	0x02
NACK_MALFORMED	equ	0x03
NACK_NUM_ARGS	equ	0x04
;
; Current version information
;
FIRMWARE_TYPE	equ	0x01
VER_MAJOR	equ	0x02
VER_MINOR	equ	0x02
;
; nRF24LE1 SPI command encodings
;
NRF_WREN	equ	0x06
NRF_WRDIS	equ	0x04
NRF_RDSR	equ	0x05
NRF_WRSR	equ	0x01
NRF_READ	equ	0x03
NRF_PROG	equ	0x02
NRF_ERASE_PG	equ	0x52
NRF_ERASE_ALL	equ	0x62
NRF_RDFPCR	equ	0x89
NRF_RDISMB	equ	0x85
NRF_ENDEBUG	equ	0x86



; ------------------------------------------------------------------------
; Macro Definitions
;

ASSERT_PROG	macro
		bsf	PORTA,PROG
		endm

DEASSERT_PROG	macro
		bcf	PORTA,PROG
		endm

ASSERT_RESET	macro
		bcf	PORTA,N_RESET
		endm

DEASSERT_RESET	macro
		bsf	PORTA,N_RESET
		endm

ASSERT_CSN	macro
		bcf	PORTA,N_CSN
		endm

DEASSERT_CSN	macro
		bsf	PORTA,N_CSN
		endm

SET_ACTIVITY	macro
		bcf	PORTC,N_ACTIVITY
		endm

CLR_ACTIVITY	macro
		bsf	PORTC,N_ACTIVITY
		endm

SET_RX_STATE	macro	value
		movlw	value
		movwf	SifRxState
		endm

SET_TX_STATE	macro	value
		movlw	value
		movwf	SifTxState
		endm



; ------------------------------------------------------------------------
; Code Block - Vector table
;

	; Reset Vector
	org		0x0
	goto		MAIN_VECTOR

; ------------------------------------------------------------------------
; Interrupt Service Routine
;

	; Interrupt Vector
	org		0x4

	; Interrupt Service Routine for RX Serial Data
	call		ProcessRxChar
	retfie


; ------------------------------------------------------------------------
; Main Code Block
;
MAIN_VECTOR
	call		InitSystem

MAIN_LOOP
	clrwdt

	; Look for a command to process
	btfsc		CmdFlags,CmdInProcess
	call		EvalCommand

	; Look for data to send from the RSP FIFO
	movf		SifTxState,W
	btfss		STATUS,Z
	call		ProcessTxData
	goto		MAIN_LOOP



; ------------------------------------------------------------------------
; Subroutines
;

; InitSystem: Power-on initialization of the processor, peripherals and variables
;
; On entry: none
;
; On exit: System ready for operation
;
InitSystem
	; Initialize the processor IO first (at slow clock)
	BANKSEL		PORTA
	movlw		PORTA_INIT
	movwf		PORTA
	movlw		PORTC_INIT
	movwf		PORTC

	BANKSEL		TRISA
	movlw		PORTA_OE_DIS
	movwf		TRISA
	movlw		PORTC_OE_DIS
	movwf		TRISC

	BANKSEL		ANSELA
	movlw		ANSELA_INIT
	movwf		ANSELA
	movlw		ANSELC_INIT
	movwf		ANSELC

	BANKSEL		WPUA
	movlw		WPUA_INIT_DIS
	movwf		WPUA
	movlw		WPUC_INIT_DIS
	movwf		WPUC

	BANKSEL		INLVLA
	movlw		INLVLA_INIT
	movwf		INLVLA
	movlw		INLVLC_INIT
	movwf		INLVLC

 ifdef USE_INT_CLOCK
	; Initialize the processor internal clock
	BANKSEL		OSCCON
	movlw		OSCCON_SETUP
	movwf		OSCCON
	; Spin waiting for the High Frequency Internal Oscillator to report ready
	btfss		OSCSTAT,HFIOFR
	goto		$-1
 endif

	; Processor core init
	BANKSEL		OPTION_REG
	movlw		OPTION_SETUP
	movwf		OPTION_REG
	movlw		PIE1_SETUP
	movwf		PIE1

	; Initialize TIMER1
	BANKSEL		T1CON
	movlw		T1CON_INIT
	movwf		T1CON
	movlw		T1GCON_INIT
	movwf		T1GCON

	; Initialize the serial port
	BANKSEL		TXSTA
	movlw		TXSTA_INIT
	movwf		TXSTA
	movlw		RCSTA_INIT
	movwf		RCSTA
	movlw		BAUDCON_INIT
	movwf		BAUDCON
	movlw		(SPBRG_INIT >> 8)
	movwf		SPBRGH
	movlw		(SPBRG_INIT & 0xFF)
	movwf		SPBRGL

	; Initialize SPI
	BANKSEL		SSP1CON1
	movlw		SSP1CON1_INIT
	movwf		SSP1CON1
	movlw		SSP1CON2_INIT
	movwf		SSP1CON2
	movlw		SSP1CON3_INIT
	movwf		SSP1CON3
	movlw		SSP1STAT_INIT
	movwf		SSP1STAT

	; Initialize pointers
	movlw		CMD_FIFO_HI
	movwf		FSR0H
	movlw		RSP_FIFO_HI
	movwf		FSR1H

	; Initialize variables
	BANKSEL		0
	clrf		CmdFlags
	clrf		CmdCount
	clrf		CmdAddrHi
	clrf		CmdAddrLo
	SET_RX_STATE	RX_IDLE_STATE
	clrf		SifCmdPushPtr
	clrf		SifCmdPopPtr
	SET_TX_STATE	TX_IDLE_STATE
	clrf		RspCount
	clrf		SifRspPushPtr
	clrf		SifRspPopPtr

	; Finally, enable interrupts
	movlw		INTCON_SETUP
	movwf		INTCON
	return


; EvalCommand: Process or continue processing the current command from the Command FIFO.
;
; On entry: CmdFlags.CmdInProcess set
;           CmdFlags.{Form2Cmd, ChksumError, BadCmdErr, ReadInProcess, SpiEnabled}
;             contain additional command information
;           CmdByte contains the current command
;           CmdCount contains the number of argument bytes in the Command FIFO
;
; On exit: Command processed (output signals may be modified by the command)
;          SifCmdPopPtr incremented
;          CmdFlags.CmdInProcess may be clear
;          CmdFlags.SpiEnabled may be set or cleared
;
EvalCommand
	; Look for indication of an error associated with the command
	btfsc		CmdFlags,ChksumError
	goto		EVAL_CMD_CHECKSUM_ERR
	btfsc		CmdFlags,BadCmdErr
	goto		EVAL_CMD_MALFORMED
	; Jump to command handler
	movlw		NUM_CMD_BYTES
	subwf		CmdByte,W		; W = CmdByte - NUM_CMD_BYTES
	btfsc		STATUS,C		; C = 0 if CmdByte < NUM_CMD_BYTES
	goto		EVAL_CMD_ILL_CMD	;   C=1: Unknown command
	movf		CmdByte,W		;   C=0: Legal command
	brw
	goto		EVAL_CMD_VERSION
	goto		EVAL_CMD_ACCESS_MODE
	goto		EVAL_CMD_SPI
	goto		EVAL_CMD_SET_ADDR
	goto		EVAL_CMD_PROG
	goto		EVAL_CMD_READ
	goto		EVAL_CMD_RESET


; --------------
; Version Command
;
EVAL_CMD_VERSION
	; Validate the argument count
	movf		CmdCount,W
	btfss		STATUS,Z
	goto		EVAL_CMD_ILL_ARGS

	; Load the response
	movlw		FIRMWARE_TYPE
	call		PushRspFifo
	movlw		VER_MAJOR
	call		PushRspFifo
	movlw		VER_MINOR
	call		PushRspFifo
	goto		EVAL_CMD_SUCCESS


; --------------
; Access Mode Command
;
EVAL_CMD_ACCESS_MODE
	; Validate the argument count
	movlw		.1
	subwf		CmdCount,W
	btfss		STATUS,Z
	goto		EVAL_CMD_ILL_ARGS

	; Get the argument byte
	call		PopCmdFifo
	movwf		TempReg
	btfss		TempReg,.0		; Bit 0 set?
	goto		EVAL_CMD_ACCESS_DIS	;   N: Disable Access mode
						;   Y: Enable Access mode
	; ------------- HACK ----------------
	; Assert PROG: A single bsf of PORTA does not work here, but two do.  Why????
	ASSERT_PROG
	ASSERT_PROG
	; -----------END HACK ---------------

	; Reset the device
	call		AssertReset

	; Enable the SPI Interface
	call		EnableSpi

	; Configure the timeout
	bcf		T1CON,TMR1ON
	movlw		(PROG_TIMEOUT >> 8)
	movwf		TMR1H
	movlw		(PROG_TIMEOUT & 0xFF)
	movwf		TMR1L
	bcf		PIR1,TMR1IF
	bsf		T1CON,TMR1ON

	; Spin waiting for the timeout
	clrwdt
	btfss		PIR1,TMR1IF		; Timer1 expired?
	goto		$-2			;   N: Spin
						;   Y: Command done
	bcf		T1CON,TMR1ON
	goto		EVAL_CMD_SUCCESS


EVAL_CMD_ACCESS_DIS
	; Disable the SPI Interface
	call		DisableSpi

	; De-assert PROG
	DEASSERT_PROG
	goto		EVAL_CMD_SUCCESS


; --------------
; SPI Command
;
EVAL_CMD_SPI
	; Make sure the SPI interface is enabled
	btfss		CmdFlags,SpiEnabled
	goto		EVAL_CMD_NOT_ACCESS

	; Indicate a command is in progress
	SET_ACTIVITY
	ASSERT_CSN

EVAL_CMD_SPI_LOOP
	; Peform a SPI operation
	call		PopCmdFifo
	call		DoSpiWrite
	call		PushRspFifo

	; Evaluate loop control
	decfsz		CmdCount,F
	goto		EVAL_CMD_SPI_LOOP

	; Clear indication that command is in progress
	CLR_ACTIVITY
	DEASSERT_CSN
	goto		EVAL_CMD_SUCCESS


; --------------
; Set Address Command
;
EVAL_CMD_SET_ADDR
	; Validate the argument count
	movlw		.2
	subwf		CmdCount,W
	btfss		STATUS,Z
	goto		EVAL_CMD_ILL_ARGS

	; Unpack the address
	call		PopCmdFifo
	movwf		CmdAddrHi
	call		PopCmdFifo
	movwf		CmdAddrLo
	goto		EVAL_CMD_SUCCESS


; --------------
; Program Command
;
EVAL_CMD_PROG
	; Make sure the SPI interface is enabled
	btfss		CmdFlags,SpiEnabled
	goto		EVAL_CMD_NOT_ACCESS

	; Execute the program sequence
	SET_ACTIVITY				; Indicate a command is in progress
	movf		CmdCount,W
	movwf		NumBytes
	call		WriteEnableDevice
	call		ProgramDevice
	call		PollProgramCompletion

	; Verify the data, breaking on the first mismatch
	movf		CmdCount,W
	movwf		NumBytes
	clrf		SifCmdPopPtr		; Reset pointer
	; Start the SPI Read operation
	ASSERT_CSN
	movlw		NRF_READ		; Command
	call		DoSpiWrite
	movf		CmdAddrHi,W		; Address
	call		DoSpiWrite
	movf		CmdAddrLo,W
	call		DoSpiWrite
EVAL_CMD_PROG_READ_LOOP
	call		PopCmdFifo		; Get the original data
	movwf		TempReg
	call		DoSpiWrite		; Get data from the device to verify
	subwf		TempReg,W		; W = Original Data - Verify Data
	btfss		STATUS,Z		; W = 0?
	goto		EVAL_CMD_PROG_FAIL	;   N: Verification failure
	decfsz		NumBytes,F		; Loop counter
	goto		EVAL_CMD_PROG_READ_LOOP

	; Programming successful: Terminate the read
	CLR_ACTIVITY
	DEASSERT_CSN
	; Update the address to point to the first byte past the programmed
	; region for a successful programming
	movf		CmdCount,W
	movwf		NumBytes
	call		UpdateAddr
	; Load success status
	movlw		PROG_SUCCESS
	call		PushRspFifo
	goto		EVAL_CMD_SUCCESS

EVAL_CMD_PROG_FAIL
	; Terminate the read
	CLR_ACTIVITY
	DEASSERT_CSN
	; Load failure status
	movlw		PROG_FAILED
	call		PushRspFifo
	goto		EVAL_CMD_SUCCESS



; --------------
; Read Memory Command
;  Note: The Read Memory Command operates in an interleaved fashion (sending data back
;        through the serial interface while it is still reading the device).  It reads
;        one byte of data for each call to EvalCommand and assumes that SPI reads operate
;        faster than sending data on the serial interface so that this process is always
;        ahead of the ProcessTxData process.
;
EVAL_CMD_READ
	; Make sure the SPI interface is enabled
	btfss		CmdFlags,SpiEnabled
	goto		EVAL_CMD_NOT_ACCESS

	; Determine if we are just starting this command or it is ongoing
	btfsc		CmdFlags,ReadInProcess
	goto		EVAL_CMD_READ_CONT
	; Start read command
	;
	; Validate the argument count
	movlw		.1
	subwf		CmdCount,W
	btfss		STATUS,Z
	goto		EVAL_CMD_ILL_ARGS

	; Get number of bytes to read
	bsf		CmdFlags,ReadInProcess
	call		PopCmdFifo
	movwf		NumBytes

	; Indicate a command is in progress
	SET_ACTIVITY

	; Start the SPI Read operation
	;   nRF24LE1 READ: 0x03 <addr_hi> <addr_lo> [byte0] [byte1] ... [byteN]
	ASSERT_CSN
	movlw		NRF_READ
	call		DoSpiWrite
	movf		CmdAddrHi,W
	call		DoSpiWrite
	movf		CmdAddrLo,W
	call		DoSpiWrite

	; Update the address to point one byte beyond this range (for any subsequent
	; reads)
	call		UpdateAddr

	; Get the first data byte
	clrw
	call		DoSpiWrite
	call		PushRspFifo
	decf		NumBytes,F

	; Trigger the response machine by indiating we have response data now
	SET_TX_STATE	TX_ACK_STATE
	goto		EVAL_CMD_RET

EVAL_CMD_READ_CONT
	; Check to see if we are done
	movf		NumBytes,W
	btfsc		STATUS,Z
	goto		EVAL_CMD_READ_DONE

	; Get the next data byte
	clrw
	call		DoSpiWrite
	call		PushRspFifo
	decf		NumBytes,F
	goto		EVAL_CMD_RET

EVAL_CMD_READ_DONE
	CLR_ACTIVITY
	DEASSERT_CSN
	bcf		CmdFlags,ReadInProcess
	goto		EVAL_CMD_DONE


; --------------
; Reset Command
;
EVAL_CMD_RESET
	; Validate the argument count
	movf		CmdCount,W
	btfss		STATUS,Z
	goto		EVAL_CMD_ILL_ARGS

	; Reset the device
	call		AssertReset

	goto		EVAL_CMD_SUCCESS


; --------------
; Checksum error with command
;
EVAL_CMD_CHECKSUM_ERR
	SET_TX_STATE	TX_NACK_STATE
	movlw		NACK_CHECKSUM
	call		PushRspFifo
	goto		EVAL_CMD_DONE


; --------------
; Malformed command
;
EVAL_CMD_MALFORMED
	SET_TX_STATE	TX_NACK_STATE
	movlw		NACK_MALFORMED
	call		PushRspFifo
	goto		EVAL_CMD_DONE


; --------------
; Unknown command
;
EVAL_CMD_ILL_CMD
	SET_TX_STATE	TX_NACK_STATE
	movlw		NACK_UNKNOWN
	call		PushRspFifo
	goto		EVAL_CMD_DONE


; --------------
; Not in access mode
;
EVAL_CMD_NOT_ACCESS
	SET_TX_STATE	TX_NACK_STATE
	movlw		NACK_NOT_ACCESS
	call		PushRspFifo
	goto		EVAL_CMD_DONE


; --------------
; Illegal number of arguments
;
EVAL_CMD_ILL_ARGS
	SET_TX_STATE	TX_NACK_STATE
	movlw		NACK_NUM_ARGS
	call		PushRspFifo
	goto		EVAL_CMD_DONE


; --------------
; Entry point for command completions generating an ACK
;
EVAL_CMD_SUCCESS
	SET_TX_STATE	TX_ACK_STATE


; --------------
; Note command done
;
EVAL_CMD_DONE
	bcf		CmdFlags,CmdInProcess
	; Reset pointers for the next command
	clrf		SifCmdPopPtr
	clrf		SifRspPushPtr

EVAL_CMD_RET
	return


; PopCmdFifo: Pop a byte from the Command FIFO.
;
; On entry: SifCmdPopPtr points to the current location to read
;           FSR0H points to the CMD FIFO page
;
; On exit: W contains the data from the Command FIFO
;          SifCmdPopPtr incremented
;
PopCmdFifo
	movf		SifCmdPopPtr,W
	movwf		FSR0L
	incf		SifCmdPopPtr,F
	movf		INDF0,W
	return


; PushRspFifo: Push a byte into the Response FIFO.
;
; On entry: W contains the value to push
;           SifRspPushPtr points to the current location to load
;           RspCount contains the number of bytes in the Response FIFO
;           FSR1H points to the RSP FIFO page
;
; On exit: W loaded into the Response FIFO
;          SifRspPushPtr incremented
;          RspCount incremented
;
PushRspFifo
	movwf		TempReg
	movf		SifRspPushPtr,W
	movwf		FSR1L
	movf		TempReg,W
	movwf		INDF1
	incf		SifRspPushPtr,F
	incf		RspCount,F
	return


; AssertReset: Assert the reset signal and then de-assert it
;
; On entry: none
;
; On exit: none
;
AssertReset
	; Assert RESET For > 0.2 uSec (We do 8 instructions for 1.0 uSec)
	ASSERT_RESET
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	DEASSERT_RESET


; UpdateAddr: Update the current address to the next address for the calling operation
;
; On entry: NumBytes contains the number of bytes the operation spans
;              0 = 256
;           CmdAddr{Hi,Lo} contain the current address
;
; On exit: CmdAddr{Hi,Lo} incremented by NumBytes (incremented by 256 if NumBytes = 0)
;
UpdateAddr
	movf		NumBytes,W
	btfsc		STATUS,Z		; NumBytes = 0?
	goto		UPDATE_ADDR_HIGH	;   Y: Just bump the high half
	addwf		CmdAddrLo,F		;   N: Add to low half
	btfsc		STATUS,C		;      And check for carry out
UPDATE_ADDR_HIGH
	incf		CmdAddrHi,F
	return


; EnableSpi: Enable the SPI interface and associated IO ports.
;
; On entry: none
;
; On exit: SPI enabled
;          CmdFlags.SpiEnabled set
;
EnableSpi
	BANKSEL		SSP1CON1
	bsf		SSP1CON1,SSPEN
	BANKSEL		TRISA
	movlw		PORTA_OE_EN
	movwf		TRISA
	movlw		PORTC_OE_EN
	movwf		TRISC
	BANKSEL		WPUA
	movlw		WPUA_INIT_EN
	movwf		WPUA
	movlw		WPUC_INIT_EN
	movwf		WPUC
	BANKSEL		0
	bsf		CmdFlags,SpiEnabled
	return


; DisableSpi: Disable the SPI interface and associated IO ports.
;
; On entry: none
;
; On exit: SPI disabled
;          CmdFlags.SpiEnabled clear
;
DisableSpi
	BANKSEL		SSP1CON1
	bcf		SSP1CON1,SSPEN
	BANKSEL		TRISA
	movlw		PORTA_OE_DIS
	movwf		TRISA
	movlw		PORTC_OE_DIS
	movwf		TRISC
	BANKSEL		WPUA
	movlw		WPUA_INIT_DIS
	movwf		WPUA
	movlw		WPUC_INIT_DIS
	movwf		WPUC
	BANKSEL		0
	bcf		CmdFlags,SpiEnabled
	return


; DoSpiWrite: Perform a single SPI operation
;
; On entry: W contains the byte to write to the SPI device
;
; On exit: W contains the received data byte
;
DoSpiWrite
	; Trigger the SPI operation and wait until it is done
	BANKSEL		SSP1BUF
	movwf		SSP1BUF

	; Poll for completion
	BANKSEL		PIR1
	btfss		PIR1,SSP1IF
	goto		$-1
	bcf		PIR1,SSP1IF

	; Store the received data in the RSP FIFO
	BANKSEL		SSP1BUF
	movf		SSP1BUF,W
	BANKSEL		0
	return


; WriteEnableDevice: Execute the WREN SPI Command sequence to enable the device
; for programming.
;
; On entry: PROG asserted to the device
;
; On exit: WREN SPI sequence completed
;
WriteEnableDevice
	ASSERT_CSN
	movlw		NRF_WREN
	call		DoSpiWrite
	DEASSERT_CSN
	return


; ProgramDevice: Program up to 256 bytes
;
; On entry: SifCmdPopPtr points to the beginning of the CMD FIFO
;           NumBytes contains the number of valid bytes (0=256 bytes)
;           CmdAddr{Hi,Lo} contain the starting address in the Device to program
;           Device has been enabled to be programmed
;
; On exit: Device programmed
;          SifCmdPopPtr points one past the last valid byte in the CMD FIFO
;          NumBytes = 0
;
ProgramDevice
	ASSERT_CSN
	movlw		NRF_PROG		; Command
	call		DoSpiWrite
	movf		CmdAddrHi,W		; Address
	call		DoSpiWrite
	movf		CmdAddrLo,W
	call		DoSpiWrite
PROG_DEV_LOOP
	call		PopCmdFifo
	call		DoSpiWrite
	decfsz		NumBytes,F
	goto		PROG_DEV_LOOP
	DEASSERT_CSN
	return


; PollProgramCompletion: Poll the Device FSR register using the SPI RDSR command until the
; FSR.WEN bit is clear (Bit 5).
;
; On entry: none
;
; On exit: FSR.WEN clear
;
PollProgramCompletion
POLL_PROG_LOOP
	clrwdt
	; Read the FSR register
	ASSERT_CSN
	movlw		NRF_RDSR
	call		DoSpiWrite		; Load Command
	call		DoSpiWrite		; Get read data
	andlw		(1 << .5)		; AND with WEN bit
	btfsc		STATUS,Z		; WEN = 0?
	goto		POLL_PROG_DONE		;   Y: Done polling
	DEASSERT_CSN				;   N: Read again
	goto		POLL_PROG_LOOP

POLL_PROG_DONE
	DEASSERT_CSN
	return


; ProcessTxData: Load one ASCII character from the Response FIFO to the serial port to
; transmit back to the host.
;
; On entry: RSP FIFO has a data to send
;           SifTxState contains the current SIF TX state
;           SifRspPopPtr points to the next data byte to send
;           RspCount contains the number of valid data bytes in the Response FIFO
;           SifTxChecksum contains the current response packet checksum
;           CmdFlags.Form2Cmd indicates the command form
;           FSR1H points to the RSP FIFO page
;
; On exit: EUSART TXREG may be loaded
;          SifTxState may be updated to a new state
;          SifRspPopPtr may be incremented
;          RspCount may be decremented
;          SifTxChecksum may be updated
;
ProcessTxData
	; Make sure the EUSART TXREG is available
	BANKSEL		TXSTA
	btfss		TXSTA,TRMT			; EUSART TX buffer empty?
	goto		PROC_TX_RET			;   N: Skip looking for new data to TX

	; Select a character to send based on the current TX state
	btfsc		SifTxState,TxAckState
	goto		PROC_TX_ACK
	btfsc		SifTxState,TxNackState
	goto		PROC_TX_NACK
	btfsc		SifTxState,TxData1State
	goto		PROC_TX_DATA_1
	btfsc		SifTxState,TxData2State
	goto		PROC_TX_DATA_2
	btfsc		SifTxState,TxData3State
	goto		PROC_TX_DATA_3
	btfsc		SifTxState,TxTerm1State
	goto		PROC_TX_TERM_1
	btfsc		SifTxState,TxTerm2State
	goto		PROC_TX_TERM_2
	btfsc		SifTxState,TxTerm3State
	goto		PROC_TX_TERM_3
	goto		PROC_TX_RET		; Do nothing for idle

; --------------
; TxAckState
;
PROC_TX_ACK
	; Load the ACK character
	movlw		'R'
	movwf		TXREG
	; Initialize TX checksum
	movwf		SifTxChecksum
	goto		PROC_TX_CHK_FOR_DATA	; Check if there is any data to send


; --------------
; TxNackState
;
PROC_TX_NACK
	; Load the NACK character
	movlw		'X'
	movwf		TXREG
	; Initialize TX checksum
	movwf		SifTxChecksum

PROC_TX_CHK_FOR_DATA
	; See if there is data to send and we should move to a data state
	movf		RspCount,W
	btfsc		STATUS,Z		; RspCount = 0?
	goto		PROC_TX_SETUP_TERM	;   Y: No data, just send term
						;   N: Setup to send data
	; We send space characters for Form 2 responses
	btfss		CmdFlags,Form2Cmd
	goto		PROC_TX_SETUP_DATA_1
	SET_TX_STATE	TX_DATA_1_STATE		; Setup for data with spaces
	goto		PROC_TX_RET

PROC_TX_SETUP_DATA_1
	; Setup for data without spaces
	SET_TX_STATE	TX_DATA_2_STATE
	goto		PROC_TX_RET

PROC_TX_SETUP_TERM
	; Always reset the pop pointer for the next response
	clrf		SifRspPopPtr
	; Setup for the first packet termination character
	SET_TX_STATE	TX_TERM_1_STATE
	goto		PROC_TX_RET


; --------------
; TxData1State
;
PROC_TX_DATA_1
	; Send a SPACE character
	movlw		SPACE_CHAR
	movwf		TXREG
	addwf		SifTxChecksum,F
	SET_TX_STATE	TX_DATA_2_STATE
	goto		PROC_TX_RET


; --------------
; TxData2State
;
PROC_TX_DATA_2
	; Send the high nibble of a data byte
	movf		SifRspPopPtr,W
	movwf		FSR1L
	swapf		INDF1,W
	call		GetHexAsciiChar
	movwf		TXREG
	addwf		SifTxChecksum,F
	SET_TX_STATE	TX_DATA_3_STATE
	goto		PROC_TX_RET


; --------------
; TxData3State
;
PROC_TX_DATA_3
	; Send the low nibble of a data byte
	movf		SifRspPopPtr,W
	movwf		FSR1L
	movf		INDF1,W
	call		GetHexAsciiChar
	movwf		TXREG
	addwf		SifTxChecksum,F
	; Check if there is more data to send
	incf		SifRspPopPtr,F
	decf		RspCount,F
	movf		RspCount,W
	btfsc		STATUS,Z		; RspCount = 0?
	goto		PROC_TX_SETUP_TERM	;   Y: No more data, setup to send term
						;   N: More data
	; Select the appropriate next data state
	btfss		CmdFlags,Form2Cmd
	goto		PROC_TX_SETUP_DATA_1
	SET_TX_STATE	TX_DATA_1_STATE		; Setup for data with spaces
	goto		PROC_TX_RET


; --------------
; TxTerm1State
;
PROC_TX_TERM_1
	; Send the first termination character based on command form
	movlw		CR_CHAR			; Will be overwritten if necessary
	btfss		CmdFlags,Form2Cmd
	movlw		'T'
	movwf		TXREG
	addwf		SifTxChecksum,F
	SET_TX_STATE	TX_TERM_2_STATE
	goto		PROC_TX_RET


; --------------
; TxTerm2State
;
PROC_TX_TERM_2
	; Send the second termination character based on command form
	btfsc		CmdFlags,Form2Cmd
	goto		PROC_TX_TERM_2_FORM2
	; Form 1: high nibble of the response checksum
	swapf		SifTxChecksum,W
	call		GetHexAsciiChar
	movwf		TXREG
	SET_TX_STATE	TX_TERM_3_STATE
	goto		PROC_TX_RET

PROC_TX_TERM_2_FORM2
	; Form 2: last character of response
	movlw		LF_CHAR
	movwf		TXREG
	SET_TX_STATE	TX_IDLE_STATE
	goto		PROC_TX_RET


; --------------
; TxTerm3State
;
PROC_TX_TERM_3
	; Send the low nibble of the response checksum
	movf		SifTxChecksum,W
	call		GetHexAsciiChar
	movwf		TXREG
	SET_TX_STATE	TX_IDLE_STATE


PROC_TX_RET
	BANKSEL		0
	return


; GetHexAsciiChar: Convert a 4-bit nibble into the equivalent ASCII character
;
; On entry: W[3:0] contains the nibble to convert to ASCII
;
; On exit: W contains the equivalent ASCII character
;
GetHexAsciiChar
	andlw		0x0F
	brw
	retlw		'0'
	retlw		'1'
	retlw		'2'
	retlw		'3'
	retlw		'4'
	retlw		'5'
	retlw		'6'
	retlw		'7'
	retlw		'8'
	retlw		'9'
	retlw		'A'
	retlw		'B'
	retlw		'C'
	retlw		'D'
	retlw		'E'
	retlw		'F'


; ProcessRxChar: ISR handler to process a character received from the serial port.
;
; On entry: RCREG may have one or more characters
;           SifTempData may contain the high half of a data byte
;           SifRxState contains current state
;
; On exit: The character (if it is a command) or a data byte may be pushed into
;            the Command Fifo.
;          SifTempData may be updated with a partially built data byte
;          SifRxState updated
;
ProcessRxChar
PROC_RX_LOOP
	; Process received characters until the RCREG FIFO is empty
	BANKSEL		PIR1
	btfss		PIR1,RCIF
	return

	; Check the RX status
	BANKSEL		RCSTA
	btfsc		RCSTA,FERR	; Framing error?
	goto		PROC_RX_RST	;   Y: Discard this character
	btfss		RCSTA,OERR	; Overrun?
	goto		PROC_RX_GET_CH	;   N: Process this character
	movf		RCREG,W		;   Y: Drain entry 1 of the RX FIFO

PROC_RX_RST
	; Drain entry 2 of the EUSART RX FIFO or clear a badly framed character
	movf		RCREG,W
	bcf		RCSTA,CREN	; Reset the RX port and clear errors
	bsf		RCSTA,CREN
	BANKSEL		SifRxState
	SET_RX_STATE	RX_IDLE_STATE
	goto		PROC_RX_LOOP

PROC_RX_GET_CH
	; Read the EUSART FIFO only once
	movf		RCREG,W
	BANKSEL		SifRxChar
	movwf		SifRxChar

	; Process the incoming character based on state:
	;
	;   Notes:
	;     1. Receiving a 'H' start-of-packet character always restarts the packet logic.
	;     2. Always skip <CR> characters
	;
	movlw		CR_CHAR
	subwf		SifRxChar,W
	btfsc		STATUS,Z		; <CR>?
	goto		PROC_RX_LOOP		;   Y: Ignore
	movlw		'H'
	subwf		SifRxChar,W
	btfss		STATUS,Z		; 'H' character?
	goto		PROC_RX_SEL_STATE	;   N: Evaluate based on state
	SET_RX_STATE	RX_CMD_STATE_H		;   Y: Move to RxCmdStateH
	clrf		CmdCount		; Setup to receive data with the command
	clrf		SifCmdPushPtr
	bcf		CmdFlags,BadCmdErr
	movf		SifRxChar,W		; Initialize running checksum
	movwf		SifRxChecksum
	bcf		CmdFlags,ChksumError
	goto		PROC_RX_LOOP

PROC_RX_SEL_STATE
	btfsc		SifRxState,RxIdleState	; By this point, we've handled legal
	goto		PROC_RX_LOOP		;   Idle state transitions
	btfsc		SifRxState,RxCmdStateH
	goto		PROC_RX_CMD_STATE_H
	btfsc		SifRxState,RxCmdStateL
	goto		PROC_RX_CMD_STATE_L
	btfsc		SifRxState,RxDataStateH
	goto		PROC_RX_DATA_STATE_H
	btfsc		SifRxState,RxDataStateL
	goto		PROC_RX_DATA_STATE_L
	btfsc		SifRxState,RxChksumStateH
	goto		PROC_RX_CHKSUM_H
	btfsc		SifRxState,RxChksumStateL
	goto		PROC_RX_CHKSUM_L
	; Illegal RX State resets RX state machine
	goto		PROC_RX_SET_IDLE

; --------------
; RxCmdStateH
;
PROC_RX_CMD_STATE_H
	call		UpdateRunningRxChksum
	movlw		SPACE_CHAR
	subwf		SifRxChar,W
	btfsc		STATUS,Z		; <SPACE>?
	goto		PROC_RX_LOOP		;   Y: Ignore
	call		CheckIfHexChar		; Check for Hex character
	btfss		CmdFlags,IsHexChar	; Legal Hex Char?
	goto		PROC_RX_NOTE_BAD_CMD	;   N: Unknown character
	movf		SifRxChar,W		;   Y: Store this nibble
	movwf		SifTempData
	SET_RX_STATE	RX_CMD_STATE_L		;      and move to RxCmdStateL
	goto		PROC_RX_LOOP

; --------------
; RxCmdStateL
;
PROC_RX_CMD_STATE_L
	call		UpdateRunningRxChksum
	movlw		SPACE_CHAR
	subwf		SifRxChar,W
	btfss		STATUS,Z		; <SPACE>?
	goto		PROC_RX_CMD_STATE_L_1	;   N: Check for Hex Char
	movf		SifTempData,W		;   Y: Use only hi-half as single-digit hex
	movwf		CmdByte			;      command
	SET_RX_STATE	RX_DATA_STATE_H		; and setup to get any data
	goto		PROC_RX_LOOP

PROC_RX_CMD_STATE_L_1
	call		CheckIfHexChar		; Check for Hex character
	btfss		CmdFlags,IsHexChar	; Legal Hex Char?
	goto		PROC_RX_CMD_STATE_L_2	;   N: Check for command termination
	swapf		SifTempData,W		;   Y: Load low-half of command byte
	iorwf		SifRxChar,W
	movwf		CmdByte
	SET_RX_STATE	RX_DATA_STATE_H		; and setup to get any data
	goto		PROC_RX_LOOP

PROC_RX_CMD_STATE_L_2
	movlw		'T'			; Check 'T'(form 1 end of command)
	subwf		SifRxChar,W
	btfss		STATUS,Z		; Form 1 termination?
	goto		PROC_RX_CMD_STATE_L_3	;   N: Check for form 2 termination
	movf		SifTempData,W		;   Y: Use only hi-half as single-digit hex
	movwf		CmdByte			;      command
	bcf		CmdFlags,Form2Cmd
	SET_RX_STATE	RX_CHK_STATE_H		; and setup to get checksum
	goto		PROC_RX_LOOP

PROC_RX_CMD_STATE_L_3
	movlw		LF_CHAR			; Check <LF> (form 2 end of command)
	subwf		SifRxChar,W
	btfss		STATUS,Z		; Form 2 termination?
	goto		PROC_RX_NOTE_BAD_CMD	;   N: Unknown character
	bsf		CmdFlags,Form2Cmd	;   Y: Note form 2 command available
	goto		PROC_RX_NOTE_CMD

; --------------
; RxDataStateH
;
PROC_RX_DATA_STATE_H
	call		UpdateRunningRxChksum
	movlw		SPACE_CHAR
	subwf		SifRxChar,W
	btfsc		STATUS,Z		; <SPACE>?
	goto		PROC_RX_LOOP		;   Y: Ignore
						;   N: Check for Hex Char
	call		CheckIfHexChar		; Check for Hex character
	btfss		CmdFlags,IsHexChar	; Legal Hex Char?
	goto		PROC_RX_DATA_STATE_H_1	;   N: Check for command termination
	swapf		SifRxChar,W		;   Y: Save first nibble
	movwf		SifTempData		;      in high part of SifTempData
	SET_RX_STATE	RX_DATA_STATE_L		; and setup to get low nibble
	goto		PROC_RX_LOOP

PROC_RX_DATA_STATE_H_1
	movlw		'T'			; Check 'T'(form 1 end of command)
	subwf		SifRxChar,W
	btfss		STATUS,Z		; Form 1 termination?
	goto		PROC_RX_DATA_STATE_H_2	;   N: Check for form 2 termination
						;   Y: Setup to get checksum
	bcf		CmdFlags,Form2Cmd
	SET_RX_STATE	RX_CHK_STATE_H
	goto		PROC_RX_LOOP

PROC_RX_DATA_STATE_H_2
	movlw		LF_CHAR			; Check <LF> (form 2 end of command)
	subwf		SifRxChar,W
	btfss		STATUS,Z		; Form 2 termination?
	goto		PROC_RX_NOTE_BAD_CMD	;   N: Unknown character
	bsf		CmdFlags,Form2Cmd	;   Y: Note form 2 command available
	goto		PROC_RX_NOTE_CMD


; --------------
; RxDataStateL
;
PROC_RX_DATA_STATE_L
	call		UpdateRunningRxChksum
	movlw		SPACE_CHAR
	subwf		SifRxChar,W
	btfss		STATUS,Z		; <SPACE>?
	goto		PROC_RX_DATA_STATE_L_1	;   N: Check for Hex Char
						;   Y: Use first nibble as data
	swapf		SifTempData,F		; Swap hi-half to be a 4-bit value
	call		PushRxFifo		; Push data
	SET_RX_STATE	RX_DATA_STATE_H		; Setup for another data byte
	goto		PROC_RX_LOOP

PROC_RX_DATA_STATE_L_1
	call		CheckIfHexChar		; Check for Hex character
	btfss		CmdFlags,IsHexChar	; Legal Hex Char?
	goto		PROC_RX_DATA_STATE_L_2	;   N: Check for command termination
	movf		SifRxChar,W		;   Y: Load low nibble
	iorwf		SifTempData,F		;      into low part of SifTempData
	call		PushRxFifo		; Push data
	SET_RX_STATE	RX_DATA_STATE_H		; and setup to another data byte
	goto		PROC_RX_LOOP

PROC_RX_DATA_STATE_L_2
	movlw		'T'			; Check 'T'(form 1 end of command)
	subwf		SifRxChar,W
	btfss		STATUS,Z		; Form 1 termination?
	goto		PROC_RX_DATA_STATE_L_3	;   N: Check for form 2 termination
						;   Y: Use first nibble as data
	swapf		SifTempData,F		; Swap hi-half to be a 4-bit value
	call		PushRxFifo		; Push data
	bcf		CmdFlags,Form2Cmd
	SET_RX_STATE	RX_CHK_STATE_H		; and setup to get checksum
	goto		PROC_RX_LOOP

PROC_RX_DATA_STATE_L_3
	movlw		LF_CHAR			; Check <LF> (form 2 end of command)
	subwf		SifRxChar,W
	btfss		STATUS,Z		; Form 2 termination?
	goto		PROC_RX_NOTE_BAD_CMD	;   N: Unknown character
						;   Y: Use first nibble as data
	swapf		SifTempData,F		; Swap hi-half to be a 4-bit value
	call		PushRxFifo		; Push data
	bsf		CmdFlags,Form2Cmd	;  Note form 2 command available
	goto		PROC_RX_NOTE_CMD

; --------------
; RxChksumStateH
;
PROC_RX_CHKSUM_H
	movlw		SPACE_CHAR
	subwf		SifRxChar,W
	btfsc		STATUS,Z		; <SPACE>?
	goto		PROC_RX_LOOP		;   Y: Ignore
						;   N: Check for Hex Char
	call		CheckIfHexChar		; Check for Hex character
	btfss		CmdFlags,IsHexChar	; Legal Hex Char?
	goto		PROC_RX_NOTE_BAD_CMD	;   N: Throw this command away
	swapf		SifRxChar,W		;   Y: Save first nibble
	movwf		SifTempData		;      in high part of SifTempData
	SET_RX_STATE	RX_CHK_STATE_L		; and setup to get low nibble
	goto		PROC_RX_LOOP

; --------------
; RxChksumStateL
;
PROC_RX_CHKSUM_L
	call		CheckIfHexChar		; Check for Hex character
	btfss		CmdFlags,IsHexChar	; Legal Hex Char?
	goto		PROC_RX_NOTE_BAD_CMD	;   N: Throw this command away
	movf		SifRxChar,W		;   Y: Get lo-half and then check
	iorwf		SifTempData,W
	subwf		SifRxChecksum,W		; W = Running checksum - RX checksum
	btfsc		STATUS,Z		; Running checksum = RX checksum?
	goto		PROC_RX_NOTE_CMD	;   Y: Note completed command
	goto		PROC_RX_NOTE_CHKSUM_ERR	;   N: Note completed command w/ checksum err

; --------------
; Indicate malformed command
;
PROC_RX_NOTE_BAD_CMD
	bsf		CmdFlags,BadCmdErr
	goto		PROC_RX_NOTE_CMD

; --------------
; Indicate checksum error
;
PROC_RX_NOTE_CHKSUM_ERR
	bsf		CmdFlags,ChksumError

; --------------
; Indicate command ready
;
PROC_RX_NOTE_CMD
	bsf		CmdFlags,CmdInProcess

; --------------
; Set RxIdleState
;
PROC_RX_SET_IDLE
	SET_RX_STATE	RX_IDLE_STATE
	goto		PROC_RX_LOOP


; UpdateRunningRxChksum: Update the running checksum
;
; On entry: SifRxChar contains the curreng RX character
;
; On exit: SifRxChecksum updated
;
UpdateRunningRxChksum
	movf		SifRxChar,W
	addwf		SifRxChecksum,F
	return


; PushRxFifo: Push a data byte into the CMD FIFO
;
; On entry: SifTempData has data to push
;           CmdCount contains the current count in the CMD FIFO
;           SifCmdPushPtr points to the next location in the CMD FIFO to load
;           FSR0H points to the high address of the CMD FIFO
;
; On exit: SifTempData pushed
;          CmdCount incremented
;          SifCmdPushPtr incremented
;
PushRxFifo
	movf		SifCmdPushPtr,W
	movwf		FSR0L
	movf		SifTempData,W
	movwf		INDF0
	incf		SifCmdPushPtr,F
	incf		CmdCount,F
	return


; CheckIfHexChar: Check if the current RX character is a hex ASCII number (0-9, a-f or A-F).
;
; On entry: SifRxChar contains the character to test
;
; On exit: CmdFlags.IsHexChar indicates if SifRxChar has been reloaded with a valid
;            4-bit value
;
CheckIfHexChar
	; Look for characters in the ranges '0' - '9', 'A' - 'F' or 'a' - 'f'
	movlw		'0'
	subwf		SifRxChar,W		; W = SifRxChar - '0'
	btfss		STATUS,C		; CARRY = 1 if SifRxChar >= '0'
	goto		CHECK_HEX_NOT_HEX	;   C = 0: SifRxChar < '0'
	movlw		':'			; ':' = '9' + 1
	subwf		SifRxChar,W
	btfss		STATUS,C		; CARRY = 0 if SifRxChar <= '9'
	goto		CHECK_HEX_HEX_0_9	;   C = 0: '0' - '9' range
	movlw		'A'
	subwf		SifRxChar,W
	btfss		STATUS,C		; CARRY = 1 if SifRxChar >= 'A'
	goto		CHECK_HEX_NOT_HEX	;   C = 0: SifRxChar < 'A'
	movlw		'G'			; 'G' = 'F' + 1
	subwf		SifRxChar,W
	btfss		STATUS,C		; CARRY = 0 if SifRxChar <= 'F'
	goto		CHECK_HEX_HEX_A_F	;   C = 0: 'A' - 'F'
	movlw		'a'
	subwf		SifRxChar,W
	btfss		STATUS,C		; CARRY = 1 if SifRxChar >= 'a'
	goto		CHECK_HEX_NOT_HEX	;   C = 0: SifRxChar < 'a'
	movlw		'g'			; 'g' = 'f' + 1
	subwf		SifRxChar,W
	btfss		STATUS,C		; CARRY = 0 if SifRxChar <= 'f'
	goto		CHECK_HEX_HEX_a_f	;   C = 0: 'a' - 'f'
						;   C = 1: Above 'f'

CHECK_HEX_NOT_HEX
	bcf		CmdFlags,IsHexChar
	return

CHECK_HEX_HEX_0_9
	; Convert to a real hex value
	movlw		'0'
	subwf		SifRxChar,F		; SifRxChar = SifRxChar - '0'
	goto		CHECK_HEX_VALID

CHECK_HEX_HEX_A_F
	movlw		'A'
	subwf		SifRxChar,W		; W = SifRxChar - 'A'
	addlw		0x0A			; W += 10
	movwf		SifRxChar
	goto		CHECK_HEX_VALID

CHECK_HEX_HEX_a_f
	movlw		'a'
	subwf		SifRxChar,W		; W = SifRxChar - 'a'
	addlw		0x0A			; W += 10
	movwf		SifRxChar

CHECK_HEX_VALID
	bsf		CmdFlags,IsHexChar
	return


	end
