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

#include <Arduino.h>

#include "ESP32Console.h"
#include <FS.h>
#include "SPIFFS.h"
#include "ESP32Console/Helpers/PWDHelpers.h"

#include <Crypto.h>
#include <RNG.h>

#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "esp_vfs.h"
#include "sdmmc_cmd.h"

#include <ArduinoJson.h>

#include "RNodeControl.h"
#include "RNodeCommands.h"
#include "RNodeHandler.h"
#include "Commands.h"
//#include "CryptoTest.h"
#include "compact25519.h"
#include <bootloader_random.h>

using namespace ESP32Console;

Console console;

// Options

#define USE_SPI_SD

#ifdef USE_SPI_SD
int SD_MISO = GPIO_NUM_19;
int SD_MOSI = GPIO_NUM_23;
int SD_CS = GPIO_NUM_5;
int SD_CLK = GPIO_NUM_18;
#endif

constexpr int LED = 2;

int led(int argc, char **argv)
{
  //Ensure that we have an argument to parse
  if (argc != 2)
  {
    printf("You have to give 'on' or 'off' as a argument (e.g. 'led on')\n");

    //Return EXIT_FAILURE if something did not worked.
    return EXIT_FAILURE;
  }

  //Take the first argument...
  auto arg = String(argv[1]);

  //and use it to decide what to do with the LED
  if (arg == "on") {
    digitalWrite(LED, HIGH);
    printf("LED is now on\n");
  } else if (arg == "off") {
    digitalWrite(LED, LOW);
    printf("LED is now off\n");
  } else {
    printf("Unknown argument!\n");
    return EXIT_FAILURE;
  }

  //Return EXIT_SUCCESS if everything worked as intended.
  return EXIT_SUCCESS;
}

int InitTestJSON(int argc, char **argv) {
  JsonDocument test;
  char filename[PATH_MAX];
  test["frequency"] = "915200000";
  test["bandwidth"] = "125000";
  test["sf"] = 8;
  test["cr"] = 5;
  test["tx"] = 3;

  ESP32Console::console_realpath("/sdcard/rnode/set0.cnf", filename);
  printf(filename);
  FILE* file = fopen(filename, "w");
  char msg[512];
  size_t len = serializeJsonPretty(test, msg);
  printf("\n");
  printf(msg);
  printf("\n");
  printf("%i characters", len);
  printf("\n");
  for (int i = 0; i < len; i++) {
    fputc(msg[i], file);
  }

  //{
  //  printf("JSON test failed");
  //}
  //else{
  //  printf("JSON file created");
  //}

  fclose(file);
  return EXIT_SUCCESS;
}

int InitDirs(int argc, char **argv) {
  int mk_ret = mkdir("/sdcard/rnode", 0775);
  printf("mkdir returned %d\n", mk_ret);
  mk_ret = mkdir("/sdcard/storage", 0775);
  printf("mkdir returned %d\n", mk_ret);
  mk_ret = mkdir("/sdcard/storage/announces", 0775);
  printf("mkdir returned %d\n", mk_ret);
  mk_ret = mkdir("/sdcard/storage/identity", 0775);
  printf("mkdir returned %d\n", mk_ret);
  mk_ret = mkdir("/sdcard/storage/known", 0775);
  printf("mkdir returned %d\n", mk_ret);
  mk_ret = mkdir("/sdcard/storage/messages", 0775);
  printf("mkdir returned %d\n", mk_ret);
  mk_ret = mkdir("/sdcard/storage/names", 0775);
  printf("mkdir returned %d\n", mk_ret);
  return EXIT_SUCCESS;
}

int MakeIdentity(int argc, char **argv) {
    bootloader_random_enable();
  printf("Making new identity\n");
  
  uint8_t seed[X25519_KEY_SIZE];
  esp_fill_random(seed, X25519_KEY_SIZE);


  uint8_t Prv[X25519_KEY_SIZE];
  uint8_t Pub[X25519_KEY_SIZE];
  
  compact_x25519_keygen(Prv, Pub, seed);

  esp_fill_random(seed, X25519_KEY_SIZE);
  RNG.stir(seed,X25519_KEY_SIZE,X25519_KEY_SIZE*5);
  esp_fill_random(seed, X25519_KEY_SIZE);
  RNG.stir(seed,X25519_KEY_SIZE,X25519_KEY_SIZE*5);

  byte Prv_Sign[32];
  byte Pub_Sign[32];

  Ed25519::generatePrivateKey(Prv_Sign);
  Ed25519::derivePublicKey(Pub_Sign,Prv_Sign);

  bootloader_random_disable();

  FILE* file = fopen("/spiffs/identity", "w");
  for (int i = 0; i < 32; i++) {
    fputc(Prv[i], file);
  }
  for (int i = 0; i < 32; i++) {
    fputc(Prv_Sign[i], file);
  }
  fclose(file);

  file = fopen("/spiffs/enc.pub", "w");
  for (int i = 0; i < 32; i++) {
    fputc(Pub[i], file);
  }
  fclose(file);

  file = fopen("/spiffs/sign.pub", "w");
  for (int i = 0; i < 32; i++) {
    fputc(Pub_Sign[i], file);
  }
  fclose(file);
  printf("Identity written.\n");
  return EXIT_SUCCESS;
}

void CryptoProto() {

  bootloader_random_enable();
  printf("Testing X25519\n");
  
  uint8_t seed1[X25519_KEY_SIZE];
  uint8_t seed2[X25519_KEY_SIZE];
  esp_fill_random(seed1, X25519_KEY_SIZE);
  esp_fill_random(seed2, X25519_KEY_SIZE);

  uint8_t sec1[X25519_KEY_SIZE];
  uint8_t pub1[X25519_KEY_SIZE];
  uint8_t sec2[X25519_KEY_SIZE];
  uint8_t pub2[X25519_KEY_SIZE];
  
  compact_x25519_keygen(sec1, pub1, seed1);
  compact_x25519_keygen(sec2, pub2, seed2);
  
  uint8_t shared1[X25519_SHARED_SIZE];
  uint8_t shared2[X25519_SHARED_SIZE];
  compact_x25519_shared(shared1, sec1, pub2);
  compact_x25519_shared(shared2, sec2, pub1);
  uint8_t derived[64];
  compact_x25519_derive_encryption_key(derived, sizeof(derived), shared1, pub1, pub2);
  if (memcmp(shared1, shared2, X25519_SHARED_SIZE) == 0) {
    printf("x25519 check complete. Success\n");
  }

  bootloader_random_disable();
}

void InitSD() {
  printf("\nInitializing SD file system\n");
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  spi_bus_config_t bus_config;

  bus_config.mosi_io_num = SD_MOSI;
  bus_config.miso_io_num = SD_MISO;
  bus_config.sclk_io_num = SD_CLK;
  bus_config.quadwp_io_num = -1;
  bus_config.quadhd_io_num = -1;
  bus_config.max_transfer_sz = 4000;
  bus_config.intr_flags = ESP_INTR_FLAG_LEVEL1;

  printf("Initalizing VSPI bus\n");
  spi_bus_initialize(VSPI_HOST, &bus_config, 1);

  printf("Initializing SPI slot\n");
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = (gpio_num_t)SD_CS;
  slot_config.host_id = SPI3_HOST;

  esp_vfs_fat_mount_config_t mount_config;
  mount_config.format_if_mount_failed = false;
  mount_config.max_files = 16;
  sdmmc_card_t *sdcard_info;
  esp_err_t e = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &sdcard_info);

  printf("SD Card Mount attempted\n");

  if (e == ESP_OK)
  {
    //printf("SD Card Capacity: %i",(uint64_t)sdcard_info->csd.capacity * sdcard_info->csd.sector_size);
    printf("SD mounted.\n");
  }
  else
  {
    printf("SD mount failed with code #%d, \"%s\"\n", e, esp_err_to_name(e));
  }
}
 

void TestFile() {
  char filename[PATH_MAX];
  ESP32Console::console_realpath("test.txt", filename);

  FILE *file = fopen(filename, "r");

  int chr;
  while ((chr = getc(file)) != EOF)
    fprintf(stdout, "%c", chr);
  fclose(file);
}

void setup()
{
  pinMode(LED, OUTPUT);

  //You can change the baud rate and pin numbers similar to Serial.begin() here.
  console.setPrompt("RTerm %pwd%> ");
  console.begin(115200);
  Serial2.begin(115200);
  Serial2.setRxBufferSize(2048);
  while (!Serial2) {
    delay(100);
  }
  //delay(2000);


  //Register builtin commands like 'reboot', 'version', or 'meminfo'
  console.registerSystemCommands();

  console.registerCommand(ConsoleCommand("rnstate", &RNode_Show_State, "Show/set the RNode Radio State [on|off|null(check)]"));
  console.registerCommand(ConsoleCommand("rnfreq", &RNode_Frequency, "Set Frequency in Hz. 0/null gets current freq"));
  console.registerCommand(ConsoleCommand("rntxp", &RNode_TXPower, "Set Transmit power in dB (0-17). 0/null gets current"));
  console.registerCommand(ConsoleCommand("rnbw", &RNode_Bandwidth, "Set Bandwidth. 0/null gets current"));
  console.registerCommand(ConsoleCommand("TestJSON", &InitTestJSON, "Make a bunch of test files"));
  console.registerCommand(ConsoleCommand("initdirs", &InitDirs, "Make stock dirs"));
  console.registerCommand(ConsoleCommand("mkdir", &mkdirCommand, "Creates the given folder. If directory exists, command is ignored."));
  console.registerCommand(ConsoleCommand("rnload", &RNode_Load_Config, "Loads RNode config from file. [0-9, default 0]"));
  console.registerCommand(ConsoleCommand("rnbatt", &RNode_Show_Battery, "Shows RNode Battery State"));
  console.registerCommand(ConsoleCommand("rnairtime", &RNode_Show_Airtime, "Shows RNode airtime/channel load (short/long)"));
  console.registerCommand(ConsoleCommand("rnphy", &RNode_Show_Physical_Parameters, "Shows RNode physical parameters"));
  console.registerCommand(ConsoleCommand("rndi", &RNode_Display_Intensity, "Sets display intensity [0-255]"));
  console.registerCommand(ConsoleCommand("makeid", &MakeIdentity, "Make a new Identity. Use \"override\" to overwrite an old Identity"));
  console.registerCommand(ConsoleCommand("rnmanannounce", &ManualLXMFAnnounce, "Manually announce."));
  




  InitSD();
  //Initalize SPIFFS and mount it on /spiffs
  SPIFFS.begin(true, "/spiffs");

  //Set HOME env for easier navigating (type cd to jump to home)
  setenv("HOME", "/", 1);

  //Set PWD to env
  console_chdir("/sdcard");

  //Enable the saving of our command history to the SDCard.
  console.enablePersistentHistory("/sdcard/.history.txt");

  //Register the VFS specific commands
  console.registerVFSCommands();
  TestFile();
  FILE *configfile = fopen("/sdcard/rterm.cnf", "r");
  if (!configfile)
  {
    printf("***   Configuration file corrupt or missing!   ***\nDirectories and configuration files MUST be written prior to operation.\nRun 'initdirs' to automatically create these requirements.\n");
  }
  fclose(configfile);

    FILE *identityfile = fopen("/spiffs/identity", "r");
  if (!identityfile)
  {
    printf("***   Identity file corrupt or missing!   ***\nRun 'makeid' to automatically create one.\n");
    fclose(identityfile);
  }
  else
  {
    fclose(identityfile);
    LoadIdentityFromFile();
  }
  

  CryptoProto();
  printf("System ready.\nRTerm /sdcard/> \n");

}

void loop()
{
  RNode_Check_UART_New();
  delay(100);
}
