int add(int a, int b) {
    return a + b;
}

int main() {
    int a = 1;
    int b = 2;
    int i = 0;
    int result;

    if (a > 0) {
        b = b + 1;
    }

    while (i < 3) {
        i++;
    }

    for (i = 0; i < 3; i++) {
        a = a + i;
    }

    result = add(a, b);
    return result;
}
