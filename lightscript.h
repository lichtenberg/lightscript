/*  *********************************************************************
    *  LightScript - A script processor for LED animations
    *  
    *  Data structures                          File: lightscript.h
    *  
    *  Our common include file, structures, constants, enums etc.
    *  
    *  Author:  Mitch Lichtenberg
    ********************************************************************* */


/*
 * lightscript stuff
 */

#include <sys/time.h>
#include "queues.h"


/*  *********************************************************************
    *  Parser-related
    ********************************************************************* */


enum {
    nNODE = 0,
    nIDLIST,
    nOPTION,
    nSCRIPT
};

enum {
    oON = 0,
    oDO,
    oCASCADE,
    oSPEED,
    oDELAY,
    oBRIGHTNESS,
    oCOUNT,
    oPALETTE,
    oMACRO,
    oREVERSE,
    oCOLOR,
    oOPTION,
};

enum {
    sAT = 0,
    sFROM,
    sMUSIC,
    sIDLE,
    sDEFINE,
    sMACRO
};
    


typedef struct node_s {
    int type;
    int line;
    struct node_s *left;
    struct node_s *right;
} node_t;



typedef struct option_s {
    int type;
    int line;
    int opttype;
    struct node_s *lvalue;
    int wvalue;
    double fvalue;
} option_t;

typedef struct scriptcmd_s {
    int type;
    int line;
    int cmdtype;
    double from,to;
    struct node_s *options;
    char *str;
    int val;
} scriptcmd_t;


typedef struct idlist_s {
    int type;
    int line;
    struct node_s *next;
    char *idstr;
} idlist_t;

node_t *newnode(node_t *l, node_t *r);
node_t *newidlist(char *idstr, node_t *rest);
node_t *newoption_idl(int opttype, node_t *optval);
node_t *newoption_w(int opttype, int w);
node_t *newoption_f(int opttype, double f);
node_t *newcmd_sched(int cmdtype, double from, double to, node_t *opts);
node_t *newcmd_str(int cmdtype, char *str);
node_t *newcmd_defval(char *str, int val);
node_t *newcmd_defidl(char *str, node_t *idl);
node_t *newcmd_defmacro(char *str, node_t *idl);


void yyerror(char  *str,...);
extern int yylex();

extern node_t *tree;


/*  *********************************************************************
    *  Symbol Table
    ********************************************************************* */

//
// Once all the symbols are resolved to values, they can take on
// a list of values (currently max'd to 256).   Most will have only one
// value, but some operations like "cascade" can iterate over
// several values, so we just keep them all in the order they were
// specified.
//

#define MAXVALUES 256

typedef struct symbol_s {
    dqueue_t link;
    char *name;
    int nvalues;                // # of values
    unsigned int wvalues[MAXVALUES];
    void *pvalue;
} symbol_t;

/*  *********************************************************************
    *  Script
    ********************************************************************* */


enum {
    CMD_AT = 0,
    CMD_FROM,
    CMD_CASCADE,
    CMD_MACRO
};

typedef struct command_s {
    dqueue_t link;
    int cmdtype;
    symbol_t animations;
    symbol_t strips;
    unsigned int speed;
    unsigned int count;
    unsigned int brightness;
    unsigned int palette;
    unsigned int direction;
    unsigned int option;
    double delay;
    double from, to;
} command_t;

typedef struct schedcmd_s {
    dqueue_t link;
    double time;
    unsigned int stripmask;
    unsigned int animation;
    unsigned int speed;
    unsigned int brightness;
    unsigned int palette;
    unsigned int direction;
    unsigned int option;
} schedcmd_t;

typedef struct script_s {
    char *musicfile;
    dqueue_t symbols;
    dqueue_t macros;
    char *idleanimation;

    // Parsed trees
    node_t *configtree;
    node_t *scripttree;

    // Command table (raw, not scheduled)
    dqueue_t commands;

    // schedule
    dqueue_t schedule;

    // Time
    time_t epoch;
    double start_offset;

    // Start cue time
    double start_cue;
    double end_cue;

    // Arduino device
    char *device_name;
    int device;
} script_t;

void initscript(script_t *script);

void savedefines(script_t *script, node_t *tree);
void savecommands(script_t *script, node_t *tree);
void printcmdtab(script_t *script);
void printsymtab(script_t *script);

void genschedule(script_t *script);

symbol_t *findsym(dqueue_t *tab, char *str);
char *findval(dqueue_t *tab, unsigned int val);


unsigned int getsymmask(symbol_t *sym);
unsigned int getsymval(symbol_t *sym);

void play_script(script_t *script,int how);
void printsched1(script_t *script, schedcmd_t * acmd);
