#ifndef DETECT_HEX
#define DETECT_HEX

#include "define.h"

int LoadHexFile(char *path, void *buf, int maxLength,int *pLen, unsigned int *startAddr, const char fillChar);

#endif