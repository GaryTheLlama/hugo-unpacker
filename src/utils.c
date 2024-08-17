#include <stdio.h>

#include "utils.h"

void showError(SDL_Window* window, const char* title, const char* details, ...)
{
	// Buffer for the formatted string.
	char buffer[256];
	va_list args;

	va_start(args, details);
	vsnprintf(buffer, sizeof(buffer), details, args);
	va_end(args);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, buffer, window);
}
