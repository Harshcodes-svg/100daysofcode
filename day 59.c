#include <stdio.h>

int main() {
    int n, k;

    // Input size of array
    scanf("%d", &n);

    int arr[n];

    // Input array elements
    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    // Input size of subarray
    scanf("%d", &k);

    // Check if k is valid
    if (k <= 0 || k > n) {
        printf("Invalid k");
        return 0;
    }

    // Calculate sum of first k elements
    int windowSum = 0;

    for (int i = 0; i < k; i++) {
        windowSum = windowSum + arr[i];
    }

    int maxSum = windowSum;

    // Slide the window
    for (int i = k; i < n; i++) {
        windowSum = windowSum - arr[i - k] + arr[i];

        if (windowSum > maxSum) {
            maxSum = windowSum;
        }
    }

    // Print maximum sum
    printf("%d", maxSum);

    return 0;
}
