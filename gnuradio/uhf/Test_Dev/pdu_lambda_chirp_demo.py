#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: PDU Lambda Chirp Demo
# Author: J. Gilbert
# Copyright: J. Gilbert
# Description: Use of PDU Lambda block
# GNU Radio version: 3.10.2.0

from packaging.version import Version as StrictVersion

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print("Warning: failed to XInitThreads()")

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import blocks
import pmt
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import pdu
import pmt, numpy as np
from gnuradio.qtgui import Range, RangeWidget
from PyQt5 import QtCore



from gnuradio import qtgui

class pdu_lambda_chirp_demo(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "PDU Lambda Chirp Demo", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("PDU Lambda Chirp Demo")
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

        self.settings = Qt.QSettings("GNU Radio", "pdu_lambda_chirp_demo")

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
        self.N = N = 1024*4
        self.vec = vec = pmt.init_f32vector(N, np.arange(N))
        self.samp_rate = samp_rate = 1000000
        self.k = k = 0.5

        ##################################################
        # Blocks
        ##################################################
        self._k_range = Range(0.1, 2, 0.01, 0.5, 200)
        self._k_win = RangeWidget(self._k_range, self.set_k, "Gain", "counter_slider", float, QtCore.Qt.Horizontal)
        self.top_layout.addWidget(self._k_win)
        self.waterfall = qtgui.waterfall_sink_c(
            128, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            0, #fc
            samp_rate, #bw
            "", #name
            0, #number of inputs
            None # parent
        )
        self.waterfall.set_update_time(0.10)
        self.waterfall.enable_grid(False)
        self.waterfall.enable_axis_labels(True)



        labels = ['', '', '', '', '',
                  '', '', '', '', '']
        colors = [0, 0, 0, 0, 0,
                  0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
                  1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.waterfall.set_line_label(i, "Data {0}".format(i))
            else:
                self.waterfall.set_line_label(i, labels[i])
            self.waterfall.set_color_map(i, colors[i])
            self.waterfall.set_line_alpha(i, alphas[i])

        self.waterfall.set_intensity_range(-140, 10)

        self._waterfall_win = sip.wrapinstance(self.waterfall.qwidget(), Qt.QWidget)

        self.top_grid_layout.addWidget(self._waterfall_win, 0, 0, 2, 1)
        for r in range(0, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.time_sink = qtgui.time_sink_c(
            1024, #size
            samp_rate, #samp_rate
            "", #name
            0, #number of inputs
            None # parent
        )
        self.time_sink.set_update_time(0.10)
        self.time_sink.set_y_axis(-1, 1)

        self.time_sink.set_y_label('Amplitude', "")

        self.time_sink.enable_tags(True)
        self.time_sink.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.time_sink.enable_autoscale(False)
        self.time_sink.enable_grid(False)
        self.time_sink.enable_axis_labels(True)
        self.time_sink.enable_control_panel(False)
        self.time_sink.enable_stem_plot(False)


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
                    self.time_sink.set_line_label(i, "Re{{Data {0}}}".format(i/2))
                else:
                    self.time_sink.set_line_label(i, "Im{{Data {0}}}".format(i/2))
            else:
                self.time_sink.set_line_label(i, labels[i])
            self.time_sink.set_line_width(i, widths[i])
            self.time_sink.set_line_color(i, colors[i])
            self.time_sink.set_line_style(i, styles[i])
            self.time_sink.set_line_marker(i, markers[i])
            self.time_sink.set_line_alpha(i, alphas[i])

        self._time_sink_win = sip.wrapinstance(self.time_sink.qwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._time_sink_win, 0, 1, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.strobe = blocks.message_strobe(pmt.cons(pmt.PMT_NIL, vec), 250)
        self.stream_time_sink = qtgui.time_sink_c(
            N*2, #size
            samp_rate, #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.stream_time_sink.set_update_time(0.10)
        self.stream_time_sink.set_y_axis(-1, 1)

        self.stream_time_sink.set_y_label('Amplitude', "")

        self.stream_time_sink.enable_tags(True)
        self.stream_time_sink.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.stream_time_sink.enable_autoscale(False)
        self.stream_time_sink.enable_grid(False)
        self.stream_time_sink.enable_axis_labels(True)
        self.stream_time_sink.enable_control_panel(False)
        self.stream_time_sink.enable_stem_plot(False)


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
                    self.stream_time_sink.set_line_label(i, "Re{{Data {0}}}".format(i/2))
                else:
                    self.stream_time_sink.set_line_label(i, "Im{{Data {0}}}".format(i/2))
            else:
                self.stream_time_sink.set_line_label(i, labels[i])
            self.stream_time_sink.set_line_width(i, widths[i])
            self.stream_time_sink.set_line_color(i, colors[i])
            self.stream_time_sink.set_line_style(i, styles[i])
            self.stream_time_sink.set_line_marker(i, markers[i])
            self.stream_time_sink.set_line_alpha(i, alphas[i])

        self._stream_time_sink_win = sip.wrapinstance(self.stream_time_sink.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._stream_time_sink_win)
        self.pdu2stream = pdu.pdu_to_stream_c(pdu.EARLY_BURST_APPEND, 64)
        self.lambda2 = pdu.pdu_lambda(lambda x: np.float32(np.unwrap(np.angle(x[1:]) - np.angle(x[:-1]))), "UVEC", pmt.intern("key"))
        self.lambda1 = pdu.pdu_lambda(lambda x: k * np.complex64(np.exp(2*np.pi * 1j  * x * x * (0.05 * (np.random.randint(40)-19.5) / len(x)) )), "UVEC", pmt.intern("key"))
        self.demod_time_sink = qtgui.time_sink_f(
            1024, #size
            samp_rate, #samp_rate
            "", #name
            0, #number of inputs
            None # parent
        )
        self.demod_time_sink.set_update_time(0.10)
        self.demod_time_sink.set_y_axis(-5, 5)

        self.demod_time_sink.set_y_label('Amplitude', "")

        self.demod_time_sink.enable_tags(True)
        self.demod_time_sink.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.demod_time_sink.enable_autoscale(False)
        self.demod_time_sink.enable_grid(False)
        self.demod_time_sink.enable_axis_labels(True)
        self.demod_time_sink.enable_control_panel(False)
        self.demod_time_sink.enable_stem_plot(False)


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


        for i in range(1):
            if len(labels[i]) == 0:
                self.demod_time_sink.set_line_label(i, "Data {0}".format(i))
            else:
                self.demod_time_sink.set_line_label(i, labels[i])
            self.demod_time_sink.set_line_width(i, widths[i])
            self.demod_time_sink.set_line_color(i, colors[i])
            self.demod_time_sink.set_line_style(i, styles[i])
            self.demod_time_sink.set_line_marker(i, markers[i])
            self.demod_time_sink.set_line_alpha(i, alphas[i])

        self._demod_time_sink_win = sip.wrapinstance(self.demod_time_sink.qwidget(), Qt.QWidget)
        self.top_grid_layout.addWidget(self._demod_time_sink_win, 1, 1, 1, 1)
        for r in range(1, 2):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.lambda1, 'pdu'), (self.lambda2, 'pdu'))
        self.msg_connect((self.lambda1, 'pdu'), (self.pdu2stream, 'pdus'))
        self.msg_connect((self.lambda1, 'pdu'), (self.time_sink, 'in'))
        self.msg_connect((self.lambda1, 'pdu'), (self.waterfall, 'in'))
        self.msg_connect((self.lambda2, 'pdu'), (self.demod_time_sink, 'in'))
        self.msg_connect((self.strobe, 'strobe'), (self.lambda1, 'pdu'))
        self.connect((self.pdu2stream, 0), (self.stream_time_sink, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "pdu_lambda_chirp_demo")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_N(self):
        return self.N

    def set_N(self, N):
        self.N = N
        self.set_vec(pmt.init_f32vector(self.N, np.arange(self.N)))

    def get_vec(self):
        return self.vec

    def set_vec(self, vec):
        self.vec = vec
        self.strobe.set_msg(pmt.cons(pmt.PMT_NIL, self.vec))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.demod_time_sink.set_samp_rate(self.samp_rate)
        self.stream_time_sink.set_samp_rate(self.samp_rate)
        self.time_sink.set_samp_rate(self.samp_rate)
        self.waterfall.set_frequency_range(0, self.samp_rate)

    def get_k(self):
        return self.k

    def set_k(self, k):
        self.k = k
        self.lambda1.set_fn(lambda x: self.k * np.complex64(np.exp(2*np.pi * 1j  * x * x * (0.05 * (np.random.randint(40)-19.5) / len(x)) )))




def main(top_block_cls=pdu_lambda_chirp_demo, options=None):

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
