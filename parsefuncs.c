/*  *********************************************************************
    *  LightScript - A script processor for LED animations
    *  
    *  Parser Functions                         File: parsefuncs.c
    *  
    *  Handy helper routines called from within our yacc/bison 
    *  file to build the parse tree of a script.
    *  
    *  Author:  Mitch Lichtenberg
    ********************************************************************* */


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include "lightscript.h"


extern void yyparse(void);

extern int yylineno;
extern FILE *yyin;
node_t *tree = NULL;



node_t *newnode(node_t *l,node_t *r)
{
    node_t *n = (node_t *) calloc(1,sizeof(node_t));

    n->left = l;
    n->right = r;
    n->type = nNODE;
    n->line = yylineno;

    return n;
}

node_t *newidlist(char *idstr,node_t *rest)
{
    idlist_t *idl = (idlist_t *) calloc(1,sizeof(idlist_t));

    idl->type = nIDLIST;
    idl->next = rest;
    idl->idstr = idstr;
    idl->line = yylineno;

    return (node_t *) idl;
}

node_t *newoption_idl(int opttype, node_t *optval)
{
    option_t *opt = (option_t *) calloc(1,sizeof(option_t));

    opt->type = nOPTION;
    opt->opttype = opttype;
    opt->lvalue = optval;
    opt->line = yylineno;

    return (node_t *) opt;
}

node_t *newoption_w(int opttype, int optval)
{
    option_t *opt = (option_t *) calloc(1,sizeof(option_t));

    opt->type = nOPTION;
    opt->opttype = opttype;
    opt->wvalue = optval;
    opt->line = yylineno;

    return (node_t *) opt;
}

node_t *newoption_f(int opttype, double optval)
{
    option_t *opt = (option_t *) calloc(1,sizeof(option_t));

    opt->type = nOPTION;
    opt->opttype = opttype;
    opt->fvalue = optval;
    opt->line = yylineno;

    return (node_t *) opt;
}


node_t *newcmd_sched(int cmdtype, double from, double to, node_t *opts)
{
    scriptcmd_t *sc = (scriptcmd_t *) calloc(1,sizeof(scriptcmd_t));

    sc->type = nSCRIPT;
    sc->cmdtype = cmdtype;
    sc->from = from;
    sc->to = to;
    sc->options = opts;
    sc->line = yylineno;

    return (node_t *) sc;
}

node_t *newcmd_str(int cmdtype, char *str)
{
    scriptcmd_t *sc = (scriptcmd_t *) calloc(1,sizeof(scriptcmd_t));

    sc->type = nSCRIPT;
    sc->cmdtype = cmdtype;
    sc->str = str;
    sc->line = yylineno;

    return (node_t *) sc;
}


node_t *newcmd_defidl(char *str, node_t *idl)
{
    scriptcmd_t *sc = (scriptcmd_t *) calloc(1,sizeof(scriptcmd_t));

    sc->type = nSCRIPT;
    sc->cmdtype = sDEFINE;
    sc->str = str;
    sc->options = idl;
    sc->line = yylineno;

    return (node_t *) sc;
}

node_t *newcmd_defmacro(char *str, node_t *idl)
{
    scriptcmd_t *sc = (scriptcmd_t *) calloc(1,sizeof(scriptcmd_t));

    sc->type = nSCRIPT;
    sc->cmdtype = sMACRO;
    sc->str = str;
    sc->options = idl;
    sc->line = yylineno;

    return (node_t *) sc;
}


node_t *newcmd_defval(char *str, int val)
{
    scriptcmd_t *sc = (scriptcmd_t *) calloc(1,sizeof(scriptcmd_t));

    sc->type = nSCRIPT;
    sc->cmdtype = sDEFINE;
    sc->str = str;
    sc->val = val;
    sc->line = yylineno;

    return (node_t *) sc;
}




void printtree(node_t *n, int depth)
{
    if (!n) {
        return;
    }
            
    printf("%*s",depth*3,"");

    switch (n->type) {
        case nNODE:
//            printf("Node\n");
            printtree(n->left,depth+1);
            printtree(n->right,depth);
            break;
        case nIDLIST:
        {
            idlist_t *idl = (idlist_t *) n;
            printf("IDList ");
            while (idl) {
                assert(idl->type == nIDLIST);
                printf("%s ",idl->idstr);
                idl = (idlist_t *) idl->next;
            }
            printf("\n");
        }
        break;
        case nOPTION:
        {
            option_t *opt = (option_t *) n;
            switch (opt->opttype) {
                case oON:
                    printf("On ");
                    printtree(opt->lvalue,depth+1);
                    break;
                case oDO:
                    printf("Do ");
                    printtree(opt->lvalue,depth+1);
                    break;
                case oMACRO:
                    printf("Macro ");
                    printtree(opt->lvalue,depth+1);
                    break;
                case oSPEED:
                    printf("Speed %u\n",opt->wvalue);
                    break;
                case oBRIGHTNESS:
                    printf("Brightness %u\n",opt->wvalue);
                    break;
                case oDELAY:
                    printf("Delay %5.3f\n",opt->fvalue);
                    break;
                case oCOUNT:
                    printf("Count %u\n",opt->wvalue);
                    break;
                case oCASCADE:
                    printf("Cascade ");
                    printtree(opt->lvalue,depth+1);
                    break;
                    
            }
        }
        break;
        case nSCRIPT:
        {
            scriptcmd_t *sc = (scriptcmd_t *) n;
            printf("(%d)ScriptCmd" ,sc->line);
            switch (sc->cmdtype) {
                case sFROM:
                    printf("From %5.3f to %5.3f\n",sc->from,sc->to);
                    printtree(sc->options,depth);
                    break;
                case sAT:
                    printf("At %5.3f\n",sc->from);
                    printtree(sc->options,depth);
                    break;
                case sMUSIC:
                    printf("Music %s\n",sc->str);
                    break;
                case sIDLE:
                    printf("Idle %s\n",sc->str);
                    break;
                case sDEFINE:
                    printf("Define %s as",sc->str);
                    if (sc->options) {
                        printf("\n");
                        printtree(sc->options,depth+1);
                    }
                    else printf("%d\n",sc->val);
                    break;
                case sMACRO:
                    printf("Macro %s\n",sc->str);
                    printtree(sc->options,depth+1);
                    break;
                    
            }
        }
    }
}

