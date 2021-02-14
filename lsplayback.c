/*  *********************************************************************
    *  LightScript - A script processor for LED animations
    *  
    *  Arduino Playback                         File: lsplayback.c
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
#include <fcntl.h>
#include <errno.h>
#include "lightscript.h"



static double current_time(script_t *script)
{
    struct timeval tv;

    gettimeofday(&tv,NULL);

    return ((double) (tv.tv_sec - script->epoch)) + ((double)(tv.tv_usec)/1000000.0);
}

void send_message(script_t *script, unsigned int strips, unsigned int anim,  unsigned int speed, unsigned int palette)
{
    uint8_t message[10];

    if (script->device <= 0) {
        return;
    }

    message[0] = 0x02;
    message[1] = 0xAA;
    message[2] = strips & 0xFF;
    message[3] = (strips >> 8);
    message[4] = anim & 0xFF;
    message[5] = (anim >> 8);
    message[6] = speed & 0xFF;
    message[7] = (speed >> 8);
    message[8] = palette & 0xFF;
    message[9] = (palette >> 8);

    if (write(script->device, message, sizeof(message)) != sizeof(message)) {
        perror("Write Error to Arduino");
        exit(1);
    }
}

static void play_events(script_t *script)
{
    dqueue_t *dq;

    double start_time = current_time(script) + script->start_offset;

    dq = script->schedule.dq_next;

    while (dq != &(script->schedule)) {
        double now;

        schedcmd_t *cmd = (schedcmd_t *) dq;

        // Figure out the difference between the time stamp
        // at the start and now.
        now = current_time(script) - start_time;

        // If the current time is past the script command's time,
        // do the command.

        if (now >= cmd->time) {
            unsigned int anim;

            anim = cmd->animation;
            if (cmd->direction) anim |= 0x8000;
            
            printsched1(cmd);
            send_message(script, cmd->stripmask, anim, cmd->speed, cmd->palette);

            dq = dq->dq_next;
        }
    }

}

static dqueue_t *musicpos;
static script_t *curscript;

int player_callback(double now)
{
    schedcmd_t *cmd = (schedcmd_t *) musicpos;

    // If the current time is past the script command's time,
    // do the command.

    if (musicpos == &(curscript->schedule)) {
        // End of script, stop playing
        return 0;
    }

    if (now >= cmd->time) {
        unsigned int anim;

        anim = cmd->animation;
        if (cmd->direction) anim |= 0x8000;
            
        printsched1(cmd);
        send_message(curscript, cmd->stripmask, anim, cmd->speed, cmd->palette);

        musicpos = musicpos->dq_next;
    }

    // Keep going
    return 1;

}

extern int playMusicFile(const char *name, int (*callback)(double),double start_cue);

static void play_music(script_t *script)
{
    curscript = script;
    musicpos = script->schedule.dq_next;

    // Seek in script to cue point

    if (script->start_cue != 0.0) {
        while (musicpos != &(curscript->schedule)) {
            schedcmd_t *cmd = (schedcmd_t *) musicpos;
            if (cmd->time > script->start_cue) break;
            musicpos = musicpos->dq_next;
        }

        // We started past the end of the script, bail.
        if (musicpos == &(curscript->schedule)) {
            return;
        }
    }


    playMusicFile((const char *) script->musicfile, player_callback, script->start_cue);
}

static void play_idle(script_t *script)
{
    symbol_t *sym;

    if (script->idleanimation == NULL) {
        return;         // no idleanimation specified
    }

    sym = findsym(&(script->symbols),script->idleanimation);

    if (sym == NULL) {
        printf("Warning: idle animation '%s' is not valid\n",script->idleanimation);
        return;
    }
    
    // Start the idle animation.
    send_message(script, 0x2FF, sym->wvalues[0], 500, 0);
}


void play_script(script_t *script, int how)
{

    if (script->device_name != NULL) {
        script->device = open(script->device_name,O_RDWR);

        if (script->device < 0) {
            fprintf(stderr,"Error: Could not open Arduino device %s: %s\n",script->device_name, strerror(errno));
            return;
        }
    } else {
        script->device = 0;             // No device, just pretend.
    }

    play_idle(script);

    printf("\n\n");
    printf("Press ENTER to start playback\n"); getchar();
    
    if (how == 0) {
        play_events(script);
    } else {
        play_music(script);
    }

    sleep(1);

    play_idle(script);

    close(script->device);
}


