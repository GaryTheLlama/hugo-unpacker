#pragma once

#include "data.h"

#define TITLE "Hugo Unpacker"
#define VERSION 0.3

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 400

#define IMAGE_WIDTH 320
#define IMAGE_HEIGHT 200

#define STRINGIFY(x) #x
#define GET_TITLE(x, y) x " " STRINGIFY(y)
#define CONCAT(x, y) x ## y

#define FULL_PATH_SIZE 256
#define MAX_FILES_TO_SEARCH 1024

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0x00000000

#define TARGET_FPS 30
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)
