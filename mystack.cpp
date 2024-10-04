#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "mystack.hpp"
#include "colors.hpp"

const uint64_t startingCapacity = 4;

enum reallocParameters{
    ADD_MEMORY = 1,
    REDUCE_MEMORY = -1,
    MLTPL_CAPACITY_BOUND = 16
};

uint64_t djb2hashFunc(void* input, size_t size){
    uint64_t hash = 0xeda;
    for (int i = 0; i < size; i++){
        hash = (hash * 31) ^ *((char*)input + i);
    }
    return hash;
}

HASH_PRT(
void PutHash(Stack_t* stk){
    stk->bufferHash = djb2hashFunc((char*)stk->data CNR_PRT(- 1 * sizeof(canary_t)), stk->capacity * sizeof(StackElem_t) CNR_PRT(+ 2 * sizeof(canary_t)));
    stk->stackHash  = 0;
    stk->stackHash  = djb2hashFunc(stk, 5 * sizeof(char*) CNR_PRT(+ 4 * sizeof(canary_t)) DBG(+ 3 * sizeof(char*)));
}


uint64_t FindBufferHash(Stack_t* stk){
    uint64_t newBufferHash = djb2hashFunc((char*)stk->data CNR_PRT(- 1 * sizeof(canary_t)), stk->capacity * sizeof(StackElem_t) CNR_PRT(+ 2 * sizeof(canary_t)));

    return newBufferHash;
}

uint64_t FindStackHash(Stack_t* stk){
    uint64_t oldStackHash = stk->stackHash;
                            stk->stackHash = 0;
    uint64_t newStackHash  = djb2hashFunc(stk, 5 * sizeof(char*) CNR_PRT(+ 4 * sizeof(canary_t)) DBG(+ 3 * sizeof(char*)));

    stk->stackHash = oldStackHash;

    return newStackHash;
}
)

stackExits StackRelocate(Stack_t* stk, reallocParameters param){
    if (param == ADD_MEMORY || param == REDUCE_MEMORY){
        CNR_PRT(*(canary_t*)((char*)stk->data + (stk->capacity * sizeof(StackElem_t) / 8) * 8 + 8) = 0;)
        uint64_t oldCapacity = stk->capacity;
        if      (param == ADD_MEMORY   ){
            if  (stk->capacity <=   MLTPL_CAPACITY_BOUND)  stk->capacity *= 2;
            else stk->capacity += MLTPL_CAPACITY_BOUND;
        }

        else if (param == REDUCE_MEMORY){
            if  (stk->capacity < 3 * MLTPL_CAPACITY_BOUND) stk->capacity /= 2;
            else stk->capacity -= MLTPL_CAPACITY_BOUND;
        }

        printf(BLU "old data pointer:%p\n" RESET, stk->data);

        StackElem_t* newDataPointer = (StackElem_t*)(realloc((char*)stk->data CNR_PRT(- 1 * sizeof(canary_t)),
                                                            stk->capacity * sizeof(StackElem_t) CNR_PRT(+ 2 * sizeof(canary_t))));

        CNR_PRT(newDataPointer = (StackElem_t*)((char*)newDataPointer + 1 * sizeof(canary_t)));

        if (param == ADD_MEMORY) memset((char*)newDataPointer + oldCapacity * sizeof(StackElem_t), 0, oldCapacity * sizeof(StackElem_t));

        printf(BLU "new data pointer:%p\n"  RESET, newDataPointer);
        printf(BLU "old capacity:%llu\n"    RESET, oldCapacity);
        printf(BLU "new capacity:%lu\n"     RESET, stk->capacity);

        if (!newDataPointer){
            return REALLOC_ERR;
        }

        stk->data = newDataPointer;
        CNR_PRT(*(canary_t*)((char*)stk->data + (stk->capacity * sizeof(StackElem_t) / 8) * 8 + 8) = 0x900deda;)
    }

    else{
        return ERR;
    }

    return OK;
}

stackExits StackCtor(Stack_t* stk DBG(, const char* fileName, int line)){
    if (!stk) return STK_NULL;

    if (!stk->capacity) stk->capacity = startingCapacity;
    stk->data = (StackElem_t*)(calloc(1,
                                      stk->capacity * sizeof(StackElem_t) CNR_PRT(+ 2 * sizeof(canary_t))));

    CNR_PRT(stk->data = (StackElem_t*)((char*)stk->data + 1 * sizeof(canary_t)));
    if (!stk->data) return MEM_FULL;

    CNR_PRT(
    *(canary_t*)((char*)stk->data - 1 * sizeof(canary_t))                              = 0xbadeda;
    *(canary_t*)((char*)stk->data + (stk->capacity * sizeof(StackElem_t) / 8) * 8 + 8) = 0x900deda;

    stk->chicken_first  = 0xBADC0DE ;
    stk->chicken_second = 0x900DC0DE;
    )

    HASH_PRT(PutHash(stk);)

    printf(BLU "stack created\n" RESET);

    STK_CHECK(stk, fileName, line)

    return OK;
}

stackExits StackDtor(Stack_t* stk DBG(, const char* fileName, int line)){
    STK_CHECK(stk, fileName, line)

    free((char*)stk->data CNR_PRT(- 1 * sizeof(canary_t)));

    stk->data = nullptr;

    printf(CYN "stack destroyed\n" RESET);

    //STK_CHECK(stk)?
    return OK;
}

stackExits StackPush(Stack_t* stk, StackElem_t item DBG(, const char* fileName, int line)){
    STK_CHECK(stk, fileName, line)

    *(stk->data + stk->size) = item;
    stk->size += 1;

    if (stk->size >= stk->capacity){
        if (!StackRelocate(stk, ADD_MEMORY)){
            printf(BLU "new memory allocated\n" RESET);
            printf(CYN "item (%lld) pushed" DBG(" %s:%d") "\n" RESET , item DBG(, fileName, line));
        }
        else{
            printf(RED "reallocation error\n"   RESET);
            return REALLOC_ERR;
        }
    }
    else printf(GRN "item (%lld) pushed" DBG(" %s:%d") "\n" RESET , item DBG(, fileName, line));

    HASH_PRT(PutHash(stk);)
    STK_CHECK(stk, fileName, line)
    return OK;
}

stackExits StackPop(Stack_t* stk, StackElem_t* item DBG(, const char* fileName, int line)){
    STK_CHECK(stk, fileName, line)

    if (stk->size == 0){
        printf(RED "stack underflow" DBG(" %s:%d") "\n" RESET DBG(, fileName, line));
        return SIZE_UNDERFLOW;
    }

    *item = *(stk->data + stk->size - 1);
            *(stk->data + stk->size - 1) = 0;
    stk->size -= 1;

    int linearReallocBound = (stk->capacity >= stk->size + 2 * MLTPL_CAPACITY_BOUND &&
                              stk->capacity >=             4 * MLTPL_CAPACITY_BOUND &&
                                  stk->size >=             2 * MLTPL_CAPACITY_BOUND   );

    if  (stk->capacity > 8 && (stk->size <= stk->capacity / 4 || linearReallocBound)){

        if(!StackRelocate(stk, REDUCE_MEMORY)){
            printf(BLU "new memory freed\n" RESET);
            printf(CYN "item (%lld) popped" DBG(" %s:%d") "\n" RESET , *item DBG(, fileName, line));
        }
        else{
            printf(RED "freeing error" DBG(" %s:%d") "\n" RESET DBG(, fileName, line));
            return REALLOC_ERR;
        }
    }
    else printf(GRN "item (%lld) popped" DBG(" %s:%d") "\n" RESET , *item DBG(, fileName, line));

    HASH_PRT(PutHash(stk);)
    STK_CHECK(stk, fileName, line)
    return OK;
}

stackExits StackDump(Stack_t* stk DBG(, const char* filename, int line)){
    if (!stk){
        printf(RED "stack does not exist"  DBGPrintLine("%s:%d") "\n" RESET DBG(, filename, line));
        return ERR;
    }
    printf(MAG "Stack_t[%p] born at " DBGPrintLine("%s:%lld, name \"%s\"") "\n" YEL, stk DBG(, stk->filename, stk->line, stk->name));
    printf("dumb dump called from:" DBGPrintLine("%s:%d") "\n" DBG(, filename, line));
    printf("{\n");

    CNR_PRT(
    printf("first chick: \t%llx", stk->chicken_first);
    if (stk->chicken_first == 0xBADC0DE) printf(GRN " \t\t<OK>\n" YEL);
        else printf(RED " \t\t<NOT OK>\n" YEL);
    printf("expected:   \tbadc0de\n");
    )

    printf("size:%lu\n",     stk->size);
    printf("capacity:%lu\n", stk->capacity);


    CNR_PRT(
    printf("first hen:\t%llx", *(canary_t*)((char*)stk->data - 1 * sizeof(canary_t)));
    if (*(canary_t*)((char*)stk->data - 1 * sizeof(canary_t)) == 0xbadeda) printf(GRN " \t\t\t<OK>\n" YEL);
        else printf(RED " \t\t\t<NOT OK>\n" YEL);
    printf("expected:   \tbadeda\n");
    )

    printf("data:%p\n",      stk->data);
    for (int i = 0; i < stk->capacity; i++){
        printf("<%d>:%lld\n", i, *(StackElem_t*)((char*)(stk->data) + i * sizeof(StackElem_t)));
    }

    CNR_PRT(
    printf("second hen: \t%llx", *(canary_t*)((char*)stk->data + (stk->capacity * sizeof(StackElem_t) / 8) * 8 + 8));
    if (*(canary_t*)((char*)stk->data + (stk->capacity * sizeof(StackElem_t) / 8) * 8 + 8) == 0x900deda) printf(GRN " \t\t<OK>\n" YEL);
        else printf(RED " \t\t<NOT OK>\n" YEL);
    printf("expected:   \t900deda\n");
    )

    CNR_PRT(

    printf("second chick:  \t%llx", stk->chicken_second);
    if (stk->chicken_second == 0x900DC0DE) printf(GRN " \t\t<OK>\n" YEL);
        else printf(RED " \t\t<NOT OK>\n" YEL);
    printf("expected:   \t900dc0de\n");
    )

    HASH_PRT(

    if (!stk->data){
        printf(RED "cannot find buffer data\n" RESET);
    }

    else{
        int64_t newBufferHash = FindBufferHash(stk);
        printf("buffer hash:\t%llx", stk->bufferHash);
        if (newBufferHash != stk->bufferHash) printf(RED " \t<NOT OK>\n" YEL);
        else printf(GRN " \t<OK>\n" YEL);
        printf("expected:   \t%llx\n", newBufferHash);
    }


    uint64_t newStackHash = FindStackHash(stk);
    printf(YEL "stack hash: \t%llx", stk->stackHash);
    if (newStackHash != stk->stackHash) printf(RED " \t<NOT OK>\n" YEL);
        else printf(GRN " \t<OK>\n" YEL);
    printf("expected:   \t%llx\n", newStackHash);

    )


    printf("}\n" RESET);
    return OK;
}

stackExits StackVerify(Stack_t* stk){
    if (!stk && stk){
        return ERR;
    }
    if (!(stk->data)){
        return DATA_EMPTY;
    }
    if (stk->size > stk->capacity){
        return SIZE_OVERFLOW;
    }

    CNR_PRT(
    if (stk->chicken_first != 0xbadc0de || stk->chicken_second != 0x900dc0de){
        return CNR_STK_ERR;
    }
    if (*(canary_t*)((char*)stk->data - 1 * sizeof(canary_t)) != 0xbadeda ||
        *(canary_t*)((char*)stk->data + (stk->capacity * sizeof(StackElem_t) / 8) * 8 + 8) != 0x900deda){

        return CNR_BUF_ERR;
    }
    )

    HASH_PRT(
    char sameStackHash  = (FindStackHash(stk)  == stk->stackHash );
    char sameBufferHash = (FindBufferHash(stk) == stk->bufferHash);
    if (!sameStackHash){
        return HASH_STK_ERR;
    } else if (!sameBufferHash){
        return HASH_BUF_ERR;
    }
    )

    return OK;
}


