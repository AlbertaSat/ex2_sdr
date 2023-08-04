# this module will be imported in the into your flowgraph
from gnuradio import gr
import threading
import time
import socket

class doppler_runner(threading.Thread):
  def __init__(self, callback, freq, gpredict_host, gpredict_port):
    threading.Thread.__init__(self)

    self.callback = callback
    self.gpredict_host = gpredict_host
    self.gpredict_port = gpredict_port
    self.freq = freq


  def run(self):
    #bind_to = (self.gpredict_host, self.gpredict_port)
    bind_to = ('127.0.0.1', 4532)
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print(bind_to)
    server.bind(bind_to)
    server.listen(0)

    time.sleep(0.5) # TODO: Find better way to know if init is all done

    while True:
      print("Waiting for connection on: %s:%d" % bind_to)
      sock, addr = server.accept()
      print("Connected from: %s:%d" % (addr[0], addr[1]))

      cur_freq = 0
      while True:
        data = sock.recv(1024)
        if not data:
          break

        if data.startswith(b'F'):
          self.freq = int(data[1:].strip())
          if cur_freq != self.freq:
            print("New frequency: %d" % self.freq)
            self.callback(self.freq)
            cur_freq = self.freq
          sock.sendall(b'RPRT 0\n')
        elif data.startswith(b'f'):
          sock.sendall(b'f: %d\n' % cur_freq)

      sock.close()
      print("Disconnected from: %s:%d" % (addr[0], addr[1]))

    #sys.exit()

class doppler(gr.sync_block):
  def __init__(self, callback, freq, gpredict_host, gpredict_port):
    gr.sync_block.__init__(self,
                           name = "Gpredict Doppler",
                           in_sig = None,
                           out_sig = None)
    doppler_runner(callback, freq, gpredict_host, gpredict_port).start()

