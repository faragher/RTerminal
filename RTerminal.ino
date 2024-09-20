#include <Arduino.h>

#include "ESP32Console.h"
#include <FS.h>
#include "SPIFFS.h"
#include "ESP32Console/Helpers/PWDHelpers.h"

#include "RNodeControl.h"

using namespace ESP32Console;

Console console;



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

void setup()
{
    pinMode(LED, OUTPUT);

    //You can change the baud rate and pin numbers similar to Serial.begin() here.
    console.begin(115200);
    Serial2.begin(115200);

    //Register builtin commands like 'reboot', 'version', or 'meminfo'
    console.registerSystemCommands();

    //Register our own command
    //First argument is the name with which the command can be executed, second argument is the function to execute and third one is the description shown in help command.
    //console.registerCommand(ConsoleCommand("led", &led, "Turn the LED on or off"));

    console.registerCommand(ConsoleCommand("rnstate", &RNode_Show_State, "Show/set the RNode Radio State [on|off|null(check)]"));
    console.registerCommand(ConsoleCommand("rnfreq", &RNode_Frequency, "Set Frequency in Hz. 0/null gets current freq"));
    console.registerCommand(ConsoleCommand("rntxp", &RNode_TXPower, "Set Transmit power in dB (0-17). 0/null gets current"));
    console.registerCommand(ConsoleCommand("rnbw", &RNode_Bandwidth, "Set Bandwidth. 0/null gets current"));
    

    //With ConsoleCommandD you can use lambda functions (and anything else that can be cast to std::function). This needs a bit more memory and CPU time than the normal ConsoleCommand.
   // console.registerCommand(ConsoleCommandD("test", [](int argc, char **argv) -> int {
   //     printf("Lambda function test\n");
   //     return EXIT_SUCCESS;
   // }, "Just a test command!"));

    //When console is in use, we can not use Serial.print but you can use printf to output text
    printf("\n\nWelcome to ESP32Console example. Try out typing 'led off' and 'led on' (without quotes) or see 'help' for all commands.");

        //Initalize SPIFFS and mount it on /spiffs
    SPIFFS.begin(true, "/spiffs");

    console.setPrompt("RTerm %pwd%> ");
    
    //Set HOME env for easier navigating (type cd to jump to home)
    setenv("HOME", "/spiffs", 1);
    //Set PWD to env
    console_chdir("/spiffs");

    //Enable the saving of our command history to SPIFFS. You will be able to see it, when you type ls in your console.
    //console.enablePersistentHistory("/spiffs/.history.txt");

    //Register the VFS specific commands
    console.registerVFSCommands();
}

void loop()
{
  RNode_Check_UART();
  delay(100);
}
