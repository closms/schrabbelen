#ifndef HS_H
#define HS_H


struct hsearch_data
{
    struct _ENTRY *table;
    unsigned int size;
    unsigned int filled;
};

#endif
