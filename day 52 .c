#include <stdio.h>

int main() {
    int n, x;

    scanf("%d", &n);

    int arr[n];

    for (int i = 0; i < n; i++) {
        scanf("%d", &arr[i]);
    }

    scanf("%d", &x);

    int low = 0;
    int high = n - 1;
    int ans = -1;

    while (low <= high) {

        int mid = low + (high - low) / 2;

        if (arr[mid] >= x) {
            ans = mid;          // Possible answer mil gaya
            high = mid - 1;     // First occurrence ke liye LEFT jao
        }
        else {
            low = mid + 1;      // arr[mid] < x, so RIGHT jao
        }
    }

    printf("%d", ans);

    return 0;
}
