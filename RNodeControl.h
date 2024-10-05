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

#include <Crypto.h>
#include <Ed25519.h>
#include <SHA256.h>
#include "RNSCrypto.h"

#include "RNSFraming.h"
#include "RNSRegisters.h"


void ShowSerial(char In) {
  char Buffer[5];
  char High = In >> 4;
  char Low = In & 0b00001111;
  Buffer[0] = '0';
  Buffer[1] = 'x';
  Buffer[2] = HexEnum[High];
  Buffer[3] = HexEnum[Low];
  Buffer[4] = 0x00;
  printf(Buffer);
  printf(" ");

}


uint32_t ParseCharInt(char *payload, int payloadlength) {
  uint32_t Buffer = 0;
  uint32_t A;
  for (int i = 0; i < payloadlength; i++) {
    A =  int(payload[i] - '0');
    A = A * (pow(10, payloadlength - i - 1));
    //printf("%i",A);
    Buffer += A;
  }
  return Buffer;
}


byte IntToByte(uint32_t I, int pos) {
  return (byte)(I >> (8 * pos));
}

//void ClearSerialBuffer() {
//  memset(SerialBuffer, 0x00, 500);
//}

int SetEcho(int argc, char **argv)
{

}



char CBGet() {
  char C = SerialBuffer[SerialBufferReadPointer];
  //printf("Get: 0x%02x \n",C);
  SerialBufferReadPointer++;
  if (SerialBufferReadPointer > 1023) {
    SerialBufferReadPointer = 0;
  }
  return C;
}

char CBGetE() {
  //  #define FEND            0xC0
  //  #define FESC            0xDB
  //  #define TFEND           0xDC
  //  #define TFESC           0xDD
  char C = SerialBuffer[SerialBufferReadPointer];
  if (C == FESC) {
    //printf("Escape! ");
    SerialBufferReadPointer++;
    if (SerialBufferReadPointer > 1023) {
      SerialBufferReadPointer = 0;
    }
    C = SerialBuffer[SerialBufferReadPointer];
    if (C == TFEND) {
      C = FEND;
      //printf("FEND\n");
    }

    if (C == TFESC) {
      C = FESC;
      //printf("FESC\n");
    }
  }
  //printf("Get: 0x%02x \n",C);
  SerialBufferReadPointer++;
  if (SerialBufferReadPointer > 1023) {
    SerialBufferReadPointer = 0;
  }
  return C;
}

void CBSet(char C) {
  if ((SerialBufferWritePointer + 1) % 1023 == SerialBufferReadPointer) {
    printf("ERROR: Circular Buffer Full!");
    return;
  }
  //printf("0x%02x ",C);
  SerialBuffer[SerialBufferWritePointer] = C;
  SerialBufferWritePointer++;
  if (SerialBufferWritePointer > 1023)SerialBufferWritePointer = 0;
}


void Process_Announce(byte header) {
  printf("Received Announce\n");
  byte IFAC = (header & 0b10000000) >> 7;
  byte HeaderType = (header & 0b01000000) >> 6;
  byte hContext = (header & 0b00100000) >> 5;
  byte Propagation = (header & 0b00010000) >> 4;
  byte Destination = (header & 0b00001100) >> 2;
  byte Type = (header & 0b00000011);
  uint16_t AppDataLength = 0;
  byte Ticket = 0;
  printf("IFAC:        %i\n", IFAC);
  printf("Header Type: %i\n", HeaderType);
  printf("Context:     %i\n", hContext);
  printf("Propagation: %i\n", Propagation);
  printf("Destination: %i\n", Destination);
  printf("Type:        %i\n", Type);
  printf("Hops:        %i\n", CBGetE());
  byte Hash[16];
  printf("Hash:        ");
  for (int i = 0; i < 16; i++) {
    Hash[i] = CBGetE();
    printf("%02x", Hash[i]);
  }
  byte HashTwo[16];
  if ((header & 0b01000000) == 0b01000000) {
    printf("\nHashTwo:     ");
    for (int i = 0; i < 16; i++) {
      HashTwo[i] = CBGetE();
      printf("%02x", HashTwo[i]);
    }
  }

  printf("\nContext:     0x%02x\n", CBGetE());
  byte PKey[32];
  printf("Public Key:  ");
  for (int i = 0; i < 32; i++) {
    PKey[i] = CBGetE();
    printf("%02x", PKey[i]);
  }
  byte SKey[32];
  printf("\nSigning Key: ");
  for (int i = 0; i < 32; i++) {
    SKey[i] = CBGetE();
    printf("%02x", SKey[i]);
  }
  printf("\nName Hash:   ");
  byte NameHash[10];
  for (int i = 0; i < 10; i++) {
    NameHash[i] = CBGetE();
    printf("%02x", NameHash[i]);
  }
  printf("\nRandom Hash: ");
  byte RandomHash[10];
  for (int i = 0; i < 10; i++) {
    RandomHash[i] = CBGetE();
    printf("%02x", RandomHash[i]);
  }
  if ((header & 0b00100001) == 0b00100001) {
    byte Ratchet[32];
    printf("\nRatchet: ");
    for (int i = 0; i < 32; i++) {
      Ratchet[i] = CBGetE();
      printf("%i", Ratchet[i]);
    }
  }

  byte ASignature[64];
  printf("\nSignature:   ");
  for (int i = 0; i < 64; i++) {
    ASignature[i] = CBGetE();
    printf("%02x", ASignature[i]);
  }
  printf("\nAppData:     ");
  byte buf;
  byte AppData[300];
  uint16_t AppDataIndex = 0;
  while (buf != FEND) {
    buf = CBGetE();
    printf("0x%02x ", buf);
    AppData[AppDataIndex] = buf;
    AppDataIndex++;
  }

  AppDataIndex = 0;
  if (AppData[0] == 0x93) { // Three field MSGPack - Indicates peopagation node
    if (AppData[1] == 0xc3)printf("\nActive propagation node.\n");
    if (AppData[1] == 0xc2)printf("\nInactive propagation node.\n");
    printf("  Time: %i\n", (uint32_t)AppData[3] << 24 | (uint32_t)AppData[4] << 16 | (uint32_t)AppData[5] << 8 | (uint32_t)AppData[6]);
    if (AppData[7] == 0xcd) {
      printf("  Max:  %u\n", (uint16_t)AppData[8] << 8 | (uint16_t)AppData[9]);
      AppDataIndex = 9;
    }
    if (AppData[7] == 0xcb) {
      printf("  Max:  Float - Ignoring");
      AppDataIndex = 15;
    }
    buf = AppData[AppDataIndex];
    while (buf != FEND) {
      AppDataIndex++;
      printf("0x%02x ", buf);
      buf = AppData[AppDataIndex];
    }
    AppDataLength = AppDataIndex;
  }
  else if (AppData[0] == 0x92) { // Two field MSGPack - Indicates modern normal announce.
    byte len = AppData[1];
    byte ADName[len];
    for (int i = 0; i < len; i++) {
      ADName[i] = AppData[2 + len];
      printf("%c", ADName[i]);
    }
    printf("\n");
    if (AppData[3 + len] != FEND) {
      printf("Ticket: %i", AppData[4 + len]);
    }
    AppDataIndex = 5 + len;
    buf = AppData[AppDataIndex];
    while (buf != FEND) {
      //printf("0x%02x ", buf);
      AppDataIndex++;
      buf = AppData[AppDataIndex];
    }
    AppDataLength = AppDataIndex;
  }
  else { // Legacy Announce
    //byte ADNameBuffer[300];
    uint16_t ADNameLen = 0;
    buf = AppData[ADNameLen];
    while (buf != FEND) {
      AppData[ADNameLen] = buf;
      //ADNameBuffer[ADNameLen] = buf;
      //printf("%c", buf);
      ADNameLen++;
      buf = AppData[ADNameLen];
    }
    byte ADName[ADNameLen];
    AppDataLength = ADNameLen;
    //byte SignedData[SignedDataLength];
    printf("\nName Length: %i\n", ADNameLen);
    for (int i = 0; i < (ADNameLen); i++) {
      //ADName[i] = ADNameBuffer[i];
      ADName[i] = AppData[i];
      printf("%c", ADName[i]);
    }

  }
  printf("\n");
  uint16_t SignedDataLength = 16 + 64 + 10 + 10 + AppDataLength;
  byte SignedData[SignedDataLength];
  if (HeaderType == 0) {
    memcpy(SignedData, Hash, 16);
  }
  else {
    memcpy(SignedData, HashTwo, 16);
  }
  memcpy(SignedData + 16, PKey, 32);
  memcpy(SignedData + 48, SKey, 32);
  memcpy(SignedData + 80, NameHash, 10);
  memcpy(SignedData + 90, RandomHash, 10);

  memcpy(SignedData + 100, AppData, AppDataLength);

  printf("\nSigned Data: ");
  for (int i = 0; i < SignedDataLength; i++) {
    printf("0x%02x ", SignedData[i]);
  }
  printf("\n");
  printf("Signed Data Length: %i\n", SignedDataLength);
  bool verified = Ed25519::verify(ASignature, SKey, SignedData, SignedDataLength);
  if (verified)
  {
    printf("Signature OK\n");
    SHA256 IDHash;
    byte Digest[16];
    GetIDfromPubKeys(Digest, PKey, SKey);

    printf("\nDerived Identity: ");
    for (int i = 0; i < 16; i++) {
      printf("%02x", Digest[i]);
    }
    //char filename[49] = "/sdcard/storage/";
    char filename[60] = "/sdcard/storage/identities/";
    //printf(filename);
    printf("\n");
    for (int i = 0; i < 16; i++) {
      char High = HexEnum[(Digest[i] & 0b11110000) >> 4];
      char Low = HexEnum[(Digest[i] & 0b00001111)];
      filename[27 + (2 * i)] = High;
      filename[27 + (2 * i) + 1] = Low;

    }
    printf(filename);
    FILE* file = fopen(filename, "w");
    for (int i = 0; i < 32; i++) {
      fputc(PKey[i], file);
    }
    for (int i = 0; i < 32; i++) {
      fputc(SKey[i], file);
    }
    fputc(Ticket, file);
    fclose(file);
    byte Aspects[14] = "lxmf.delivery";
    byte AspectHash[16];
    byte FullDestination[26];
    GetNameHash(AspectHash, Aspects, 13);
    byte DestinationHash[16];
    GetDestinationFromIDandNameHash(DestinationHash, Digest, AspectHash);
    printf("\nDerived Hash:\n");
    for (int i = 0; i < 16; i++) {
      printf("%02x", DestinationHash[i]);
    }
  }
  else {
    printf("Bad Signature.\n");
  }


}

void SendRNode(byte* B, uint16_t Len) {
  Serial2.write(FEND);
  Serial2.write(CMD_DATA);
  for (int i = 0; i < Len; i++) {
    if (B[i] == FESC) {
      Serial2.write(FESC);
      Serial2.write(TFESC);
    }
    else if ( B[i] == FEND) {
      Serial2.write(FESC);
      Serial2.write(TFEND);
    }

    else {
      Serial2.write(B[i]);
    }
  }
  Serial2.write(FEND);
  //Serial2.write(Command, Len);
}

void SendAnnounce(byte* NameHash, byte* AppData, uint16_t Len) {
  //uint16_t Len;
  byte Buffer[500];
  Buffer[0] = 1;
  Buffer[1] = 0;
  byte DestinationHash[16];
  GetDestinationFromIDandNameHash(DestinationHash, Identity, NameHash);
  memcpy(Buffer+2,DestinationHash,16);
  Buffer[18] = 0;
  memcpy(Buffer+19,xPublicKey,32);
  memcpy(Buffer+51,edPublicKey,32);
  memcpy(Buffer+83,NameHash,10);
  byte RandomBuffer[10];
  GetRandomHash(RandomBuffer);
  memcpy(Buffer+93,RandomBuffer,10);
  byte SignedData[100+Len]; 
  //memcpy(SignedData, Buffer+19, 103-19); // The Easy Way That Doesn't Work
  memcpy(SignedData,DestinationHash,16);
  memcpy(SignedData+16,xPublicKey,32);
  memcpy(SignedData+48,edPublicKey,32);
  memcpy(SignedData+80,NameHash,10);
  memcpy(SignedData+90,RandomBuffer,10);
  if(Len>0){
    memcpy(SignedData+100, AppData, Len);
  }
  byte Signature[64];
  SignBytes(Signature,SignedData,100+Len);
  memcpy(Buffer+103, Signature, 64);
  memcpy(Buffer+167, AppData, Len);
  SendRNode(Buffer, 167+Len);

  printf("\nAnnounce Debug:\n");
  for (int i = 0; i < 167+Len; i++){
    printf("%02x",Buffer[i]);
  }
  printf("\n");
  
}

int ManualLXMFAnnounce(int argc, char **argv){
  byte NameHash[10];
  byte Name[14] = "lxmf.delivery";
  GetNameHash(NameHash, Name,13);
  byte AppData[11] = "Fishmonger";
  SendAnnounce(NameHash, AppData, 10);
  return EXIT_SUCCESS;
}


void Unknown_Packet_Decoder(byte header) {
  //byte header = CBGetE();
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
  byte Hops = CBGetE();
  printf("Hops:        %i\n", Hops);
  byte Hash[16];
  printf("Hash One: ");
  for (int i = 0; i < 16; i++) {
    Hash[i] = CBGetE();
    printf("%02x", Hash[i]);
  }
  printf("\n");
  if (HeaderType == 1) {
    byte HashTwo[16];
    printf("Hash Two: ");
    for (int i = 0; i < 16; i++) {
      Hash[i] = CBGetE();
      printf("%02x", HashTwo[i]);
    }
    printf("\n");
  }
  byte Context = CBGetE();
  printf("Context:     %i\n", Context);
  char payload = CBGetE();
  while (payload != FEND) {
    printf("0x%02x ", payload);
    payload = CBGetE();
  }
}


// Airtime
int RNode_Show_Airtime(int argc, char **argv) {
  printf("Airtime:      %i / %i\n", airtimeShort, airtimeLong);
  printf("Channel Load: %i / %i\n", channelLoadShort, channelLoadLong);
  return EXIT_SUCCESS;
}


// Physical Parameters
int RNode_Show_Physical_Parameters(int argc, char **argv) {
  printf("Symbol time:     %i microseconds\n", symbolTimeMS);
  printf("Symbol rate:     %i baud\n", symbolRate);
  printf("Preamble length: %i\n", preambleSymbols);
  printf("Preamble time:   %i ms\n", preambleTimeMS);
  printf("CSMA slot time:  %i ms\n", csmaSlotTimeMS);
  return EXIT_SUCCESS;
}

// Battery State
int RNode_Show_Battery(int argc, char **argv) {
  printf("Battery State: ");
  if (BatteryState == 0x01) {
    printf("Discharging ");
  }
  else if (BatteryState == 2) {
    printf("Charging ");
  }
  else if (BatteryState == 3) {
    printf("Charged ");
  }
  else {
    printf("Undefined <0x%02x> ", BatteryState);
  }
  printf("%i\n", BatteryLevel);
  return EXIT_SUCCESS;
}

// SHOW RNode State

// SAVE RNode State

// LOAD RNode State
int RNode_Load_Config(int argc, char **argv) {
  char filename[23] = "/sdcard/rnode/set0.cnf";
  char Buffer[512];
  byte slot = 0;
  JsonDocument doc;
  FILE *file = fopen(filename, "r");
  char chr;
  int Index = 0;
  while (Index < 512) {
    chr = getc(file);
    if ( chr == EOF) break;
    Buffer[Index] = chr;
    Index++;
  }
  printf("\n");
  fclose(file);
  deserializeJson(doc, Buffer);
  uint32_t frequency = doc["frequency"];
  uint32_t bandwidth = doc["bandwidth"];
  byte sf = doc["sf"];
  byte cr = doc["cr"];
  byte tx = doc["tx"];

  byte CommandL[7] = {FEND, CMD_FREQUENCY, 0x00, 0x00, 0x00, 0x00, FEND};
  for (int i = 0; i < 4; i++) {
    CommandL[5 - i] = IntToByte(frequency, i);
  }
  //radioHz = frequency;
  Serial2.write(CommandL, 7);

  CommandL[1] = CMD_BANDWIDTH;
  for (int i = 0; i < 4; i++) {
    //Convert endianness
    CommandL[5 - i] = IntToByte(bandwidth, i);
  }
  //radioBW = bandwidth;
  Serial2.write(CommandL, 7);

  byte CommandS[4] = {FEND, CMD_SF, 0xff, FEND};
  if (sf > 12) sf = 12;
  if (sf < 5) sf = 5;
  CommandS[2] = sf;
  //radioSF = sf;
  Serial2.write(CommandS, 4);
  CommandS [1] = CMD_CR;
  if (cr > 8) cr = 8;
  if (cr < 5) cr = 5;
  CommandS[2] = cr;
  //radioCR = cr;
  Serial2.write(CommandS, 4);

  CommandS [1] = CMD_TXPOWER;
  if (tx > 17) tx = 17;
  if (tx < 0) tx = 0;
  CommandS[2] = tx;
  //radioTX = tx;
  Serial2.write(CommandS, 4);

  printf("\nConfiguration %i loaded.\n", slot);
  printf("Hz: %i\n", frequency);
  printf("BW: %i\n", bandwidth);
  printf("SF: %i\n", sf);
  printf("CR: %i\n", cr);

  //delay(250); //@115200 buad, ~ 3kb Plenty of time?

  //  printf("\Returned\n", slot);
  //  printf("Hz: %i\n", radioHz);
  //  printf("BW: %i\n", radioBW);
  //  printf("SF: %i\n", radioSF);
  //  printf("CR: %i\n", radioCR);


  return EXIT_SUCCESS;
}
