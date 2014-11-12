#!/usr/bin/env python
'''
This tool reads a firmwre image from nrf_spi.asm
'''
from __future__ import print_function

import argparse
import sys
import serial
from binascii import unhexlify


class NRFProgrammer(object):
    ''' Interface with the nrf_spi programmer hardware over serial '''

    VERSION = '00'
    ACCESS_MODE = '01'
    SPI = '02'
    ADDRESS = '03'
    PROGRAM = '04'
    READ = '05'
    RESET = '06'

    ACCESS_MODE_ENTER = '01'
    ACCESS_MODE_EXIT = '00'

    def __init__(self, args):
        self.args = args
        try:
            self.uart = serial.Serial(self.args.tty, self.args.baudrate)
        except serial.SerialException as error:
            print("Unable to open port %s: %s" % (self.args.tty, error))
            sys.exit(1)

    def command(self, command, arguments=""):
        ''' Send a command to the programmer and read the response '''
        message = "H" + command + arguments + "\n"
        self.uart.write(message)
        data = self.uart.readline()
        data = data.decode('ascii', errors='replace').strip()
        data = data.replace(" ", "")
        code = data[0]
        if code != "R":
            raise ValueError("Command {} failed".format(command))
        return data[1:]

    def version(self):
        ''' Return the version number of the nrf_spi programmer hardware '''
        response = self.command(self.VERSION)
        return response

    def read(self, start, length=0x10000):
        ''' Read *length* bytes, starting at *start* from the NRF flash '''
        self.command(self.ACCESS_MODE, self.ACCESS_MODE_ENTER)
        self.command(self.ADDRESS, "{:04X}".format(start))

        data = ""
        while length:
            if length >= 256:
                size = 0
            else:
                size = length
            data += self.command(self.READ, "{:02X}".format(size))
            if size == 0:
                size = 256
            length -= size

        self.command(self.ACCESS_MODE, self.ACCESS_MODE_EXIT)
        return data


def parse_commandline():
    ''' Command line argument parsing '''
    parser = argparse.ArgumentParser(
        description="Read a firmware image from the NRF24LE1.")

    parser.add_argument("-b", "--baudrate", type=int, default=115200,
        help='Baudrate to use. Default is 115200.')

    parser.add_argument("tty", nargs="?", default="/dev/ttyUSB0",
        help="serial port to use. ")

    parser.add_argument("binfile", nargs=1, type=argparse.FileType('wb'),
        help="the file name of the binary image file to write")

    return parser.parse_args()


def read_nrf_firmware():
    ''' Interface with nrf_spi.asm '''

    args = parse_commandline()
    programmer = NRFProgrammer(args)

    firmware = programmer.read(0, 0x4000)
    binary = unhexlify(firmware)
    args.binfile[0].write(binary)


def main():
    ''' Program start '''
    try:
        read_nrf_firmware()
    except KeyboardInterrupt:
        print("")
        sys.exit(0)


if __name__ == '__main__':
    main()
