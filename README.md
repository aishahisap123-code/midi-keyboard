# MIDI35

MIDI35 is a custom geometric touch MIDI controller: a compact desktop instrument featuring tactile controls, capacitive piano keys, and drum pads. Instead of standard bulky hardware, you interact with it using direct touch surfaces and physical sliders.

# What it is
MIDI35 is designed to sit on your desk or studio workspace, giving you a full hands-on musical setup driven by an ESP32-S3 module. It combines 7 capacitive touch piano keys, 4 circular drum pads, slide potentiometers, rotary encoders, and push buttons into a custom-shaped, ergonomic PCB layout.

# Project status
The PCB design, custom touch footprint physics, Edge.Cuts board outline, and manufacturing Gerber files are fully complete. The hardware phase is wrapped up and ready for fabrication, with firmware development up next.

# Software
The firmware handles ESP32 capacitive touch thresholds, input mapping, and USB/BLE-MIDI communication protocols through a custom Arduino IDE sketch. Builders can customize pin assignments, touch sensitivity levels, and MIDI note or CC values directly in the source code.

# Repository contents

PCB/ - KiCad source files and fabrication Gerber exports

Renders/ - front and back PCB 3D renders

Guide.md - project documentation and step-by-step workflow guide

Firmware/ - Arduino IDE starter sketch and MIDI protocol configuration
