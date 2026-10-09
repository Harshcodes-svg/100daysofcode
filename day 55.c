#include <stdio.h>

int main() {
    int n;
    scanf("%d", &n);

    int nums[n];

    for (int i = 0; i < n; i++) {
        scanf("%d", &nums[i]);
    }

    int candidate = nums[0];
    int count = 0;

    // Step 1: Find possible majority element
    for (int i = 0; i < n; i++) {
        if (count == 0) {
            candidate = nums[i];
            count = 1;
        }
        else if (nums[i] == candidate) {
            count++;
        }
        else {
            count--;
        }
    }

    // Step 2: Verify the candidate
    count = 0;

    for (int i = 0; i < n; i++) {
        if (nums[i] == candidate) {
            count++;
        }
    }

    if (count > n / 2)
        printf("%d", candidate);
    else
        printf("-1");

    return 0;
}
