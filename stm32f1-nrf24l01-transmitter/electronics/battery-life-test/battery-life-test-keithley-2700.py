#!/usr/bin/env python
'''
This script is used to test the battery life on our STM32 based nRF24L01
RC transmitter.

It is using a Keithley 2700 DMM with one 7700 card.
The battery voltage is connected to contacts 101, the 3.3V supply to 102.

This script connects to two serial ports.

/dev/ttyUSB0 connects to the serial output of the STM32 in the transmitter.
It monitors the battery voltage in mV it calculates internally, as well as
the raw ADV values for the battery voltage in the internal 1.2V reference.

/dev/ttyUSB1 connects to the Keithley 2700 and performs a "scan" (sequence of
measurements) roughly every minute using SCPI commands.

The results from both serial ports are combined into a CSV file.

'''
from __future__ import print_function

from scpi import scpi_device
from scpi.transports import rs232 as serial_transport
import serial as pyserial
import time

TX_PORT = '/dev/ttyUSB0'
DMM_PORT = '/dev/ttyUSB1'


tx_message = dict(time=0, msg="")


with open("measurements.csv", "at") as f:
    def convert_voltage(s):
        if s.endswith("VDC"):
            return float(s[:-3])
        return s


    def tx_callback(message):
        tx_message['time'] = time.time()
        tx_message['msg'] = message


    # We abuse the SCPI low-level interface to give read the STM32 serial
    # output, which it generates every second, in the background.
    tx_serial_port = pyserial.Serial(TX_PORT, 115200, timeout=0)
    tx_transport = serial_transport(tx_serial_port)
    tx_transport.line_terminator = "\n"
    tx_transport._terminator_slice = -1*len(tx_transport.line_terminator)
    tx_transport.set_message_callback(tx_callback)


    dmm_serial_port = pyserial.Serial(DMM_PORT, 19200, timeout=0)
    dmm_transport = serial_transport(dmm_serial_port)
    keithley = scpi_device(dmm_transport)

    # Set measurement slots 1..10 to 10 Volt, 7 digits resolution
    keithley.scpi.send_command("FUNC 'VOLT', (@101:110)", False)
    keithley.scpi.send_command("VOLT:RANG 10, (@101:110)", False)
    keithley.scpi.send_command("VOLT:DC:DIG 7", False)

    # Setup the scan and start it
    keithley.scpi.send_command("TRIG:SOUR IMM", False)
    keithley.scpi.send_command("TRIG:COUN 1", False)
    keithley.scpi.send_command("SAMP:COUN 2", False)
    keithley.scpi.send_command("ROUT:SCAN (@101, 102)", False)
    keithley.scpi.send_command("ROUT:SCAN:TSO IMM", False)
    keithley.scpi.send_command("ROUT:SCAN:LSEL INT", False)

    keithley.scpi.send_command("INIT", False)


    # Print the CSV header
    print("{}, {}, {}, {}, {}, {}".format("seconds", "tx_mv", "tx_adc_vcc", "tx_adc_vref", "Vbatt", "Vcc"), file=f)
    start = time.time();


    while (True):
        keithley.scpi.send_command("READ?")
        now = time.time();

        dmm_data = keithley.scpi.message_stack.pop().split(',')
        voltages = [convert_voltage(dmm_data[0]), convert_voltage(dmm_data[3])]

        tx_adc_values = tx_message['msg']
        # Buggy code, needs braces around subtraction. With the bug, the output
        # has the last STM32 message repeated forever.
        if now - tx_message['time'] > 10:
            tx_adc_values = ', , '

        # Output to the console as well as the CSV file
        print("{}, {}, {}, {}".format(int(now-start), tx_message['msg'], voltages[0], voltages[1]))
        print("{}, {}, {}, {}".format(int(now-start), tx_message['msg'], voltages[0], voltages[1]), file=f)
        f.flush()
        time.sleep(60)