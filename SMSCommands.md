# SMS commands

## General format

`pw,{PASSWORD},...#`
Where `{PASSWORD}` is the device password. By default, it's `123456`. Parameters are separated by comma.

## Known commands
* `ts`: Get device status; no further parameters. Example: `pw,123456,ts#`
* `factory`: Factory reset; no further parameters.
* `ip`: Set server address; parameters:
  * IPAddress
  * Port
* `apn`: Set APN to use with the current SIM card; parameters:
  * APN
  * UserName
  * Password
* `find`: Find watch indoors; no further parameters. Makes the watch beep.
* `time`: Set date/time. Example: `pw,123456,time,8.46.40,date,2001.09.11#`
  * Hour
  * Minute
  * Second
  * `date`
  * Year
  * Month
  * Day
* `cr`: Force wakeup; no further parameters.
