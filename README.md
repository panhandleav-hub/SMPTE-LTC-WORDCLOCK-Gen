# Teensy LTC/Word Clock Generator for Eagle Automation

Complete build package for SMPTE LTC timecode and Word Clock generator using Teensy 4.x

## Features
- SMPTE LTC timecode generation (24, 25, 29.97DF, 30 fps)
- Dual Word Clock outputs (44.1, 48, 88.2, 96 kHz)
- 128x64 OLED display showing current timecode
- Rotary encoder for menu navigation
- Non-volatile settings storage
- Professional balanced LTC output via transformer
- 75Ω terminated Word Clock outputs

## Complete Bill of Materials

### Core Components (You Already Have)
- 1x Teensy 4.0 or 4.1

### Amazon Shopping List
1. **Hammond 1590BB Aluminum Enclosure** - $16.99
   - https://www.amazon.com/dp/B0002BSRIO
   
2. **BNC Female Panel Mount Connectors (4-pack)** - $9.99
   - https://www.amazon.com/dp/B01MRZ5U7B
   - Only need 2, spares for future projects
   
3. **1/4" TRS Jack Panel Mount (6-pack)** - $11.99
   - https://www.amazon.com/dp/B091GBJCCJ
   - Need 2, rest are spares
   
4. **0.96" OLED Display 128x64 I2C (2-pack)** - $12.99
   - https://www.amazon.com/dp/B079BN2J8V
   - White display, 1 spare
   
5. **Rotary Encoder with Cap** - $8.99
   - https://www.amazon.com/dp/B08728K3YB
   - Includes push button function
   
6. **LED Kit with Resistors** - $9.99
   - https://www.amazon.com/dp/B07PG84V17
   - Includes 3mm LEDs and resistors
   
7. **22AWG Hook-up Wire Kit** - $14.99
   - https://www.amazon.com/dp/B07V57R3MG
   - Multiple colors for easy wiring
   
8. **M3 Brass Standoff Kit** - $11.99
   - https://www.amazon.com/dp/B07D7828LC
   - For mounting boards
   
9. **10K Trimmer Potentiometer Kit** - $7.99
   - https://www.amazon.com/dp/B07S69443J
   - For output level adjustment

### Mouser Electronics Order
Order Number | Part | Quantity | Price Each
-------------|------|----------|------------
553-TY141P | Triad TY-141P Audio Transformer 600Ω:600Ω | 1 | $11.84
271-75-RC | 75Ω 1/4W 1% Resistor | 10 | $0.10
647-UVR1H100MDD | 10µF 50V Electrolytic Capacitor | 5 | $0.18
C320C104K5R5TA | 100nF Ceramic Capacitor | 10 | $0.10
68000-236HLF | 0.1" Pin Headers | 2 | $1.25

### PJRC Order (Teensy Store)
- Audio Adaptor Board Rev D - $14.25
  https://www.pjrc.com/store/teensy3_audio.html
- 14-pin Socket Headers - $0.60
  https://www.pjrc.com/store/header_14x1.html

**Total Project Cost: ~$140**

## Wiring Diagram

```
TEENSY 4.1 CONNECTIONS
======================

POWER:
------
USB 5V → Teensy VUSB
Teensy 3.3V → OLED VCC, Encoder VCC
Teensy GND → Common Ground (star ground)

AUDIO BOARD (Stack on top of Teensy):
--------------------------------------
Audio Board Line Out L → 10µF cap → TY-141P Pin 1
TY-141P Pin 2 → GND
TY-141P Pin 3 → TRS Jack Tip
TY-141P Pin 4 → TRS Jack Ring
TRS Jack Sleeve → GND
10K pot wiper → TY-141P Pin 3
10K pot ends → TY-141P Pin 3 & 4

DIGITAL I/O:
------------
Teensy Pin 2 → 75Ω → BNC #1 center (Word Clock 1)
Teensy Pin 3 → 75Ω → BNC #2 center (Word Clock 2)
BNC shields → GND

Teensy Pin 4 → 220Ω → Green LED anode (Power)
Teensy Pin 5 → 220Ω → Yellow LED anode (LTC Active)
LED cathodes → GND

Teensy Pin 6 → Encoder A
Teensy Pin 7 → Encoder B
Teensy Pin 8 → Encoder Switch
Encoder Common → GND
Encoder VCC → 3.3V

Teensy Pin 18 (SDA) → OLED SDA
Teensy Pin 19 (SCL) → OLED SCL
OLED VCC → 3.3V
OLED GND → GND
```

## Build Instructions

### Step 1: Prepare Enclosure
1. Print drilling template (see template.pdf)
2. Tape template to enclosure
3. Drill holes:
   - 2x 3/8" (9.5mm) for BNC connectors
   - 2x 1/4" (6.35mm) for TRS jacks
   - 1x 7mm for rotary encoder shaft
   - 2x 3mm for LEDs
   - Rectangle cutout 26mm x 13mm for OLED
   - 8mm hole for USB cable access

### Step 2: Install Components
1. Mount BNC connectors with lock washers
2. Install TRS jacks
3. Mount OLED with M2 screws/standoffs
4. Press fit LEDs (use drop of hot glue)
5. Install rotary encoder with nut

### Step 3: Solder Audio Board to Teensy
1. Use stacking headers for removable installation
2. Align carefully - Audio board uses specific pins
3. Solder all connections
4. Test fit in enclosure

### Step 4: Wire Audio Path
1. Mount transformer with double-sided foam tape
2. Connect Audio Board Line Out L to transformer primary
3. Wire transformer secondary to TRS jack
4. Add 10K pot for level control
5. Keep audio wires short and away from digital

### Step 5: Wire Digital Connections
1. Solder 75Ω resistors directly to BNC center pins
2. Use twisted pair for I2C connections to OLED
3. Keep encoder wires short
4. Use heatshrink on all connections

### Step 6: Final Assembly
1. Secure Teensy with standoffs
2. Route USB cable to access hole
3. Test all connections with multimeter
4. Close enclosure

## Software Installation

### Prerequisites
1. Download Arduino IDE 1.8.19 or 2.x
   https://www.arduino.cc/en/software

2. Install Teensyduino
   https://www.pjrc.com/teensy/td_download.html

3. Install required libraries via Arduino Library Manager:
   - Adafruit SSD1306
   - Adafruit GFX Library
   - Encoder by Paul Stoffregen
   - Bounce2

### Upload Code
1. Open LTC_Generator.ino in Arduino IDE
2. Select Tools → Board → Teensy 4.1 (or 4.0)
3. Select Tools → Port → (Your Teensy Port)
4. Click Upload button
5. Teensy Loader will flash automatically

## Operation Guide

### Display Layout
```
01:00:00:00        <- Current Timecode (HH:MM:SS:FF)
FPS: 30   WC: 48k  <- Frame rate & Word Clock
Mode: RUNNING      <- Current mode/status
[LOCKED] Level:80% <- System status
Push:Mode Turn:Adj <- Control hints
```

### Controls
- **Encoder Button**: Cycle through modes
- **Encoder Rotation**: Adjust selected parameter

### Modes
1. **RUN**: Start/Stop timecode generation
2. **SET FPS**: 24, 25, 29.97DF, 30
3. **SET SAMPLE**: 44.1k, 48k, 88.2k, 96k
4. **SET TIME**: Adjust start hour
5. **SET LEVEL**: Output level 0-100%

### LED Indicators
- **Green**: Power on
- **Yellow**: LTC active (blinks when running)

## Connecting to Your System

### Eagle Automation (Windows 98)
1. Connect LTC Output (TRS) to PC Sound Card Line In
2. Set level to avoid clipping (typically 50-80%)
3. Configure Eagle software for external timecode
4. Verify sync lock indicator

### Ferrofish A32 Pro Units
1. Connect BNC Out 1 → A32 Pro #1 Word Clock In
2. Connect BNC Out 2 → A32 Pro #2 Word Clock In
3. Set A32 Pro #1 to external clock, 75Ω terminated
4. Set A32 Pro #2 to external clock, 75Ω terminated
5. Verify lock indicators on both units

### Pro Tools
1. Use second TRS output or split LTC signal
2. Route to Pro Tools sync interface
3. Set Pro Tools to external LTC sync
4. Verify frame rate matches

## Testing

### Initial Tests
1. **Power Test**: Green LED should light
2. **Display Test**: Shows timecode on OLED
3. **Encoder Test**: Turn and press, display responds

### Word Clock Verification
1. Connect oscilloscope to BNC output
2. Should see square wave at sample rate
3. Verify 50% duty cycle
4. Check amplitude ~3.3V p-p

### LTC Audio Test
1. Connect headphones to TRS output
2. Should hear distinctive "warble" sound
3. Frequency changes with bit pattern
4. No distortion or dropouts

### System Integration Test
1. Start with one connection at a time
2. Verify each device locks properly
3. Run for extended period (1+ hours)
4. Check for drift or unlock events

## Troubleshooting

Problem | Possible Cause | Solution
--------|---------------|----------
No display | I2C connection issue | Check SDA/SCL connections, verify 3.3V
No LTC audio | Audio board not connected | Verify Audio board soldering
Word clock not locking | Impedance mismatch | Check 75Ω termination
Eagle won't sync | Level too high/low | Adjust with trim pot
Timecode drifting | No common clock | Ensure all devices share clock
Display garbled | I2C pullup issue | Add 4.7K pullups if needed
Encoder erratic | Debounce issue | Check connections, adjust code
No power LED | Power connection | Check USB cable and connections

## Specifications

### LTC Output
- Format: SMPTE 12M Linear Timecode
- Encoding: Biphase Mark
- Output Level: -10 to +4 dBu adjustable
- Impedance: 600Ω balanced
- Connector: 1/4" TRS balanced

### Word Clock Output
- Format: Square wave
- Frequencies: 44.1, 48, 88.2, 96 kHz
- Level: 3.3V CMOS
- Impedance: 75Ω source terminated
- Connector: BNC female

### Frame Rates
- 24 fps (Film)
- 25 fps (PAL)
- 29.97 fps DF (NTSC Drop Frame)
- 30 fps (NTSC Non-Drop)

### Power
- Input: USB 5V, 500mA max
- Current Draw: ~200mA typical

## Theory of Operation

### LTC Generation
LTC uses Biphase Mark encoding where:
- '0' = one transition at bit start
- '1' = two transitions (start and middle)
- 80 bits per frame
- Bits 64-79 contain sync word (0xBFFC)
- Audio frequency varies 960-2400 Hz

### Word Clock
- Square wave at sample frequency
- 50% duty cycle
- Rising edge marks sample point
- 75Ω termination prevents reflections

### Drop Frame
- Actual rate is 29.97 fps (not 30)
- Drops frames 00 and 01 each minute
- Except minutes divisible by 10
- Keeps timecode aligned with real time

## Advanced Modifications

### GPS Time Sync
Add GPS module for atomic time reference:
- Connect GPS to Serial2
- Parse NMEA sentences
- Sync timecode to UTC

### Network Control
Add Ethernet module for remote control:
- Web interface for settings
- NTP time sync
- Remote monitoring

### Multiple Outputs
Add more LTC/Word Clock outputs:
- Use additional DACs for LTC
- Buffer word clock with 74HC04
- Add output selection matrix

## Support

For issues, questions, or contributions:
- GitHub: [your-repo-url]
- Email: [your-email]

## License

This project is released under MIT License.
Free to use, modify, and distribute.

## Acknowledgments

- PJRC for Teensy platform
- Adafruit for display libraries
- Pete Henderson for Eagle Automation project

---
Version 1.0 - November 2024
