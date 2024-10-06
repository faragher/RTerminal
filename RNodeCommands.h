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

#ifndef RNODECOMMANDS
#define RNODECOMMANDS

#include "RNSFraming.h"
#include "RNSRegisters.h"
#include "RNSDestination.h"

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

#endif
