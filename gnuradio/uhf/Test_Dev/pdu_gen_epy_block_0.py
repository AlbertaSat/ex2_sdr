"""
Embedded Python Block
"""

import numpy as np
from gnuradio import gr
import pmt

class blk(gr.sync_block):
    """Packet Format"""

    def __init__(self):
        gr.sync_block.__init__(self,
            name = "Packet Format",
            in_sig = None,
            out_sig = None)
        self.message_port_register_in(pmt.intern('pdu_in'))
        self.message_port_register_out(pmt.intern('pdu_out'))
        self.set_msg_handler(pmt.intern('pdu_in'), self.handle_msg)

    def handle_msg(self, msg):
        inMsg = pmt.to_python (msg)
        pld = inMsg[1]
        # print (pld)
        mLen = len(pld)
        # print (mLen)
        if (mLen > 0):
            # 10 preamble bytes, 4 access code bytes
            char_list = [0xAA,0xAA,0xAA,0xAA,0xAA,0x7E]
            char_list.append (mLen & 255)
            char_list.extend (pld)
            print (char_list)
            out_len = len(char_list)
            # print (out_len)
            self.message_port_pub(pmt.intern('pdu_out'), pmt.cons(pmt.PMT_NIL,pmt.init_u8vector(out_len,(char_list))))
