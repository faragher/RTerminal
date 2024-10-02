# RTerminal
Terminal mode RNode controller with extra steps.

## This is highly experimental software

It isn't user friendly, half the features do half of what they're intended to do, and it's not terribly useful. However, as a proof of concept, it's solid. The major hurdles are addressed, and while there is a lot of work to be done, it seems entirely possible.

## Hardware Configuration

Pin|Description|Device
---|---|---
TX2 | UART Transmit | RNode
RX2 | UART Receive | RNode
D5 | SPI Cable Select | SD Card
D18 | SPI Clock | SD Card
D19 | MISO | SD Card
D23 | MOSI | SD Card

## Commands

Command | Use | Args | Functionality
---|---|---|---
  rnstate | Show/set the RNode Radio State | on(1)/off(0)/null(check) | Full
  rnfreq|Set Frequency in Hz. | 0/null gets current freq | Full
  rntxp|Set Transmit power in dB (0-17). | 0/null gets current | Full
  rnbw|Set Bandwidth.| 0/null gets current | Full
  TestJSON|Make a bunch of test files| |Testbed Only
  initdirs|Make stock dirs||Partial
  mkdir|Creates the given folder. If directory exists, command is ignored.||Full
  rnload|Loads RNode config from file. | [0-9, default 0] | Partial
  rnbatt|Shows RNode Battery State|| Full
  rnairtime|Shows RNode airtime/channel load (short/long)||Full
  rnphy|Shows RNode physical parameters||Full
  rndi|Sets display intensity |[0-255]| Full
  makeid|Make a new Identity. Use \"override\" to overwrite an old Identity||Partial

## Capabilities

SD Card storage for bulk data storage, such as the Identities and configuration files. 

Local SPIFF storage for cryptographic keys. This is not secure, but more secure than a removable card. 
