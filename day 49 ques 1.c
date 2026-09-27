#include <stdio.h>

int main() {
    char name[100];
    int i;

    printf("Enter your full name: ");
    fgets(name, sizeof(name), stdin);

    printf("Initials: ");

    // First character is an initial
    if (name[0] != ' ')
        printf("%c", name[0]);

    // Character after every space is an initial
    for (i = 0; name[i] != '\0'; i++) {
        if (name[i] == ' ' && name[i + 1] != ' ' && name[i + 1] != '\0')
            printf("%c", name[i + 1]);
    }

    return 0;
}
