#ifndef HUMANIZE_H
#define HUMANIZE_H

#include <stddef.h>


const char* humanize_bytesize(size_t value);
size_t      humanize_bytesize_ex(size_t value, char* buffer, size_t buffer_size);


#endif
