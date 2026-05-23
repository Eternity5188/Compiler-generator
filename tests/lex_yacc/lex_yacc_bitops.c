int main() {
    int a = 1;
    int b = 2;
    int c = (a << 2) | (b & 3);

    c ^= 1;
    c &= 7;
    c = ~c;

    return c >> 1;
}
