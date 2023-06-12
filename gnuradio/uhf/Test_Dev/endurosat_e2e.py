#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
# GNU Radio version: 3.10.6.0

from packaging.version import Version as StrictVersion
from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio import blocks
import pmt
from gnuradio import digital
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import gr, pdu
from gnuradio import pdu
import endurosat_e2e_epy_block_0 as epy_block_0  # embedded python block
import numpy as np
import satellites.hier
import sip



class endurosat_e2e(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Not titled yet", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Not titled yet")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except BaseException as exc:
            print(f"Qt GUI: Could not set Icon: {str(exc)}", file=sys.stderr)
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

        self.settings = Qt.QSettings("GNU Radio", "endurosat_e2e")

        try:
            if StrictVersion(Qt.qVersion()) < StrictVersion("5.0.0"):
                self.restoreGeometry(self.settings.value("geometry").toByteArray())
            else:
                self.restoreGeometry(self.settings.value("geometry"))
        except BaseException as exc:
            print(f"Qt GUI: Could not restore geometry: {str(exc)}", file=sys.stderr)

        ##################################################
        # Variables
        ##################################################
        self.spsym = spsym = 100
        self.mpdu_header_len = mpdu_header_len = 3*3
        self.max_pipe_payload_len = max_pipe_payload_len = 128
        self.fsk_dev = fsk_dev = 4800
        self.baud_bit = baud_bit = 19200
        self.tx_gain = tx_gain = 0.8
        self.sensitivity = sensitivity = 2*3.14159*(fsk_dev/(baud_bit*spsym))
        self.rx_gain = rx_gain = 20
        self.mpdu_payload_len = mpdu_payload_len = max_pipe_payload_len- mpdu_header_len
        self.interval_ms = interval_ms = 1000
        self.fec_rate = fec_rate = 1/2
        self.center_freq = center_freq = 435000000
        self.baud_byte = baud_byte = baud_bit/8

        ##################################################
        # Blocks
        ##################################################

        self.satellites_sync_to_pdu_packed_0_0 = satellites.hier.sync_to_pdu_packed(
            packlen=(int(np.floor(mpdu_payload_len*fec_rate))),
            sync='0111111000111011',
            threshold=0,
        )
        self.qtgui_time_sink_x_0_0 = qtgui.time_sink_f(
            int(100), #size
            baud_bit*10, #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_0_0.set_update_time(0.5)
        self.qtgui_time_sink_x_0_0.set_y_axis(-0.5, 1.5)

        self.qtgui_time_sink_x_0_0.set_y_label('Bitstream', "")

        self.qtgui_time_sink_x_0_0.enable_tags(False)
        self.qtgui_time_sink_x_0_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "")
        self.qtgui_time_sink_x_0_0.enable_autoscale(True)
        self.qtgui_time_sink_x_0_0.enable_grid(False)
        self.qtgui_time_sink_x_0_0.enable_axis_labels(True)
        self.qtgui_time_sink_x_0_0.enable_control_panel(False)
        self.qtgui_time_sink_x_0_0.enable_stem_plot(True)


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
                self.qtgui_time_sink_x_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_time_sink_x_0_0.set_line_label(i, labels[i])
            self.qtgui_time_sink_x_0_0.set_line_width(i, widths[i])
            self.qtgui_time_sink_x_0_0.set_line_color(i, colors[i])
            self.qtgui_time_sink_x_0_0.set_line_style(i, styles[i])
            self.qtgui_time_sink_x_0_0.set_line_marker(i, markers[i])
            self.qtgui_time_sink_x_0_0.set_line_alpha(i, alphas[i])

        self._qtgui_time_sink_x_0_0_win = sip.wrapinstance(self.qtgui_time_sink_x_0_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_time_sink_x_0_0_win)
        self.pdu_random_pdu_0 = pdu.random_pdu((int(np.floor(mpdu_payload_len*fec_rate))), (int(np.floor(mpdu_payload_len*fec_rate))), 0xFF, 1)
        self.pdu_pdu_to_tagged_stream_0 = pdu.pdu_to_tagged_stream(gr.types.byte_t, 'packet_len')
        self.epy_block_0 = epy_block_0.blk()
        self.digital_gfsk_mod_0_0 = digital.gfsk_mod(
            samples_per_symbol=spsym,
            sensitivity=sensitivity,
            bt=0.5,
            verbose=False,
            log=False,
            do_unpack=True)
        self.digital_gfsk_demod_0 = digital.gfsk_demod(
            samples_per_symbol=spsym,
            sensitivity=sensitivity,
            gain_mu=0.175,
            mu=0.5,
            omega_relative_limit=0.005,
            freq_error=0.0,
            verbose=False,
            log=False)
        self.blocks_uchar_to_float_0_0 = blocks.uchar_to_float()
        self.blocks_throttle2_0 = blocks.throttle( gr.sizeof_char*1, baud_bit, True, 0 if "auto" == "auto" else max( int(float(0.1) * baud_bit) if "auto" == "time" else int(0.1), 1) )
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.cons(pmt.make_dict(), pmt.init_u8vector(10, (1,2,3,4,5,6,7,8,9,10))), interval_ms)
        self.blocks_message_debug_0_0 = blocks.message_debug(True)
        self.blocks_message_debug_0 = blocks.message_debug(True)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.pdu_random_pdu_0, 'generate'))
        self.msg_connect((self.epy_block_0, 'pdu_out'), (self.blocks_message_debug_0_0, 'print'))
        self.msg_connect((self.epy_block_0, 'pdu_out'), (self.pdu_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.pdu_random_pdu_0, 'pdus'), (self.epy_block_0, 'pdu_in'))
        self.msg_connect((self.satellites_sync_to_pdu_packed_0_0, 'out'), (self.blocks_message_debug_0, 'print'))
        self.connect((self.blocks_throttle2_0, 0), (self.digital_gfsk_mod_0_0, 0))
        self.connect((self.blocks_uchar_to_float_0_0, 0), (self.qtgui_time_sink_x_0_0, 0))
        self.connect((self.digital_gfsk_demod_0, 0), (self.blocks_uchar_to_float_0_0, 0))
        self.connect((self.digital_gfsk_demod_0, 0), (self.satellites_sync_to_pdu_packed_0_0, 0))
        self.connect((self.digital_gfsk_mod_0_0, 0), (self.digital_gfsk_demod_0, 0))
        self.connect((self.pdu_pdu_to_tagged_stream_0, 0), (self.blocks_throttle2_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "endurosat_e2e")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_spsym(self):
        return self.spsym

    def set_spsym(self, spsym):
        self.spsym = spsym
        self.set_sensitivity(2*3.14159*(self.fsk_dev/(self.baud_bit*self.spsym)))

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

    def get_fsk_dev(self):
        return self.fsk_dev

    def set_fsk_dev(self, fsk_dev):
        self.fsk_dev = fsk_dev
        self.set_sensitivity(2*3.14159*(self.fsk_dev/(self.baud_bit*self.spsym)))

    def get_baud_bit(self):
        return self.baud_bit

    def set_baud_bit(self, baud_bit):
        self.baud_bit = baud_bit
        self.set_baud_byte(self.baud_bit/8)
        self.set_sensitivity(2*3.14159*(self.fsk_dev/(self.baud_bit*self.spsym)))
        self.blocks_throttle2_0.set_sample_rate(self.baud_bit)
        self.qtgui_time_sink_x_0_0.set_samp_rate(self.baud_bit*10)

    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain

    def get_sensitivity(self):
        return self.sensitivity

    def set_sensitivity(self, sensitivity):
        self.sensitivity = sensitivity

    def get_rx_gain(self):
        return self.rx_gain

    def set_rx_gain(self, rx_gain):
        self.rx_gain = rx_gain

    def get_mpdu_payload_len(self):
        return self.mpdu_payload_len

    def set_mpdu_payload_len(self, mpdu_payload_len):
        self.mpdu_payload_len = mpdu_payload_len
        self.satellites_sync_to_pdu_packed_0_0.set_packlen((int(np.floor(self.mpdu_payload_len*self.fec_rate))))

    def get_interval_ms(self):
        return self.interval_ms

    def set_interval_ms(self, interval_ms):
        self.interval_ms = interval_ms
        self.blocks_message_strobe_0.set_period(self.interval_ms)

    def get_fec_rate(self):
        return self.fec_rate

    def set_fec_rate(self, fec_rate):
        self.fec_rate = fec_rate
        self.satellites_sync_to_pdu_packed_0_0.set_packlen((int(np.floor(self.mpdu_payload_len*self.fec_rate))))

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq

    def get_baud_byte(self):
        return self.baud_byte

    def set_baud_byte(self, baud_byte):
        self.baud_byte = baud_byte




def main(top_block_cls=endurosat_e2e, options=None):

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
