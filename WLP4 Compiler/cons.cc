#include "a10p1.h"
#include "a10p2.h"
#include <cstdint>
#include <iostream>

int64_t *freeLst = arena() + 1;

//returns the address of a pair of words, initialized to a and b respectively, or 0 if no memory is available
int64_t *cons(int64_t a, int64_t *b) {
    if (freeLst >= (arena() + (arena()[0] / 8) - 1)) {
        return 0;
    }
    int64_t *loc = freeLst;
    if ((int64_t *)(loc[1])) {
        freeLst = (int64_t *)(loc[1]);
    } else {
        freeLst += 2;
    }
    loc[0] = a;
    loc[1] = (int64_t)(b);
    return loc;
}

//returns the first element of the pair whose address is p
int64_t car(int64_t *p) {
    return p[0];
}

//returns the second element of the pair whose address is p
int64_t *cdr(int64_t *p) {
    return (int64_t *)(p[1]);
}

//sets the first element of p to the value v and returns p
int64_t *setcar(int64_t *p, int64_t v) {
    p[0] = v;
    return p;
}

//sets the second element of p to the value v and returns p
int64_t *setcdr(int64_t *p, int64_t *v) {
    p[1] = (int64_t)(v);
    return p;
}

//deletes the pair whose address is p
void snoc(int64_t *address) {
    if (address) {
        address[0] = 0;
        address[1] = (int64_t)(freeLst);
        freeLst = address;
    }
}
