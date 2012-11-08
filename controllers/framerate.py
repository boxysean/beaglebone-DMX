import socket
import time
import random
import sys
import argparse
import os

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 9930

DEFAULT_CHANNELS = 4
nChannels = DEFAULT_CHANNELS
channels = [0] * nChannels

DEFAULT_FPS = 32
FPS = DEFAULT_FPS
frameCount = 0
currentTime = 0
startTime = 0

DEFAULT_FRAMES = 0
frames = DEFAULT_FRAMES

################################################################################

def constructPayload():
  res = "%3d " % (nChannels)

  for c in channels:
    res += "%3d " % (c)

  return res

################################################################################

def output(s):
  global outputMode, sock
#  if outputMode == "network":
  sock.send(s)
#  elif outputMode == "render":
#    renderFile.write(s)
#    renderFile.write('\n')

################################################################################

def setup():
  pass

################################################################################

def loop():
#  if inputMode == "playback":
#    payload = playbackFile.readline()
#    if len(payload) == 0:
#      playbackFile.seek(0)
#      payload = playbackFile.readline()
#    if payload[-1] == "\n":
#      payload = payload[:-1]
#    print "playback payload", payload
#    output(payload)
#    return

  time = (currentTime - startTime) * 1000

  payload = constructPayload()
  print("payload %s" % (payload))
  output(payload)

################################################################################

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description='A client that sends DMX at a constant frame rate')
  parser.add_argument('--framerate', metavar='f', type=int, nargs='?', default=DEFAULT_FPS, help='how often a new frame will be send to DMX server')
  parser.add_argument('--host', metavar='h', type=str, nargs='?', default=DEFAULT_HOST, help='DMX server address')
  parser.add_argument('--port', metavar='p', type=int, nargs='?', default=DEFAULT_PORT, help='DMX server port')
  parser.add_argument('--frames', metavar='r', type=int, nargs='?', default=0, help='number of frames to send before exiting')
  args = parser.parse_args()

  print "[OUTPUT] connecting to", args.host, args.port
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
  sock.connect((args.host, args.port))

  setup()

  loopDelta = 1./FPS
  startTime = currentTime = targetTime = time.time()

  while frames <= 0 or frameCount < frames:
    previousTime, currentTime = currentTime, time.time()
    timeDelta = currentTime - previousTime

    time.sleep(random.uniform(0, loopDelta / 2.))

    loop()

    targetTime += loopDelta
    sleepTime = targetTime - time.time()
    if sleepTime > 0:
    #  print "%.5f delta %.5f" % (sleepTime, loopDelta)
      time.sleep(sleepTime)
    else:
      print 'took too long'

    frameCount = frameCount + 1

  if outputMode == "network":
    sock.close()
  elif outputMode == "render":
    renderFile.close()

