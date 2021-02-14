#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

#include <stdlib.h>

extern int debug;

extern "C" {
    int playMusicFile(const char *filename, int (*callback)(double time), double start_cue);
};

int playMusicFile(const char *filename, int (*callback)(double time),double start_cue)
{
    AVAudioPlayer *audioPlayer;
    NSString *path = [NSString stringWithFormat:@"file://%@", [NSString stringWithCString:filename encoding:NSUTF8StringEncoding]];
    NSURL *soundUrl = [NSURL URLWithString:path];
    NSError *error;
    bool playIt = false;
    
    NSLog(@"Music playback URL = %@",[soundUrl absoluteURL]);
    audioPlayer = [[AVAudioPlayer alloc] initWithContentsOfURL:soundUrl error:&error];

    if (audioPlayer != nil) {
//        NSLog(@"Hey, we did get an object");
    }
    
    if (error) {
           NSLog(@"Error in audioPlayer: %@", [error localizedDescription]);
           return -1;
       } else {
           [audioPlayer prepareToPlay];
           playIt = true;
           //_audioPlayer.enableRate = YES;
           //_audioPlayer.volume = 0.5f;
           //_audioPlayer.rate = 1.0f;
       }
    
    if (playIt) {
        [audioPlayer play];
        if (start_cue != 0.0) {
            audioPlayer.currentTime = start_cue;
        }

        while (audioPlayer.playing) {
            NSTimeInterval playTime = [audioPlayer currentTime];
            if (debug) {printf("PlayTime = %6.4f\r",playTime); fflush(stdout); }
            if (callback) {
                if ( (*callback)(playTime) == 0) {
                    break;
                }
                [NSThread sleepForTimeInterval:0.01];
            }
        }

        [audioPlayer stop];
    }

    return 0;
}



#ifdef TESTPROG
int main(int argc, char *argv[])
{
    if (argc > 1) {
        char *path = realpath(argv[1], NULL);
        //playMusicFile("/Users/mpl/proj/Africa.m4a");
        if (path != NULL) {
            printf("Playing: %s\n",path);
            playMusicFile(path);
        } else {
            printf("Could not find file %s\n",argv[1]);
        }
    }

    exit(0);
    return 0;
}
#endif
