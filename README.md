Pinball Dreams
==============

This repo documents the process we followed to hack into a [Gottlieb Top Card](http://www.ipdb.org/machine.cgi?id=2580) pinball machine from 1974, in order detect the events triggered in the machine and forward them to a software aplication that displays artful animations in real time.

The overall arquitecture is the following:    
Hall effect sensors / Photocells -> Arduino app -> (serial) -> Cinder app -> (OSC) -> client graphics app

Software
--------
Tools required:
- [Cinder v0.9.0](https://github.com/cinder/Cinder/tree/v0.9.0)
- [Arduino](https://www.arduino.cc/en/Main/Software)
- clone the repo and make sure to run ``git submodule init`` and ``git submodule update`` to fetch the Cinder blocks used. 

Prototype list:
- **pinball_serialGraph** (Arduino): Reads from the 20 sensors installed in the machine and forwards that data as a csv string via serial interface. Mapped to a specific wiring schematic. See Hardware sections for more info.
- **SerialGraph** (Cinder): Reads the serial string sent by the Arduino, stores a data histogram for every sensor, detects when spikes happen and forwards the events via OSC. Has a calibration mode to adjust thresholds individually for every sensor.
- **FakeSignal** (Cinder): Replacement for SerialGraph for quick testing. Simulates the pinball machine.
- **FakeSignalReceiver** (Cinder): Simple app to test the OSC connection.
- **PinballWarping** (Cinder): Receives OSC from SerialGraph or FakeSignal and displays graphics to project them on top of the machine. Useful for debugging. The code is a bit outdated. 
- **OCR** (Python): First attempt at reading the actual game score with a webcam.

Hardware
--------
**Parts list:**
- 10x DRV5053 Hall effect sensors with 100nF capacitors
- 10x Photocells (33 KOhm) with 1K Ohm resistors
- 2x CD4067BE multiplexers
- 1x [QRD1114](https://www.sparkfun.com/products/246) IR reflective sensor 
- 1x Arduino UNO

Photocells are used to detect changes in lights, which only happen once during a 3 or 5 ball game.   
Hall effect sensors are used to detect activity in the coils of the pinball actuators: flippers, bumpers and slingshots.    
The IR reflective distance sensor is used to detect the plunger.

Check out the *img* folder for schematics and pictures of the final soldered board.

Resources
--------
- Top Card replacement parts [1](http://actionpinball.com/parts.php?item=FKGTBEM), [2](http://www.marcospecialties.com/pinball-parts/730)
- [Electro-Mechanical Pinball repairing guide](http://www.pinrepair.com/em/)
- [Hack to set the machine for free play](http://www.pinrepair.com/em/index3.htm#free). Check the *img* folder for more info.
- [Top Card manual and schematic](http://www.marcospecialties.com/pinball-parts/DOC1684)
- [Multiplexer tutorial](http://tronixstuff.com/2013/08/05/part-review-74hc4067-16-channel-analog-multiplexerdemultiplexer/)

License
-------
Copyright (c) 2016, Red Paper Heart, All rights reserved. To contact Red Paper Heart, email hello@redpaperheart.com or tweet @redpaperhearts

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


