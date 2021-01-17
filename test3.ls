//
// OK here is where our script starts
//

music "sunny.aac";

idle IDLEWHITE;

//
// Scheduled items start here.
//

define MAC1 {
    at 0.0 do COMET on SPOKE1 speed 500;
    at 0.15 do COMET on SPOKE3 speed 500;
    at 0.30 do COMET on SPOKE5 speed 500;
    at 0.45 do COMET on SPOKE7 speed 500;
};

from 0.5 to 1.5 count 5 do SOUNDPULSE on ALLSTRIPS speed 50;

at 2.0 macro MAC1;

at 5.0 cascade MOVINGBARS on ALLSTRIPS delay 0.1 speed 1000;

at 9.0 cascade SOUNDPULSE on ALLSTRIPS delay 0.25 speed 100;

at 12.0 do OFF on ALLSTRIPS;


