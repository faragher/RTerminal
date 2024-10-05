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

#include RNodeUtil.h

// Alignment:        4 bytes
// Theoretical size: 42 bytes + 1 bit
// Packing:          1 byte + 7 bits
// Size in memeory:  44 Bytes
typedef struct destination_struct{
  byte Hash[16];
  void* Callback;
  byte NameHash[10];
  bool isActive;
} destination_t;

destination_t Destinations[32];

void SweepDestinations(byte DestinationHash[]){
  for(int i = 0; i < 32; i++){
    if(Destinations[i].isActive){
      byte HashOut[33];
      BytesToASCII(HashOut,Destinations[i].Hash,16);
      HashOut[33] = 0;
      printf("Destination Handler found for %s",HashOut);
    }
  }
}

void LXMFDestinationHandler(byte Header){
  
}

void InitTestDestination(){
  Destinations[0] = LXMFDestinationHandler;
}
