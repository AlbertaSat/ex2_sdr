This folder contains gnuradio work that is unrelated to the C++ meson project in the other folders here.

The flows require GNURadio 3.9.2+ and the Out of Tree module gr-satellites from Daniel Estevez at >v4.4 to function.

The setup works with the USRP sample data that was taken at 5e6 samples/second at 2400 baud.

Endurosat_e2e is a test that uses network interfaces for input and output of data.

First run output_listen.sh, then run endurosat_e2e.py (or the GRC flow) and then run input.sh to trigger a packet.

output_listen.sh will write to the file output_data.bin as well as STDOUT.

pipe_command.bin contains a full packet for the Endurosat UHF radio to enter pipe mode with the default settings as shown in Table 15 of the manual. (This is also the example command at the bottom of that section.) The packet is wrapped on both sides with 16 bytes of 0xAA.

loopback_with_framing is a flow that includes both virtual connections and UHD blocks for testing with SDR hardware. Same as above, start output_listen.sh and then start the GNURadio flow. Use input.sh to send the pipe_command.bin packet.

