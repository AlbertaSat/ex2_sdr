#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: UHF Mode 5 Loopback no dopper
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

import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import blocks
from gnuradio import gr
from gnuradio.fft import window
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from uhf_pdu_demodulate import uhf_pdu_demodulate  # grc-generated hier_block
from uhf_pdu_modulate import uhf_pdu_modulate  # grc-generated hier_block
import satellites.components.datasinks



from gnuradio import qtgui

class uhf_mode5_loopback_nodoppler(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "UHF Mode 5 Loopback no dopper", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("UHF Mode 5 Loopback no dopper")
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

        self.settings = Qt.QSettings("GNU Radio", "uhf_mode5_loopback_nodoppler")

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
        self.spsym_frontend = spsym_frontend = 256
        self.oversample_rate = oversample_rate = 8
        self.spsym = spsym = spsym_frontend/oversample_rate
        self.baud_bit = baud_bit = 9600
        self.tx_gain = tx_gain = 0.9
        self.samp_rate = samp_rate = baud_bit*spsym
        self.rx_gain = rx_gain = 30
        self.out_len = out_len = 129
        self.in_len = in_len = 158
        self.center_freq = center_freq = 437875000

        ##################################################
        # Blocks
        ##################################################
        self.uhf_pdu_modulate_0 = uhf_pdu_modulate(
            fm_modulation_index=1,
        )
        self.uhf_pdu_demodulate_0 = uhf_pdu_demodulate(
            baud_rate=baud_bit,
            fm_modulation_index=1,
            samp_rate=samp_rate,
        )
        self.satellites_hexdump_sink_0 = satellites.components.datasinks.hexdump_sink(options="")
        self.qtgui_time_sink_x_1 = qtgui.time_sink_c(
            int(baud_bit*spsym_frontend*5), #size
            int(baud_bit*spsym_frontend*5), #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_1.set_update_time(5)
        self.qtgui_time_sink_x_1.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_1.set_y_label('Raw', "")

        self.qtgui_time_sink_x_1.enable_tags(True)
        self.qtgui_time_sink_x_1.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_1.enable_autoscale(False)
        self.qtgui_time_sink_x_1.enable_grid(False)
        self.qtgui_time_sink_x_1.enable_axis_labels(True)
        self.qtgui_time_sink_x_1.enable_control_panel(False)
        self.qtgui_time_sink_x_1.enable_stem_plot(False)


        labels = ['Signal 1', 'Signal 2', 'Signal 3', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(2):
            if len(labels[i]) == 0:
                if (i % 2 == 0):
                    self.qtgui_time_sink_x_1.set_line_label(i, "Re{{Data {0}}}".format(i/2))
                else:
                    self.qtgui_time_sink_x_1.set_line_label(i, "Im{{Data {0}}}".format(i/2))
            else:
                self.qtgui_time_sink_x_1.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_1.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_1.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_1.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_1.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_1.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_1_win = sip.wrapinstance(self.qtgui_time_sink_x_1.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_1_win)
        self.qtgui_time_sink_x_0 = qtgui.time_sink_c(
            int(spsym_frontend*50)*10, #size
            int(spsym_frontend*50), #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_0.set_update_time(1)
        self.qtgui_time_sink_x_0.set_y_axis(-0.1, 0.1)

        self.qtgui_time_sink_x_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0.enable_tags(True)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_0.enable_autoscale(True)
        self.qtgui_time_sink_x_0.enable_grid(False)
        self.qtgui_time_sink_x_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_0.enable_control_panel(False)
        self.qtgui_time_sink_x_0.enable_stem_plot(False)


        labels = ['Signal 1', 'Signal 2', 'Signal 3', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [-1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(2):
            if len(labels[i]) == 0:
                if (i % 2 == 0):
                    self.qtgui_time_sink_x_0.set_line_label(i, "Re{{Data {0}}}".format(i/2))
                else:
                    self.qtgui_time_sink_x_0.set_line_label(i, "Im{{Data {0}}}".format(i/2))
            else:
                self.qtgui_time_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_0.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_0.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_0_win = sip.wrapinstance(self.qtgui_time_sink_x_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_0_win)
        self.fm_sensitivity = qtgui.number_sink(
            gr.sizeof_float,
            0,
            qtgui.NUM_GRAPH_NONE,
            1,
            None # parent
        )
        self.fm_sensitivity.set_update_time(1)
        self.fm_sensitivity.set_title("FM Sensitivity")

        labels = ['', '', '', '', '',
            '', '', '', '', '']
        units = ['', '', '', '', '',
            '', '', '', '', '']
        colors = [("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"),
            ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black"), ("black", "black")]
        factor = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]

        for i in range(1):
            self.fm_sensitivity.set_min(i, 0)
            self.fm_sensitivity.set_max(i, 1)
            self.fm_sensitivity.set_color(i, colors[i][0], colors[i][1])
            if len(labels[i]) == 0:
                self.fm_sensitivity.set_label(i, "Data {0}".format(i))
            else:
                self.fm_sensitivity.set_label(i, labels[i])
            self.fm_sensitivity.set_unit(i, units[i])
            self.fm_sensitivity.set_factor(i, factor[i])

        self.fm_sensitivity.enable_autoscale(False)
        self._fm_sensitivity_win = sip.wrapinstance(self.fm_sensitivity.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._fm_sensitivity_win)
        self.blocks_socket_pdu_1_0 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '1235', 10000, False)
        self.blocks_socket_pdu_1 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '1234', 10000, False)
        self.blocks_socket_pdu_0_0 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '4322', 10000, False)
        self.blocks_socket_pdu_0 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '4321', 10000, False)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_socket_pdu_1, 'pdus'), (self.uhf_pdu_modulate_0, 'pdus'))
        self.msg_connect((self.blocks_socket_pdu_1_0, 'pdus'), (self.uhf_pdu_modulate_0, 'pdus'))
        self.msg_connect((self.uhf_pdu_demodulate_0, 'pdus'), (self.blocks_socket_pdu_0, 'pdus'))
        self.msg_connect((self.uhf_pdu_demodulate_0, 'pdus'), (self.blocks_socket_pdu_0_0, 'pdus'))
        self.msg_connect((self.uhf_pdu_demodulate_0, 'pdus'), (self.satellites_hexdump_sink_0, 'in'))
        self.connect((self.uhf_pdu_modulate_0, 1), (self.fm_sensitivity, 0))
        self.connect((self.uhf_pdu_modulate_0, 0), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.uhf_pdu_modulate_0, 0), (self.qtgui_time_sink_x_1, 0))
        self.connect((self.uhf_pdu_modulate_0, 0), (self.uhf_pdu_demodulate_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "uhf_mode5_loopback_nodoppler")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_spsym_frontend(self):
        return self.spsym_frontend

    def set_spsym_frontend(self, spsym_frontend):
        self.spsym_frontend = spsym_frontend
        self.set_spsym(self.spsym_frontend/self.oversample_rate)
        self.qtgui_time_sink_x_0.set_samp_rate(int(self.spsym_frontend*50))
        self.qtgui_time_sink_x_1.set_samp_rate(int(self.baud_bit*self.spsym_frontend*5))

    def get_oversample_rate(self):
        return self.oversample_rate

    def set_oversample_rate(self, oversample_rate):
        self.oversample_rate = oversample_rate
        self.set_spsym(self.spsym_frontend/self.oversample_rate)

    def get_spsym(self):
        return self.spsym

    def set_spsym(self, spsym):
        self.spsym = spsym
        self.set_samp_rate(self.baud_bit*self.spsym)

    def get_baud_bit(self):
        return self.baud_bit

    def set_baud_bit(self, baud_bit):
        self.baud_bit = baud_bit
        self.set_samp_rate(self.baud_bit*self.spsym)
        self.qtgui_time_sink_x_1.set_samp_rate(int(self.baud_bit*self.spsym_frontend*5))
        self.uhf_pdu_demodulate_0.set_baud_rate(self.baud_bit)

    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.uhf_pdu_demodulate_0.set_samp_rate(self.samp_rate)

    def get_rx_gain(self):
        return self.rx_gain

    def set_rx_gain(self, rx_gain):
        self.rx_gain = rx_gain

    def get_out_len(self):
        return self.out_len

    def set_out_len(self, out_len):
        self.out_len = out_len

    def get_in_len(self):
        return self.in_len

    def set_in_len(self, in_len):
        self.in_len = in_len

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq




def main(top_block_cls=uhf_mode5_loopback_nodoppler, options=None):
    if gr.enable_realtime_scheduling() != gr.RT_OK:
        print("Error: failed to enable real-time scheduling.")

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
