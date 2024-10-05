// Copyright (C) 2024, Michael Faragher
// Based on the RNode Firmware / Reticulum Network Stack by Mark Qvist

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef RNSREGISTERS
#define RNSREGISTERS

byte SerialBuffer[1024];
byte CommandBuffer[512];
uint16_t SerialBufferWritePointer = 0;
uint16_t SerialBufferReadPointer = 0;
uint16_t CommandBufferPointer = 0;
bool inFrame = false;

uint32_t radioHz;
uint32_t radioBW;
byte radioSF;
byte radioCR;
byte radioTX;
byte radioState;

uint16_t airtimeShort;
uint16_t airtimeLong;
uint16_t channelLoadShort;
uint16_t channelLoadLong;

uint16_t symbolTimeMS;
uint16_t symbolRate;
uint16_t preambleSymbols;
uint16_t preambleTimeMS;
uint16_t csmaSlotTimeMS;


byte BatteryState = 0x00;
byte BatteryLevel = 0x00;

// Cryptography

byte edPrivateKey[32];
byte edPublicKey[32];
byte xPrivateKey[32];
byte xPublicKey[32];
byte Identity[16];
// byte edSignature[64]; // I think this doesn't need to be global

// General Enums
const char HexEnum[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#endif
