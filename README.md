# MIDI35

MIDI35 is a custom geometric touch MIDI controller: a compact desktop instrument featuring tactile controls, capacitive piano keys, and drum pads. Instead of standard bulky hardware, you interact with it using direct touch surfaces and physical sliders.
<img width="1735" height="1378" alt="image" src="https://github.com/user-attachments/assets/f96c0139-8130-4c1d-8ec5-5efcd41ab9ff" />
<img width="1509" height="1507" alt="image" src="https://github.com/user-attachments/assets/6957a18e-6152-448a-81d9-b5925e5ea3dd" />
<img width="1413" height="1382" alt="image" src="https://github.com/user-attachments/assets/11fc8782-debd-4b3b-abdf-38918e37d9ca" />
<img width="1532" height="1332" alt="image" src="https://github.com/user-attachments/assets/60005f62-8376-432a-9edb-87632d131898" />
<img width="1468" height="1505" alt="image" src="https://github.com/user-attachments/assets/aaccd124-d733-45f9-b09c-7e68ee377d7b" />
<img width="1545" height="1506" alt="image" src="https://github.com/user-attachments/assets/e147d4dc-8758-422c-9ebd-88a3c7a5249f" />



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
