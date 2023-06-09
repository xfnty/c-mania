#include <humanize.h>

#include <string.h>
#include <stdio.h>

#define STACKARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))


size_t humanize_bytesize_ex(size_t value, char* buffer, size_t buffer_size) {
	const char* suffixes[] = { " B", " kB", " MB", " GB", " TB" };

	int i = 0;
	while (value > 1024 && i < STACKARRAY_SIZE(suffixes) - 1) {
		i++;
		value /= 1024;
	}

	int bytes_written = snprintf(buffer, buffer_size, "%lu%s", value, suffixes[i]);
	return (bytes_written > 0) ? bytes_written : 0;
}

const char* humanize_bytesize(size_t value) {
    static char buffer[16];

    humanize_bytesize_ex(value, buffer, sizeof(buffer));
    
    return buffer;
}
