#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void my_wait(size_t secs)
{
	sleep(secs);
}

void print_progress(size_t count, size_t max)
{
	const char prefix[] = "Waiting: [";
	const char suffix[] = "]";
	const size_t prefix_length = sizeof(prefix) - 1;
	const size_t suffix_length = sizeof(suffix) - 1;
	char *buffer = calloc(max + prefix_length + suffix_length + 1, 1); // +1 for \0
	size_t i = 0;

	strcpy(buffer, prefix);
	for (; i < max; ++i)
	{
		buffer[prefix_length + i] = i < count ? '#' : ' ';
	}

	strcpy(&buffer[prefix_length + i], suffix);
	printf("\b%c[2K\r%s\n", 27, buffer);
	fflush(stdout);
	free(buffer);
}


int wait_for(size_t max_secs){

	float step_secs = 1;
	if (max_secs > 100){
		step_secs = max_secs / 100.0;
	}
	printf("%s\n", "");
	for (float start_secs = 1; start_secs <= max_secs; start_secs+=step_secs)
	{
		print_progress((int)start_secs/step_secs, (int) max_secs/step_secs);
		my_wait(step_secs);
	}
}


