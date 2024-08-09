#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "constants.h"
#include "data.h"
#include "utils.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;

static int initSDL(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL: %s\n", SDL_GetError());
		showError(NULL, "Error", "Error initializing SDL.\n%s", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (SDL_CreateWindowAndRenderer(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN | SDL_RENDERER_ACCELERATED,
		&window,
		&renderer))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating window and renderer: %s\n", SDL_GetError());
		showError(window, "Error", "Error creating window and renderer.\n%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_SetWindowTitle(window, GET_TITLE(TITLE, VERSION));
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	return 0;
}

static void shutdownSDL(void)
{
	if (renderer)
	{
		SDL_DestroyRenderer(renderer);
	}
	
	if (window)
	{
		SDL_DestroyWindow(window);
	}

	SDL_Quit();
}

int main(int argc, char* argv[])
{
	int initSuccess = initSDL();

	if (initSuccess != 0)
	{
		return EXIT_FAILURE;
	}

	char fullPath[FULL_PATH_SIZE];
	char* dataFolder = DEFAULT_DATA_FOLDER;
	char* filename = DEFAULT_FILE;
	snprintf(fullPath, sizeof(fullPath), "%s/%s", dataFolder, filename);

	FILE* file = fopen(fullPath, "rb");

	if (!file)
	{
		showError(window, "Error", "Error opening file %s.\n", fullPath);
		shutdownSDL();
		return EXIT_FAILURE;
	}

	// Determine file size.
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate memory to store file contents and texture's pixel data.
	uint8_t* buffer = malloc(fileSize * sizeof(uint8_t));
	uint32_t* pixels = malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint32_t));
	
	if (!buffer || !pixels)
	{
		showError(window, "Error", "Memory allocation failed.\n");
		fclose(file);
		free(buffer);
		free(pixels);
		shutdownSDL();
		return EXIT_FAILURE;
	}

	// Copy the bytes from the file into memory.
	if (fread(buffer, sizeof(uint8_t), fileSize, file) != fileSize)
	{
		showError(window, "Error", "Failed to write file into memory.\n");
		fclose(file);
		free(buffer);
		free(pixels);
		shutdownSDL();
		return EXIT_FAILURE;
	}

	// Close the file, it's in memory we're done with it.
	fclose(file);

	SDL_Texture* texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBA8888,
		SDL_TEXTUREACCESS_STREAMING,
		IMAGE_WIDTH,
		IMAGE_HEIGHT
	);

	if (!texture)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating texture: %s\n", SDL_GetError());
		showError(window, "Error", "Error creating texture.\n%s\n", SDL_GetError());
		free(buffer);
		free(pixels);
		shutdownSDL();
		return EXIT_FAILURE;
	}

	// Get the first byte.
	uint8_t* b = buffer;

	for (int y = 0; y < IMAGE_HEIGHT; y++)
	{
		for (int x = 0; x < IMAGE_WIDTH / 8; x++)
		{
			// Get byte so we can figure out the bits.
			uint8_t currentByte = *b++;

			// Decode the bits.
			for (int i = 7; i >= 0; i--)
			{
				// Extract the current bit.
				int currentBit = (currentByte >> i) & 0x01;
				uint32_t color = currentBit ? COLOR_WHITE : COLOR_BLACK;
				
				// y * WINDOW_WIDTH gives us the starting index for each row.
				// x * 8 calculates the offset within the current row to the beginning of a group of 8 pixels
				// that the current byte represents.
				// (7 - i) calculates the position of a specific pixel within the 8-pixel group.
				// (7 - i) inverts the bit order so that most significant bit (7) is the leftmost pixel and least (0)
				// is the rightmost pixel.

				int pixelIndexInRow = x * 8 + (7 - i);
				int pixelIndex = y * IMAGE_WIDTH + pixelIndexInRow;
				pixels[pixelIndex] = color;
			}
		}
	}
	
	SDL_UpdateTexture(texture, NULL, pixels, IMAGE_WIDTH * sizeof(uint32_t));
	
	int quit = 0;
	SDL_Event event;

	while (!quit)
	{
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				quit = 1;
			}
		}

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	free(buffer);
	free(pixels);

	SDL_DestroyTexture(texture);
	shutdownSDL();

	return 0;
}
