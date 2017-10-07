
#include <ctype.h>

int pl_strisalpha(const char *s) {
    for(int i = 0; s[i] != '\0'; ++i) {
        if(isalpha(s[i])) {
            return i;
        }
    }
    return -1;
}

int pl_strnisalpha(const char *s, unsigned int n) {
    for(int i = 0; i < n; ++i) {
        if(s[i] == '\0') {
            break;
        }
        if(isalpha(s[i])) {
            return i;
        }
    }
    return -1;
}