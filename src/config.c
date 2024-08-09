#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

KeyValue* readConfig(const char* filename, int* count)
{
	FILE* file = fopen(filename, "r");

	if (!file)
	{
		return NULL;
	}

	KeyValue* kv = NULL;
	int keyCount = 0;
	char line[MAX_LINE_LENGTH];

	while (fgets(line, sizeof(line), file))
	{
		// Ignore empty lines and comments.
		if (line[0] == '\n' || line[0] == '#')
		{
			continue;
		}

		line[strcspn(line, "\n\r")] = 0;

		char* delimiter = strchr(line, '=');

		if (!delimiter)
		{
			continue;
		}

		// Get key value pair.
		*delimiter = '\0';
		char* currentKey = line;
		char* currentValue = delimiter + 1;

		// Kill leading white space.
		while (*currentKey == ' ' || *currentKey == '\t')
		{
			currentKey++;
		}

		while (*currentValue == ' ' || *currentValue == '\t')
		{
			currentValue++;
		}

		// Now kill trailing.
		char* keyEndOfString = currentKey + strlen(currentKey) - 1;
		while (keyEndOfString > currentKey && (*keyEndOfString == ' ' || *keyEndOfString == '\t'))
		{
			*keyEndOfString-- = '\0';
		}

		char* valueEndOfString = currentValue + strlen(currentValue) - 1;
		while (valueEndOfString > currentValue && (*valueEndOfString == ' ' || *valueEndOfString == '\t'))
		{
			*valueEndOfString-- = '\0';
		}

		kv = realloc(kv, (keyCount + 1) * sizeof(KeyValue));

		if (!kv)
		{
			fclose(file);
			return NULL;
		}

		strncpy(kv[keyCount].key, currentKey, MAX_KEY_LENGTH);
		strncpy(kv[keyCount].value, currentValue, MAX_VALUE_LENGTH);
		keyCount++;
	}

	fclose(file);
	*count = keyCount;

	return kv;
}

// This is absolutely naive and stupid. It's a linear search so if the key/value array grows in size (think, millions
// of keys) we might be in trouble. But that's not our use case right now. So for today, naivety ahoy! For tomorrow,
// hash tables or something.
char* getValue(KeyValue* kvs, int kvsCount, const char* key)
{
	if (!kvs || !key)
	{
		return NULL;
	}

	char* value = malloc(MAX_KEY_LENGTH + 1);

	if (!value)
	{
		return NULL;
	}

	for (int i = 0; i < kvsCount; i++)
	{
		if (strcmp(kvs[i].key, key) == 0)
		{
			strncpy(value, kvs[i].value, MAX_KEY_LENGTH);
			value[MAX_KEY_LENGTH] = '\0';
			return value;
		}
	}

	// Not found, bye!
	free(value);
	return NULL;
}
