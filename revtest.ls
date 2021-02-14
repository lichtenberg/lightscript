//
// OK here is where our script starts
//

music "sunny.aac";

idle IDLEWHITE;

//
// Scheduled items start here.
//

at 0.2 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 500;
at 0.2 do MOVINGBARS on SPOKE2, SPOKE4, SPOKE6, SPOKE8 speed 500 reverse;

at 2.0 do SOUNDPULSE on ALLSTRIPS;


