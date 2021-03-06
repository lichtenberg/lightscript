//
// First, define all our strips
//

define SPOKE1 as 0 ;
define SPOKE2 as 1 ;
define SPOKE3 as 2 ;
define SPOKE4 as 3 ;
define SPOKE5 as 4 ;
define SPOKE6 as 5 ;
define SPOKE7 as 6 ;
define SPOKE8 as 7 ;
define RING   as 8 ;

define ALLSPOKES as SPOKE1,SPOKE2,SPOKE3,SPOKE4,SPOKE5,SPOKE6,SPOKE7,SPOKE8 ;
define ALLSTRIPS as SPOKE1,SPOKE2,SPOKE3,SPOKE4,SPOKE5,SPOKE6,SPOKE7,SPOKE8,RING ;

//
// Define names for ALA animations
//

define MOVINGBARS as 101 ;
define SOUNDPULSE as 102 ;
define IDLEWHITE as 103 ;

//
// OK here is where our script starts
//

music "sunny.aac";

idle IDLEWHITE;

//
// Scheduled items start here.
//

at 0.0 do SOUNDPULSE on ALLSTRIPS;
at 0.2 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.4 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.5 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.7 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
at 0.9 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 50;
from 0.2 to 1.8 count 5 do MOVINGBARS on SPOKE1, SPOKE3, SPOKE5, SPOKE7 speed 20;



