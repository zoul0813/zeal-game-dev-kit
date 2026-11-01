#include <assert.h>
#include <stdio.h>
#include "helper.h"
unsigned from_peer(unsigned value);
int main(void) {
    assert(increment(41) == 42);
    assert(from_peer(99) == 100);
    puts("inline: multiple translation units passed");
    return 0;
}
