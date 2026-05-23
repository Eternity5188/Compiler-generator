int main() {
    char* message = "ok";
    int a = 1;
    int b = 2;
    int c = (a < b) ? a : b;
    int d = *(&a);

    return c + d;
}
