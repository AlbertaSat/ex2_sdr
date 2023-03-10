#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: PDU Tools Example
# Author: J. Gilbert
# Copyright: 2021 J. Gilbert
# Description: Example usage of a range of PDU tools.
# GNU Radio version: 3.9.7.0

from distutils.version import StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from gnuradio import blocks
import pmt
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio.qtgui import Range, RangeWidget
from PyQt5 import QtCore



from gnuradio import qtgui

class pdu_tools_demo(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "PDU Tools Example", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("PDU Tools Example")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "pdu_tools_demo")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except:
            pass

        ##################################################
        # Variables
        ##################################################
        self.val = val = 123.4
        self.interval = interval = 0.400

        ##################################################
        # Blocks
        ##################################################
        self._val_range = Range(0, 500.0, 0.1, 123.4, 200)
        self._val_win = RangeWidget(self._val_range, self.set_val, "KEY1 Metadata Value", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_layout.addWidget(self._val_win)
        self.msg_strobe = blocks.message_strobe(pmt.PMT_T, int(interval*1000))
        self.msg_dbg_0 = blocks.message_debug(True)
        self.blocks_random_pdu_0 = blocks.random_pdu(10, 10, 0xFF, 2)
        self.blocks_pdu_set_0 = blocks.pdu_set(pmt.intern("KEY1"), pmt.from_double(val))


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_pdu_set_0, 'pdus'), (self.msg_dbg_0, 'print_pdu'))
        self.msg_connect((self.blocks_random_pdu_0, 'pdus'), (self.blocks_pdu_set_0, 'pdus'))
        self.msg_connect((self.msg_strobe, 'strobe'), (self.blocks_random_pdu_0, 'generate'))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "pdu_tools_demo")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_val(self):
        return self.val

    def set_val(self, val):
        self.val = val
        self.blocks_pdu_set_0.set_val(pmt.from_double(self.val))

    def get_interval(self):
        return self.interval

    def set_interval(self, interval):
        self.interval = interval
        self.msg_strobe.set_period(int(self.interval*1000))




def main(top_block_cls=pdu_tools_demo, options=None):

    if StrictVersion("4.5.0") <= StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()
