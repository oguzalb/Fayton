#include "utils.h"

char* fay_strcat(char **dest, char *source, char *cursor) {
    if (*dest == NULL) {
        *dest = strdup(source);
        cursor = *dest;
        while (*cursor != '\0') cursor++;
        return cursor;
    }
    if (cursor == NULL) {
        cursor = *dest;
        while (*cursor != '\0') cursor++;
    }
    int cursor_index = (cursor - *dest);
    *dest = realloc(*dest, sizeof(char) * (cursor_index + strlen(source) + 1));
    for (cursor=*dest+cursor_index; *source != '\0'; cursor++, source++) {
        *cursor = *source;
    }
    *cursor = '\0';
    return cursor;
}


