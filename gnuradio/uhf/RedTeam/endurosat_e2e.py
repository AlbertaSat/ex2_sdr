#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
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

from PyQt5 import Qt
from gnuradio import qtgui
from gnuradio.filter import firdes
import sip
from gnuradio import blocks
from gnuradio import digital
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from xmlrpc.server import SimpleXMLRPCServer
import threading



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
        self.spsym = spsym = 32
        self.nfilts = nfilts = 16
        self.fsk_dev = fsk_dev = 4800
        self.center_freq_rx = center_freq_rx = 437875000
        self.center_freq = center_freq = 437875000
        self.baud_bit = baud_bit = 9600
        self.tx_gain = tx_gain = 1
        self.taps = taps = firdes.root_raised_cosine(nfilts,nfilts,1/float(spsym),0.35,11*spsym*nfilts)
        self.sensitivity = sensitivity = 2*3.14159265358979323846*(fsk_dev/(baud_bit*spsym))
        self.samp_rate = samp_rate = baud_bit*spsym
        self.rx_gain = rx_gain = 0.9
        self.out_len = out_len = 129
        self.in_len = in_len = 158
        self.center_freq_tx = center_freq_tx = int(2*center_freq - center_freq_rx)
        self.bits_to_capture = bits_to_capture = 800
        self.baud_byte = baud_byte = baud_bit/8

        ##################################################
        # Blocks
        ##################################################
        self.xmlrpc_server_0 = SimpleXMLRPCServer(('localhost', 8080), allow_none=True)
        self.xmlrpc_server_0.register_instance(self)
        self.xmlrpc_server_0_thread = threading.Thread(target=self.xmlrpc_server_0.serve_forever)
        self.xmlrpc_server_0_thread.daemon = True
        self.xmlrpc_server_0_thread.start()
        self.qtgui_freq_sink_x_1_0_0 = qtgui.freq_sink_c(
            512, #size
            window.WIN_BLACKMAN_hARRIS, #wintype
            437875000, #fc
            2*baud_bit*spsym, #bw
            "Test B", #name
            1,
            None # parent
        )
        self.qtgui_freq_sink_x_1_0_0.set_update_time(0.10)
        self.qtgui_freq_sink_x_1_0_0.set_y_axis(-140, 30)
        self.qtgui_freq_sink_x_1_0_0.set_y_label('Relative Gain', 'dB')
        self.qtgui_freq_sink_x_1_0_0.set_trigger_mode(qtgui.TRIG_MODE_FREE, 0.0, 0, "")
        self.qtgui_freq_sink_x_1_0_0.enable_autoscale(False)
        self.qtgui_freq_sink_x_1_0_0.enable_grid(False)
        self.qtgui_freq_sink_x_1_0_0.set_fft_average(0.2)
        self.qtgui_freq_sink_x_1_0_0.enable_axis_labels(True)
        self.qtgui_freq_sink_x_1_0_0.enable_control_panel(False)
        self.qtgui_freq_sink_x_1_0_0.set_fft_window_normalized(False)



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
                self.qtgui_freq_sink_x_1_0_0.set_line_label(i, "Data {0}".format(i))
            else:
                self.qtgui_freq_sink_x_1_0_0.set_line_label(i, labels[i])
            self.qtgui_freq_sink_x_1_0_0.set_line_width(i, widths[i])
            self.qtgui_freq_sink_x_1_0_0.set_line_color(i, colors[i])
            self.qtgui_freq_sink_x_1_0_0.set_line_alpha(i, alphas[i])

        self._qtgui_freq_sink_x_1_0_0_win = sip.wrapinstance(self.qtgui_freq_sink_x_1_0_0.qwidget(), Qt.QWidget)
        self.top_layout.addWidget(self._qtgui_freq_sink_x_1_0_0_win)
        self.digital_gfsk_mod_0 = digital.gfsk_mod(
            samples_per_symbol=spsym,
            sensitivity=sensitivity,
            bt=0.5,
            verbose=False,
            log=False,
            do_unpack=True)
        self.blocks_var_to_msg_0_0_0_0 = blocks.var_to_msg_pair('center_freq')
        self.blocks_var_to_msg_0_0_0 = blocks.var_to_msg_pair('center_freq')
        self.blocks_var_to_msg_0_0 = blocks.var_to_msg_pair('baud_bit')
        self.blocks_var_to_msg_0 = blocks.var_to_msg_pair('fsk_dev')
        self.blocks_throttle_0 = blocks.throttle(gr.sizeof_gr_complex*1, samp_rate,True)
        self.blocks_socket_pdu_1_0 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '1235', 10000, False)
        self.blocks_socket_pdu_1 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '1234', 10000, False)
        self.blocks_socket_pdu_0_0 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '4322', 10000, False)
        self.blocks_socket_pdu_0 = blocks.socket_pdu('TCP_SERVER', '127.0.0.1', '4321', 10000, False)
        self.blocks_pdu_to_tagged_stream_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, 'packet_len')
        self.blocks_message_debug_0 = blocks.message_debug(True)
        self.blocks_file_sink_1_0 = blocks.file_sink(gr.sizeof_char*1, 'commands.dat', False)
        self.blocks_file_sink_1_0.set_unbuffered(False)
        self.blocks_file_sink_1 = blocks.file_sink(gr.sizeof_gr_complex*1, 'test.dat', False)
        self.blocks_file_sink_1.set_unbuffered(False)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_socket_pdu_1, 'pdus'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.blocks_socket_pdu_1_0, 'pdus'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.blocks_var_to_msg_0, 'msgout'), (self.blocks_message_debug_0, 'print'))
        self.msg_connect((self.blocks_var_to_msg_0_0, 'msgout'), (self.blocks_message_debug_0, 'print'))
        self.msg_connect((self.blocks_var_to_msg_0_0_0, 'msgout'), (self.blocks_message_debug_0, 'print'))
        self.msg_connect((self.blocks_var_to_msg_0_0_0_0, 'msgout'), (self.blocks_message_debug_0, 'print'))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.blocks_file_sink_1_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.digital_gfsk_mod_0, 0))
        self.connect((self.blocks_throttle_0, 0), (self.blocks_file_sink_1, 0))
        self.connect((self.digital_gfsk_mod_0, 0), (self.blocks_throttle_0, 0))
        self.connect((self.digital_gfsk_mod_0, 0), (self.qtgui_freq_sink_x_1_0_0, 0))


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
        self.set_samp_rate(self.baud_bit*self.spsym)
        self.set_sensitivity(2*3.14159265358979323846*(self.fsk_dev/(self.baud_bit*self.spsym)))
        self.set_taps(firdes.root_raised_cosine(self.nfilts,self.nfilts,1/float(self.spsym),0.35,11*self.spsym*self.nfilts))
        self.qtgui_freq_sink_x_1_0_0.set_frequency_range(437875000, 2*self.baud_bit*self.spsym)

    def get_nfilts(self):
        return self.nfilts

    def set_nfilts(self, nfilts):
        self.nfilts = nfilts
        self.set_taps(firdes.root_raised_cosine(self.nfilts,self.nfilts,1/float(self.spsym),0.35,11*self.spsym*self.nfilts))

    def get_fsk_dev(self):
        return self.fsk_dev

    def set_fsk_dev(self, fsk_dev):
        self.fsk_dev = fsk_dev
        self.set_sensitivity(2*3.14159265358979323846*(self.fsk_dev/(self.baud_bit*self.spsym)))
        self.blocks_var_to_msg_0.variable_changed(self.fsk_dev)

    def get_center_freq_rx(self):
        return self.center_freq_rx

    def set_center_freq_rx(self, center_freq_rx):
        self.center_freq_rx = center_freq_rx
        self.set_center_freq_tx(int(2*self.center_freq - self.center_freq_rx))
        self.blocks_var_to_msg_0_0_0.variable_changed(self.center_freq_rx)

    def get_center_freq(self):
        return self.center_freq

    def set_center_freq(self, center_freq):
        self.center_freq = center_freq
        self.set_center_freq_tx(int(2*self.center_freq - self.center_freq_rx))

    def get_baud_bit(self):
        return self.baud_bit

    def set_baud_bit(self, baud_bit):
        self.baud_bit = baud_bit
        self.set_baud_byte(self.baud_bit/8)
        self.set_samp_rate(self.baud_bit*self.spsym)
        self.set_sensitivity(2*3.14159265358979323846*(self.fsk_dev/(self.baud_bit*self.spsym)))
        self.blocks_var_to_msg_0_0.variable_changed(self.baud_bit)
        self.qtgui_freq_sink_x_1_0_0.set_frequency_range(437875000, 2*self.baud_bit*self.spsym)

    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain

    def get_taps(self):
        return self.taps

    def set_taps(self, taps):
        self.taps = taps

    def get_sensitivity(self):
        return self.sensitivity

    def set_sensitivity(self, sensitivity):
        self.sensitivity = sensitivity

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.blocks_throttle_0.set_sample_rate(self.samp_rate)

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

    def get_center_freq_tx(self):
        return self.center_freq_tx

    def set_center_freq_tx(self, center_freq_tx):
        self.center_freq_tx = center_freq_tx
        self.blocks_var_to_msg_0_0_0_0.variable_changed(self.center_freq_tx)

    def get_bits_to_capture(self):
        return self.bits_to_capture

    def set_bits_to_capture(self, bits_to_capture):
        self.bits_to_capture = bits_to_capture

    def get_baud_byte(self):
        return self.baud_byte

    def set_baud_byte(self, baud_byte):
        self.baud_byte = baud_byte




def main(top_block_cls=endurosat_e2e, options=None):
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
