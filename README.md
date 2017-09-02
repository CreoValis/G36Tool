# G36 Tool

Simple companion tool for the G36 line of smartwatches / children trackers. These are sold under various names, such
as:
* MediaTech MT851
* Q50 kids smart watch
* etc.

These tracker contain the following hardware:
* MT6261 chipset, which is probably used as the GSM modem and application processor
* ESP8266, used to detect nearby Wifi networks
* The GPS chipset is probably MT3337, as referenced to in the firmware binary.
* EM30713 is used as a proximity detector on the back of the watch
* A BMA250 3-axis accelerometer is used to detect movement and fall events

Most of this was guessed by reading through the log dumps.

## Features
* Serial log reading: using a log file works, basic serial port support

### Supported log messages
The following log message types are known or decoded to some degree:
* 0x62: text log message; timestamp, text decoded
* 0x65: possibly text log message with some extended info; timestamp, text decoded
* 0x78: possible boot banner; not decoded
* 0x83: possible SW / HW info message on boot; not decoded

## Building
The project currently builds in Visual Studio 2017, but only uses cross platform libraries.

## Usage
Specify the log file or serial port name on the command line to be decoded. Output will be
written to stdout.

To explicitly use a serial port, prefix it with ':'.

## On-wire message format
The following was learned by reading through serial port dumps.
* Sync (2): 0x55 0x00
* PayloadLength (1)
* Payload (PayloadLength)
  * PayloadType (1)
  * (PayloadType-specific data)
* Checksum (1): XOR of bytes [Sync, Checksum)
* Footer (1): 0x00

### Known PayloadTypes
* 0x62: text log:
** TickCount (4): ms since boot
** Unknown (4)
** Text Data (x): zero padded, so length % 4 == 0

* 0x65: extended text log:
** Unknown (4)
** TickCount (4): ms since boot
** Unknown (4)
** Text Data (x): zero padded, so length % 4 == 0

* 0x78: boot banner (?): sent on boot once
** Unknown (4); example: `08 00 08 00`

* 0x83: boot info / HW/SW info (?): sent on boot once
** Unknown (24); example: `02 00 00 00 10 00 00 00 10 00 00 00 01 00 00 00 01 00 00 00 FF FF FF FF`
