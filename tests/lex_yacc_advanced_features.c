typedef unsigned long size_t;

typedef struct Point {
    int x;
    int y;
} Point;

union Number {
    int i;
    double d;
};

enum Mode {
    MODE_ZERO = 0,
    MODE_ONE,
    MODE_TWO = 2,
};

static const unsigned long global_limit = sizeof(size_t);

int eval(Point* p, enum Mode mode) {
    union Number number;
    number.i = p->x + p->y;

    switch (mode) {
    case MODE_ZERO:
        number.i += (int)sizeof(Point);
        break;
    case MODE_ONE:
        number.i = number.i + 1;
        break;
    default:
        number.i = number.i - 1;
        break;
    }

    return number.i;
}

int main() {
    Point p;
    p.x = 1;
    p.y = 2;

    return eval(&p, MODE_ONE);
}
