#include <stdio.h>

int main() {
    char str[200], longest[100];
    int i = 0, j = 0;
    int len = 0, maxLen = 0, start = 0;

    printf("Enter a sentence: ");
    fgets(str, sizeof(str), stdin);

    while (str[i] != '\0') {

        if (str[i] != ' ' && str[i] != '\n') {
            len++;
        } 
        else {
            if (len > maxLen) {
                maxLen = len;
                start = i - len;
            }
            len = 0;
        }

        i++;
    }

    // Check the last word
    if (len > maxLen) {
        maxLen = len;
        start = i - len;
    }

    // Copy longest word
    for (i = 0; i < maxLen; i++) {
        longest[i] = str[start + i];
    }

    longest[i] = '\0';

    printf("Longest word is: %s\n", longest);
    printf("Length = %d\n", maxLen);

    return 0;
}
