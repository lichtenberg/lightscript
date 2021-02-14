//
// OK here is where our script starts
//

//music "sunny.m4a";
//music "../Africa.m4a";

idle IDLEWHITE;

//
// Scheduled items start here.
//

define MAC1 {
    at 0.0 do COMET on SPOKE1 speed 500;
    at 0.15 do COMET on SPOKE3 speed 500;
    at 0.30 do COMET on SPOKE5 speed 500;
    at 0.45 do COMET on SPOKE7 speed 500;
    at 0.0 do COMET on SPOKE2 speed 500 reverse;
    at 0.15 do COMET on SPOKE4 speed 500 reverse;
    at 0.30 do COMET on SPOKE6 speed 500 reverse;
    at 0.45 do COMET on SPOKE8 speed 500 reverse;
};

from 0.5 to 1.5 count 5 do SOUNDPULSE on ALLSTRIPS speed 50;

at 2.0 macro MAC1;

at 3.0 cascade MOVINGBARS on EVENSPOKES delay 0.1 speed 5000;
at 3.0 cascade MOVINGBARS on ODDSPOKES delay 0.1 speed 5000 reverse;

at 5.0 cascade MOVINGBARS on ALLSTRIPS delay 0.1 speed 1000;

at 9.0 cascade SOUNDPULSE on ALLSTRIPS delay 0.25 speed 100;

at 12.0 do OFF on ALLSTRIPS;


