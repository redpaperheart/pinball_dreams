Pinball Dreams
==============

This repo documents the process we followed to hack into a [Gottlieb Top Card](http://www.ipdb.org/machine.cgi?id=2580) pinball machine from 1974, in order detect the events triggered in the machine and forward them to a software aplication that displays artful animations in real time.

The overall arquitecture is the following:

Hall effect sensors / Photocells -> Arduino app -> (serial) -> Cinder app -> (OSC) -> client graphics app

Software
--------


Hardware
--------

Resources
--------

More details to follow

Arduino -> reading Hall effect sensors, one reflective IR sensor, and photocells to tell what pinball events have happened

Cinder -> communicating with Arduino through Serial, mapping the location of the pinball events onto the machine (projection) and illuminating when they are hit
