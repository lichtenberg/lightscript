/*  *********************************************************************
    *  LightScript - A script processor for LED animations
    *  
    *  Main Program                             File: lsmain.c
    *  
    *  Top-level functions for the lightscript parser.
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
#include <errno.h>
#include "lightscript.h"

extern void yyparse(void);

extern int yylineno;
extern FILE *yyin;

void printtree(node_t *node, int depth);

extern node_t *tree;
script_t script;
char *inpfilename = "input";
int debug = 0;

static void usage(void)
{
    fprintf(stderr,"Usage: lightscript [-c configfile] [-v] script-file\n\n");
    fprintf(stderr,"    -c configfile       Specifies the name of a configuration file\n");
    fprintf(stderr,"    -v                  Print diagnostic output\n");
    fprintf(stderr,"    script-file         Name of script file to process\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"    If not specified, and 'lightscript.cfg' exists, this file will be processed\n");
    fprintf(stderr,"    as a configuration file if '-c' is not specified\n");
    fprintf(stderr,"\n");
    exit(1);

}

void yyerror(char *str,...)
{
    va_list ap;
    va_start(ap,str);

    fprintf(stderr,"%s(%d) : ",inpfilename,yylineno);
    vfprintf(stderr,str,ap);
    fprintf(stderr,"\n");
}


static node_t *parse_file(char *filename)
{
    yyin = fopen(filename,"r");

    if (!yyin) {
        fprintf(stderr,"Could not open %s : %s\n",filename,strerror(errno));
        return NULL;
    }

    tree = NULL;

    // Save file name for error messages.
    inpfilename = filename;
    yyparse();

    fclose(yyin);

    return tree;
}


int main(int argc,char *argv[])
{
    FILE *str = stdin;
    char *configfilename = "lightscript.cfg";
    char *scriptfilename = NULL;
    int userconfig = 0;
    char ch;
    struct stat statbuf;

    initscript(&script);

    while ((ch = getopt(argc,argv,"c:v")) != -1) {
        switch (ch) {
            case 'c':
                configfilename = optarg;
                userconfig = 1;
                break;
            case 'v':
                debug = 1;
                break;
        }
    }
    
    // Skip over options, what's left ar bare args.
    argc -= optind;
    argv += optind;

    if (argc < 1) {
        usage();

    }

    scriptfilename = argv[0];

    script.configtree = parse_file(configfilename);

    if (!script.configtree) {
        fprintf(stderr,"[Proceeding without a config file]\n");
    }

    script.scripttree = parse_file(scriptfilename);

    if (!script.scripttree) {
        fprintf(stderr,"Could not read script file.\n");
        exit(1);
    }
    
    // First, walk the tree and find all our "defines"
    if (script.configtree) {
        printf("* Processing configuration file\n");
        savedefines(&script,script.configtree);
    }
    printf("* Processing script file\n");
    savedefines(&script,script.scripttree);
    printf("* Finding script commands\n");
    savecommands(&script, script.scripttree);

    if (debug > 0) {
        if (script.configtree) {
            printf("------------------------------------------------------------------------\n");
            printf("-- Configuration file: %s\n",configfilename);
            printtree(script.configtree,0);
        }
        printf("------------------------------------------------------------------------\n");
        printf("-- Script file: %s\n",scriptfilename);
        printtree(script.scripttree,0);
        printf("------------------------------------------------------------------------\n");

        printsymtab(&script);

        printf("------------------------------------------------------------------------\n");
        printf("-- Commands from file: %s\n",scriptfilename);

        printcmdtab(&script);
    }


    printf("* Generating schedule\n");
    genschedule(&script);
    
    return 0;
}


void initscript(script_t *script)
{
    memset(script,0,sizeof(script_t));

    dq_init(&(script->symbols));
    dq_init(&(script->macros));
    dq_init(&(script->commands));
    dq_init(&(script->schedule));
}


void insert_sched(script_t *script, schedcmd_t *scmd)
{
    dqueue_t *dq = script->schedule.dq_next;

    while (dq != &(script->schedule)) {
        schedcmd_t *acmd = (schedcmd_t *) dq;

        // Look for our place in the schedule to insert.
        if (scmd->time < acmd->time) {
            break;
        }

        dq = dq->dq_next;
    }

    // The place where we stopped is where we want to insert the next command.
    dq_enqueue(dq,&(scmd->link));
}



static void schedule_from(script_t *script,double basetime,command_t *cmd)
{
    int i;

    for (i = 0; i < cmd->count; i++) {
        double t = cmd->from + (cmd->to - cmd->from) * ((double) i / (double) (cmd->count-1));
    
        schedcmd_t *scmd = (schedcmd_t *) calloc(1,sizeof(schedcmd_t));

        scmd->time = basetime + t;
        scmd->speed = cmd->speed;
        scmd->brightness = cmd->brightness;
        scmd->palette = cmd->palette;
        scmd->animation = getsymval(&(cmd->animations));
        scmd->stripmask = getsymmask(&(cmd->strips));

        insert_sched(script,scmd);

    }
    
}

static void schedule_at(script_t *script,double basetime,command_t *cmd)
{
    schedcmd_t *scmd = (schedcmd_t *) calloc(1,sizeof(schedcmd_t));

    scmd->time = basetime + cmd->from;
    scmd->speed = cmd->speed;
    scmd->brightness = cmd->brightness;
    scmd->palette = cmd->palette;
    scmd->animation = getsymval(&(cmd->animations));
    scmd->stripmask = getsymmask(&(cmd->strips));

    insert_sched(script,scmd);
}




static void schedule_cascade(script_t *script,double basetime,command_t *cmd)
{
    int i;
    int count = cmd->strips.nvalues;        // we iterate across strips

    for (i = 0; i < count; i++) {
        double t = cmd->from + (((double) i) * cmd->delay);
    
        schedcmd_t *scmd = (schedcmd_t *) calloc(1,sizeof(schedcmd_t));

        scmd->time = basetime + t;
        scmd->speed = cmd->speed;
        scmd->brightness = cmd->brightness;
        scmd->palette = cmd->palette;
        scmd->animation = getsymval(&(cmd->animations));

        // For CASCADE we start each animation on a different strip at a different time.
        scmd->stripmask = (1 << cmd->strips.wvalues[i]);

        insert_sched(script,scmd);

    }
    
}

static char *maskstr(char *str,uint32_t m)
{
    int i;

    for (i = 0; i<16; i++) {
        str[15-i] = ((1 << i) & m) ? "0123456789ABCDEF"[i] : '.';
    }
    str[16] = 0;
    return str;
}

void dumpschedule(script_t *script)
{
    dqueue_t *dq = script->schedule.dq_next;
    char tmpstr[17];

    while (dq != &(script->schedule)) {
        schedcmd_t *acmd = (schedcmd_t *) dq;

        printf("Time %6.3f anim %3u strips %s\n",acmd->time,acmd->animation,maskstr(tmpstr,acmd->stripmask));
        
        dq = dq->dq_next;
    }

}

static void genschedlist(script_t *script, double basetime, dqueue_t *list)
{
    dqueue_t *dq = list;

    dq = dq->dq_next;   // advance to first one
    
    while (dq != list) {
        command_t *cmd = (command_t *) dq;

        switch (cmd->cmdtype) {
            case CMD_AT:
                schedule_at(script,basetime,cmd);
                break;
            case CMD_FROM:
                schedule_from(script,basetime,cmd);
                break;
            case CMD_CASCADE:
                schedule_cascade(script,basetime,cmd);
                break;
        }

        dq = dq->dq_next;
    }

}


void genschedule(script_t *script)
{
    dqueue_t *list = &(script->commands);

    genschedlist(script, 0, list);

    dumpschedule(script);
    
}
