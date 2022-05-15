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
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include "lightscript.h"

extern void yyparse(void);

extern int yylineno;
extern FILE *yyin;

void printtree(node_t *node, int depth);

extern node_t *tree;
script_t script;
char *inpfilename = "input";
int debug = 0;
int script_error = 0;

static char *findarduino(void)
{
    DIR *dir;
    struct dirent *dp;
    char *devicenames[10];
    int picked;
    int devcnt = 0;
    int i;
    char devname[256];

    dir = opendir("/dev");              // We're going to look through /dev for files
    while ((dp = readdir(dir)) != NULL) {
        if ((devcnt < 10) && (strstr(dp->d_name,"cu.usbmodem"))) {
            devicenames[devcnt++] = strdup(dp->d_name);
        }
    }
    closedir(dir);


    if (devcnt == 1) {
        picked = 0;
        printf("[Found only one device, trying /dev/%s]\n",devicenames[0]);
    } else {
        printf("More than one possible device found.  Please choose one.\n");
        for (i = 0; i < devcnt; i++) {
            printf("  %d: /dev/%s\n",i,devicenames[i]);
        }
        for (;;) {
            printf("Choose the one that is connected to your Arduino:  ");
            fgets(devname,sizeof(devname),stdin);
            picked = atoi(devname);
            if ((picked < 0) || (picked >= devcnt)) continue;
            break;
        }
    }


    sprintf(devname,"/dev/%s",devicenames[picked]);

    for (i = 0; i < devcnt; i++) {
        free(devicenames[i]);
    }
    
    return strdup(devname);

}

static void usage(void)
{
    fprintf(stderr,"Usage: lightscript [-c configfile] [-v] [-p device] command script-file\n\n");
    fprintf(stderr,"    -c configfile       Specifies the name of a configuration file\n");
    fprintf(stderr,"    -p device           Specifies the name of the Arduino device\n");
    fprintf(stderr,"    -s time             Starting time for playback\n");
    fprintf(stderr,"    -v                  Print diagnostic output\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"  Commands:\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"      check     Check but do not play the script\n");
    fprintf(stderr,"      play      Check, then play the script\n");
    fprintf(stderr,"      mplay     Check, then play the script with background music\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"    script-file         Name of script file to process\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"    If not specified, and 'lightscript.cfg' exists, this file will be processed\n");
    fprintf(stderr,"    as a configuration file if '-c' is not specified\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"Example usage\n");
    fprintf(stderr,"\n");
    fprintf(stderr,"    ./lightscript play test1.ls          Play test1.ls on the LED system\n");
    fprintf(stderr,"    ./lightscript check test1.ls         Check for syntax errors but do not play\n");
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

    script_error = 1;
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

double get_time(void)
{
    struct timeval tv;

    gettimeofday(&tv,NULL);

    return ((double) tv.tv_sec + ((double) tv.tv_usec)/1000000.0);
}


#define CMD_PLAY        1
#define CMD_CHECK       2
#define CMD_MPLAY       3


static double _parsetime(char *str) {
    double minutes = 0;
    double seconds = 0;
    char *colon = strchr(str,':');
    if (colon) {
        *colon++ = '\0';
        if (*str != ':') minutes = atof(str);
        seconds = atof(colon);
    } else {
        seconds = atof(str);
    }
    seconds = minutes*60.0 + seconds;
    return seconds;
}

static int parse_range(char *str, double *start, double *end)
{
    char *x;
    
    *end = 0;
    *start = 0;

    // See if it's just the start, or the start and end
    if ((x = strchr(str,'-'))) {
        *x++ = 0;
        *start = _parsetime(str);
        *end = _parsetime(x);
    } else {
        *start = _parsetime(str);
    }

    return  1;
    
}

int main(int argc,char *argv[])
{
    FILE *str = stdin;
    char *configfilename = "lightscript.cfg";
    char *scriptfilename = NULL;
    int userconfig = 0;
    char ch;
    struct stat statbuf;
    char *playdevice = NULL;
    char *command;
    int cmdnum = 0;
    int skipflag = 0;
    double start_cue = 0;
    double end_cue = 0;

    initscript(&script);

    while ((ch = getopt(argc,argv,"c:vp:s:")) != -1) {
        switch (ch) {
            case 'c':
                configfilename = optarg;
                userconfig = 1;
                break;
            case 'v':
                debug = 1;
                break;
            case 'p':
                playdevice = optarg;
                break;
            case 's':
                parse_range(optarg,&start_cue,&end_cue);
                break;
        }
    }

    // Skip over options, what's left ar bare args.
    argc -= optind;
    argv += optind;

    if (argc < 2) {
        usage();

    }

    command = argv[0];
    scriptfilename = argv[1];

    if (strcmp(command,"play") == 0) cmdnum = CMD_PLAY;
    else if (strcmp(command,"check") == 0) cmdnum = CMD_CHECK;
    else if (strcmp(command,"mplay") == 0) cmdnum = CMD_MPLAY;

    if (cmdnum == 0) {
        fprintf(stderr,"You must specify a command, 'play', 'mplay', or 'check' before the file name\n");
        fprintf(stderr,"\n");
        usage();
    }

    if (!playdevice && ((cmdnum == CMD_PLAY) || (cmdnum == CMD_MPLAY))) {
        playdevice = findarduino();
    }

    script.device_name = playdevice;
    script.configtree = parse_file(configfilename);
    script.start_cue = start_cue;
    script.end_cue = end_cue;

    if (!script.configtree) {
        fprintf(stderr,"[Proceeding without a config file]\n");
    }

    yylineno = 0;

    script.scripttree = parse_file(scriptfilename);

    if (script_error) {
        fprintf(stderr,"There was an error in the script file\n");
        exit(1);
    }

    if (!script.scripttree) {
        fprintf(stderr,"Could not read script file.\n");
        exit(1);
    }

    if (start_cue != 0) printf("Will start playback at %5.2f seconds\n",start_cue);
    if (end_cue != 0) printf("Will stop playback at %5.2f seconds\n",end_cue);

    if ((end_cue != 0) && (end_cue < start_cue)) {
        printf("The end of the playback can't be before the beginning!\n");
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


    // If we're playing a real music file, the file must exist.
    if (cmdnum == CMD_MPLAY) {
        char *mfile = script.musicfile;
        if (!script.musicfile) {
            printf("Your script file does not contain a 'music' statement to identify the music file\n");
            exit(1);
        }
        script.musicfile = realpath(script.musicfile,NULL);
        if (!script.musicfile) {
            printf("The music file %s in your script was not found\n",mfile);
            exit(1);
        }
        
    }
    

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

    if (playdevice && (cmdnum == CMD_PLAY)) {
        play_script(&script, 0);
    } else if (playdevice && (cmdnum == CMD_MPLAY)) {
        play_script(&script, 1);
    }
    
    return 0;
}


void initscript(script_t *script)
{
    memset(script,0,sizeof(script_t));

    dq_init(&(script->symbols));
    dq_init(&(script->macros));
    dq_init(&(script->commands));
    dq_init(&(script->schedule));

    // Set the epoch (the start of 'time')
    time(&script->epoch);
    script->start_offset = 0.1;
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
        scmd->direction = cmd->direction;
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
    scmd->direction = cmd->direction;
    scmd->brightness = cmd->brightness;
    scmd->palette = cmd->palette;
    scmd->option = cmd->option;
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
        scmd->direction = cmd->direction;
        scmd->brightness = cmd->brightness;
        scmd->palette = cmd->palette;
        scmd->option = cmd->option;
        scmd->animation = getsymval(&(cmd->animations));

        // For CASCADE we start each animation on a different strip at a different time.
        scmd->stripmask = (1 << cmd->strips.wvalues[i]);

        insert_sched(script,scmd);

    }
    
}

#define MAXSTRIPS 31
static char *maskstr(char *str,uint32_t m)
{
    int i;

    for (i = 0; i<MAXSTRIPS; i++) {
        str[(MAXSTRIPS-1)-i] = ((1 << i) & m) ? "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i] : '.';
    }
    str[MAXSTRIPS] = 0;
    return str;
}

static void fmttime(char *dest, double t)
{
    unsigned int minutes = (int) (t / 60.0);
    double seconds = (t - ((double) minutes)*60.0);
    sprintf(dest,"%2u:%05.02f",
            minutes,seconds);
}

static void animname(script_t *script, unsigned int anim, char *buffer)
{
    char *name;
    
    name = findval(&(script->symbols), anim);
    if (!name) {
        sprintf(buffer, "anim %u",anim);
    } else {
        strcpy(buffer, name);
    }
}

void printsched1(script_t *script, schedcmd_t * acmd)
{
    char tmpstr[64];
    char timestr[32];
    char colorstr[32];
    char animstr[40];

    fmttime(timestr,acmd->time);

    if (acmd->palette & 0x1000000) {
        sprintf(colorstr,"color 0x%06X", acmd->palette & 0x00FFFFFF);
    } else {
        sprintf(colorstr,"palette %2u    ",acmd->palette);
    }

    animname(script, acmd->animation, animstr);
    
    printf("Time %8s | %-15.15s %c | speed %5u | option %5u | %s | strips %s\n",timestr,animstr,
           acmd->direction ? 'R' : 'F',
           acmd->speed, acmd->option,
           colorstr, maskstr(tmpstr,acmd->stripmask));
}


void dumpschedule(script_t *script)
{
    dqueue_t *dq = script->schedule.dq_next;

    while (dq != &(script->schedule)) {
        schedcmd_t *acmd = (schedcmd_t *) dq;

        printsched1(script,acmd);

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
