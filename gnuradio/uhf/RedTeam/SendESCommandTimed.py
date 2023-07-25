#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# File : SendESCommandTimed.py
#
# Purpose : Generate complete EnduroSAT ESTTC commands as selected
#  by the user, place them in a packet as defined in the EnduroSat
#  manual including the CRC16 at the end, and send via UDP to a
#  client application for a specific duration at a specific rate.
#  For example, the client may be a GNU Radio flowgraph that
#  modulates the received packet bits and sends the result to a
#  UHD USRP Sink block.
#
# Author : Steven Knudsen
# Date : June 22, 2023
# Copyright : University of Alberta, AlbertaSat, 2023
#
# Requirements :
# InquirerPy - pip3 install InquirerPy
# pycrc      - pip3 install pycrc
#

from __future__ import print_function, unicode_literals
from pprint import pprint
from InquirerPy import prompt, inquirer
from InquirerPy.validator import EmptyInputValidator
from prompt_toolkit.validation import ValidationError, Validator

import binascii
import pycrc.algorithms
import math
from os import system
import socket
import time

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
addr = ("127.0.0.1", 52001)

clear = lambda: system('clear')

#
# The CRC16 parameters are selected to match the CRC16 algorithm used by
# EnduroSat, namely
#
#    CRC-16/CCITT-FALSE
#
#  which has
#    num bits    = 16
#    poly        = 0x1021
#    reflect-in  = false
#    xor-in      = 0xFFFF
#    reflect-out = false
#    xor-out     = 0x0000
#
# The calculated CRC16 can be checked at crccalc.com
#

crc16 = pycrc.algorithms.Crc(width = 16, poly = 0x1021,
        reflect_in = False, xor_in = 0xFFFF,
        reflect_out = False, xor_out = 0x0000 )

#
# The CRC32 parameters are selected to match the CRC32 algorithm used by
# EnduroSat, namely
#
#    CRC-32
#
#  which has
#    num bits    = 32 
#    poly        = 0x04C11DB7
#    reflect-in  = true 
#    xor-in      = 0xFFFFFFFF
#    reflect-out = true
#    xor-out     = 0xFFFFFFFF
#
# The calculated CRC32 can be checked at crccalc.com
#

crc32 = pycrc.algorithms.Crc(width = 32, poly = 0x04C11DB7,
        reflect_in = True, xor_in = 0xFFFFFFFF,
        reflect_out = True, xor_out = 0xFFFFFFFF )

#
# InquirerPy question lists
#
esttc_msg_questions = [
    {
        'type': 'list',
        'name': 'esttc_msg',
        'message': 'What ESTTC command would you like to send?',
        'choices': [
            'Read Status Control Word (SCW)',
            'Set beacon period 2 s',
            'Set beacon period 5 s',
            'Set beacon period 30 s',
            'Set beacon period 60 s',
            'Get radio uptime',
            'Get radio received packets',
            'Set RF Mode w/ Beacon On (DANGEROUS)',
            'Set RF Mode w/ Beacon Off (DANGEROUS)',
            'Quit',
        ]
    },
]

baud_questions = [
    {
        'type': 'list',
        'name': 'baud',
        'message': 'Select the radio baud rate [sps is bps for 2GFSK]:',
        'choices': [
            str(1200),
            str(2400),
            str(4800),
            str(9600),
            str(19200),
        ]
    },
]

rf_mode_questions = [
    {
        'type': 'list',
        'name': 'rf_mode',
        'message': 'Which RF Mode?',
        'choices': [
            str(0),
            str(1),
            str(2),
            str(3),
            str(4),
            str(5),
            str(6),
            str(7),
        ]
    },
]

#
# Functions to generate the ASCII ESTTC command strings
#
def readSCW():
    print('Command is Read Status Control Word.')
    readSCWMsg = b"ES+R2200 BD888E1E"+b"\r"
    return readSCWMsg

def writeSCWRFMode0BeaconOn():
    print('Command is Set RF Mode 0 with Beacon On.')
    writeSCWRFModeMsg = b"ES+W22003043 0C87165C"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode1BeaconOn():
    print('Command is Set RF Mode 1 with Beacon On.')
    writeSCWRFModeMsg = b"ES+W22003143 0D457C6B"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode2BeaconOn():
    print('Command is Set RF Mode 2 with Beacon On.')
    writeSCWRFModeMsg = b"ES+W22003243 0F03C232"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode3BeaconOn():
    print('Command is Set RF Mode 3 with Beacon On.')
    writeSCWRFModeMsg = b"ES+W22003343 0EC1A805"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode4BeaconOn():
    print('Command is Set RF Mode 4 with Beacon On.')
    writeSCWRFModeMsg = b"ES+W22003443 0B8EBE80"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode5BeaconOn():
    print('Command is Set RF Mode 5 with Beacon On.')
    writeSCWRFModeMsg = b"ES+W22003543 0A4CD4B7"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode6BeaconOn():
    print('Command is Set RF Mode 6 with Beacon On.')
    writeSCWRFModeMsg = b"ES+W22003643 080A6AEE"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode7BeaconOn():
    print('Command is Set RF Mode 7 with Beacon On.')
    writeSCWRFModeMsg = b"ES+W22003743 09C800D9"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode0BeaconOff():
    print('Command is Set RF Mode 0 with Beacon Off.')
    writeSCWRFModeMsg = b"ES+W22003003 68EBD358"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode1BeaconOff():
    print('Command is Set RF Mode 1 with Beacon Off.')
    writeSCWRFModeMsg = b"ES+W22003103 6929B96F"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode2BeaconOff():
    print('Command is Set RF Mode 2 with Beacon Off.')
    writeSCWRFModeMsg = b"ES+W22003203 6B6F0736"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode3BeaconOff():
    print('Command is Set RF Mode 3 with Beacon Off.')
    writeSCWRFModeMsg = b"ES+W22003303 6AAD6D01"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode4BeaconOff():
    print('Command is Set RF Mode 4 with Beacon Off.')
    writeSCWRFModeMsg = b"ES+W22003403 6FE27B84"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode5BeaconOff():
    print('Command is Set RF Mode 5 with Beacon Off.')
    writeSCWRFModeMsg = b"ES+W22003503 6E2011B3"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode6BeaconOff():
    print('Command is Set RF Mode 6 with Beacon Off.')
    writeSCWRFModeMsg = b"ES+W22003603 6C66AFEA"+b"\r"
    return writeSCWRFModeMsg

def writeSCWRFMode7BeaconOff():
    print('Command is Set RF Mode 7 with Beacon Off.')
    writeSCWRFModeMsg = b"ES+W22003703 6DA4C5DD"+b"\r"
    return writeSCWRFModeMsg

def setBeaconPeriod_2s():
    print('Command is Set beacon period, which is hard-coded to 2 seconds.')
    beaconPeriodMsg = b"ES+W220700000002 304EDACD"+b"\r"
    return beaconPeriodMsg

def setBeaconPeriod_5s():
    print('Command is Set beacon period, which is hard-coded to 5 seconds.')
    beaconPeriodMsg = b"ES+W220700000005 AE2A4F6E"+b"\r"
    return beaconPeriodMsg

def setBeaconPeriod_30s():
    print('Command is Set beacon period, which is hard-coded to 30 seconds.')
    beaconPeriodMsg = b"ES+W22070000001E E7340F13"+b"\r"
    return beaconPeriodMsg

def setBeaconPeriod_60s():
    print('Command is Set beacon period, which is hard-coded to 60 seconds.')
    beaconPeriodMsg = b"ES+W22070000003C 3C61C8A4"+b"\r"
    return beaconPeriodMsg

def readUptime():
    print('Read radio uptime.')
    uptimeMsg = b"ES+R2202 5386EF33"+b"\r"
    return uptimeMsg

def readNumReceivedPackets():
    print('Read radio number of received packets.')
    numPacketsMsg = b"ES+R2204 BAE54A06"+b"\r"
    return numPacketsMsg

#
# Functions to query for program parameters
#
def getCurrentRFMode():
    print('Enter right RF Mode. If incorrect, the new more will result in a misconfigured radio we cannot talk to...')
    mode = inquirer.number(
        message="Enter the current RF Mode [0,7]:",
        min_allowed=0,
        max_allowed=7,
        validate=EmptyInputValidator(),
    ).execute()
    print('Using RF Mode '+mode)
    return mode

def getTransmissionInterval(minInterval):
    intervalResult = prompt(interval_questions)
    txInterval = intervalResult['interval']
    return txInterval

def getTotalTransmissionDuration(minDur=1,maxDur=300):
    txSecs = inquirer.number(
        message="Enter the total number of seconds to transmit packets [1,300]:",
        min_allowed=minDur,
        max_allowed=maxDur,
        validate=EmptyInputValidator(),
    ).execute()
    print('Will transmit for '+txSecs+' seconds.')
    return txSecs

def getTransmissionInterval(minInterval):
    interval_questions = [
        {
            'type': 'list',
            'name': 'interval',
            'message': 'Select the message interval [ms]:',
            'choices': [
                str(minInterval),
                str(minInterval*2),
                str(minInterval*3),
                str(minInterval*4),
                str(minInterval*5),
                str(minInterval*6),
                str(minInterval*7),
                str(minInterval*8),
                str(minInterval*9),
                str(minInterval*10),
            ]
        },
    ]

    intervalResult = prompt(interval_questions)
    txInterval = intervalResult['interval']
    return txInterval

def getPacketTimeInterval(minInt=0.1,maxInt=1):
    print(minInterval)
    print(txDuration)
    intervalSecs= inquirer.number(
        message="Enter the time between packets in seconds["+str(minInt)+","+str(maxInt)+"]:",
        float_allowed = True,
        validate=TimeIntervalValidator(minInt),
    ).execute()
    print('Will transmit every '+intervalSecs+' seconds.')
    return intervalSecs

#
# Make an ESTTC packet for the input command
#
def makeESTTCPacket(command):
    print("Make the ESTTC Packet")
    dataFields = len(command).to_bytes(1, "big")+command
    # Calculate the CRC16 over the Data Field 1 and 2 as per the ES manual
    dataFieldsCRC = crc16.bit_by_bit_fast(dataFields)
    print('For '+str(dataFields)+' CRC is {:#04x}'.format(dataFieldsCRC))
    dataFieldsCRC = dataFields + dataFieldsCRC.to_bytes(2,"big")
    #leadIn = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    leadIn = ""
    preambleSync = "AAAAAAAAAA7E"
    postambleSync = "AAAAAAAAAAAA"
    packet = bytes.fromhex(leadIn)+bytes.fromhex(preambleSync)+dataFieldsCRC+bytes.fromhex(postambleSync)
    print("The whole packet is : "+str(packet))
    return packet

def main():
    #
    # Get the radio baud rate
    #
    clear()
    baudResult = prompt(baud_questions)
    baudRate = int(baudResult['baud'])

    #
    # Calculate some packet tx/rx times based on slowest baud rate.
    # Need to allow the radio time to receive and response
    #
    maxESPacketBits = (5 + 1 + 1 + 128 + 2) * 8 # based on ES packet definition
    # Need to account for ES radio response time, assume 1.5x as long as packet
    guardBits = math.ceil(1.5*maxESPacketBits)
    minInterval = math.ceil((maxESPacketBits + guardBits) / baudRate * 1000) # milliseconds

    #
    # Get the total duration to send and ESTTC command, the time between
    # commands, and the command
    #
    txDuration = float(getTotalTransmissionDuration())
    txIntervalMillis = float(getTransmissionInterval(minInterval))

    clear()
    print("Command will be transmitted every "+str(txIntervalMillis)+" ms for "+str(txDuration)+" s")
    time.sleep(3)
    clear()
    #
    # Now get the ESTTC command string to send
    #
    commandStr = ""
    result = prompt(esttc_msg_questions)
    if result['esttc_msg'] != 'Quit':
        if result['esttc_msg'] == 'Read Status Control Word (SCW)':
            commandStr = readSCW()

        if result['esttc_msg'] == 'Set beacon period 2 s':
            commandStr = setBeaconPeriod_2s()

        if result['esttc_msg'] == 'Set beacon period 5 s':
            commandStr = setBeaconPeriod_5s()

        if result['esttc_msg'] == 'Set beacon period 30 s':
            commandStr = setBeaconPeriod_30s()

        if result['esttc_msg'] == 'Set beacon period 60 s':
            commandStr = setBeaconPeriod_60s()

        if result['esttc_msg'] == 'Get radio uptime':
            commandStr = readUptime()

        if result['esttc_msg'] == 'Get radio received packets':
            commandStr = readNumReceivedPackets()

        if result['esttc_msg'] == 'Set RF Mode w/ Beacon On (DANGEROUS)':
            modeResult = prompt(rf_mode_questions)
            rfMode = int(modeResult['rf_mode'])
            if rfMode == 0:
                commandStr = writeSCWRFMode0BeaconOn()
            elif rfMode == 1:
                commandStr = writeSCWRFMode1BeaconOn()
            elif rfMode == 2:
                commandStr = writeSCWRFMode2BeaconOn()
            elif rfMode == 3:
                commandStr = writeSCWRFMode3BeaconOn()
            elif rfMode == 4:
                commandStr = writeSCWRFMode4BeaconOn()
            elif rfMode == 5:
                commandStr = writeSCWRFMode5BeaconOn()
            elif rfMode == 6:
                commandStr = writeSCWRFMode6BeaconOn()
            elif rfMode == 7:
                commandStr = writeSCWRFMode7BeaconOn()

        if result['esttc_msg'] == 'Set RF Mode w/ Beacon Off (DANGEROUS)':
            modeResult = prompt(rf_mode_questions)
            rfMode = int(modeResult['rf_mode'])
            if rfMode == 0:
                commandStr = writeSCWRFMode0BeaconOff()
            elif rfMode == 1:
                commandStr = writeSCWRFMode1BeaconOff()
            elif rfMode == 2:
                commandStr = writeSCWRFMode2BeaconOff()
            elif rfMode == 3:
                commandStr = writeSCWRFMode3BeaconOff()
            elif rfMode == 4:
                commandStr = writeSCWRFMode4BeaconOff()
            elif rfMode == 5:
                commandStr = writeSCWRFMode5BeaconOff()
            elif rfMode == 6:
                commandStr = writeSCWRFMode6BeaconOff()
            elif rfMode == 7:
                commandStr = writeSCWRFMode7BeaconOff()

        clear()
        print("The command string is : "+str(commandStr))
        thePacket = makeESTTCPacket(commandStr)

        txDuration = txDuration * 1000 # Change to milliseconds
        packetCount = 1
        while txDuration > 0:
            txDuration = txDuration - txIntervalMillis;
            print("sending packet "+str(packetCount)+"...")
            client_socket.sendto(thePacket, addr)
            packetCount = packetCount + 1
            time.sleep(txIntervalMillis/1000.0)

    else:
        print("No command will be sent")

if __name__ == "__main__":
    main()
