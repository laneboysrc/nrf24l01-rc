#!/usr/bin/env python
'''
This tool programs a firmware image via nrf_spi.asm
'''
from __future__ import print_function

import argparse
import sys
import serial
from binascii import hexlify


class NRFProgrammer(object):
    ''' Interface with the nrf_spi programmer hardware over serial '''

    VERSION = '00'
    ACCESS_MODE = '01'
    SPI = '02'
    ADDRESS = '03'
    PROGRAM = '04'
    ERASE_PAGE = '52'
    READ = '05'
    RESET = '06'

    ACCESS_MODE_ENTER = '01'
    ACCESS_MODE_EXIT = '00'

    PAGE_SIZE = 512
    MAX_PARAMETER_LENGTH = 256

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

    def read(self, start, length=0x4000):
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

    def write(self, start, data):
        ''' Write binary *data*, starting at *start* to the NRF flash '''
        self.command(self.ACCESS_MODE, self.ACCESS_MODE_ENTER)

        # First erase all the pages we will be programming
        # NOTE: we do not perform ERASE ALL as it clears the info page in the
        # chip, destroying important chip data
        start_page = start // self.PAGE_SIZE
        end_page = (start + len(data)) // self.PAGE_SIZE
        while start_page <= end_page:
            self.command(self.ERASE_PAGE, "{:02X}".format(start_page))
            start_page += 1

        # Write the start address, then the new firmware image in 256 byte
        # chunks
        self.command(self.ADDRESS, "{:04X}".format(start))
        while len(data):
            firmware = hexlify(data[:self.MAX_PARAMETER_LENGTH])
            self.command(self.PROGRAM, firmware)
            data = data[self.MAX_PARAMETER_LENGTH:]

        self.command(self.ACCESS_MODE, self.ACCESS_MODE_EXIT)
        return data


def parse_commandline():
    ''' Command line argument parsing '''
    parser = argparse.ArgumentParser(
        description="Program a firmware image into the NRF24LE1.")

    parser.add_argument("-b", "--baudrate", type=int, default=115200,
        help='Baudrate to use. Default is 115200.')

    parser.add_argument("tty", nargs="?", default="/dev/ttyUSB0",
        help="serial port to use. ")

    parser.add_argument("binfile", nargs=1, type=argparse.FileType('rb'),
        help="the file name of the binary image file to program")

    return parser.parse_args()


def program_nrf_firmware():
    ''' Interface with nrf_spi.asm '''

    args = parse_commandline()
    programmer = NRFProgrammer(args)

    programmer.write(0, args.binfile[0].read())


def main():
    ''' Program start '''
    try:
        program_nrf_firmware()
    except KeyboardInterrupt:
        print("")
        sys.exit(0)


if __name__ == '__main__':
    main()
