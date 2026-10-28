#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);

    int nums[n];
    int answer[n];

    for (int i = 0; i < n; i++) {
        scanf("%d", &nums[i]);
    }

    // Step 1: Store product of all elements to the LEFT
    answer[0] = 1;

    for (int i = 1; i < n; i++) {
        answer[i] = answer[i - 1] * nums[i - 1];
    }

    // Step 2: Multiply by product of all elements to the RIGHT
    int right = 1;

    for (int i = n - 1; i >= 0; i--) {
        answer[i] = answer[i] * right;
        right = right * nums[i];
    }

    // Print answer
    for (int i = 0; i < n; i++) {
        printf("%d", answer[i]);

        if (i != n - 1)
            printf(" ");
    }

    return 0;
}
