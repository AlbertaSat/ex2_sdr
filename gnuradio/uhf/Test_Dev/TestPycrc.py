#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# File : TestPycrc.py
#
# Purpose : Test that the correct parameters are selected to match
#  the CRC16 algorithm used by EnduroSat, namely 
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
# Author : Steven Knudsen
# Date : June 21, 2023
# Copyright : University of Alberta, AlbertaSat, 2023
#
# Requirements :
#
# pycrc      - pip3 install pycrc
#
import binascii
import pycrc.algorithms

crc = pycrc.algorithms.Crc(width = 16, poly = 0x1021,
        reflect_in = False, xor_in = 0xFFFF,
        reflect_out = False, xor_out = 0x0000 )

data = '123456789'
my_crc = crc.bit_by_bit_fast(data)
print(my_crc)
#print(my_crc+b'29'+b'B1')
print(my_crc.to_bytes(2,"big"))
print('For '+data+' CRC is {:#04x}'.format(my_crc))
data = 'ES+R2200'
my_crc = crc.bit_by_bit_fast(data)
print('For '+data+' CRC is {:#04x}'.format(my_crc))

readSCWMsg = b'ES+R2200 BD888E1F\x0D'
my_crc = crc.bit_by_bit_fast(readSCWMsg)
print("For ", end="")
print(readSCWMsg,end='')
print(' CRC is {:#04x}'.format(my_crc))

