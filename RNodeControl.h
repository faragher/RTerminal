// Copyright (C) 2024, Michael Faragher
// Based on the RNode Firmware by Mark Qvist

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

#ifndef FRAMING_H
#define FRAMING_H

#define FEND            0xC0
#define FESC            0xDB
#define TFEND           0xDC
#define TFESC           0xDD

#define CMD_UNKNOWN     0xFE
#define CMD_DATA        0x00
#define CMD_FREQUENCY   0x01
#define CMD_BANDWIDTH   0x02
#define CMD_TXPOWER     0x03
#define CMD_SF          0x04
#define CMD_CR          0x05
#define CMD_RADIO_STATE 0x06
#define CMD_RADIO_LOCK  0x07
#define CMD_DETECT      0x08
#define CMD_IMPLICIT    0x09
#define CMD_LEAVE       0x0A
#define CMD_ST_ALOCK    0x0B
#define CMD_LT_ALOCK    0x0C
#define CMD_PROMISC     0x0E
#define CMD_READY       0x0F

#define CMD_STAT_RX     0x21
#define CMD_STAT_TX     0x22
#define CMD_STAT_RSSI   0x23
#define CMD_STAT_SNR    0x24
#define CMD_STAT_CHTM   0x25
#define CMD_STAT_PHYPRM 0x26
#define CMD_STAT_BAT    0x27
#define CMD_BLINK       0x30
#define CMD_RANDOM      0x40

#define CMD_FB_EXT      0x41
#define CMD_FB_READ     0x42
#define CMD_FB_WRITE    0x43
#define CMD_FB_READL    0x44
#define CMD_DISP_INT    0x45
#define CMD_DISP_ADDR   0x63
#define CMD_BT_CTRL     0x46
#define CMD_BT_PIN      0x62

#define CMD_BOARD       0x47
#define CMD_PLATFORM    0x48
#define CMD_MCU         0x49
#define CMD_FW_VERSION  0x50
#define CMD_ROM_READ    0x51
#define CMD_ROM_WRITE   0x52
#define CMD_CONF_SAVE   0x53
#define CMD_CONF_DELETE 0x54
#define CMD_DEV_HASH    0x56
#define CMD_DEV_SIG     0x57
#define CMD_FW_HASH     0x58
#define CMD_HASHES      0x60
#define CMD_FW_UPD      0x61
#define CMD_UNLOCK_ROM  0x59
#define ROM_UNLOCK_BYTE 0xF8
#define CMD_RESET       0x55
#define CMD_RESET_BYTE  0xF8

#define DETECT_REQ      0x73
#define DETECT_RESP     0x46

#define RADIO_STATE_OFF 0x00
#define RADIO_STATE_ON  0x01

#define CMD_ERROR       0x90

#endif

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
// byte edSignature[64]; // I think this doesn't need to be global

// General Enums
const char HexEnum[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

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

// CMD_DATA        0x00
// CMD_FREQUENCY   0x01
int RNode_Frequency(int argc, char **argv) {
  byte Command[7] = {FEND, CMD_FREQUENCY, 0x00, 0x00, 0x00, 0x00, FEND};
  auto Arg = String(argv[1]);
  if (argc > 1 && Arg != "0") {
    //uint32_t T =ParseCharInt(argv[0],9);
    uint32_t T = strtoul(argv[1], NULL, 0);
    //printf("Setting %i",T);
    for (int i = 0; i < 4; i++) {
      //printf("0x%02x ",IntToByte(T,i));
      //Convert endianness
      Command[5 - i] = IntToByte(T, i);
    }
  }
  //  printf("TX: ");
  //  for (int i = 0; i < 7; i++) {
  //    printf("0x%02x ", Command[i]);
  //  }

  Serial2.write(Command, 7);
  return EXIT_SUCCESS;
}

// CMD_BANDWIDTH   0x02
int RNode_Bandwidth(int argc, char **argv) {
  byte Command[7] = {FEND, CMD_BANDWIDTH, 0x00, 0x00, 0x00, 0x00, FEND};
  auto Arg = String(argv[1]);
  if (argc > 1 && Arg != "0") {
    uint32_t T = strtoul(argv[1], NULL, 0);
    for (int i = 0; i < 4; i++) {
      Command[5 - i] = IntToByte(T, i);
    }
  }
  Serial2.write(Command, 7);
  return EXIT_SUCCESS;
}

// CMD_TXPOWER     0x03
int RNode_TXPower(int argc, char **argv) {
  byte Command[4] = {FEND, CMD_TXPOWER, 0xff, FEND};
  auto Arg = String(argv[1]);
  if (argc > 1 && Arg != "0") {
    byte T = atoi(argv[1]);
    if (T > 17) T = 17;
    if (T < 0) T = 0;
    //Command[2]=IntToByte(T,0);
    Command[2] = T;
  }
  Serial2.write(Command, 4);
  return EXIT_SUCCESS;
}

// CMD_SF          0x04
int RNode_SF(int argc, char **argv) {
  byte Command[4] = {FEND, CMD_SF, 0xff, FEND};
  auto Arg = String(argv[1]);
  if (argc > 1 && Arg != "0") {
    byte T = atoi(argv[1]);
    if (T > 12) T = 12;
    if (T < 5) T = 5;
    Command[2] = T;
  }
  Serial2.write(Command, 4);
  return EXIT_SUCCESS;
}

// CMD_CR          0x05
int RNode_CR(int argc, char **argv) {
  byte Command[4] = {FEND, CMD_CR, 0xff, FEND};
  auto Arg = String(argv[1]);
  if (argc > 1 && Arg != "0") {
    byte T = atoi(argv[1]);
    if (T > 8) T = 8;
    if (T < 5) T = 5;
    Command[2] = T;
  }
  Serial2.write(Command, 4);
  return EXIT_SUCCESS;
}
// CMD_RADIO_STATE 0x06
int RNode_Show_State(int argc, char **argv) {
  //printf("Show my radio state.");
  byte Command[4] = {0xc0, 0x06, 0xff, 0xc0};
  if (argc > 1) {
    auto Arg = String(argv[1]);
    if (Arg == "0") {
      printf("Attempting to shut down radio\n");
      Command[2] = 0x00;
    }
    else if (Arg == "1") {
      printf("Attempting to bring up radio\n");
      Command[2] = 0x01;
    }
  }
  //printf("0x%02x 0x%02x 0x%02x 0x%02x", Command[0], Command[1], Command[2], Command[3]);
  Serial2.write(Command, 4);
  return EXIT_SUCCESS;
}

// CMD_RADIO_LOCK  0x07
// CMD_DETECT      0x08
// CMD_IMPLICIT    0x09
// CMD_LEAVE       0x0A
int RNode_Leave(int argc, char **argv) {
  byte Command[4] = {FEND, CMD_LEAVE, 0xff, FEND};
  Serial2.write(Command, 4);
  return EXIT_SUCCESS;
}

// CMD_ST_ALOCK    0x0B
// CMD_LT_ALOCK    0x0C
// CMD_PROMISC     0x0E
// CMD_READY       0x0F

// CMD_STAT_RX     0x21
// CMD_STAT_TX     0x22
// CMD_STAT_RSSI   0x23
// CMD_STAT_SNR    0x24
// CMD_STAT_CHTM   0x25
// CMD_STAT_PHYPRM 0x26
// CMD_STAT_BAT    0x27
// CMD_BLINK       0x30
// CMD_RANDOM      0x40

// CMD_FB_EXT      0x41
// CMD_FB_READ     0x42
// CMD_FB_WRITE    0x43
// CMD_FB_READL    0x44
// CMD_DISP_INT    0x45
int RNode_Display_Intensity(int argc, char **argv) {
  byte Command[4] = {FEND, CMD_DISP_INT, 0x00, FEND};
  byte intensity = 0x00;
  if (argc > 1) {
    auto Arg = String(argv[1]);
    intensity = Arg.toInt();
    if (intensity < 0)intensity = 0;
    if (intensity > 255)intensity = 255;
    //printf("Setting intensity to %i\n",intensity);
  }

  Command[2] = intensity;
  Serial2.write(Command, 4);
  return EXIT_SUCCESS;
}
// CMD_DISP_ADDR   0x63
// CMD_BT_CTRL     0x46
// CMD_BT_PIN      0x62

// CMD_BOARD       0x47
// CMD_PLATFORM    0x48
// CMD_MCU         0x49
// CMD_FW_VERSION  0x50
// CMD_ROM_READ    0x51
// CMD_ROM_WRITE   0x52
// CMD_CONF_SAVE   0x53
// CMD_CONF_DELETE 0x54
// CMD_DEV_HASH    0x56
// CMD_DEV_SIG     0x57
// CMD_FW_HASH     0x58
// CMD_HASHES      0x60
// CMD_FW_UPD      0x61
// CMD_UNLOCK_ROM  0x59
// ROM_UNLOCK_BYTE 0xF8
// CMD_RESET       0x55
// CMD_RESET_BYTE  0xF8

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
    //char filename[49] = "/sdcard/storage/";
    char filename[59] = "/sdcard/storage/announces/";
    printf(filename);
    printf("\n");
    for(int i = 0; i < 16; i++){
      char High = HexEnum[(Hash[i]&0b11110000)>>4];
      char Low = HexEnum[(Hash[i]&0b00001111)];
      filename[26+(2*i)] = High;
      filename[26+(2*i)+1] = Low;
      
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
  }
  else {
    printf("Bad Signature.\n");
  }
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

void RNode_Process() {
  char cmd = CBGetE();
  bool wasData = false;
  if (cmd == FEND) cmd = CBGetE();
  //printf("Command: 0x%02x", cmd);
  if (cmd == CMD_DATA) {
    printf("Incoming data: \n");
    char payload = CBGetE();
    if ((payload & 0b00000001)  == 0b00000001) {
      Process_Announce(payload);
    }
    //else if (payload == 0b01010001 || payload == 0b01110001) {
    //  Process_Type_Two_Announce(payload);
    //}
    else {
      Unknown_Packet_Decoder(payload);
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
