#!/usr/bin/env python
'''
This script is used to test the battery life on our STM32 based nRF24L01
RC transmitter.

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
        # print(message)


    tx_serial_port = pyserial.Serial(TX_PORT, 115200, timeout=0)
    tx_transport = serial_transport(tx_serial_port)
    tx_transport.line_terminator = "\n"
    tx_transport._terminator_slice = -1*len(tx_transport.line_terminator)
    tx_transport.set_message_callback(tx_callback)


    dmm_serial_port = pyserial.Serial(DMM_PORT, 19200, timeout=0)
    dmm_transport = serial_transport(dmm_serial_port)
    keithley = scpi_device(dmm_transport)

    keithley.scpi.send_command("FUNC 'VOLT', (@101:110)", False)
    keithley.scpi.send_command("VOLT:RANG 10, (@101:110)", False)
    keithley.scpi.send_command("VOLT:DC:DIG 7", False)

    keithley.scpi.send_command("TRIG:SOUR IMM", False)
    keithley.scpi.send_command("TRIG:COUN 1", False)
    keithley.scpi.send_command("SAMP:COUN 2", False)
    keithley.scpi.send_command("ROUT:SCAN (@101, 102)", False)
    keithley.scpi.send_command("ROUT:SCAN:TSO IMM", False)
    keithley.scpi.send_command("ROUT:SCAN:LSEL INT", False)

    keithley.scpi.send_command("INIT", False)


    print("{}, {}, {}, {}, {}, {}".format("seconds", "tx_mv", "tx_adc_vcc", "tx_adc_vref", "Vbatt", "Vcc"), file=f)
    start = time.time();


    while (True):
        keithley.scpi.send_command("READ?")
        now = time.time();

        dmm_data = keithley.scpi.message_stack.pop().split(',')
        voltages = [convert_voltage(dmm_data[0]), convert_voltage(dmm_data[3])]

        tx_adc_values = tx_message['msg']
        if now - tx_message['time'] > 10:
            tx_adc_values = ', , '


        print("{}, {}, {}, {}".format(int(now-start), tx_message['msg'], voltages[0], voltages[1]))
        print("{}, {}, {}, {}".format(int(now-start), tx_message['msg'], voltages[0], voltages[1]), file=f)
        f.flush()
        time.sleep(60)