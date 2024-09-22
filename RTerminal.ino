#include <Arduino.h>

#include "ESP32Console.h"
#include <FS.h>
#include "SPIFFS.h"
#include "ESP32Console/Helpers/PWDHelpers.h"

#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "esp_vfs.h"
#include "sdmmc_cmd.h"

#include <ArduinoJson.h>

#include "RNodeControl.h"
#include "Commands.h"

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
    } else if(arg == "off") {
        digitalWrite(LED, LOW);
        printf("LED is now off\n");
    } else {
        printf("Unknown argument!\n");
        return EXIT_FAILURE;
    }
    
    //Return EXIT_SUCCESS if everything worked as intended.
    return EXIT_SUCCESS;
}

int InitTestJSON(int argc, char **argv){
  JsonDocument test;
  char filename[PATH_MAX];
  test["frequency"] = "915200000";
  test["bandwidth"] = "125000";
  test["sf"] = 8;
  test["cr"] = 5;

  ESP32Console::console_realpath("/sdcard/rnode/set0.cnf", filename);
  printf(filename);
  FILE* file = fopen(filename, "w");
  char msg[512];
  size_t len = serializeJsonPretty(test,msg);
  printf("\n");
  printf(msg);
  printf("\n");
  printf("%i characters",len);
  printf("\n");
  for(int i = 0; i < len; i++){
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

int InitDirs(int argc, char **argv){
    int mk_ret = mkdir("/sdcard/rnode", 0775);
    printf("mkdir ret %d", mk_ret);
    return EXIT_SUCCESS;
}

void InitSD(){
  printf("\nInitializing SD file system\n");
  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  //host.max_freq_khz = 5000;
  //sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
  //slot_config.gpio_miso = (gpio_num_t)SD_MISO;
  //slot_config.gpio_mosi = (gpio_num_t)SD_MOSI;
  //slot_config.gpio_cs   = (gpio_num_t)SD_CS;
  //slot_config.gpio_sck  = (gpio_num_t)SD_CLK;
  spi_bus_config_t bus_config; 
  
    bus_config.mosi_io_num = SD_MOSI; 
    bus_config.miso_io_num = SD_MISO; 
    bus_config.sclk_io_num = SD_CLK;
    bus_config.quadwp_io_num=-1; 
    bus_config.quadhd_io_num=-1;
    bus_config.max_transfer_sz = 4000;
    bus_config.intr_flags = ESP_INTR_FLAG_LEVEL1; 
  
  printf("Initalizing bus.\n");
  spi_bus_initialize(VSPI_HOST,&bus_config,1);
  
  printf("Initializing slot.\n");
  sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  slot_config.gpio_cs = (gpio_num_t)SD_CS;
  slot_config.host_id = SPI3_HOST;
  
  printf("SD Card Init OK\n");

  esp_vfs_fat_mount_config_t mount_config;
  mount_config.format_if_mount_failed = false;
  mount_config.max_files = 16;
  sdmmc_card_t *sdcard_info;
  esp_err_t e = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &sdcard_info);

  printf("SD Card Mount attempted\n");

  if(e == ESP_OK)
  {
    //printf("SD Card Capacity: %i",(uint64_t)sdcard_info->csd.capacity * sdcard_info->csd.sector_size);
    printf("SD mounted.\n");
  }
  else 
 {
   printf("SD mount failed with code #%d, \"%s\"\n", e, esp_err_to_name(e));
 }
}


void TestFile(){
        char filename[PATH_MAX];
        // We have manually do resolving of . and .., as VFS does not do it
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
    while(!Serial2){delay(100);}
    delay(2000);
    

    //Register builtin commands like 'reboot', 'version', or 'meminfo'
    console.registerSystemCommands();

    //Register our own command
    //First argument is the name with which the command can be executed, second argument is the function to execute and third one is the description shown in help command.
    //console.registerCommand(ConsoleCommand("led", &led, "Turn the LED on or off"));

    console.registerCommand(ConsoleCommand("rnstate", &RNode_Show_State, "Show/set the RNode Radio State [on|off|null(check)]"));
    console.registerCommand(ConsoleCommand("rnfreq", &RNode_Frequency, "Set Frequency in Hz. 0/null gets current freq"));
    console.registerCommand(ConsoleCommand("rntxp", &RNode_TXPower, "Set Transmit power in dB (0-17). 0/null gets current"));
    console.registerCommand(ConsoleCommand("rnbw", &RNode_Bandwidth, "Set Bandwidth. 0/null gets current"));
    console.registerCommand(ConsoleCommand("TestJSON", &InitTestJSON, "Make a bunch of test files"));
    console.registerCommand(ConsoleCommand("TestDirs", &InitDirs, "Make stock dirs"));
    console.registerCommand(ConsoleCommand("mkdir", &mkdirCommand, "Creates the given folder. If directory exists, command is ignored."));
    console.registerCommand(ConsoleCommand("rnload", &RNode_Load_Config, "Loads RNode config from file. [0-9, default 0]"));
    

    //With ConsoleCommandD you can use lambda functions (and anything else that can be cast to std::function). This needs a bit more memory and CPU time than the normal ConsoleCommand.
   // console.registerCommand(ConsoleCommandD("test", [](int argc, char **argv) -> int {
   //     printf("Lambda function test\n");
   //     return EXIT_SUCCESS;
   // }, "Just a test command!"));

    //When console is in use, we can not use Serial.print but you can use printf to output text

    InitSD();
    printf("\n\nWelcome to ESP32Console example. Try out typing 'led off' and 'led on' (without quotes) or see 'help' for all commands.\n");

        //Initalize SPIFFS and mount it on /spiffs
    SPIFFS.begin(true, "/spiffs");
    
    //Set HOME env for easier navigating (type cd to jump to home)
    setenv("HOME", "/", 1);
    //Set PWD to env
    console_chdir("/sdcard");

    //Enable the saving of our command history to SPIFFS. You will be able to see it, when you type ls in your console.
    console.enablePersistentHistory("/sdcard/.history.txt");

    //Register the VFS specific commands
    console.registerVFSCommands();
    TestFile();
    printf("System ready.\nRTerm /sdcard/> \n");

}

void loop()
{
  RNode_Check_UART();
  delay(100);
}
