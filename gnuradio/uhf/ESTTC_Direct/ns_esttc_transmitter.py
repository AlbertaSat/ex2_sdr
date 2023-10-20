#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
# Author: knud
# GNU Radio version: 3.10.7.0

from packaging.version import Version as StrictVersion
from PyQt5 import Qt
from gnuradio import qtgui
import os
import sys
sys.path.append(os.environ.get('GRC_HIER_PATH', os.path.expanduser('~/.grc_gnuradio')))

from gnuradio import analog
from gnuradio import blocks
import pmt
from gnuradio import blocks, gr
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import network
from gnuradio import uhd
import time
from gnuradio.qtgui import Range, RangeWidget
from PyQt5 import QtCore
from uhf_pdu_demodulate import uhf_pdu_demodulate  # grc-generated hier_block
from uhf_pdu_modulate import uhf_pdu_modulate  # grc-generated hier_block
import math
import numpy as np
import sip



class ns_esttc_transmitter(gr.top_block, Qt.QWidget):

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

        self.settings = Qt.QSettings("GNU Radio", "ns_esttc_transmitter")

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
        self.es_mode = es_mode = 4
        self.samples_per_symbol = samples_per_symbol = 128
        self.freq_dev = freq_dev = {es_mode == 0: 600, es_mode==1: 600, es_mode==2:1200,es_mode==3:2400,es_mode==4:4800,es_mode==5:4800,es_mode==6:9600}.get(True,19200)
        self.data_rate = data_rate = {es_mode == 0: 1200, es_mode==1: 2400, es_mode==2:4800,es_mode==3:9600,es_mode==4:9600}.get(True,19200)
        self.sensitivity_tx = sensitivity_tx = 2*np.math.pi*(freq_dev/(data_rate*samples_per_symbol))
        self.mod_index = mod_index = {es_mode == 0: 1, es_mode==1: 0.5, es_mode==2:0.5,es_mode==3:0.5,es_mode==4:1,es_mode==5:0.5,es_mode==6:1}.get(True,2)
        self.center_freq_rx = center_freq_rx = 437875000
        self.center_freq = center_freq = 437875000
        self.tx_gain = tx_gain = .4
        self.sensitivity_label = sensitivity_label = sensitivity_tx
        self.samp_rate = samp_rate = data_rate*samples_per_symbol
        self.rx_threshold = rx_threshold = -30
        self.rx_gain = rx_gain = 0.6
        self.mod_index_label = mod_index_label = mod_index
        self.freq_dev_label = freq_dev_label = freq_dev
        self.es_mode_label = es_mode_label = es_mode
        self.data_rate_label = data_rate_label = data_rate
        self.center_freq_tx = center_freq_tx = int(2 * center_freq - center_freq_rx)

        ##################################################
        # Blocks
        ##################################################

        self.tab_id = Qt.QTabWidget()
        self.tab_id_widget_0 = Qt.QWidget()
        self.tab_id_layout_0 = Qt.QBoxLayout(Qt.QBoxLayout.TopToBottom, self.tab_id_widget_0)
        self.tab_id_grid_layout_0 = Qt.QGridLayout()
        self.tab_id_layout_0.addLayout(self.tab_id_grid_layout_0)
        self.tab_id.addTab(self.tab_id_widget_0, 'RX')
        self.tab_id_widget_1 = Qt.QWidget()
        self.tab_id_layout_1 = Qt.QBoxLayout(Qt.QBoxLayout.TopToBottom, self.tab_id_widget_1)
        self.tab_id_grid_layout_1 = Qt.QGridLayout()
        self.tab_id_layout_1.addLayout(self.tab_id_grid_layout_1)
        self.tab_id.addTab(self.tab_id_widget_1, 'TX')
        self.top_layout.addWidget(self.tab_id)
        self._tx_gain_range = Range(0, 0.6, 0.05, .4, 200)
        self._tx_gain_win = RangeWidget(self._tx_gain_range, self.set_tx_gain, "tx_gain", "counter_slider", float, QtCore.Qt.Horizontal)
        self.tab_id_grid_layout_1.addWidget(self._tx_gain_win, 0, 0, 1, 5)
        for r in range(0, 1):
            self.tab_id_grid_layout_1.setRowStretch(r, 1)
        for c in range(0, 5):
            self.tab_id_grid_layout_1.setColumnStretch(c, 1)
        self._rx_threshold_range = Range(-80, 5, 5, -30, 200)
        self._rx_threshold_win = RangeWidget(self._rx_threshold_range, self.set_rx_threshold, "rx_threshold", "counter_slider", float, QtCore.Qt.Horizontal)
        self.tab_id_grid_layout_0.addWidget(self._rx_threshold_win, 1, 0, 1, 5)
        for r in range(1, 2):
            self.tab_id_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 5):
            self.tab_id_grid_layout_0.setColumnStretch(c, 1)
        self._rx_gain_range = Range(0, 1, 0.05, 0.6, 200)
        self._rx_gain_win = RangeWidget(self._rx_gain_range, self.set_rx_gain, "rx_gain", "counter_slider", float, QtCore.Qt.Horizontal)
        self.tab_id_grid_layout_0.addWidget(self._rx_gain_win, 0, 0, 1, 5)
        for r in range(0, 1):
            self.tab_id_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 5):
            self.tab_id_grid_layout_0.setColumnStretch(c, 1)
        self.uhf_pdu_modulate_0 = uhf_pdu_modulate(
            FSK_level=2,
            fm_baud=data_rate,
            fm_modulation_index=mod_index,
            fm_samples_per_symbol=samples_per_symbol,
        )
        self.uhf_pdu_demodulate_0 = uhf_pdu_demodulate(
            baud_rate=data_rate,
            fm_modulation_index=mod_index,
            fm_samples_per_symbol=samples_per_symbol,
            frequency_deviation=freq_dev,
            packet_sync_word='0111111000010010',
            pdu_nominal_length=128,
        )
        self.uhd_usrp_source_0_0 = uhd.usrp_source(
            ",".join(("", "")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
        )
        self.uhd_usrp_source_0_0.set_samp_rate(samp_rate)
        self.uhd_usrp_source_0_0.set_time_now(uhd.time_spec(time.time()), uhd.ALL_MBOARDS)

        self.uhd_usrp_source_0_0.set_center_freq(center_freq, 0)
        self.uhd_usrp_source_0_0.set_antenna('RX2', 0)
        self.uhd_usrp_source_0_0.set_rx_agc(False, 0)
        self.uhd_usrp_source_0_0.set_normalized_gain(rx_gain, 0)
        self.uhd_usrp_sink_0 = uhd.usrp_sink(
            ",".join(("", "")),
            uhd.stream_args(
                cpu_format="fc32",
                args='',
                channels=list(range(0,1)),
            ),
            '',
        )
        self.uhd_usrp_sink_0.set_samp_rate(samp_rate)
        self.uhd_usrp_sink_0.set_time_now(uhd.time_spec(time.time()), uhd.ALL_MBOARDS)

        self.uhd_usrp_sink_0.set_center_freq(center_freq, 0)
        self.uhd_usrp_sink_0.set_antenna('TX/RX', 0)
        self.uhd_usrp_sink_0.set_normalized_gain(tx_gain, 0)
        self._sensitivity_label_tool_bar = Qt.QToolBar(self)

        if None:
            self._sensitivity_label_formatter = None
        else:
            self._sensitivity_label_formatter = lambda x: eng_notation.num_to_str(x)

        self._sensitivity_label_tool_bar.addWidget(Qt.QLabel("FM Sensitivity"))
        self._sensitivity_label_label = Qt.QLabel(str(self._sensitivity_label_formatter(self.sensitivity_label)))
        self._sensitivity_label_tool_bar.addWidget(self._sensitivity_label_label)
        self.top_grid_layout.addWidget(self._sensitivity_label_tool_bar, 0, 4, 1, 1)
        for r in range(0, 1):
            self.top_grid_layout.setRowStretch(r, 1)
        for c in range(4, 5):
            self.top_grid_layout.setColumnStretch(c, 1)
        self.qtgui_freq_sink_x_1 = qtgui.freq_sink_c(
            4096, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            center_freq, #fc
            100000, #bw
            "", #name
            1,
            None # parent
        )
        self.qtgui_freq_sink_x_1.set_update_time(0.05)
        self.qtgui_freq_sink_x_1.set_y_axis((-140), 10)
        self.qtgui_freq_sink_x_1.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_1.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_1.enable_autoscale(False)
        self.qtgui_freq_sink_x_1.enable_grid(True)
        self.qtgui_freq_sink_x_1.set_fft_average(1.0)
        self.qtgui_freq_sink_x_1.enable_axis_labels(True)
        self.qtgui_freq_sink_x_1.enable_control_panel(True)
        self.qtgui_freq_sink_x_1.set_fft_window_normalized(False)



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
                self.qtgui_freq_sink_x_1.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_1.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_1.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_1.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_1.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_1_win = sip.wrapinstance(self.qtgui_freq_sink_x_1.qwidget(), Qt.QWidget)
        self.tab_id_grid_layout_0.addWidget(self._qtgui_freq_sink_x_1_win, 2, 0, 3, 5)
        for r in range(2, 5):
            self.tab_id_grid_layout_0.setRowStretch(r, 1)
        for c in range(0, 5):
            self.tab_id_grid_layout_0.setColumnStretch(c, 1)
        self.qtgui_freq_sink_x_0 = qtgui.freq_sink_c(
            4096, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            0, #fc
            samp_rate, #bw
            "", #name
            1,
            None # parent
        )
        self.qtgui_freq_sink_x_0.set_update_time(0.05)
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
        self.tab_id_grid_layout_1.addWidget(self._qtgui_freq_sink_x_0_win, 1, 0, 3, 5)
        for r in range(1, 4):
            self.tab_id_grid_layout_1.setRowStretch(r, 1)
        for c in range(0, 5):
            self.tab_id_grid_layout_1.setColumnStretch(c, 1)
        self.network_socket_pdu_0_0 = network.socket_pdu('TCP_SERVER', '127.0.0.1', '52002', 10000, False)
        self.network_socket_pdu_0 = network.socket_pdu('TCP_SERVER', '127.0.0.1', '52001', 10000, False)
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
        self.blocks_message_strobe_0_1_4 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'ATR_XX', 'value': 0x1, 'mask': 0x1})), 3400)
        self.blocks_message_strobe_0_1_2 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'ATR_TX', 'value': 0xFE, 'mask': 0xFE})), 3200)
        self.blocks_message_strobe_0_1_1 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'ATR_RX', 'value': 0x10, 'mask': 0x10})), 2600)
        self.blocks_message_strobe_0_1_0 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'ATR_RX', 'value': 0, 'mask': 0xEF})), 2800)
        self.blocks_message_strobe_0_1 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'ATR_0X', 'value': 0x00, 'mask': 0xFF})), 2400)
        self.blocks_message_strobe_0_0_0 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'DDR', 'value': 0xFFFF, 'mask': 0xFFFF})), 2200)
        self.blocks_message_strobe_0_0 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'CTRL', 'value': 0xFFFF, 'mask': 0xFFFF})), 2000)
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.dict_add( pmt.make_dict(), pmt.to_pmt('gpio'), pmt.to_pmt({'bank':'FP0', 'attr':'ATR_TX', 'value': 0x1, 'mask': 0x1})), 3000)
        self.blocks_message_debug_0 = blocks.message_debug(True, gr.log_levels.info)
        self.analog_simple_squelch_cc_0 = analog.simple_squelch_cc(rx_threshold, 1)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_0, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_0_0, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_1, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_1_0, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_1_1, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_1_2, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.blocks_message_strobe_0_1_4, 'strobe'), (self.uhd_usrp_sink_0, 'command'))
        self.msg_connect((self.network_socket_pdu_0, 'pdus'), (self.uhf_pdu_modulate_0, 'pdus'))
        self.msg_connect((self.uhf_pdu_demodulate_0, 'pdus'), (self.blocks_message_debug_0, 'print'))
        self.msg_connect((self.uhf_pdu_demodulate_0, 'pdus'), (self.network_socket_pdu_0_0, 'pdus'))
        self.connect((self.analog_simple_squelch_cc_0, 0), (self.qtgui_freq_sink_x_1, 0))
        self.connect((self.analog_simple_squelch_cc_0, 0), (self.uhf_pdu_demodulate_0, 0))
        self.connect((self.uhd_usrp_source_0_0, 0), (self.analog_simple_squelch_cc_0, 0))
        self.connect((self.uhf_pdu_modulate_0, 0), (self.qtgui_freq_sink_x_0, 0))
        self.connect((self.uhf_pdu_modulate_0, 0), (self.uhd_usrp_sink_0, 0))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "ns_esttc_transmitter")
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
        self.uhf_pdu_demodulate_0.set_fm_samples_per_symbol(self.samples_per_symbol)
        self.uhf_pdu_modulate_0.set_fm_samples_per_symbol(self.samples_per_symbol)

    def get_freq_dev(self):
        return self.freq_dev

    def set_freq_dev(self, freq_dev):
        self.freq_dev = freq_dev
        self.set_freq_dev_label(self.freq_dev)
        self.set_sensitivity_tx(2*np.math.pi*(self.freq_dev/(self.data_rate*self.samples_per_symbol)))
        self.uhf_pdu_demodulate_0.set_frequency_deviation(self.freq_dev)

    def get_data_rate(self):
        return self.data_rate

    def set_data_rate(self, data_rate):
        self.data_rate = data_rate
        self.set_data_rate_label(self.data_rate)
        self.set_samp_rate(self.data_rate*self.samples_per_symbol)
        self.set_sensitivity_tx(2*np.math.pi*(self.freq_dev/(self.data_rate*self.samples_per_symbol)))
        self.uhf_pdu_demodulate_0.set_baud_rate(self.data_rate)
        self.uhf_pdu_modulate_0.set_fm_baud(self.data_rate)

    def get_sensitivity_tx(self):
        return self.sensitivity_tx

    def set_sensitivity_tx(self, sensitivity_tx):
        self.sensitivity_tx = sensitivity_tx
        self.set_sensitivity_label(self.sensitivity_tx)

    def get_mod_index(self):
        return self.mod_index

    def set_mod_index(self, mod_index):
        self.mod_index = mod_index
        self.set_mod_index_label(self.mod_index)
        self.uhf_pdu_demodulate_0.set_fm_modulation_index(self.mod_index)
        self.uhf_pdu_modulate_0.set_fm_modulation_index(self.mod_index)

    def get_center_freq_rx(self):
        return self.center_freq_rx

    def set_center_freq_rx(self, center_freq_rx):
        self.center_freq_rx = center_freq_rx
        self.set_center_freq_tx(int(2 * self.center_freq - self.center_freq_rx))

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.set_center_freq_tx(int(2 * self.center_freq - self.center_freq_rx))
        self.qtgui_freq_sink_x_1.set_frequency_range(self.center_freq, 100000)
        self.uhd_usrp_sink_0.set_center_freq(self.center_freq, 0)
        self.uhd_usrp_source_0_0.set_center_freq(self.center_freq, 0)

    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain
        self.uhd_usrp_sink_0.set_normalized_gain(self.tx_gain, 0)

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
        self.uhd_usrp_sink_0.set_samp_rate(self.samp_rate)
        self.uhd_usrp_source_0_0.set_samp_rate(self.samp_rate)

    def get_rx_threshold(self):
        return self.rx_threshold

    def set_rx_threshold(self, rx_threshold):
        self.rx_threshold = rx_threshold
        self.analog_simple_squelch_cc_0.set_threshold(self.rx_threshold)

    def get_rx_gain(self):
        return self.rx_gain

    def set_rx_gain(self, rx_gain):
        self.rx_gain = rx_gain
        self.uhd_usrp_source_0_0.set_normalized_gain(self.rx_gain, 0)

    def get_mod_index_label(self):
        return self.mod_index_label

    def set_mod_index_label(self, mod_index_label):
        self.mod_index_label = mod_index_label
        Qt.QMetaObject.invokeMethod(self._mod_index_label_label, "setText", Qt.Q_ARG("QString", str(self._mod_index_label_formatter(self.mod_index_label))))

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

    def get_center_freq_tx(self):
        return self.center_freq_tx

    def set_center_freq_tx(self, center_freq_tx):
        self.center_freq_tx = center_freq_tx




def main(top_block_cls=ns_esttc_transmitter, options=None):

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
