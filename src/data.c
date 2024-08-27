#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

const char** getFilenames(int* count)
{
	static const char* files[] = {
		"basement.b",
		"basement.o",
		"basement.ob",
		"batcave.b",
		"batcave.o",
		"batcave.ob",
		"bathroom.b",
		"bathroom.o",
		"bathroom.ob",
		"bed1.b",
		"bed1.o",
		"bed1.ob",
		"deadend.b",
		"deadend.o",
		"deadend.ob",
		"diningrm.b",
		"diningrm.o",
		"diningrm.ob",
		"garden.b",
		"garden.o",
		"garden.ob",
		"hall.b",
		"hall.o",
		"hall.ob",
		"house.b",
		"house.o",
		"house.ob",
		"jail.b",
		"jail.o",
		"jail.ob",
		"kitchen.b",
		"kitchen.o",
		"kitchen.ob",
		"lab.b",
		"lab.o",
		"lab.ob",
		"lakeroom.b",
		"lakeroom.o",
		"lakeroom.ob",
		"mummyrm.b",
		"mummyrm.o",
		"mummyrm.ob",
		"storerm.b",
		"storerm.o",
		"storerm.ob",
		"the_end.b",
		"the_end.o",
		"the_end.ob"
	};

	*count = sizeof(files) / sizeof(files[0]);

	return files;
}
