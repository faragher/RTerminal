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

#ifndef RNSCRYPTO
#define RNSCRYPTO

#include <SHA256.h>
#include <ArduinoJson.h>

#include "RNSRegisters.h"
#include "RNodeUtil.h"

void GetNameHash(byte Buffer[10], void* FullName, int Len) {
  SHA256 S;
  S.update(FullName, Len);
  S.finalize(Buffer, 10);
  // Save Hash
  char Filename[43] = "/sdcard/storage/names/XXXXXXXXXX";
  byte ASCIIBuffer[20];
  BytesToASCII(ASCIIBuffer,Buffer,10);
  memcpy(Filename + 22, ASCIIBuffer, 20);
  //printf("Opening %s",Filename);
  FILE *namefile = fopen(Filename, "r");
  if (!namefile)
  {
    fclose(namefile);
    namefile = fopen(Filename, "w");
    //printf("New Namehash! Saving %s", Filename);
    JsonDocument NameDoc;
    char Convert[Len+1];
    Convert[Len] = 0;
    memcpy(Convert,FullName,Len);
    NameDoc["Aspects"] = Convert;
    char msg[512];
    size_t jsonlen = serializeJsonPretty(NameDoc, msg);
    for (int i = 0; i < jsonlen; i++) {
      fputc(msg[i], namefile);
    }
  }
  fclose(namefile);
}

void GetIDfromPubKeys(byte Buffer[16], byte PubKey[32], byte PubSignKey[32]) {
  SHA256 S;
  S.update(PubKey, 32);
  S.update(PubSignKey, 32);
  S.finalize(Buffer, 16);
}

void GetDestinationFromIDandNameHash(byte Buffer[16], byte Identity[16], byte NameHash[10]) {
  SHA256 S;
  S.update(NameHash, 10);
  S.update(Identity, 16);
  S.finalize(Buffer, 16);
}

void GetRandomHash(byte Buffer[10]) {
  byte RandomBuffer[32];
  esp_fill_random(RandomBuffer, 32); //This is almost certainly PRNG, but it's not critical
  SHA256 S;
  S.update(RandomBuffer, 32);
  S.finalize(Buffer, 10);
}

void SignBytes(byte Signature[64], byte* Signed, uint16_t Len) {
  Ed25519::sign(Signature, edPrivateKey, edPublicKey, Signed, Len);
}

void LoadIdentityFromFile() {
  FILE *Working = fopen("/spiffs/identity", "r");
  for (int i = 0; i < 32; i++) {
    xPrivateKey[i] = getc(Working);
  }
  for (int i = 0; i < 32; i++) {
    edPrivateKey[i] = getc(Working);
  }
  fclose(Working);

  Working = fopen("/spiffs/enc.pub", "r");
  for (int i = 0; i < 32; i++) {
    xPublicKey[i] = getc(Working);
  }
  fclose(Working);

  Working = fopen("/spiffs/sign.pub", "r");
  for (int i = 0; i < 32; i++) {
    edPublicKey[i] = getc(Working);
  }
  fclose(Working);

  GetIDfromPubKeys(Identity, xPublicKey, edPublicKey);

}

#endif
