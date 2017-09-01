# G36 Tool

Simple companion tool for the G36 line of smartwatches / children trackers. These are sold under various names, such
as:
* MediaTech MT851
* Q50 kids smart watch
* etc.

The trackers are built on the MT6261 chipset, which is probably used as the GSM modem and application processor. An
ESP8266 is used to detect nearby Wifi networks. The GPS chipset is probably something from MediaTek, but one page hints
of an uBlox 7020.

# Features
* Serial log reading: using a log file works, serial port support to come

## Supported log messages
The following log message types are known or decoded to some degree:
* 0x62: text log message; timestamp, text decoded
* 0x65: possibly text log message with some extended info; timestamp, text decoded
* 0x78: possible boot banner; not decoded
* 0x83: possible SW / HW info message on boot; not decoded

# Building
The project currently builds in Visual Studio 2017, but only uses cross platform libraries.

# Usage
Specify the log file on the command line to be decoded. Output will be written to stdout.
