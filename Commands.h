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
#include "ESP32Console/Helpers/PWDHelpers.h"

#include "ESP32Console/ConsoleCommand.h"
#include "esp_vfs.h"

#include "compact25519.h"


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
