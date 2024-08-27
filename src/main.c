#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "SDL.h"
#include "constants.h"
#include "data.h"
#include "utils.h"
#include "config.h"

uint8_t applicationIsRunning = 0;

uint32_t lastFrameTime = 0;
uint32_t currentImageIndex = 0;
uint32_t totalFileCount = 0;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;

typedef struct {
	char* filename;
	char* data;
	long size;
} File;

static int initSDL(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL: %s\n", SDL_GetError());
		showError(NULL, "Error", "Error initializing SDL.\n%s", SDL_GetError());
		return EXIT_FAILURE;
	}

	window = SDL_CreateWindow(GET_TITLE(TITLE, VERSION),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		0
	);

	if (!window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating window: %s\n", SDL_GetError());
		showError(window, "Error", "Error creating window.\n%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

	if (!renderer)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating renderer: %s\n", SDL_GetError());
		showError(window, "Error", "Error creating renderer.\n%s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

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

static File* readFile(const char* filename)
{
	FILE* file = fopen(filename, "rb");

	if (!file)
	{
		showError(window, "Error", "Error opening file %s.\n", filename);
		shutdownSDL();
		return NULL;
	}

	// Determine file size.
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate memory to store file data.
	uint8_t* data = (uint8_t*)malloc(fileSize);

	if (!data)
	{
		showError(window, "Error", "Memory allocation failed.\n");
		fclose(file);
		free(data);
		shutdownSDL();
		return NULL;
	}

	if (fread(data, sizeof(uint8_t), fileSize, file) != fileSize)
	{
		showError(window, "Error", "Failed to write file into memory.\n");
		fclose(file);
		free(data);
		shutdownSDL();
		return NULL;
	}

	// Close the file, it's in memory we're done with it.
	fclose(file);

	File* f = (File*)malloc(sizeof(File));

	if (!f)
	{
		showError(window, "Error", "Memory allocation failed.\n");
		free(data);
		shutdownSDL();
		return NULL;
	}

	if (!filename)
	{
		showError(window, "Error", "Filename is null.\n");
		free(data);
		shutdownSDL();
		return NULL;
	}

	f->filename = (char*)filename;
	f->data = data;
	f->size = fileSize;

	return f;
}

static void loadImage(File* file, uint32_t* pixels)
{
	// Get the first byte.
	uint8_t* b = file->data;

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
}

static void getNextImage(File** files, uint32_t* pixels)
{
	if (currentImageIndex == totalFileCount - 1) {
		currentImageIndex = 0;
	}
	else
	{
		currentImageIndex++;
	}

	loadImage(files[currentImageIndex], pixels);
}

static void getPreviousImage(File** files, uint32_t* pixels)
{
	if (currentImageIndex == 0) {
		currentImageIndex = totalFileCount - 1;
	}
	else
	{
		currentImageIndex--;
	}

	loadImage(files[currentImageIndex], pixels);
}

// TODO: This function should probably not receive these arguments.
static void handleInput(File** files, uint32_t* pixels)
{
	SDL_Event event;
	SDL_PollEvent(&event);
	
	switch (event.type)
	{
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				applicationIsRunning = 0;
			}
			if (event.key.keysym.sym == SDLK_LEFT)
			{
				getPreviousImage(files, pixels);
			}
			if (event.key.keysym.sym == SDLK_RIGHT)
			{
				getNextImage(files, pixels);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == 1)
			{
				getNextImage(files, pixels);
			}
			break;
		case SDL_QUIT:
			applicationIsRunning = 0;
			break;
	}
}

static void update(void)
{
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), lastFrameTime + TARGET_FRAME_TIME));

	lastFrameTime = SDL_GetTicks();

	// TODO: Do any updates here.
}

static void render(void)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_RenderCopy(renderer, texture, NULL, NULL);

	SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{
	int initSuccess = initSDL();

	if (initSuccess != 0)
	{
		return EXIT_FAILURE;
	}

	int kvsCount = 0;
	static char* configFile = "config/hugo.config";
	KeyValue* kvs = readConfig(configFile, &kvsCount);
	
	if (!kvs)
	{
		showError(window, "Error", "Error reading configuration file %s.\n", configFile);
		shutdownSDL();
		return EXIT_FAILURE;
	}

	char* dataFolder = getValue(kvs, kvsCount, "data_folder");
	char* fileToOpen = getValue(kvs, kvsCount, "file");

	if (!dataFolder || !fileToOpen)
	{
		showError(window, "Error", "Missing required config values.\n");
		free(kvs);
		shutdownSDL();
		return EXIT_FAILURE;
	}

	free(kvs);
	free(fileToOpen);

	int fileCount;
	char** filenames = getFilenames(&fileCount);

	totalFileCount = fileCount;

	if (!filenames)
	{
		showError(window, "Error", "filesnames is null.\n");
		shutdownSDL();
		return EXIT_FAILURE;
	}

	File** files = (File**)malloc(sizeof(File*) * fileCount);

	if (!files)
	{
		showError(window, "Error", "Memory allocation failed.\n");
		shutdownSDL();
		return EXIT_FAILURE;
	}

	for (int i = 0; i < fileCount; i++)
	{
		char fullPath[FULL_PATH_SIZE] = {0};
		int n = snprintf(fullPath, sizeof(fullPath), "%s/%s", dataFolder, filenames[i]);

		if (n < 0 || n >= sizeof(fullPath)) {
			showError(window, "Error", "Buffer overflow while constructing full path.\n");
			free(dataFolder);
			shutdownSDL();
			return EXIT_FAILURE;
		}

		files[i] = readFile(fullPath);

		if (!files)
		{
			showError(window, "Error", "files is null.\n");
			free(dataFolder);
			shutdownSDL();
			return EXIT_FAILURE;
		}
	}

	free(dataFolder);

	uint32_t* pixels = (uint32_t*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint32_t));

	if (!pixels)
	{
		showError(window, "Error", "Memory allocation failed.\n");
		free(pixels);
		shutdownSDL();
		return EXIT_FAILURE;
	}

	texture = SDL_CreateTexture(
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
		free(pixels);
		shutdownSDL();
		return EXIT_FAILURE;
	}
	
	loadImage(files[currentImageIndex], pixels);

	applicationIsRunning = 1;

	while (applicationIsRunning)
	{
		handleInput(files, pixels);
		update();
		render();
	}

	for (int i = 0; i < fileCount; i++)
	{
		if (!files)
		{
			free(files[i]->data);
			free(files[i]);
		}
	}

	free(pixels);
	free(files);

	SDL_DestroyTexture(texture);
	shutdownSDL();
	applicationIsRunning = 0;

	return 0;
}
