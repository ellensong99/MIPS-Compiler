#include <iostream>
#include <cstdint>
#include "a10p1.h"

int64_t *freeLst = arena() + 1;
bool full = false;

// allocates a block of memory of at least size words (not bytes), 
// and returns the address of that memory or 0 if memory could not be allocated.
int64_t *mymalloc(int64_t size) {
    if (full) {
        return 0;
    }
    int64_t *next = freeLst;
    int64_t *prev = next;
    int64_t *ret = 0;
    if ((freeLst[0] == 0) | (freeLst[0] >= (size + 1))) {
        freeLst[0] = size + 1;
        ret = freeLst + 1;
        freeLst += size + 1;
        return ret;
    } else if ((freeLst + size + 1) == (arena() + arena()[0] / 8 - 1)) {
        full = true;
        next[0] = size + 1;
        ret = next + 1;
        freeLst += size + 1;
        return ret;
    }
    while (1) {
        if (!freeLst[0]) {
            if ((next + size + 1) > (arena() + arena()[0] / 8 - 1)) {
                full = true;
                return 0;
            } else if ((next + size + 1) == (arena() + arena()[0] / 8 - 1)) {
                full = true;
                next[0] = size + 1;
                ret = next + 1;
            } else {
                prev[1] = (int64_t)(next + size + 1);
                next[0] = size + 1;
                ret = next + 1;
            }
            break;
        } else if (((size + 1) > next[0]) | ((next[0] - size - 1) == 1)) {
            prev = next;
            next = (int64_t *)(prev[1]);
        } else if (next[0] >= (size + 1)) {
            int64_t prevBlock = next[0];
            int64_t nextBlock = next[1];
            next[0] = size + 1;
            ret = next + 1;
            if (prevBlock - size == 1) {
                prev[1] = nextBlock;
            } else {
                next[size + 1] = prevBlock - size - 1;
                next[size + 2] = nextBlock;
                prev[1] = (int64_t)(next + size + 1);
            }
            break;
        }
    }
    return ret;
}

// deallocates the memory stored at address and returns 0. 
// assumes that address contains either an address allocated by mymalloc, in which case it deallocates that memory, 
// or the value 0 (NULL), in which case myfree does nothing.
void myfree(int64_t *address) {
    if (!address) {
        return;
    }
    int64_t *next = freeLst;
    int64_t *prev = next;
    if (freeLst > address) {
        address[0] = (int64_t)(next);
        freeLst = address - 1;
        if ((freeLst + freeLst[0] + 1) == next) {
            freeLst[0] += next[0];
            freeLst[1] = next[1];
        }
    } else {
        while (1) {
            if (address > next + 1) {
                prev = next;
                next = (int64_t *)(prev[1]);
            } else if ((!next) | (address < next + 1)) {
                prev[1] = (int64_t)(address - 1);
                address[0] = (int64_t)(next);
                break;
            }
        }
        if ((prev + prev[0] == (address - 1)) & ((address - 1 + address[-1]) == next)) {
            prev[0] += next[0] + address[-1];
            prev[1] = next[1];
        } else if (prev + prev[0] == (address - 1)) {
            prev[0] += address[-1];
            prev[1] = address[0];
        } else if ((address - 1 + address[-1]) == next) {
            address[-1] += next[0];
            address[0] = next[1];
        }
    }
    full = false;
    return;
}

