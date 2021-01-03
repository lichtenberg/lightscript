//
// OK here is where our script starts
//

music "sunny.aac";

idle IDLEWHITE;

//
// Scheduled items start here.
//

define MAC1 {
    at 0.0 do FLICKER on SPOKE8;
    at 0.15 do FLICKER on SPOKE8;
    at 0.30 do FLICKER on SPOKE8;
    at 0.45 do FLICKER on SPOKE8;
};

at 0.2 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.5 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.7 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.4 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.9 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.0 do SOUNDPULSE on ALLSTRIPS;
at 5.0 macro MAC1;
from 0.2 to 1.8 count 5 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 20;

at 2.0 cascade SOUNDPULSE on ALLSTRIPS delay 0.25;



