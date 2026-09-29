#include <stdio.h>

int firstOccurrence(int nums[], int n, int target) {
    int low = 0, high = n - 1;
    int first = -1;

    while (low <= high) {
        int mid = (low + high) / 2;

        if (nums[mid] == target) {
            first = mid;
            high = mid - 1;   // search on left side
        }
        else if (nums[mid] < target) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }

    return first;
}

int lastOccurrence(int nums[], int n, int target) {
    int low = 0, high = n - 1;
    int last = -1;

    while (low <= high) {
        int mid = (low + high) / 2;

        if (nums[mid] == target) {
            last = mid;
            low = mid + 1;    // search on right side
        }
        else if (nums[mid] < target) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }

    return last;
}

int main() {
    int nums[100], n, target;

    printf("Enter number of elements: ");
    scanf("%d", &n);

    printf("Enter sorted array: ");
    for (int i = 0; i < n; i++) {
        scanf("%d", &nums[i]);
    }

    printf("Enter target: ");
    scanf("%d", &target);

    int first = firstOccurrence(nums, n, target);
    int last = lastOccurrence(nums, n, target);

    printf("First occurrence index = %d\n", first);
    printf("Last occurrence index = %d\n", last);

    return 0;
}
