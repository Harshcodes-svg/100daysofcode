#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);

    int arr[n];

    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    int totalSum = 0;

    // Calculate total sum of array
    for (int i = 0; i < n; i++) {
        totalSum += arr[i];
    }

    int leftSum = 0;

    for (int i = 0; i < n; i++) {

        // Sum on right side
        int rightSum = totalSum - leftSum - arr[i];

        if (leftSum == rightSum) {
            printf("%d", i);
            return 0;   // leftmost pivot mil gaya
        }

        // Current element becomes part of left side
        leftSum += arr[i];
    }

    printf("-1");

    return 0;
}
