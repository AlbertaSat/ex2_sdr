This HOWTO provides information on several GNU Radio flowgraphs that support UHF radio operations. 

In general, the the following modes of operation are assumed:
* *Software-only;* GNU Radio implements both transmit and receive functions without modulating the signal to a carrier
* *SDR-only;* GNU Radio connects to an SDR device or devices. The transmit and receive functions rely on the devices to modulate and demodulate the baseband signals at the carrier frequency, respectively.
* *SDR and COTS;* GNU Radio connects to an SDR device and a COTS transceiver. The SDR device is used to modulate and demodulate transmit and receive baseband signals from GNU Radio, respectively. The COTS transceiver is connected to a separate program that manages baseband data (not signals) that is transmitted or received (i.e., packets).

## Software-only

## SDR-only

## SDR and COTS
