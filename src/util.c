#include "util.h"

#include <stdarg.h>
#include <stdio.h>

#include <raylib.h>


static void _raylib_log_callback(int logLevel, const char *text, va_list args);

const char* error_get_message(error_t err);

void logging_init() {
    SetTraceLogCallback(_raylib_log_callback);
}

void logging_shutdown() {
    SetTraceLogCallback(NULL);
}

void _raylib_log_callback(int logLevel, const char *text, va_list args) {
    if (logLevel < LOG_WARNING)
        return;

    char buffer[512] = {0};
    vsnprintf(buffer, sizeof(buffer), text, args);

    switch (logLevel) {
    case LOG_DEBUG:
        LOGF_DEBUG("%s", buffer);
        break;

    case LOG_WARNING:
        LOGF_WARNING("%s", buffer);
        break;

    case LOG_ERROR:
        LOGF_ERROR("%s", buffer);
        break;

    case LOG_FATAL:
        LOGF_ERROR("%s", buffer);
        break;

    default:
        break;
    }

    printf("%s", (logLevel == LOG_WARNING ? ANSI_YELLOW : ANSI_RED));
    vprintf(text, args);
    printf(ANSI_RESET "\n");
}
