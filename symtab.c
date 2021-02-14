/*  *********************************************************************
    *  LightScript - A script processor for LED animations
    *  
    *  Symbol Table                                File: symtab.c
    *  
    *  This module contains routines to maintain the symbol table,
    *  a list of mappings of names to various other objects
    *  used within lightscript.
    *  
    *  Author:  Mitch Lichtenberg
    ********************************************************************* */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#include "lightscript.h"


symbol_t *findsym(dqueue_t *tab, char *str)
{
    dqueue_t *qb;

    qb = tab->dq_next;

    while (qb != tab) {
        symbol_t *sym = (symbol_t *) qb;

        if (strcmp(sym->name, str) == 0) {
            return sym;
        }

        qb = qb->dq_next;
    }

    return NULL;
}


symbol_t *newsym(dqueue_t *tab, char *str)
{
    symbol_t *sym;

    // return it if we already have it.
    sym = findsym(tab, str);
    if (sym) {
        return sym;
    }

    // Otherwise make a new symbol
    // calloc() takes care of zeroing fields.
    sym = (symbol_t *) calloc(1,sizeof(symbol_t));

    sym->name = str;

    dq_enqueue(tab,&(sym->link));

    return sym;
}

static inline void addsymval(symbol_t *sym, unsigned int val)
{
    assert(sym->nvalues < MAXVALUES);

    sym->wvalues[sym->nvalues] = val;
    sym->nvalues++;
}

unsigned int getsymval(symbol_t *sym)
{
    assert(sym->nvalues == 1);

    return sym->wvalues[0];
}

unsigned int getsymmask(symbol_t *sym)
{
    int i;
    unsigned int ret = 0;

    for (i = 0; i < sym->nvalues; i++) {
        ret |= (1 << sym->wvalues[i]);
    }

    return ret;
}



void addsymp(dqueue_t *tab, char *str, void *ptr)
{
    symbol_t *sym;

    // Just update the symbol if it's already there
    sym = findsym(tab, str);
    if (sym) {
        sym->pvalue = ptr;
        return;
    }

    // Otherwise make a new symbol
    sym = (symbol_t *) calloc(1,sizeof(symbol_t));

    sym->name = str;
    sym->pvalue = ptr;

    dq_enqueue(tab,&(sym->link));
}

//
// Append values stored for one symbol into another
//
static void appendvals(symbol_t *dest, symbol_t *src)
{
    int i;

    for (i = 0; i < src->nvalues; i++) {
        addsymval(dest,src->wvalues[i]);
    }
}

static void defsym(script_t *script, scriptcmd_t *sc)
{
    symbol_t *sym;
    symbol_t *s;
    idlist_t *idl;

    // Put the symbol in the table, or find it if it's there.
    sym = newsym(&(script->symbols),sc->str);
    
    // two cases:  either we have a numeric value or we have a list of named values.
    if (sc->options == NULL) {    // it's not a list
        addsymval(sym,sc->val);
    } else {
        // Other case: we have an IDLIST to walk.
        // insert ids here
        idl = (idlist_t *) sc->options;
        while (idl) {
            assert(idl->type == nIDLIST);
            s = findsym(&(script->symbols),idl->idstr);
            if (!s) {
                printf("Warning: Symbol %s not defined\n",idl->idstr);
            } else {
                appendvals(sym,s);
            }
            idl = (idlist_t *) idl->next;
        }

    }
}

static void defmacro(script_t *script, scriptcmd_t *sc)
{
    symbol_t *sym;
    symbol_t *s;
    idlist_t *idl;

    // Put the symbol in the table, or find it if it's there.
    sym = newsym(&(script->macros),sc->str);
    
    sym->pvalue = sc->options;
}


static void defines1(script_t *script, node_t *n)
{
    scriptcmd_t *sc = (scriptcmd_t *) n;
    
    if (n == NULL) {
        return;
    }
    
    switch (n->type) {
        case nNODE:
            defines1(script, n->left);
            defines1(script, n->right);
            break;
        case nSCRIPT:
            switch (sc->cmdtype) {
                case sDEFINE:
                    defsym(script,sc);
                    break;
                case sIDLE:
                    script->idleanimation = sc->str;
                    break;
                case sMUSIC:
                    script->musicfile = sc->str;
                    break;
                case sMACRO:
                    defmacro(script,sc);
                    break;
            }

            break;
    }
}

void printsymtab(script_t *script)
{
    dqueue_t *qb;
    symbol_t *sym;
    int i;

    printf("Music file:      %s\n",script->musicfile ? script->musicfile : "not defined");
    printf("Idle animation:  %s\n",script->idleanimation ? script->idleanimation : "not defined");

    qb = script->symbols.dq_next;
    while (qb != &(script->symbols)) {

        sym = (symbol_t *) qb;

        printf("%-20.20s = ",sym->name);
        for (i = 0; i < sym->nvalues; i++) {
            printf("%u ",sym->wvalues[i]);
        }
        printf("\n");

        qb = qb->dq_next;
    }

    printf("- - - - - \n");
    printf("Macros: ");
    qb = script->macros.dq_next;
    while (qb != &(script->macros)) {
        sym = (symbol_t *) qb;

        printf("%s ",sym->name);

        qb = qb->dq_next;
    }
    printf("\n");
    
}

void savedefines(script_t *script, node_t *tree)
{
    defines1(script, tree);
}


static void collectids(script_t *script, symbol_t *dest, idlist_t *list)
{
    symbol_t *sym;
    
    while (list) {
        assert(list->type == nIDLIST);

        sym = findsym(&(script->symbols),list->idstr);

        if (sym) {
            appendvals(dest,sym);
        } else {
            printf("Script command identifier '%s' not found\n",list->idstr);
        }

        list = (idlist_t *) list->next;
    }
}

static void commands1(script_t *script, double basetime, node_t *n);

static void addcommand(script_t *script, double basetime, scriptcmd_t *sc)
{
    command_t *cmd;
    option_t *opt;
    node_t *n;
    symbol_t *macro;

    cmd = (command_t *) calloc(1,sizeof(command_t));

    cmd->from = sc->from + basetime;
    cmd->to = sc->to + basetime;

    cmd->cmdtype = CMD_AT;
    if (cmd->from != cmd->to) cmd->cmdtype = CMD_FROM;

    n = sc->options;

    while (n) {
        opt = (option_t *) n->left;

        assert(opt != NULL);

        if (opt) {
            assert(opt->type == nOPTION);

            switch (opt->opttype) {
                case oON:
                    collectids(script,&(cmd->strips),(idlist_t *) opt->lvalue);
                    break;
                case oMACRO:
                    // Expand macro here.
                    macro = findsym(&(script->macros),((idlist_t *) opt->lvalue)->idstr);
                    if (macro) {
                        commands1(script, cmd->from, (node_t *) macro->pvalue);
                    } else {
                        printf("Warning: Macro '%s' not found\n",
                               ((idlist_t *) opt->lvalue)->idstr);
                    }
                    // but this is not really a command of its own.
                    free(cmd);
                    cmd = NULL;
                    break;
                case oDO:
                    collectids(script,&(cmd->animations),(idlist_t *) opt->lvalue);
                    break;
                case oCASCADE:
                    cmd->cmdtype = CMD_CASCADE;
                    collectids(script,&(cmd->animations),(idlist_t *) opt->lvalue);
                    break;
                case oSPEED:
                    cmd->speed = opt->wvalue;
                    break;
                case oDELAY:
                    cmd->delay = opt->fvalue;
                    break;
                case oBRIGHTNESS:
                    cmd->brightness = opt->wvalue;
                    break;
                case oCOUNT:
                    cmd->count = opt->wvalue;
                    break;
                case oPALETTE:
                    cmd->palette = opt->wvalue;
                    break;
                case oREVERSE:
                    cmd->direction = 1;
                    break;
                default:
                    break;
            }

        }

        n = n->right;
    }

    if (cmd) {
        dq_enqueue(&(script->commands),&(cmd->link));
    }
}


static void commands1(script_t *script, double basetime, node_t *n)
{
    scriptcmd_t *sc = (scriptcmd_t *) n;
    
    if (n == NULL) {
        return;
    }
    
    switch (n->type) {
        case nNODE:
            commands1(script, basetime, n->left);
            commands1(script, basetime, n->right);
            break;
        case nSCRIPT:
            switch (sc->cmdtype) {
                case sAT:
                case sFROM:
                    addcommand(script,basetime, sc);
                    break;
                default:
                    break;
            }
            break;
    }

}



void printcmdtab(script_t *script)
{
    static char *cmdnames[] = {"AT","FROM","CASCADE","MACRO"};
    int i;

    dqueue_t *dq = script->commands.dq_next;

    while (dq != &(script->commands)) {
        command_t *cmd = (command_t *) dq;

        printf("%-8.8s ",cmdnames[cmd->cmdtype]);

        printf("From %.3f  To %.3f ",cmd->from,cmd->to);
        printf("Speed %u  Bright %u  Count %u  Delay %.3f  ",
               cmd->speed,cmd->brightness,cmd->count,cmd->delay);

        printf("Strips=[ ");
        for (i = 0; i < cmd->strips.nvalues; i++) printf("%d ",cmd->strips.wvalues[i]);
        printf("]  Anims=[ ");
        for (i = 0; i < cmd->animations.nvalues; i++) printf("%d ",cmd->animations.wvalues[i]);
        printf("] ");

        printf("\n");

        dq = dq->dq_next;
    }

}

void savecommands(script_t *script, node_t *tree)
{
    commands1(script, 0, tree);
}
