################################################################################
# dmxPerformance.py
# ----------------
# 
# An example script that cycles through each DMX channel, turning each fully on
# one at a time. Exits after sending so many frames
#
# Usage: python dmxPerformance.py [frames]
#
# Cycles refers to the number of frame updates to send to the DMX server. The
# default is 1000.
################################################################################

import socket
import time
import sys

# address of the BeagleBone DMX
IP = "127.0.0.1"
PORT = 9930

# number of DMX channels
CHANNELS = 512

# how many seconds to wait between sending a DMX update
DELAY = 0

# how many updates to send
if len(sys.argv) > 1:
  UPDATES = int(sys.argv[1])
else:
  UPDATES = 1000

def constructPayload(ch):
  res = "%d " % (CHANNELS)

  for i in range(CHANNELS):
    x = 255 if i == ch else 0
    res += "%d " % (x)

  return res

def loop():
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
  sock.connect((IP, PORT))

  channel = 0
  updates = 0
  while updates < UPDATES:
    payload = constructPayload(channel)
#    print "payload %s" % (payload)
    sock.send(payload)
    channel = (channel + 1) % CHANNELS
    time.sleep(DELAY)
    updates = updates+1

  sock.close()

if __name__ == "__main__":
  loop()

