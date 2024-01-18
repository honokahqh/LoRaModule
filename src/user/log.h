#ifndef __LOG_H__
#define __LOG_H__
#include <stdint.h>
#include <stdarg.h>

#include "APP.h"

typedef enum {
    LOG_NONE = 0,       /*!< No log output */
    LOG_ERROR,      /*!< Critical errors, software module can not recover on its own */
    LOG_WARN,       /*!< Error conditions from which recovery measures have been taken */
    LOG_INFO,       /*!< Information messages which describe normal flow of events */
    LOG_ALL,      /*!< Extra information which is not necessary for normal use (values, pointers, sizes, etc). */
} log_level_t;

#define LOG_LEVEL LOG_ALL

#define log_printf(level, tag, fmt, ...) do { \
        printf("%s (%u) %s: " fmt "", level >= LOG_ALL ? "D" : \
                                     level >= LOG_INFO ? "I" : \
                                     level >= LOG_WARN ? "W" : \
                                     level >= LOG_ERROR ? "E" : "?", \
                                     millis(), \
                                     tag, ##__VA_ARGS__); \
    } while(0) 

#if LOG_LEVEL >= LOG_ERROR
#define LOG_E(tag, fmt, ...) log_printf(LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_E(tag, fmt, ...)
#endif

#if LOG_LEVEL >= LOG_WARN
#define LOG_W(tag, fmt, ...) log_printf(LOG_WARN, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_W(tag, fmt, ...)
#endif

#if LOG_LEVEL >= LOG_INFO
#define LOG_I(tag, fmt, ...) log_printf(LOG_INFO, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_I(tag, fmt, ...)
#endif

#if LOG_LEVEL >= LOG_DEBUG
#define LOG_D(tag, fmt, ...) log_printf(LOG_DEBUG, tag, fmt, ##__VA_ARGS__)
#else
#define LOG_D(tag, fmt, ...)
#endif

#endif // !
