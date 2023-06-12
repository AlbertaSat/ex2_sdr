#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
# Author: knud
# GNU Radio version: 3.10.6.0

from packaging.version import Version as StrictVersion
from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio import blocks
import pmt
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import gr, pdu
from gnuradio import pdu
import GS_baseband_to_file_epy_block_0 as epy_block_0  # embedded python block
import numpy as np
import sip



class GS_baseband_to_file(gr.top_block, Qt.QWidget):

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

        self.settings = Qt.QSettings("GNU Radio", "GS_baseband_to_file")

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
        self.es_mode = es_mode = 6
        self.samples_per_symbol = samples_per_symbol = 100
        self.mpdu_header_len = mpdu_header_len = 3*3
        self.max_pipe_payload_len = max_pipe_payload_len = 128
        self.freq_dev = freq_dev = {es_mode == 0: 600, es_mode==1: 600, es_mode==2:1200,es_mode==3:2400,es_mode==4:4800,es_mode==5:4800,es_mode==6:9600}.get(True,19200)
        self.data_rate = data_rate = {es_mode == 0: 1200, es_mode==1: 2400, es_mode==2:4800,es_mode==3:9600,es_mode==4:9600}.get(True,19200)
        self.sensitivity_tx = sensitivity_tx = 2*np.math.pi*(freq_dev/(data_rate*samples_per_symbol))
        self.mpdu_payload_len = mpdu_payload_len = max_pipe_payload_len- mpdu_header_len
        self.mod_index = mod_index = {es_mode == 0: 1, es_mode==1: 0.5, es_mode==2:0.5,es_mode==3:0.5,es_mode==4:1,es_mode==5:0.5,es_mode==6:1}.get(True,2)
        self.fec_rate = fec_rate = 1/2
        self.user_payload_len = user_payload_len = int(np.floor(mpdu_payload_len*fec_rate))
        self.sensitivity_label = sensitivity_label = sensitivity_tx
        self.samp_rate = samp_rate = data_rate*samples_per_symbol
        self.pipe_preamble_sync_data_field_1_len = pipe_preamble_sync_data_field_1_len = 7
        self.mod_index_label = mod_index_label = mod_index
        self.interval_ms = interval_ms = 1000
        self.freq_dev_label = freq_dev_label = freq_dev
        self.es_mode_label = es_mode_label = es_mode
        self.data_rate_label = data_rate_label = data_rate

        ##################################################
        # Blocks
        ##################################################

        self._sensitivity_label_tool_bar = Qt.QToolBar(self)

        if None:
            self._sensitivity_label_formatter = None
        else:
            self._sensitivity_label_formatter = lambda x: eng_notation.num_to_str(x)

        self._sensitivity_label_tool_bar.addWidget(Qt.QLabel("Mod Index"))
        self._sensitivity_label_label = Qt.QLabel(str(self._sensitivity_label_formatter(self.sensitivity_label)))
        self._sensitivity_label_tool_bar.addWidget(self._sensitivity_label_label)
        self.top_grid_layout.addWidget(self._sensitivity_label_tool_bar, 0, 4, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(4, 5):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_time_sink_x_0 = qtgui.time_sink_c(
            (samples_per_symbol*(user_payload_len+pipe_preamble_sync_data_field_1_len)*8), #size
            samp_rate, #samp_rate
            "", #name
            1, #number of inputs
            None # parent
        )
        self.qtgui_time_sink_x_0.set_update_time(0.10)
        self.qtgui_time_sink_x_0.set_y_axis(-1, 1)

        self.qtgui_time_sink_x_0.set_y_label('Amplitude', "")

        self.qtgui_time_sink_x_0.enable_tags(True)
        self.qtgui_time_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_TAG, qtgui.TRIG_SLOPE_POS, 0.0, 0, 0, "packet_len")
        self.qtgui_time_sink_x_0.enable_autoscale(False)
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
            32768, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            0, #fc
            samp_rate, #bw
            "", #name
            1,
            None # parent
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_0.set_y_axis((-140), 10)
        self.qtgui_freq_sink_x_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_0.enable_grid(False)
        self.qtgui_freq_sink_x_0.set_fft_average(1.0)
        self.qtgui_freq_sink_x_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_0.enable_control_panel(True)
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
        self.pdu_random_pdu_0 = pdu.random_pdu(user_payload_len, user_payload_len, 0xFF, 1)
        self.pdu_pdu_to_tagged_stream_0 = pdu.pdu_to_tagged_stream(gr.types.byte_t, 'packet_len')
        self._mod_index_label_tool_bar = Qt.QToolBar(self)

        if None:
            self._mod_index_label_formatter = None
        else:
            self._mod_index_label_formatter = lambda x: eng_notation.num_to_str(x)

        self._mod_index_label_tool_bar.addWidget(Qt.QLabel("Mod Index"))
        self._mod_index_label_label = Qt.QLabel(str(self._mod_index_label_formatter(self.mod_index_label)))
        self._mod_index_label_tool_bar.addWidget(self._mod_index_label_label)
        self.top_grid_layout.addWidget(self._mod_index_label_tool_bar, 0, 3, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(3, 4):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._freq_dev_label_tool_bar = Qt.QToolBar(self)

        if None:
            self._freq_dev_label_formatter = None
        else:
            self._freq_dev_label_formatter = lambda x: str(x)

        self._freq_dev_label_tool_bar.addWidget(Qt.QLabel("Freq Dev"))
        self._freq_dev_label_label = Qt.QLabel(str(self._freq_dev_label_formatter(self.freq_dev_label)))
        self._freq_dev_label_tool_bar.addWidget(self._freq_dev_label_label)
        self.top_grid_layout.addWidget(self._freq_dev_label_tool_bar, 0, 2, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(2, 3):
            self.top_grid_layout.setColumnStretch(c, 1)
        self._es_mode_label_tool_bar = Qt.QToolBar(self)

        if None:
            self._es_mode_label_formatter = None
        else:
            self._es_mode_label_formatter = lambda x: str(x)

        self._es_mode_label_tool_bar.addWidget(Qt.QLabel("EnduroSat UHF II Mode : "))
        self._es_mode_label_label = Qt.QLabel(str(self._es_mode_label_formatter(self.es_mode_label)))
        self._es_mode_label_tool_bar.addWidget(self._es_mode_label_label)
        self.top_grid_layout.addWidget(self._es_mode_label_tool_bar, 0, 0, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(0, 1):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.epy_block_0 = epy_block_0.blk(fec_rate=fec_rate, mpdu_payload_len=mpdu_payload_len, mpdu_header_len=mpdu_header_len)
        self.digital_gfsk_mod_0 = digital.gfsk_mod(
            samples_per_symbol=samples_per_symbol,
            sensitivity=sensitivity_tx,
            bt=0.5,
            verbose=False,
            log=False,
            do_unpack=True)
        self._data_rate_label_tool_bar = Qt.QToolBar(self)

        if None:
            self._data_rate_label_formatter = None
        else:
            self._data_rate_label_formatter = lambda x: str(x)

        self._data_rate_label_tool_bar.addWidget(Qt.QLabel("Data Rate"))
        self._data_rate_label_label = Qt.QLabel(str(self._data_rate_label_formatter(self.data_rate_label)))
        self._data_rate_label_tool_bar.addWidget(self._data_rate_label_label)
        self.top_grid_layout.addWidget(self._data_rate_label_tool_bar, 0, 1, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(1, 2):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.blocks_throttle2_0 = blocks.throttle( gr.sizeof_char*1, data_rate, True, 0 if "auto" == "auto" else max( int(float(0.1) * data_rate) if "auto" == "time" else int(0.1), 1) )
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.cons(pmt.make_dict(), pmt.init_u8vector(10, (1,2,3,4,5,6,7,8,9,10))), interval_ms)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.pdu_random_pdu_0, 'generate'))
        self.msg_connect((self.epy_block_0, 'pdu_out'), (self.pdu_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.pdu_random_pdu_0, 'pdus'), (self.epy_block_0, 'pdu_in'))
        self.connect((self.blocks_throttle2_0, 0), (self.digital_gfsk_mod_0, 0))
        self.connect((self.digital_gfsk_mod_0, 0), (self.qtgui_freq_sink_x_0, 0))
        self.connect((self.digital_gfsk_mod_0, 0), (self.qtgui_time_sink_x_0, 0))
        self.connect((self.pdu_pdu_to_tagged_stream_0, 0), (self.blocks_throttle2_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "GS_baseband_to_file")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_es_mode(self):
        return self.es_mode

    def set_es_mode(self, es_mode):
        self.es_mode = es_mode
        self.set_data_rate({self.es_mode == 0: 1200, self.es_mode==1: 2400, self.es_mode==2:4800,self.es_mode==3:9600,self.es_mode==4:9600}.get(True,19200))
        self.set_es_mode_label(self.es_mode)
        self.set_freq_dev({self.es_mode == 0: 600, self.es_mode==1: 600, self.es_mode==2:1200,self.es_mode==3:2400,self.es_mode==4:4800,self.es_mode==5:4800,self.es_mode==6:9600}.get(True,19200))
        self.set_mod_index({self.es_mode == 0: 1, self.es_mode==1: 0.5, self.es_mode==2:0.5,self.es_mode==3:0.5,self.es_mode==4:1,self.es_mode==5:0.5,self.es_mode==6:1}.get(True,2))

    def get_samples_per_symbol(self):
        return self.samples_per_symbol

    def set_samples_per_symbol(self, samples_per_symbol):
        self.samples_per_symbol = samples_per_symbol
        self.set_samp_rate(self.data_rate*self.samples_per_symbol)
        self.set_sensitivity_tx(2*np.math.pi*(self.freq_dev/(self.data_rate*self.samples_per_symbol)))

    def get_mpdu_header_len(self):
        return self.mpdu_header_len

    def set_mpdu_header_len(self, mpdu_header_len):
        self.mpdu_header_len = mpdu_header_len
        self.set_mpdu_payload_len(self.max_pipe_payload_len- self.mpdu_header_len)
        self.epy_block_0.mpdu_header_len = self.mpdu_header_len

    def get_max_pipe_payload_len(self):
        return self.max_pipe_payload_len

    def set_max_pipe_payload_len(self, max_pipe_payload_len):
        self.max_pipe_payload_len = max_pipe_payload_len
        self.set_mpdu_payload_len(self.max_pipe_payload_len- self.mpdu_header_len)

    def get_freq_dev(self):
        return self.freq_dev

    def set_freq_dev(self, freq_dev):
        self.freq_dev = freq_dev
        self.set_freq_dev_label(self.freq_dev)
        self.set_sensitivity_tx(2*np.math.pi*(self.freq_dev/(self.data_rate*self.samples_per_symbol)))

    def get_data_rate(self):
        return self.data_rate

    def set_data_rate(self, data_rate):
        self.data_rate = data_rate
        self.set_data_rate_label(self.data_rate)
        self.set_samp_rate(self.data_rate*self.samples_per_symbol)
        self.set_sensitivity_tx(2*np.math.pi*(self.freq_dev/(self.data_rate*self.samples_per_symbol)))
        self.blocks_throttle2_0.set_sample_rate(self.data_rate)

    def get_sensitivity_tx(self):
        return self.sensitivity_tx

    def set_sensitivity_tx(self, sensitivity_tx):
        self.sensitivity_tx = sensitivity_tx
        self.set_sensitivity_label(self.sensitivity_tx)

    def get_mpdu_payload_len(self):
        return self.mpdu_payload_len

    def set_mpdu_payload_len(self, mpdu_payload_len):
        self.mpdu_payload_len = mpdu_payload_len
        self.set_user_payload_len(int(np.floor(self.mpdu_payload_len*self.fec_rate)))
        self.epy_block_0.mpdu_payload_len = self.mpdu_payload_len

    def get_mod_index(self):
        return self.mod_index

    def set_mod_index(self, mod_index):
        self.mod_index = mod_index
        self.set_mod_index_label(self.mod_index)

    def get_fec_rate(self):
        return self.fec_rate

    def set_fec_rate(self, fec_rate):
        self.fec_rate = fec_rate
        self.set_user_payload_len(int(np.floor(self.mpdu_payload_len*self.fec_rate)))
        self.epy_block_0.fec_rate = self.fec_rate

    def get_user_payload_len(self):
        return self.user_payload_len

    def set_user_payload_len(self, user_payload_len):
        self.user_payload_len = user_payload_len

    def get_sensitivity_label(self):
        return self.sensitivity_label

    def set_sensitivity_label(self, sensitivity_label):
        self.sensitivity_label = sensitivity_label
        Qt.QMetaObject.invokeMethod(self._sensitivity_label_label, "setText", Qt.Q_ARG("QString", str(self._sensitivity_label_formatter(self.sensitivity_label))))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.qtgui_freq_sink_x_0.set_frequency_range(0, self.samp_rate)
        self.qtgui_time_sink_x_0.set_samp_rate(self.samp_rate)

    def get_pipe_preamble_sync_data_field_1_len(self):
        return self.pipe_preamble_sync_data_field_1_len

    def set_pipe_preamble_sync_data_field_1_len(self, pipe_preamble_sync_data_field_1_len):
        self.pipe_preamble_sync_data_field_1_len = pipe_preamble_sync_data_field_1_len

    def get_mod_index_label(self):
        return self.mod_index_label

    def set_mod_index_label(self, mod_index_label):
        self.mod_index_label = mod_index_label
        Qt.QMetaObject.invokeMethod(self._mod_index_label_label, "setText", Qt.Q_ARG("QString", str(self._mod_index_label_formatter(self.mod_index_label))))

    def get_interval_ms(self):
        return self.interval_ms

    def set_interval_ms(self, interval_ms):
        self.interval_ms = interval_ms
        self.blocks_message_strobe_0.set_period(self.interval_ms)

    def get_freq_dev_label(self):
        return self.freq_dev_label

    def set_freq_dev_label(self, freq_dev_label):
        self.freq_dev_label = freq_dev_label
        Qt.QMetaObject.invokeMethod(self._freq_dev_label_label, "setText", Qt.Q_ARG("QString", str(self._freq_dev_label_formatter(self.freq_dev_label))))

    def get_es_mode_label(self):
        return self.es_mode_label

    def set_es_mode_label(self, es_mode_label):
        self.es_mode_label = es_mode_label
        Qt.QMetaObject.invokeMethod(self._es_mode_label_label, "setText", Qt.Q_ARG("QString", str(self._es_mode_label_formatter(self.es_mode_label))))

    def get_data_rate_label(self):
        return self.data_rate_label

    def set_data_rate_label(self, data_rate_label):
        self.data_rate_label = data_rate_label
        Qt.QMetaObject.invokeMethod(self._data_rate_label_label, "setText", Qt.Q_ARG("QString", str(self._data_rate_label_formatter(self.data_rate_label))))




def main(top_block_cls=GS_baseband_to_file, options=None):

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
