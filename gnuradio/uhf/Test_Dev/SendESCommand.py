#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, unicode_literals
from pprint import pprint
from InquirerPy import prompt, inquirer
from InquirerPy.validator import EmptyInputValidator
from prompt_toolkit.validation import ValidationError, Validator

from os import system

import socket

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
addr = ("127.0.0.1", 52001)

clear = lambda: system('clear')

level_1_questions = [
    {
        'type': 'list',
        'name': 'level_1',
        'message': 'What ESTTC command would you like to send?',
        'choices': [
            'Read Status Control Word (SCW)',
            'Enable beacons',
            'Disable beacons',
            'Set beacon period',
            'Get radio uptime',
            'Get radio received packets',
            'Quit',
        ]
    },
]

def readSCW():
    print('Read Status Control Word.')
    readSCWMsg = b"ES+R2200 BD888E1F"+b"\x0D"
    client_socket.sendto(readSCWMsg, addr)
    print(readSCWMsg)

def enableBeacons():
    print('Enable beacons bit in ESTTC Status Word and sending command.')
    ebMsg = b"ES+W22003440 9287EF3A"+b"\0D"
    client_socket.sendto(ebMsg, addr)
    print(ebMsg)

def disableBeacons():
    print('Disable beacons bit in ESTTC Status Word and sending command.')
    dbMsg = b"ES+W22003400 F6EB2A3E"+b"\0D"
    client_socket.sendto(dbMsg, addr)
    print(dbMsg)

#class beaconPeriodValidator(Validator):
#    def validate(self, numberText):
#        status = 1 == 0
#        if str.isnumeric(numberText): 
#            val = int(numberText)
#            status = val > 0 and val < 256
#        return status

def setBeaconPeriod():
    print('Ask for beacon period.')
    integer_val = inquirer.number(
        message="Enter integer:",
        min_allowed=5,
        max_allowed=5,
        validate=EmptyInputValidator(),
    ).execute()
    bPeriodMsg = b"ES+W220700000005 AE2A4F6E"+b"\0D"
    client_socket.sendto(bPeriodMsg, addr)
    print(bPeriodMsg)

def readUptime():
    print('Read radio uptime.')
    uptimeMsg = b"ES+R2202 5386EF33"+b"\0D"
    client_socket.sendto(uptimeMsg, addr)
    print(uptimeMsg)

def readNumReceivedPackets():
    print('Read radio number of received packets.')
    numPacketsMsg = b"ES+R2204 BAE54A06"+b"\0D"
    client_socket.sendto(numPacketsMsg, addr)
    print(numPacketsMsg)

 
clear()

result = prompt(level_1_questions)

while result['level_1'] != 'Quit':
    if result['level_1'] == 'Read Status Control Word (SCW)':
        readSCW()

    if result['level_1'] == 'Enable beacons':
        enableBeacons()

    if result['level_1'] == 'Disable beacons':
        disableBeacons()

    if result['level_1'] == 'Set beacon period':
        setBeaconPeriod()

    if result['level_1'] == 'Get radio uptime':
        readUptime()

    if result['level_1'] == 'Get radio received packets':
        readNumReceivedPackets()

    clear()
    result = prompt(level_1_questions)

