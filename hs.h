#ifndef HS_H
#define HS_H


struct hsearch_data
{
    struct _ENTRY *table;
    unsigned int size;
    unsigned int filled;
};

int mhcreate_r(size_t, struct hsearch_data *);
void mhdestroy_r(struct hsearch_data *);
int mhsearch_r(ENTRY, ACTION, ENTRY **, struct hsearch_data *);

ENTRY *mhsearch(ENTRY, ACTION);
int mhcreate(size_t);
void mhdestroy();


#endif
