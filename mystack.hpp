#pragma once

//#define DBG_ENABLE
#define CNR_ENABLE
#define HASH_ENABLE

#ifdef CNR_ENABLE
    #define CNR_PRT(...)  __VA_ARGS__
#else
    #define CNR_PRT(...)
#endif

#ifdef HASH_ENABLE
    #define HASH_PRT(...)  __VA_ARGS__
#else
    #define HASH_PRT(...)
#endif


#ifdef DBG_ENABLE
    #define DBG(...)      __VA_ARGS__

    #define STK_CHECK(stk, file, line)\
    if (StackVerify(stk)){\
        StackDump(stk, file, line);\
        return ERR;\
    }
    #define DBGStackCtor(stk)\
        StackCtor(stk , __FILE__, __LINE__)
    #define DBGStackDtor(stk)\
        StackDtor(stk , __FILE__, __LINE__)
    #define DBGStackPush(stk, x)\
        StackPush(stk, x, __FILE__, __LINE__)
    #define DBGStackPop(stk, x)\
        StackPop(stk, x, __FILE__, __LINE__)
    #define DBGPrintLine(...)\
        __VA_ARGS__
#else
    #define STK_CHECK(stk,...)\
    if (StackVerify(stk)){\
        StackDump(stk);\
        return ERR;\
    }
    #define DBG(...)
    #define DBGStackCtor(stk)\
        StackCtor(stk)
    #define DBGStackDtor(stk)\
        StackDtor(stk)
    #define DBGStackPush(stk, x)\
        StackPush(stk, x)
    #define DBGStackPop(stk, x)\
        StackPop(stk, x)
    #define DBGPrintLine(...)\
        "unknown"
#endif

enum stackExits{
    OK = 0,
    ERR = 1,
    DATA_EMPTY = 2,
    SIZE_OVERFLOW = 3,
    SIZE_UNDERFLOW = 4,
    STK_NULL = 5,
    MEM_FULL = 6,
    CNR_STK_ERR = 7,
    CNR_BUF_ERR = 6,
    HASH_STK_ERR = 9,
    HASH_BUF_ERR = 10,
    REALLOC_ERR = 11

};

typedef uint64_t canary_t;

typedef uint64_t StackElem_t;

typedef struct Stack_t{
    CNR_PRT         (canary_t     chicken_first;)
    DBG             (const char*  name;)
    DBG             (const char*  filename;)
    DBG             (uint64_t     line;)

                     StackElem_t* data;
                     size_t       size;
                     size_t       capacity;

    HASH_PRT        (uint64_t     bufferHash;)
    HASH_PRT        (uint64_t     stackHash;)
    CNR_PRT         (canary_t     chicken_second;)
} Stack_t;

stackExits StackCtor   (Stack_t* stk                     DBG(, const char* fileName, int line));
stackExits StackDtor   (Stack_t* stk                     DBG(, const char* fileName, int line));
stackExits StackPush   (Stack_t* stk, StackElem_t  item  DBG(, const char* fileName, int line));
stackExits StackPop    (Stack_t* stk, StackElem_t* item  DBG(, const char* fileName, int line));
stackExits StackDump   (Stack_t* stk                     DBG(, const char* filename, int line));
stackExits StackVerify (Stack_t* stk);
