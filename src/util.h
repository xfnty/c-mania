#ifndef LOGC_LOG_H
#define LOGC_LOG_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


/* macros */
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))
#define IS_OUT_OF_BOUNDS(i, array) (i < 0 || i >= ARRAY_LENGTH(array))
#define CONSTRAIN(v, a, b) ((v < a) ? a : ((v > b) ? b : v))
#define WRAP(v, a, b) ((v < a) ? b : ((v > b) ? a : v))
#define MIN(a, b) ((a < b) ? a : b)
#define MAX(a, b) ((a > b) ? a : b)


/* common types */
typedef enum {
    /* Common */
    ERROR_SUCCESS = 0,
    ERROR_UNDEFINED,
    /* IO */
    ERROR_FILE_NOT_FOUND,
} error_t;

static const char* ERROR_MESSAGES[] = {
    [ERROR_SUCCESS]         = "OK",
    [ERROR_UNDEFINED]       = "Undefined error",  // used when `error_t err` is out of bounds of ERROR_MESSAGES
    [ERROR_FILE_NOT_FOUND]  = "File not found",
};

inline static const char* error_get_message(error_t err) {
    return ERROR_MESSAGES[IS_OUT_OF_BOUNDS(err, ERROR_MESSAGES) ? ERROR_UNDEFINED : err];
}


/* logging */
#define ANSI_RESET          "\033[0;97m"
#define ANSI_WHITE          "\033[0;97m"
#define ANSI_GRAY           "\033[0;37m"
#define ANSI_GREEN          "\033[0;92m"
#define ANSI_YELLOW         "\033[0;93m"
#define ANSI_RED            "\033[0;91m"
#define ANSI_CYAN           "\033[0;96m"
#define ANSI_WHITE_BOLD     "\033[1;97m"
#define ANSI_GREEN_BOLD     "\033[1;92m"
#define ANSI_YELLOW_BOLD    "\033[1;93m"
#define ANSI_RED_BOLD       "\033[1;91m"
#define ANSI_CYAN_BOLD      "\033[1;96m"

#define __FILENAME__ ((strrchr(__FILE__, '/') != NULL)                      \
     ? strrchr(__FILE__, '/') + 1                                           \
     : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))

#define LOG_DESC(msg)           printf(ANSI_GRAY msg ANSI_RESET "\n")
#define LOGF_DESC(fmt, ...)     printf(ANSI_GRAY fmt ANSI_RESET "\n", __VA_ARGS__)
#ifdef SCOPE_NAME  // Should be defined at the very beginning of the file
    #define LOG(msg)                printf(ANSI_WHITE_BOLD SCOPE_NAME ": " ANSI_RESET msg ANSI_RESET "\n")
    #define LOGF(fmt, ...)          printf(ANSI_WHITE_BOLD SCOPE_NAME ": " ANSI_RESET fmt ANSI_RESET "\n", __VA_ARGS__)
#else
    #define LOG(msg)                printf(ANSI_RESET msg ANSI_RESET "\n")
    #define LOGF(fmt, ...)          printf(ANSI_RESET fmt ANSI_RESET "\n", __VA_ARGS__)
#endif
#define __LOG_LEVELNAME_BOILERPLATE(c, s, m)       LOG(c s ANSI_RESET ": " m)
#define __LOGF_LEVELNAME_BOILERPLATE(c, s, f, ...) LOGF(c s ANSI_RESET ": " f, __VA_ARGS__)
#define LOG_DEBUG(msg)              __LOG_LEVELNAME_BOILERPLATE(ANSI_CYAN_BOLD, "debug", msg)
#define LOG_SUCCESS(msg)            __LOG_LEVELNAME_BOILERPLATE(ANSI_GREEN_BOLD, "success", msg)
#define LOG_WARNING(msg)            __LOG_LEVELNAME_BOILERPLATE(ANSI_YELLOW_BOLD, "warning", msg)
#define LOG_ERROR(msg)              __LOG_LEVELNAME_BOILERPLATE(ANSI_RED_BOLD, "error", msg)
#define LOG_FATAL_ERROR(msg)        __LOG_LEVELNAME_BOILERPLATE(ANSI_RED_BOLD, "fatal error", msg)
#define LOGF_DEBUG(fmt, ...)        __LOGF_LEVELNAME_BOILERPLATE(ANSI_CYAN_BOLD, "debug", fmt, __VA_ARGS__)
#define LOGF_SUCCESS(fmt, ...)      __LOGF_LEVELNAME_BOILERPLATE(ANSI_GREEN_BOLD, "success", fmt, __VA_ARGS__)
#define LOGF_WARNING(fmt, ...)      __LOGF_LEVELNAME_BOILERPLATE(ANSI_YELLOW_BOLD, "warning", fmt, __VA_ARGS__)
#define LOGF_ERROR(fmt, ...)        __LOGF_LEVELNAME_BOILERPLATE(ANSI_RED_BOLD, "error", fmt, __VA_ARGS__)
#define LOGF_FATAL_ERROR(fmt, ...)  __LOGF_LEVELNAME_BOILERPLATE(ANSI_RED_BOLD, "fatal error", fmt, __VA_ARGS__)

void logging_init();
void logging_shutdown();


/* assertions and error checks */
#define ASSERT(cond)                                    do { if (!(cond)) { LOGF_FATAL_ERROR("assertion " ANSI_WHITE_BOLD "`%s`" ANSI_WHITE " failed" ANSI_GRAY " at %s:%d:%s()", #cond, __FILENAME__, __LINE__, __FUNCTION__); exit(-1); } } while(0);
#define ASSERT_RETURN(cond)                             do { if (!(cond)) return; } while(0);
#define ASSERT_RETURN_VALUE(cond, value)                do { if (!(cond)) return (value); } while(0);
#define __ASSERT_LOG_BOILERPLATE(expr, msg, end)        do { if (!(expr)) { LOG_ERROR(msg); end; } } while(0);
#define ASSERT_LOG(expr, msg)                           __ASSERT_LOG_BOILERPLATE(expr, msg, exit(err))
#define ASSERT_LOG_RETURN(expr, msg)                    __ASSERT_LOG_BOILERPLATE(expr, msg, return)
#define ASSERT_LOG_RETURN_VALUE(expr, msg, value)       __ASSERT_LOG_BOILERPLATE(expr, msg, return value)
#define ASSERT_LOG_PROPAGATE(expr, msg)                 __ASSERT_LOG_BOILERPLATE(expr, msg, return err)
#define __ASSERT_LOGF_BOILERPLATE(expr, fmt, end, ...)  do { if (!(expr)) { LOGF_ERROR(fmt, __VA_ARGS__); end; } } while(0);
#define ASSERT_LOGF(expr, fmt, ...)                     __ASSERT_LOGF_BOILERPLATE(expr, fmt, exit(err), __VA_ARGS__)
#define ASSERT_LOGF_RETURN(expr, fmt, ...)              __ASSERT_LOGF_BOILERPLATE(expr, fmt, return, __VA_ARGS__)
#define ASSERT_LOGF_RETURN_VALUE(expr, value, fmt, ...) __ASSERT_LOGF_BOILERPLATE(expr, fmt, return value, __VA_ARGS__)
#define ASSERT_LOGF_PROPAGATE(expr, fmt, ...)           __ASSERT_LOGF_BOILERPLATE(expr, fmt, return err, __VA_ARGS__)

#define CHECK_ERROR(expr)                       do { error_t err = (expr); if (err != ERROR_SUCCESS) { LOGF_FATAL_ERROR("expression " ANSI_WHITE_BOLD "`%s`" ANSI_WHITE " returned an error " ANSI_GRAY "(%s) [%d] at %s:%d:%s()", #expr, error_get_message(err), err, __FILENAME__, __LINE__, __FUNCTION__); exit(-1); } } while(0);
#define CHECK_ERROR_RETURN(expr)                do { error_t err = (expr); if (err != ERROR_SUCCESS) return; } while(0);
#define CHECK_ERROR_RETURN_VALUE(expr)          do { error_t err = (expr); if (err != ERROR_SUCCESS) return (value); } while(0);
#define CHECK_ERROR_PROPAGATE(expr)             do { error_t err = (expr); if (err != ERROR_SUCCESS) return err; } while(0);
#define __CHECK_ERROR_LOG_BOILERPLATE(expr, msg, end)                                 \
    do {                                                                                        \
        error_t err = (expr);                                                                   \
        if (err != ERROR_SUCCESS) {                                                             \
            LOGF_ERROR(                                                                           \
                msg " " ANSI_GRAY "(%s) [%d]",                                                  \
                error_get_message(err),                                                         \
                err                                                                             \
            );                                                                                  \
            end;                                                                                \
        }                                                                                       \
    } while(0);
#define CHECK_ERROR_LOG(expr, msg)                      __CHECK_ERROR_LOG_BOILERPLATE(expr, msg, exit(err))
#define CHECK_ERROR_LOG_RETURN(expr, msg)               __CHECK_ERROR_LOG_BOILERPLATE(expr, msg, return)
#define CHECK_ERROR_LOG_RETURN_VALUE(expr, msg, value)  __CHECK_ERROR_LOG_BOILERPLATE(expr, msg, return value)
#define CHECK_ERROR_LOG_PROPAGATE(expr, msg)            __CHECK_ERROR_LOG_BOILERPLATE(expr, msg, return err)
#define __CHECK_ERROR_LOGF_BOILERPLATE(expr, fmt, end, ...)                           \
    do {                                                                                        \
        error_t err = (expr);                                                                   \
        if (err != ERROR_SUCCESS) {                                                             \
            LOGF_ERROR(                                                                           \
                fmt " " ANSI_GRAY "(%s) [%d]",                                                  \
                __VA_ARGS__,                                                                    \
                error_get_message(err),                                                         \
                err                                                                             \
            );                                                                                  \
            end;                                                                                \
        }                                                                                       \
    } while(0);
#define CHECK_ERROR_LOGF(expr, fmt, ...)                      __CHECK_ERROR_LOGF_BOILERPLATE(expr, fmt, exit(err), __VA_ARGS__)
#define CHECK_ERROR_LOGF_RETURN(expr, fmt, ...)               __CHECK_ERROR_LOGF_BOILERPLATE(expr, fmt, return, __VA_ARGS__)
#define CHECK_ERROR_LOGF_RETURN_VALUE(expr, value, fmt, ...)  __CHECK_ERROR_LOGF_BOILERPLATE(expr, fmt, return value, __VA_ARGS__)
#define CHECK_ERROR_LOGF_PROPAGATE(expr, fmt, ...)            __CHECK_ERROR_LOGF_BOILERPLATE(expr, fmt, return err, __VA_ARGS__)


#endif
