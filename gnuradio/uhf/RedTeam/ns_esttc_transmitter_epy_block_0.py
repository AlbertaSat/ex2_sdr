"""
Beacon Info:

Extract info from decoded AX.25 beacon

Note : This is a hack, no error checking or other proper stuff...
"""

import numpy as np
from gnuradio import gr

import pmt


class blk(gr.sync_block):  # other base classes are basic_block, decim_block, interp_block
    """Extract Beacon Info"""

    def __init__(self):  # only default arguments here
        """arguments to this function show up as parameters in GRC"""
        gr.sync_block.__init__(
            self,
            name='Beacon Info',
            in_sig=None,
            out_sig=None
        )
        self.message_port_register_in(pmt.intern('msg_in'))
        self.set_msg_handler(pmt.intern('msg_in'), self.handle_msg)



    def handle_msg(self, msg):
  
        msg_data = pmt.cdr(msg)
        parray = pmt.to_python(msg_data)
        destCallSign = ""
        raw = parray[0:6]
        for asc in raw:
            destCallSign += chr(asc)
            
        srcCallSign = ""
        raw = parray[7:7+6]
        for asc in raw:
            srcCallSign += chr(asc)

        payload = ""
        parrayLen = len(parray)
        raw = parray[16:parrayLen]
        for asc in raw:
            payload += chr(asc)
            
        print("Raw data : ")
        print(msg_data)
        print("Dest Call Sign : "+destCallSign)
        print("Src Call Sign : "+srcCallSign)
        print("Payload : "+payload)
        
    def work(self, input_items, output_items):
            return (0)
