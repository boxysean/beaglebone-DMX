### Output DMX from your BeagleBone

DMX is a standard in the lighting industry and is used in applications from architecture to art installations to stage production.

Build instructions
------------------

1. build pasm by `cd`ing into `pasm/pasm_source` and running the appropriate pasm build script (linux, mac, windows)
2. type make in the project's root directory

Features
--------

* *Client-server architecture.* One of the cool things about the client-server setup is that the client can either be on the BeagleBone or your computer. This makes it easy for both testing and production.

* *DMX interface and controller combined.* Not only can you use this as a DMX interface for your computer, but now the BeagleBone can be a dedicated DMX controller.

* *Extensible.* You can use this with all the other features that the Bone gives you, like an Ethernet port for easy Internet connection Internet, or add your own custom hardware interface to it.

* *Easy to use.* You won't have to worry about the DMX protocol or low-level code. The client-server architecture allows you to control the DMX from python, Processing, OpenFrameworks, or whatever high-level languages with their awesome interactive libraries.

* *Example DMX client scripts.* This comes with example client scripts in the `controller` directory to help you get started with your idea.

* *Cheap.* No longer do you need a Mac Mini and ENTTEC DMX interface to run your lighting installation. Now all you need is an $80 BeagleBone and a $5 circuit.

How to run: Software
--------------------

1. Run `modprobe uio_pruss`
2. Launch DMX server: `cd` into `bin` and run `./dmx`
3. Launch DMX client: run a DMX client script, e.g., `cd contollers; python cycle.py`

How to run: Hardware
--------------------

1. [Make this circuit.](http://code.google.com/p/tinkerit/wiki/DmxSimpleBuilding)
2. Connect pin 3 of the BeagleBone's P8 header to the input (pin 4) of the IC.
3. Connect pins 5, 6, and 7 to ground, signal, and signal inversion respectively to target unit.

![image of DMX circuit](http://www.arduino.cc/playground/uploads/DMX/send_sn75276a.jpg)

You can change the Bone's pin by editing the defined pin in `src/dmx.c` and recompiling.

DMX Server Protocol
-------------------

The DMX server expects UDP string packets in the following format:

    N <value1> <value2> ... <valueN>

`N` refers to the number of channels, and `valueI` is the value of the Ith channel, an integer between 0-255. A single space is expected between the values, and there should be no space after the last value.

Send these packets to port 9930 on the BeagleBone. You can change the port by editing the variable in `src/dmx.c` and recompiling.

Production Mode
---------------

It's possible to make the DMX client and/or server launch when the BeagleBone boots up. [Follow these instructions.](http://beaglebone.cameon.net/home/autostarting-services)

Benchmark
---------

BeagleBone rev A6 running both the DMX server and a DMX client written in python. The client sends 1000 updates messages back-to-back, changing all 512 channels.

    root@beaglebone:~/workspace/beaglebone-DMX/controllers# time python performance.py 1000
    
    real    0m11.088s
    user    0m8.610s
    sys     0m0.120s

1000 frames updating all 512 channels takes 11.088s, which is about 90 FPS. No packet drops by the server.

How it works
------------

This library takes advantage of the BeagleBone's PRU (Programmable Realtime Unit). The DMX server passes the DMX values to the BeagleBone's PRU, which is constantly bit-banging the DMX protocol in realtime. ([Read more.](http://blog.boxysean.com/2012/08/12/first-steps-with-the-beaglebone-pru/)) The hardware circuit converts the bit-banged protocol into [RS-485](http://en.wikipedia.org/wiki/RS-485), which is what all standard DMX units expect.
