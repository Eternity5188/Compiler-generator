#include <stdio.h>

int sum_array(int* values, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += values[i];
    }
    return total;
}

int main() {
    int data[3];
    data[0] = 1;
    data[1] = 2;
    data[2] = 3;

    int* ptr = data;
    if (ptr != 0 && data[1] >= 2) {
        return sum_array(ptr, 3);
    }

    return 0;
}
