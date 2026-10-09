#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);

    int arr[n];

    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    for (int i = 0; i < n; i++) {

        int previousGreater = -1;

        // Check elements towards the left
        for (int j = i - 1; j >= 0; j--) {

            if (arr[j] > arr[i]) {
                previousGreater = arr[j];
                break;
            }
        }

        printf("%d", previousGreater);

        if (i < n - 1) {
            printf(",");
        }
    }

    return 0;
}
