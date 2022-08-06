#ifndef __DATA_TYPE_H__
#define __DATA_TYPE_H__

// #include <stdint.h>
// #include <string.h>

#define ERR_OK  (0U)
#define ERR_NG  (0xFFFFFFFF)

#define ADD_FLAG(status, flag)      ((status) |= (flag))
#define REMOVE_FLAG(status, flag)   ((status) &= (~(flag)))
#define CHECK_FLAG(status, flag)    ((status) & (flag))

#define __ATTRIBUTE_WEAK__ __attribute__((weak))
#define __ATTRIBUTE_UNUSED__ __attribute__((unused))
#define __ATTRIBUTE_ALIAS__(func) __attribute__((alias(#func)))

#endif
