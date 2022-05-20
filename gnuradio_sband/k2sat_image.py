#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: K2SAT S-band image decoder
# Author: Daniel Estevez
# Description: K2SAT S-band image decoder
# GNU Radio version: 3.9.4.0

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

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import fec
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import uhd
import time
import satellites.hier



from gnuradio import qtgui

class k2sat_image(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "K2SAT S-band image decoder", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("K2SAT S-band image decoder")
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

        self.settings = Qt.QSettings("GNU Radio", "k2sat_image")

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
        self.samp_per_sym = samp_per_sym = 8
        self.nfilts = nfilts = 32
        self.baud_bit = baud_bit = 2e6
        self.syncword = syncword = "0101010101111110"
        self.samp_rate = samp_rate = baud_bit*(samp_per_sym/2)
        self.rx_gain = rx_gain = 42
        self.rrc_taps = rrc_taps = firdes.root_raised_cosine(nfilts,nfilts,1/float(samp_per_sym),0.35,11*samp_per_sym*nfilts)
        self.qpsk_const = qpsk_const = digital.constellation_rect([-0.707-0.707j, -0.707+0.707j, 0.707-0.707j, 0.707+0.707j], [0, 1, 2, 3],
        4, 2, 2, 1, 1).base()
        self.dec_cc = dec_cc = fec.cc_decoder.make(80,7, 2, [79, 109], 0, -1, fec.CC_STREAMING, False)
        self.center_freq = center_freq = 2.228e9

        ##################################################
        # Blocks
        ##################################################
        self.uhd_usrp_source_0 = uhd.usrp_source(
            ",".join(("", '')),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
        )
        self.uhd_usrp_source_0.set_samp_rate(samp_rate)
        self.uhd_usrp_source_0.set_time_now(uhd.time_spec(time.time()), uhd.ALL_MBOARDS)

        self.uhd_usrp_source_0.set_center_freq(center_freq, 0)
        self.uhd_usrp_source_0.set_antenna("TX/RX", 0)
        self.uhd_usrp_source_0.set_rx_agc(False, 0)
        self.uhd_usrp_source_0.set_gain(rx_gain, 0)
        self.satellites_sync_to_pdu_packed_0_0_1_0 = satellites.hier.sync_to_pdu_packed(
            packlen=50,
            sync='10111100101100001101101101011011',
            threshold=0,
        )
        self.satellites_sync_to_pdu_packed_0_0_1 = satellites.hier.sync_to_pdu_packed(
            packlen=50,
            sync='11011010110110110000110100111101',
            threshold=0,
        )
        self.qtgui_time_sink_x_0_0_0 = qtgui.time_sink_f(
            200, #size
            samp_rate/samp_per_sym, #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_0_0_0.set_update_time(0.2)
        self.qtgui_time_sink_x_0_0_0.set_y_axis(-1.5, 1.5)

        self.qtgui_time_sink_x_0_0_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0_0_0.enable_tags(True)
        self.qtgui_time_sink_x_0_0_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_0_0_0.enable_autoscale(False)
        self.qtgui_time_sink_x_0_0_0.enable_grid(False)
        self.qtgui_time_sink_x_0_0_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_0_0_0.enable_control_panel(False)
        self.qtgui_time_sink_x_0_0_0.enable_stem_plot(False)


        labels = ['Signal 1', 'Signal 2', 'Signal 3', 'Signal 4', 'Signal 5',
            'Signal 6', 'Signal 7', 'Signal 8', 'Signal 9', 'Signal 10']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ['blue', 'red', 'green', 'black', 'cyan',
            'magenta', 'yellow', 'dark red', 'dark green', 'dark blue']
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]
        styles = [0, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        markers = [0, -1, -1, -1, -1,
            -1, -1, -1, -1, -1]


        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_time_sink_x_0_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_0_0_0.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_0_0_0.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_0_0_0.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_0_0_0.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_0_0_0.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_0_0_0.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_0_0_0_win = sip.wrapinstance(self.qtgui_time_sink_x_0_0_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_0_0_0_win)
        self.qtgui_const_sink_x_0_0 = qtgui.const_sink_c(
            1024, #size
            'After costas', #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_const_sink_x_0_0.set_update_time(0.10)
        self.qtgui_const_sink_x_0_0.set_y_axis(-2, 2)
        self.qtgui_const_sink_x_0_0.set_x_axis(-2, 2)
        self.qtgui_const_sink_x_0_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, "")
        self.qtgui_const_sink_x_0_0.enable_autoscale(False)
        self.qtgui_const_sink_x_0_0.enable_grid(False)
        self.qtgui_const_sink_x_0_0.enable_axis_labels(True)


        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "red", "red", "red",
            "red", "red", "red", "red", "red"]
        styles = [0, 0, 0, 0, 0,
            0, 0, 0, 0, 0]
        markers = [0, 0, 0, 0, 0,
            0, 0, 0, 0, 0]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_const_sink_x_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_const_sink_x_0_0.set_line_label(i, labels[i])
            self.qtgui_const_sink_x_0_0.set_line_width(i, widths[i])
            self.qtgui_const_sink_x_0_0.set_line_color(i, colors[i])
            self.qtgui_const_sink_x_0_0.set_line_style(i, styles[i])
            self.qtgui_const_sink_x_0_0.set_line_marker(i, markers[i])
            self.qtgui_const_sink_x_0_0.set_line_alpha(i, alphas[i])

        self._qtgui_const_sink_x_0_0_win = sip.wrapinstance(self.qtgui_const_sink_x_0_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_const_sink_x_0_0_win)
        self.fec_extended_decoder_0_0_1 = fec.extended_decoder(decoder_obj_list=dec_cc, threading= None, ann=None, puncpat='11', integration_period=10000)
        self.digital_pfb_clock_sync_xxx_0_0 = digital.pfb_clock_sync_ccf(samp_per_sym, 0.01, rrc_taps, nfilts, 0, 0.1, 1)
        self.digital_map_bb_1 = digital.map_bb([-1, 1])
        self.digital_map_bb_0 = digital.map_bb([0, 1, 2, 3])
        self.digital_diff_decoder_bb_0_1 = digital.diff_decoder_bb(2, digital.DIFF_DIFFERENTIAL)
        self.digital_costas_loop_cc_1 = digital.costas_loop_cc(0.01, 4, False)
        self.digital_constellation_decoder_cb_0 = digital.constellation_decoder_cb(qpsk_const)
        self.blocks_unpack_k_bits_bb_0 = blocks.unpack_k_bits_bb(2)
        self.blocks_socket_pdu_0 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '4321', 10000, False)
        self.blocks_multiply_const_vxx_1 = blocks.multiply_const_cc(1)
        self.blocks_char_to_float_1 = blocks.char_to_float(1, 1)
        self.blocks_char_to_float_0 = blocks.char_to_float(1, 1)



        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.satellites_sync_to_pdu_packed_0_0_1, 'out'), (self.blocks_socket_pdu_0, 'pdus'))
        self.msg_connect((self.satellites_sync_to_pdu_packed_0_0_1_0, 'out'), (self.blocks_socket_pdu_0, 'pdus'))
        self.connect((self.blocks_char_to_float_0, 0), (self.fec_extended_decoder_0_0_1, 0))
        self.connect((self.blocks_char_to_float_1, 0), (self.qtgui_time_sink_x_0_0_0, 0))
        self.connect((self.blocks_multiply_const_vxx_1, 0), (self.digital_costas_loop_cc_1, 0))
        self.connect((self.blocks_unpack_k_bits_bb_0, 0), (self.digital_map_bb_1, 0))
        self.connect((self.digital_constellation_decoder_cb_0, 0), (self.digital_map_bb_0, 0))
        self.connect((self.digital_costas_loop_cc_1, 0), (self.digital_pfb_clock_sync_xxx_0_0, 0))
        self.connect((self.digital_diff_decoder_bb_0_1, 0), (self.blocks_char_to_float_1, 0))
        self.connect((self.digital_diff_decoder_bb_0_1, 0), (self.satellites_sync_to_pdu_packed_0_0_1, 0))
        self.connect((self.digital_diff_decoder_bb_0_1, 0), (self.satellites_sync_to_pdu_packed_0_0_1_0, 0))
        self.connect((self.digital_map_bb_0, 0), (self.blocks_unpack_k_bits_bb_0, 0))
        self.connect((self.digital_map_bb_1, 0), (self.blocks_char_to_float_0, 0))
        self.connect((self.digital_pfb_clock_sync_xxx_0_0, 0), (self.digital_constellation_decoder_cb_0, 0))
        self.connect((self.digital_pfb_clock_sync_xxx_0_0, 0), (self.qtgui_const_sink_x_0_0, 0))
        self.connect((self.fec_extended_decoder_0_0_1, 0), (self.digital_diff_decoder_bb_0_1, 0))
        self.connect((self.uhd_usrp_source_0, 0), (self.blocks_multiply_const_vxx_1, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "k2sat_image")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_samp_per_sym(self):
        return self.samp_per_sym

    def set_samp_per_sym(self, samp_per_sym):
        self.samp_per_sym = samp_per_sym
        self.set_rrc_taps(firdes.root_raised_cosine(self.nfilts,self.nfilts,1/float(self.samp_per_sym),0.35,11*self.samp_per_sym*self.nfilts))
        self.set_samp_rate(self.baud_bit*(self.samp_per_sym/2))
        self.qtgui_time_sink_x_0_0_0.set_samp_rate(self.samp_rate/self.samp_per_sym)

    def get_nfilts(self):
        return self.nfilts

    def set_nfilts(self, nfilts):
        self.nfilts = nfilts
        self.set_rrc_taps(firdes.root_raised_cosine(self.nfilts,self.nfilts,1/float(self.samp_per_sym),0.35,11*self.samp_per_sym*self.nfilts))

    def get_baud_bit(self):
        return self.baud_bit

    def set_baud_bit(self, baud_bit):
        self.baud_bit = baud_bit
        self.set_samp_rate(self.baud_bit*(self.samp_per_sym/2))

    def get_syncword(self):
        return self.syncword

    def set_syncword(self, syncword):
        self.syncword = syncword

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.qtgui_time_sink_x_0_0_0.set_samp_rate(self.samp_rate/self.samp_per_sym)
        self.uhd_usrp_source_0.set_samp_rate(self.samp_rate)

    def get_rx_gain(self):
        return self.rx_gain

    def set_rx_gain(self, rx_gain):
        self.rx_gain = rx_gain
        self.uhd_usrp_source_0.set_gain(self.rx_gain, 0)

    def get_rrc_taps(self):
        return self.rrc_taps

    def set_rrc_taps(self, rrc_taps):
        self.rrc_taps = rrc_taps
        self.digital_pfb_clock_sync_xxx_0_0.update_taps(self.rrc_taps)

    def get_qpsk_const(self):
        return self.qpsk_const

    def set_qpsk_const(self, qpsk_const):
        self.qpsk_const = qpsk_const

    def get_dec_cc(self):
        return self.dec_cc

    def set_dec_cc(self, dec_cc):
        self.dec_cc = dec_cc

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.uhd_usrp_source_0.set_center_freq(self.center_freq, 0)




def main(top_block_cls=k2sat_image, options=None):

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
