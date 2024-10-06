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

#ifndef RNODEHANDLER
#define RNODEHANDLER

#include "RNSFraming.h"
#include "RNSRegisters.h"

void RNode_Process() {
  char cmd = CBGetE();
  bool wasData = false;
  if (cmd == FEND) cmd = CBGetE();
  //printf("Command: 0x%02x", cmd);
  if (cmd == CMD_DATA) {
    printf("Incoming data: \n");
    char header = CBGetE();
    if (header == 0) {
      TestPacketHandler(header);
      //printf("Returned from Handler\n");
    }
    else if ((header & 0b00000001)  == 0b00000001) {
      Process_Announce(header);
    }
    //else if (header == 0b01010001 || header == 0b01110001) {
    //  Process_Type_Two_Announce(header);
    //}
    else {
      Unknown_Packet_Decoder(header);
    }

    printf("\n");
    cmd = FEND;
    wasData = true;

  }
  else if (cmd == CMD_FREQUENCY) {
    radioHz = (uint32_t)CBGetE() << 24 | (uint32_t)CBGetE() << 16 | (uint32_t)CBGetE() << 8 | (uint32_t)CBGetE();
    printf("Reported Frequency: %i Hz\n", radioHz);
  }
  else if (cmd == CMD_BANDWIDTH) {
    radioBW = (uint32_t)CBGetE() << 24 | (uint32_t)CBGetE() << 16 | (uint32_t)CBGetE() << 8 | (uint32_t)CBGetE();
    printf("Reported Bandwidth: %i Hz\n", radioBW);
  }
  else if (cmd == CMD_TXPOWER) {
    radioTX = CBGetE();
    printf("Reported TX Power %i dBm\n", radioTX);
  }
  else if (cmd == CMD_SF) {
    radioSF = CBGetE();
    printf("Reported Spreading Factor: %i\n", radioSF);
  }
  else if (cmd == CMD_CR) {
    radioCR = CBGetE();
    printf("Reported Coding Rate: %i\n", radioCR);
  }
  else if (cmd == CMD_RADIO_STATE) {
    radioState = CBGetE();
    printf("Reported Radio state:");
    if (radioState == 0x00) printf (" Offline\n");
    else if (radioState == 0x01) printf (" Online\n");
    else printf("Undefined 0x%02x\n", radioState);
  }
  else if (cmd == CMD_RADIO_LOCK) {

  }
  else if (cmd == CMD_DETECT) {

  }
  else if (cmd == CMD_IMPLICIT) {

  }
  else if (cmd == CMD_LEAVE) {
    // No RX
  }
  else if (cmd == CMD_ST_ALOCK) {

  }
  else if (cmd == CMD_LT_ALOCK) {

  }
  else if (cmd == CMD_PROMISC) {

  }
  else if (cmd == CMD_READY) {

  }

  // CMD_STAT_RX     0x21
  // CMD_STAT_TX     0x22
  else if (cmd == CMD_STAT_RSSI) {
    printf("RSSI: %i\n", CBGetE());
  }
  else if (cmd == CMD_STAT_SNR) {
    printf("SNR: %i\n", CBGetE());
  }

  else if (cmd == CMD_STAT_CHTM) {
    airtimeShort = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
    airtimeLong = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
    channelLoadShort = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
    channelLoadLong = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
  }
  // CMD_STAT_PHYPRM 0x26
  else if (cmd == CMD_STAT_PHYPRM) {
    symbolTimeMS = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
    symbolRate = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
    preambleSymbols = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
    preambleTimeMS = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
    csmaSlotTimeMS = (uint16_t)CBGetE() << 8 | (uint16_t)CBGetE();
  }
  else if (cmd == 0x27) {
    BatteryState = CBGetE();
    BatteryLevel = CBGetE();
  }
  //while (SerialBufferWritePointer != SerialBufferReadPointer) {
  //  printf("OVERFLOW: 0x%02x", CBGet());
  //}
  // CMD_BOARD       0x47
  // CMD_PLATFORM    0x48
  // CMD_MCU         0x49
  // CMD_FW_VERSION  0x50
  // CMD_FB_READ     0x42
  // CMD_FB_WRITE    0x43
  // CMD_DEV_HASH    0x56
  // CMD_DEV_SIG     0x57
  //-- CMD_FW_HASH     0x58
  // CMD_HASHES      0x60

  else if (cmd == CMD_ERROR) {
    printf("Error! Code: 0x%02x\n", CBGetE());
  }


  else {
    printf("0x%02x Unhandled!\n", cmd);
  }
  if (!wasData) {
    cmd = CBGetE();
  }
  if (cmd != FEND) {
    printf("OVERFLOW/UNHANDLED!");
    while ((cmd != FEND) && (SerialBufferWritePointer != SerialBufferReadPointer)) {
      printf("0x%02x ", cmd);
      cmd = CBGetE();
    }
    //printf("\n");
  }
  //printf("RNode Processing Complete. Buffer pointers %i read, %i write\n", SerialBufferReadPointer, SerialBufferWritePointer);

}

void RNode_Check_UART_New() {
  char coi[1];
  while (Serial2.available() > 0) {
    Serial2.readBytes(coi, 1);
    if (coi[0] == FEND) {
      if (!inFrame) {
        //CBSet(FEND);
        inFrame = true;
      }
      else {
        //Process
        CBSet(FEND);
        RNode_Process();
        //while (SerialBufferWritePointer != SerialBufferReadPointer) {
        //  printf("0x%02x ", CBGet());
        //  //SerialBufferReadPointer++;
        //}
        inFrame = false;
        //CommandBufferPointer = 0;
        //printf("\n");
      }
    }
    else {
      //CommandBuffer[CommandBufferPointer] = coi[0];
      //SerialBuffer[SerialBufferWritePointer] = coi[0];
      //SerialBufferWritePointer++;
      CBSet(coi[0]);
    }
  }
}

#endif
