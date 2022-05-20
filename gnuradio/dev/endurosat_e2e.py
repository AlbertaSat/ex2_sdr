#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
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
import pmt
from gnuradio import digital
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import uhd
import time
from gnuradio import zeromq
import satellites.components.datasinks
import satellites.components.deframers
import satellites.components.demodulators
import satellites.hier



from gnuradio import qtgui

class endurosat_e2e(gr.top_block, Qt.QWidget):

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

        self.settings = Qt.QSettings("GNU Radio", "endurosat_e2e")

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
        self.spsym = spsym = 100
        self.fsk_dev = fsk_dev = 4800
        self.baud_bit = baud_bit = 19200
        self.tx_gain = tx_gain = 1
        self.sensitivity = sensitivity = 2*3.14159*(fsk_dev/(baud_bit*spsym))
        self.rx_gain = rx_gain = 40
        self.center_freq = center_freq = 437875000
        self.baud_byte = baud_byte = baud_bit/8

        ##################################################
        # Blocks
        ##################################################
        self.zeromq_pub_msg_sink_0 = zeromq.pub_msg_sink('tcp://127.0.0.1:4321', 100, True)
        self.uhd_usrp_source_0_0 = uhd.usrp_source(
            ",".join(("", "")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
        )
        self.uhd_usrp_source_0_0.set_samp_rate(baud_bit*spsym)
        self.uhd_usrp_source_0_0.set_time_now(uhd.time_spec(time.time()), uhd.ALL_MBOARDS)

        self.uhd_usrp_source_0_0.set_center_freq(center_freq, 0)
        self.uhd_usrp_source_0_0.set_antenna('RX2', 0)
        self.uhd_usrp_source_0_0.set_rx_agc(False, 0)
        self.uhd_usrp_source_0_0.set_gain(rx_gain, 0)
        self.uhd_usrp_sink_0 = uhd.usrp_sink(
            ",".join(("", "")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
            '',
        )
        self.uhd_usrp_sink_0.set_samp_rate(baud_bit*spsym)
        self.uhd_usrp_sink_0.set_time_now(uhd.time_spec(time.time()), uhd.ALL_MBOARDS)

        self.uhd_usrp_sink_0.set_center_freq(center_freq, 0)
        self.uhd_usrp_sink_0.set_antenna('TX/RX', 0)
        self.uhd_usrp_sink_0.set_normalized_gain(tx_gain, 0)
        self.satellites_sync_to_pdu_packed_0_0 = satellites.hier.sync_to_pdu_packed(
            packlen=129,
            sync='01111110',
            threshold=0,
        )
        self.satellites_rms_agc_0 = satellites.hier.rms_agc(alpha=1e-2, reference=1.0)
        self.satellites_hexdump_sink_0 = satellites.components.datasinks.hexdump_sink(options="")
        self.satellites_fsk_demodulator_0 = satellites.components.demodulators.fsk_demodulator(baudrate = baud_bit, samp_rate = spsym*baud_bit, iq = True, subaudio = False, options="")
        self.satellites_ax25_deframer_0 = satellites.components.deframers.ax25_deframer(g3ruh_scrambler=True, options="")
        self.qtgui_time_sink_x_1 = qtgui.time_sink_c(
            256000*2, #size
            256000, #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_1.set_update_time(2)
        self.qtgui_time_sink_x_1.set_y_axis(1, -1)

        self.qtgui_time_sink_x_1.set_y_label('Received Amplitude', "")

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
            spsym*50, #size
            spsym*50, #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_0.set_update_time(1)
        self.qtgui_time_sink_x_0.set_y_axis(-0.1, 0.1)

        self.qtgui_time_sink_x_0.set_y_label('Transmitted Amplitude', "")

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
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
            10*1024, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            center_freq, #fc
            2*baud_bit*spsym, #bw
            "", #name
            1,
            None # parent
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0.set_y_axis(-140, 10)
        self.qtgui_freq_sink_x_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(False)
        self.qtgui_freq_sink_x_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0.enable_control_panel(False)
        self.qtgui_freq_sink_x_0.set_fft_window_normalized(False)



        labels = ['', '', '', '', '',
            '', '', '', '', '']
        widths = [1, 1, 1, 1, 1,
            1, 1, 1, 1, 1]
        colors = ["blue", "red", "green", "black", "cyan",
            "magenta", "yellow", "dark red", "dark green", "dark blue"]
        alphas = [1.0, 1.0, 1.0, 1.0, 1.0,
            1.0, 1.0, 1.0, 1.0, 1.0]

        for i in range(1):
            if len(labels[i]) == 0:
                self.qtgui_freq_sink_x_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_0.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_freq_sink_x_0_win)
        self.digital_gfsk_mod_0 = digital.gfsk_mod(
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
        self.blocks_unpacked_to_packed_xx_0 = blocks.unpacked_to_packed_bb(8, gr.GR_MSB_FIRST)
        self.blocks_socket_pdu_1 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '1234', 10000, False)
        self.blocks_pdu_to_tagged_stream_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, 'packet_len')
        self.blocks_message_strobe_0_0_0 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'DDR', 'value': 1, 'mask': ((1 << 11) - 1)})), 8500)
        self.blocks_message_strobe_0_0 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'CTRL', 'value': 1, 'mask': ((1 << 11) - 1)})), 7000)
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'ATR_TX', 'value': 1, 'mask': ((1 << 11) - 1)})), 10000)
        self.analog_simple_squelch_cc_0 = analog.simple_squelch_cc(-40, 1)



        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_0_0, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_socket_pdu_1, 'pdus'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.satellites_ax25_deframer_0, 'out'), (self.satellites_hexdump_sink_0, 'in'))
        self.msg_connect((self.satellites_sync_to_pdu_packed_0_0, 'out'), (self.satellites_hexdump_sink_0, 'in'))
        self.msg_connect((self.satellites_sync_to_pdu_packed_0_0, 'out'), (self.zeromq_pub_msg_sink_0, 'in'))
        self.connect((self.analog_simple_squelch_cc_0, 0), (self.satellites_fsk_demodulator_0, 0))
        self.connect((self.analog_simple_squelch_cc_0, 0), (self.satellites_rms_agc_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.blocks_unpacked_to_packed_xx_0, 0))
        self.connect((self.blocks_unpacked_to_packed_xx_0, 0), (self.digital_gfsk_mod_0, 0))
        self.connect((self.digital_gfsk_demod_0, 0), (self.satellites_sync_to_pdu_packed_0_0, 0))
        self.connect((self.digital_gfsk_mod_0, 0), (self.qtgui_freq_sink_x_0, 0))
        self.connect((self.digital_gfsk_mod_0, 0), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.digital_gfsk_mod_0, 0), (self.uhd_usrp_sink_0, 0))
        self.connect((self.satellites_fsk_demodulator_0, 0), (self.satellites_ax25_deframer_0, 0))
        self.connect((self.satellites_rms_agc_0, 0), (self.digital_gfsk_demod_0, 0))
        self.connect((self.uhd_usrp_source_0_0, 0), (self.analog_simple_squelch_cc_0, 0))
        self.connect((self.uhd_usrp_source_0_0, 0), (self.qtgui_time_sink_x_1, 0))


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
        self.qtgui_freq_sink_x_0.set_frequency_range(self.center_freq, 2*self.baud_bit*self.spsym)
        self.qtgui_time_sink_x_0.set_samp_rate(self.spsym*50)
        self.uhd_usrp_sink_0.set_samp_rate(self.baud_bit*self.spsym)
        self.uhd_usrp_source_0_0.set_samp_rate(self.baud_bit*self.spsym)

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
        self.qtgui_freq_sink_x_0.set_frequency_range(self.center_freq, 2*self.baud_bit*self.spsym)
        self.uhd_usrp_sink_0.set_samp_rate(self.baud_bit*self.spsym)
        self.uhd_usrp_source_0_0.set_samp_rate(self.baud_bit*self.spsym)

    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain
        self.uhd_usrp_sink_0.set_normalized_gain(self.tx_gain, 0)

    def get_sensitivity(self):
        return self.sensitivity

    def set_sensitivity(self, sensitivity):
        self.sensitivity = sensitivity

    def get_rx_gain(self):
        return self.rx_gain

    def set_rx_gain(self, rx_gain):
        self.rx_gain = rx_gain
        self.uhd_usrp_source_0_0.set_gain(self.rx_gain, 0)

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.qtgui_freq_sink_x_0.set_frequency_range(self.center_freq, 2*self.baud_bit*self.spsym)
        self.uhd_usrp_sink_0.set_center_freq(self.center_freq, 0)
        self.uhd_usrp_source_0_0.set_center_freq(self.center_freq, 0)

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
