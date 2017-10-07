
#include <ctype.h>

int pl_strisdigit(const char *s) {
    for(int i = 0; s[i] != '\0'; ++i) {
        if(isdigit(s[i])) {
            return i;
        }
    }
    return -1;
}

int pl_strnisdigit(const char *s, unsigned int n) {
    for(int i = 0; i < n; ++i) {
        if(s[i] == '\0') {
            break;
        }
        if(isdigit(s[i])) {
            return i;
        }
    }
    return -1;
}