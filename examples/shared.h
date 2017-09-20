
#pragma once

#define PRINT(...)          printf(__VA_ARGS__);
#define PRINT_ERROR(...)    PRINT(__VA_ARGS__); exit(-1)
#ifdef _DEBUG
#   define DPRINT(...)      PRINT(__VA_ARGS__)
#else
#   define DPRINT(...)      (__VA_ARGS__)
#endif