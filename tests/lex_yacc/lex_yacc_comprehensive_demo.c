#include <stdio.h>

/* Block comments are skipped by lexical rules. */
int sum_array(int* values, int n) {
    int total = 0;

    for (int i = 0; i < n; i++) {
        total += values[i];
    }

    return total;
}

int choose(int a, int b) {
    return (a > b) ? a : b;
}

int main() {
    // Line comments are skipped too.
    int data[3];
    data[0] = 1;
    data[1] = 2;
    data[2] = 3;

    char newline = '\n';
    char quote = '\'';
    char slash = '\\';
    char* text = "line\nquote\"";

    int* ptr = data;
    int total = sum_array(ptr, 3);
    int selected = choose(total, data[1]);
    int flags = (selected << 1) | (data[0] & 3);

    flags ^= 1;
    flags &= 7;

    if (flags != 0 && selected >= 3) {
        total = total + flags;
    } else {
        total = total - 1;
    }

    while (total < 20) {
        total++;
    }

    return total + newline + quote + slash;
}
