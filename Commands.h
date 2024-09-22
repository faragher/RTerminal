#include <Arduino.h>
#include "ESP32Console/Helpers/PWDHelpers.h"

#include "ESP32Console/ConsoleCommand.h"
#include "esp_vfs.h"


int mkdirCommand(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "You have to pass exactly one file. Syntax mkdir [FIILE]\n");
        return EXIT_SUCCESS;
    }

    char filename[PATH_MAX];
    ESP32Console::console_realpath(argv[1], filename);

    if(mkdir(filename, 0775)) {
        fprintf(stderr, "Error creating %s: %s\n", filename, strerror(errno));
    }

    return EXIT_SUCCESS;
}
