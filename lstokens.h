
enum {
    tMUSIC = 0,
    tFROM,
    tTO,
    tDO,
    tAT,
    tCOUNT,
    tIDLE,
    tWAIT,
    tSPEED,
    tCOMMA,
    tCASCADE,
    tBRIGHTNESS,
    
    tIDENT,
    tFLOAT,
    tWHOLE,
    tSTRING,

    tNEWLINE
};


typedef struct lstoken_s {
    int lst_type;
    double lst_float;
    int lst_whole;
    char *lst_str;
} lstoken_t;
    
