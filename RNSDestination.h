// Destination handler

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

#ifndef RNSDESTINATION
#define RNSDESTINATION

#include "RNodeUtil.h"

// Alignment:        4 bytes
// Theoretical size: 42 bytes + 1 bit
// Packing:          1 byte + 7 bits
// Size in memeory:  44 Bytes
typedef struct destination_struct {
  byte Hash[16];
  void (*Callback)();
  byte NameHash[10];
  bool isActive;
} destination_t;

destination_t Destinations[32];


void SweepDestinations(byte DestinationHash[]) {
  bool isHandled = false;
  byte HashOut[33];
  printf("\nDestination: ");
  for (int i = 0; i < 16; i++) {
    printf("%02x", DestinationHash[i]);
  }

  printf("\nDestination handling...\n");
  for (int i = 0; i < 32; i++) {
    //printf("Round %i:\n", i);
    if (Destinations[i].isActive) {
      printf("Active Destination...\n");
      printf("\nHandler Destination: ");
      for (int j = 0; j < 16; j++) {
        printf("%02x", Destinations[i].Hash[j]);
      }
      printf("\n");
      int T = memcmp(DestinationHash, Destinations[i].Hash, 16);
      printf("Memcmp reports %i\n", T);
      if (T == 0) {
        //  BytesToASCII(HashOut, Destinations[i].Hash, 16);
        //  HashOut[33] = 0;
        printf("Destination Handler found for ", HashOut);
        for (int j = 0; j < 16; j++) {
          ShowSerialNeat(Destinations[i].Hash[j]);
        }
        printf("\n");
        //byte payload = CBGetE();
        //while (payload != FEND) {
        //  printf("0x%02x ", payload);
        //  payload = CBGetE();
        //}
        Destinations[i].Callback();
        printf("\n");
        isHandled = true;
      }
      else {
        //  BytesToASCII(HashOut, Destinations[i].Hash, 16);
        //  HashOut[33] = 0;
        //printf("Apparently we're not ");
        //for (int j = 0; j < 16; j++) {
        //  ShowSerialNeat(Destinations[i].Hash[j]);
        //}
      }
    }

  }

  if (!isHandled) {
    byte payload = CBGetE();
    while (payload != FEND) {
      printf("0x%02x ", payload);
      payload = CBGetE();
    }
  }
}

void TestPacketHandler(byte header) {
  /*
    byte IFAC = (header & 0b10000000) >> 7;
    byte HeaderType = (header & 0b01000000) >> 6;
    byte hContext = (header & 0b00100000) >> 5;
    byte Propagation = (header & 0b00010000) >> 4;
    byte Destination = (header & 0b00001100) >> 2;
    byte Type = (header & 0b00000011);
    printf("IFAC:        %i\n", IFAC);
    printf("Header Type: %i\n", HeaderType);
    printf("Context:     %i\n", hContext);
    printf("Propagation: %i\n", Propagation);
    printf("Destination: %i\n", Destination);
    printf("Type:        %i\n", Type);
  */
  byte Hops = CBGetE();
  //printf("Hops:        %i\n", Hops);
  byte Hash[16];
  for (int i = 0; i < 16; i++) {
    Hash[i] = CBGetE();
  }
  byte Context = CBGetE();
  /*char payload = CBGetE();
    while (payload != FEND) {
    //printf("0x%02x ", payload);
    payload = CBGetE();
    }
  */
  SweepDestinations(Hash);
}


void LXMFDestinationHandler() {
  printf("Handling message\n");
  char payload = CBGetE();
  while (payload != FEND) {
    printf("0x%02x ", payload);
    payload = CBGetE();
  }
  printf("\n");
}

void DummyHandler(){
  printf("You should never see me\n");
}

void InitTestDestination() {
  Destinations[0].Callback = &LXMFDestinationHandler;
  char Buffer[] = {0x92, 0x57, 0xac, 0x31, 0x75, 0x79, 0xd7, 0x28, 0x21, 0x52, 0x7d, 0x63, 0x7b, 0x3f, 0xfa, 0x28};
  memcpy(Destinations[0].Hash, Buffer, 16);
  Destinations[0].isActive = true;

  Destinations[1].Callback = &DummyHandler;
  Destinations[1].isActive=true;
}
#endif
