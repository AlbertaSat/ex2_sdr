#!/usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function, unicode_literals
from pprint import pprint
from InquirerPy import prompt
#from examples import custom_style_2
from os import system


import socket

level_1_questions = [
    {
        'type': 'list',
        'name': 'level_1',
        'message': 'What ESTTC command would you like to send?',
        'choices': [
            'Enable beacons',
            'Disable beacons',
            'Set beacon period',
            'Quit',
        ]
    },
]

def enableBeacons():
    print('Enable beacons bit in ESTTC Status Word and sending command.')

def disableBeacons():
    print('Disable beacons bit in ESTTC Status Word and sending command.')

result = prompt(level_1_questions)

while result['level_1'] != 'Quit':
    if result['level_1'] == 'Enable beacons':
        enableBeacons()
    if result['level_1'] == 'Disable beacons':
        disableBeacons()
    if result['level_1'] == 'Set beacon period':
        print('disable beacons')

    result = prompt(level_1_questions)

