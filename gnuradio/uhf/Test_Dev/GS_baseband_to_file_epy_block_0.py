"""
Embedded Python Block
"""

import numpy as np
from gnuradio import gr
import pmt

class blk(gr.sync_block):
    """Packet Format"""

    def __init__(self, fec_rate=0.5, mpdu_payload_len=119, mpdu_header_len=9):
        gr.sync_block.__init__(self,
            name = "Packet Format",
            in_sig = None,
            out_sig = None)
        self.message_port_register_in(pmt.intern('pdu_in'))
        self.message_port_register_out(pmt.intern('pdu_out'))
        self.set_msg_handler(pmt.intern('pdu_in'), self.handle_msg)
        self.fec_rate = fec_rate
        self.mpdu_payload_len = mpdu_payload_len
        self.mpdu_header_len = mpdu_header_len

    def handle_msg(self, msg):
        inMsg = pmt.to_python (msg)
        pld = inMsg[1]
        #print (pld)
        mUserPayloadLen = len(pld)
        #print (mUserPayloadLen)
        if (mUserPayloadLen > 0 and mUserPayloadLen <= self.mpdu_payload_len):
            # Not actually going to use the FEC rate now, just pad the packet
            # to the max MPDU len
            mPaddingLen = self.mpdu_payload_len - mUserPayloadLen
            print (mPaddingLen)
            # 5 preamble bytes, 1 sync byte
            char_list = [0xAA,0xAA,0xAA,0xAA,0xAA,0x7E]
            # data field 1
            total_payload_length = self.mpdu_payload_len + self.mpdu_header_len
            char_list.append (total_payload_length & 255)
            # MPDU header is next. Make it all 0xFF
            for i in range(0,self.mpdu_header_len):
                char_list.append(255)
            # We'll make the FEC method systemic, keeping the original message 
            # at the start of the data 2 field and then padding out with zeros
            char_list.extend (pld)
            for i in range(0,mPaddingLen):
                char_list.append(0)
            print (char_list)
            out_len = len(char_list)
            self.message_port_pub(pmt.intern('pdu_out'), pmt.cons(pmt.PMT_NIL,pmt.init_u8vector(out_len,(char_list))))
