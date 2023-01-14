
#pragma once

#define IS_FLAG(x, flag) ((x & flag) == flag)
#define RESET_FLAG(x,flag) (x &= ~flag)
#define SET_FLAG(x, flag) (x |= flag)
#define BLINK_FLAG(x, flag) (x ^= flag)

