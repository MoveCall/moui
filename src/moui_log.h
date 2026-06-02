#ifndef MOUI_LOG_H
#define MOUI_LOG_H

#include "moui_conf.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if MOUI_LOG_ENABLE
#define MOUI_LOG(fmt, ...) fprintf(stderr, "[MOUI] " fmt "\n", ##__VA_ARGS__)
#define MOUI_WARN(fmt, ...) fprintf(stderr, "[MOUI WARN] " fmt "\n", ##__VA_ARGS__)
#else
#define MOUI_LOG(fmt, ...) ((void)0)
#define MOUI_WARN(fmt, ...) ((void)0)
#endif

#define MOUI_ASSERT(cond) do { \
    if (!(cond)) { \
        MOUI_WARN("ASSERT FAILED: %s (%s:%d)", #cond, __FILE__, __LINE__); \
    } \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
