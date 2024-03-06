# Monoprice_M86A
Teensy based USB protocol translator allowing Home Assistant integration with Elan M86A multi-zone amplifiers.
It provides an API compatible with the Monoprice 6 zone amplifier, which is already supported in Home Assistant.

This is a clean room reverse engineering of the communication protocol used between an Elan HC6 controller & 1 to 4 M86a Multi-Zone Amplifiers.
This protocol is not publicly documented anywhere; this is an effort to implement the minimal commands required to match the Monoprice API features.

The Elan M86a units are very well built, using high quality components - for instance, all aop-amps used in the audio signal path are OPA2134.
Zone volume & tone control is implemented with the TDA7418 audio processor. Power amplifiers are linear/monolithic (not sure which part is used).
This offers very low noise and ample output power (~50w per channel), with better specs than the Monoprice units overall.

These units are fairly common on eBay/Marketplace, but unfortunately they are not intended to integrate with 3rd party systems (unlike the older S86a on which they are based).
This is an attempt to extend the life of these nice units as they will very likely outlive the Elan controllers they were designed to be used with - ask me how I know! 
---
## Requirements
- Arduino (tested with 1.8.19)
- Teensyduino add-on (tested with 1.58)
- Basic wiring skills
- The desire to salvage perfectly working hardware that would otherwise endup in a landfill
---
## Features
- Supports 4 M86a units for 24 zones in total (limited to 18 zones in Home Assistant)
- 8 audio sources (limited to 6 in Home Assistant)
- Monoprice Serial API exposed via USB
- Zone Loudness control instead of Balance (center = loudness on)
- Zone Sense inputs reported as Keypad connection status (not currently visible in HA but could be easily added)

## Missing/Unsupported
- Unit configuration such as source input levels, min/max volume, etc.
- Firmware updates
- Doorbell control
- Any other Via!Net accessory such as TS2 touchpanels
---
## Hardware implementation:

```
         |                          ____ 47k
  +3.3v  |---------------------+---|____|--+
         |                0.1u |           |
         |             GND -)|-+           |
 T       |                     |           |
 E       |             +-------x-------+   |
 E       |             |      3.3v     |   |
 N       |         +-->| DE            |   |      ____ 100
 S       |         |   |              A|---+-----|____|----<> 4
 Y     2 |---- TE -+---| /RE           |
         |             |    RS-485     |          ____ 100
   TX1 1 |---- TXD --->| D            B|---+-----|____|----<> 3   Via!Net
         |             |               |   |                        RJ45
   RX1 0 |<--- RXD ----| R    GND      |   |
         |             |               |   |
       G |---+         +-------x-------+   |        +-------- 7
         |   |                 |    ____   |        |
         |   |                 +---|____|--+        |
---------+   |                 |         47k        |
            GND               GND                  GND

```

Pretty much all components other than the RS485 driver & Teensy can be omitted, but are strongly recommended for long term reliability & stability.

- Teensy: tested with LC & T4.0; should work with all T3.x & T4.x
  Note that I could only get this to work reliably with Teensy boards, thanks to hardware control of the Transmit Enable pin.
  It may be possible to get the timing right on a RP2040 by leveraging the PIO hardware.
  
- RS-485 driver: tested with a 75HVD10; viable alternatives are MAX3483, 65HVD10, LTC1480, ADM3072
  There are many suitable RS-485 drivers available; make sure to use only 3.3v rated components

- Note that the Via!Net RJ45 pinout is not standard - 3&4 are on the same twisted pair.
  It doesn't matter much for short cable distances however.
---
## Via!Net Protocol overview
This is my interpretation of the signaling layer:
- Serial Format is 115200 bauds, 8 bits, no parity, 1 stop bit 
- There are 128 symbols (7bits encoded as 2 bytes/16bits), with different (but similar) encodings for frame & payload data:
```
MSB
15:12:  Some kind of cyclic value (not sure how this is computed, different sequence per symbol type)
11:9:   Symbol type
8:      Always 1 for high byte (sent first)

7:1:    Encoded value (bits 6:0)
0:      Always 0 for low byte (sent last)
LSB
```
The extra bits 15:12 are likely used to discard corrupted symbols; they are not evenly distributed as would be the case with transition-centered encodings such as Manchester, thus they don't do a terrible job at keeping dc balance. Galvanic isolation may work, but I did not try it.
The master unit polls each 128 frame in sequence continuously, waiting ~100us for an reply before polling the next frame. This results in a ~35ms full cycle time, or around 25Hz
Each of the 4 physical unit will report status on a different frame using this format, if present & powered on:
```
Response:
0xfd56 + 45 symbols (92 bytes in total)

Payload (after decoding to their 7bit values):

0:  always 0x05 (0x610a)
1:  unit id 
2:  zone data (6 zones X 6 symbols)

Zones:   1  2  3  4  5  6       bits
---------------------------------------------------
Offsets: 2  8  14 20 26 32  :   6   ? (always 0)
                                5   ? (always 0)
                                4:0 zone
         3  9  15 21 27 33  :   6   Global power
                                5   Do not disturb
                                4   Mute
                                3   (input sense?)
                                2:0 Source 1
         4  10 16 22 28 34  :   6   Whole house mode?
                                5   Page
                                4   ?
                                3   Loudness
                                2   ?
                                1:0 Source 2
         5  11 17 23 29 35  :   6:  ? (always 0)
                                5:0 volume (attenuation?) 48=min, 0=max
         6  12 18 24 30 36  :   6:  ? (always 0)
                                5:  ? (always 0)
                                4:0 treble  4=min, 28=max
         7  13 19 25 31 37  :   6:  ? (always 0)
                                5:  loudness changing?
                                4:0 bass    4=min, 28=max

38: ?
39: 6: ? (always 0)
    5: Sense input 6
    4: Sense input 5
    3: Sense input 4
    2: Sense input 3
    1: Sense input 2
    0: Sense input 1
40: ? (always 0x50)
41: 6: ? (always 0)
    5: Audio Source Sense input 3
    4: Audio Source Sense input 1
    3: Audio Source Sense input 2
    2: ? (always 0)
    1: ? (always 0)
    0: ? (always 0)
42: 6: Audio Source Sense input 6
    5: Audio Source Sense input 5
    4: Audio Source Sense input 4
    3: ? (always 0)
    2: ? (always 0)
    1: ? (always 0)
    0: ? (always 0)
43: 6: ? (always 0)
    5: Audio Source Sense input 8
    4: Audio Source Sense input 7
    3: ? (always 0)
    2: ? (always 0)
    1: ? (always 0)
    0: ? (always 0)
44: ? (always 0x00)
```

Commands are inserted anywhere in the poll sequence using this format:
```
(payload symbols s0 to s3 are represented as their 7 bit value here)

0xed06 0xd3fe [s0] [s1] [s2] [s3]
0       1       2       3
------------------------------------------------------------------------
0x04    zone    0x04    zonecmd
        0x02            0x00:   all zones off       (default zone is always 0x02)
        0x02            0x04:   Page off            (default zone is always 0x02)
        0x02            0x05:   Page on             (default zone is always 0x02)
        0x02            0x07:   Doorbell off        (default zone is always 0x02)
        0x02            0x08:   Doorbell on         (default zone is always 0x02)
                        0x0a:   DND off
                        0x0b:   DND on
                        0x0d:   Whole house off
                        0x0e:   Whole house on
                        0x10:   Zone off
                        0x11:   Zone on (source selected before zone on)
                        0x13:   mute off
                        0x14:   mute on
                        0x16:   Loudness off
                        0x17:   Loudness on
                        0x30:   Source 1 selected
                        0x31:   Source 2 selected
                        0x32:   Source 3 selected
                        0x33:   Source 4 selected
                        0x34:   Source 5 selected
                        0x35:   Source 6 selected
                        0x36:   Source 7 selected
                        0x37:   Source 8 selected
                                
0x0b    0x00    zone    volume (0x00 (0) = min, 0x64 (100) = max)
        0x01    zone    bass (0x3a (58) = min, 0x46 (70) = max)
        0x02    zone    treble (0x3a (58) = min, 0x46 (70) = max)
```
An interresting fact is that the primary M86a unit will start acting as a Master if there's no activity on the bus for a certain (short) period of time.
To re-gain control of the bus, we wait for a certain symbol & then repeat it twice before resuming the polling sequence.
This often causes collisions for a while, but will resorb within a few polling cycles
