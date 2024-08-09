#pragma once

#define MAX_LINE_LENGTH 256
#define MAX_KEY_LENGTH 64
#define MAX_VALUE_LENGTH 128

typedef struct {
	char key[MAX_KEY_LENGTH];
	char value[MAX_VALUE_LENGTH];
} KeyValue;

extern KeyValue* readConfig(const char* filename, int* count);
extern char* getValue(KeyValue* kvs, int kvsCount, const char* key);
