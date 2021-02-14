# Siedle-prs602-switcher
Switching module for Siedle PRS602 programmer. 

This is a simple relay card built for Brf Albert that enables
programming of the three Siedle sections without physically
moving the programming cable.

## Function
One of the three sections can be selected either by a push on
one of the corresponding buttons on the case or by sending the
number 1-3 followed by a carrage return on the serial port. 

The relay card has a timeout of 10 minutes after it releases
the relays. The indicating leds blinks slowly two minutes before
the timeout and blinks rapidly one minute before the timeout.
A new 10 minute period is started if the active button or 
corresponding serial command is resent.

## Content
* schematic: Eagle schematic and board layout. The scematic is also expored as a pdf.
* arduino: A simple sketch for the Arduino firmware.
* case: STL files for the case for 3D printng.
* desktop: Simple Windows Form application for controlling the switch box
