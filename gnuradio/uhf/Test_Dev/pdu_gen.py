#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
# Author: knud
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
from gnuradio import gr, pdu
from gnuradio import pdu
import numpy as np
import pdu_gen_epy_block_0 as epy_block_0  # embedded python block



from gnuradio import qtgui

class pdu_gen(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Not titled yet", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Not titled yet")
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

        self.settings = Qt.QSettings("GNU Radio", "pdu_gen")

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
        self.mpdu_header_len = mpdu_header_len = 3*3
        self.max_pipe_payload_len = max_pipe_payload_len = 128
        self.samp_rate = samp_rate = 32000
        self.mpdu_payload_len = mpdu_payload_len = max_pipe_payload_len- mpdu_header_len
        self.interval = interval = 0.400
        self.header_len = header_len = 7
        self.fec_rate = fec_rate = 1/2

        ##################################################
        # Blocks
        ##################################################
        self.qtgui_time_sink_x_0 = qtgui.time_sink_f(
            (max_pipe_payload_len+header_len)*8*2, #size
            samp_rate, #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_0.set_update_time(0.10)
        self.qtgui_time_sink_x_0.set_y_axis(-0.1, 1.2)

        self.qtgui_time_sink_x_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0.enable_tags(True)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_TAG, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "packet_len")
        self.qtgui_time_sink_x_0.enable_autoscale(False)
        self.qtgui_time_sink_x_0.enable_grid(False)
        self.qtgui_time_sink_x_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_0.enable_control_panel(False)
        self.qtgui_time_sink_x_0.enable_stem_plot(True)


        labels = ['source packet', 'Signal 2', 'Signal 3', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [0, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_0.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_0.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_0_win = sip.wrapinstance(self.qtgui_time_sink_x_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_0_win)
        self.pdu_tagged_stream_to_pdu_0 = pdu.tagged_stream_to_pdu(gr.types.byte_t, 'packet_len')
        self.pdu_random_pdu_0 = pdu.random_pdu(int(np.floor(mpdu_payload_len*fec_rate)), int(np.floor(mpdu_payload_len*fec_rate)), 0xFF, 2)
        self.pdu_pdu_to_tagged_stream_0 = pdu.pdu_to_tagged_stream(gr.types.byte_t, 'packet_len')
        self.epy_block_0 = epy_block_0.blk()
        self.blocks_unpack_k_bits_bb_0 = blocks.unpack_k_bits_bb(8)
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_char*1, samp_rate,True)
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.cons(pmt.make_dict(), pmt.init_u8vector(10, (1,2,3,4,5,6,7,8,9,10))), 1000)
        self.blocks_message_debug_0 = blocks.message_debug(True)
        self.blocks_char_to_float_0 = blocks.char_to_float(1, 1)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.pdu_random_pdu_0, 'generate'))
        self.msg_connect((self.epy_block_0, 'pdu_out'), (self.pdu_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.pdu_random_pdu_0, 'pdus'), (self.epy_block_0, 'pdu_in'))
        self.msg_connect((self.pdu_tagged_stream_to_pdu_0, 'pdus'), (self.blocks_message_debug_0, 'print_pdu'))
        self.connect((self.blocks_char_to_float_0, 0), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.pdu_tagged_stream_to_pdu_0, 0))
        self.connect((self.blocks_unpack_k_bits_bb_0, 0), (self.blocks_char_to_float_0, 0))
        self.connect((self.pdu_pdu_to_tagged_stream_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.pdu_pdu_to_tagged_stream_0, 0), (self.blocks_unpack_k_bits_bb_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "pdu_gen")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_mpdu_header_len(self):
        return self.mpdu_header_len

    def set_mpdu_header_len(self, mpdu_header_len):
        self.mpdu_header_len = mpdu_header_len
        self.set_mpdu_payload_len(self.max_pipe_payload_len- self.mpdu_header_len)

    def get_max_pipe_payload_len(self):
        return self.max_pipe_payload_len

    def set_max_pipe_payload_len(self, max_pipe_payload_len):
        self.max_pipe_payload_len = max_pipe_payload_len
        self.set_mpdu_payload_len(self.max_pipe_payload_len- self.mpdu_header_len)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)
        self.qtgui_time_sink_x_0.set_samp_rate(self.samp_rate)

    def get_mpdu_payload_len(self):
        return self.mpdu_payload_len

    def set_mpdu_payload_len(self, mpdu_payload_len):
        self.mpdu_payload_len = mpdu_payload_len

    def get_interval(self):
        return self.interval

    def set_interval(self, interval):
        self.interval = interval

    def get_header_len(self):
        return self.header_len

    def set_header_len(self, header_len):
        self.header_len = header_len

    def get_fec_rate(self):
        return self.fec_rate

    def set_fec_rate(self, fec_rate):
        self.fec_rate = fec_rate




def main(top_block_cls=pdu_gen, options=None):

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
