#include <assert.h>
#include <string.h>
#include "zgdk/entities.h"

typedef struct { uint8_t value, kind, ticks; } record;
static unsigned visit(entity_pool_t *pool, record *expected) {
    record *p;
    unsigned n = 0;
    ENTITY_POOL_FOR_EACH(pool, p, kind) {
        if (!n) assert(p == expected);
        n++;
    }
    return n;
}
int main(void) {
    record records[255];
    uint8_t bytes[255];
    entity_pool_t pool, other, saved;
    record *a, *b, *c;
    unsigned i;
    memset(records, 0x5a, sizeof(records));
    assert(entity_pool_init(&pool, records, 255, sizeof(record), offsetof(record, kind)) == ERR_SUCCESS);
    assert(pool.used_end == (uint8_t *)records);
    assert(visit(&pool,NULL) == 0);
    for (i=0;i<255;i++) {
        assert(records[i].kind == ENTITY_FREE && records[i].value == 0x5a);
        assert(entity_pool_alloc(&pool, i) == &records[i]);
    }
    /* Every slot is full; tag value 255 is reserved. */
    assert(!entity_pool_alloc(&pool, 0));
    a = &records[0]; b = &records[127]; c = &records[254];
    entity_pool_free(&pool,a); entity_pool_free(&pool,b); entity_pool_free(&pool,c);
    assert(visit(&pool,&records[1]) == 252);
    {
        record *p;
        unsigned n = 0;
        ENTITY_POOL_FOR_EACH(&pool, p, kind) {
            assert(p != a && p != b && p != c);
            if (p == &records[1]) continue;
            if (++n == 3) break;
        }
        assert(n == 3);
    }
    assert(entity_pool_alloc(&pool,42) == a);
    assert(entity_pool_alloc(&pool,43) == b);
    assert(entity_pool_alloc(&pool,44) == c);
    assert(!entity_pool_alloc(&pool,ENTITY_FREE));
    assert(entity_pool_init(&other,bytes,255,1,0) == ERR_SUCCESS);
    assert(entity_pool_alloc(&other,0) == bytes);
    entity_pool_free(&pool,NULL);
    entity_pool_clear(&pool);
    assert(visit(&pool,NULL) == 0);
    assert(pool.used_end == (uint8_t *)(records+255));
    assert(bytes[0] == 0 && other.used_end == bytes + 1);
    assert(entity_pool_alloc(&pool,1) == a && a->value == 0x5a);
    {
        record *p;
        unsigned visits = 0;
        ENTITY_POOL_FOR_EACH(&pool, p, kind) {
            if (!visits) assert(entity_pool_alloc(&pool,2) == records+1);
            entity_pool_free(&pool,p);
            visits++;
        }
        assert(visits == 2 && visit(&pool,NULL) == 0);
        assert(entity_pool_alloc(&pool,1) == a);
    }
    saved = pool;
#define BAD(p,s,n,z,t) do { assert(entity_pool_init(p,s,n,z,t)==ERR_INVALID_PARAMETER); assert(!memcmp(&saved,&pool,sizeof(pool))); assert(a->kind==1); } while(0)
    BAD(NULL,records,1,3,1); BAD(&pool,NULL,1,3,1);
    BAD(&pool,records,0,3,1); BAD(&pool,records,256,3,1);
    BAD(&pool,records,1,0,0); BAD(&pool,records,1,256,1);
    BAD(&pool,records,1,3,3); BAD(&pool,records,1,3,256);
    assert(entity_pool_init(&pool, records, 1, sizeof(record), 1) == ERR_SUCCESS);
    assert(visit(&pool,NULL) == 0);
    assert(entity_pool_alloc(&pool,0) == records);
    assert(!entity_pool_alloc(&pool,1));
    {
        uint8_t wide[255];
        memset(wide,0x5a,sizeof(wide));
        assert(entity_pool_init(&pool,wide,1,255,254) == ERR_SUCCESS);
        assert(wide[0] == 0x5a && wide[254] == ENTITY_FREE);
        assert(entity_pool_alloc(&pool,0) == wide && wide[254] == 0);
        assert(!entity_pool_alloc(&pool,1));
        entity_pool_clear(&pool);
        assert(wide[254] == ENTITY_FREE && wide[253] == 0x5a);
    }
    return 0;
}
