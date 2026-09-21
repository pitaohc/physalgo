#pragma once

struct Pair {
    int a = 0;
    int b = 0;
};

inline Pair makePair(int x, int y) {
    return x < y ? Pair{x, y} : Pair{y, x};
}

inline bool operator==(const Pair& p, const Pair& q) {
    return p.a == q.a && p.b == q.b;
}

inline bool operator!=(const Pair& p, const Pair& q) {
    return !(p == q);
}

inline bool operator<(const Pair& p, const Pair& q) {
    if (p.a != q.a) return p.a < q.a;
    return p.b < q.b;
}
