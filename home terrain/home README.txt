terrain.party README
====================

Hooray! You've exported some stuff from terrain.party v1.2!

If you should want to export this again for any reason:
  http://terrain.party/api/export?name=home&box=-1.751688,52.697565,-1.870273,52.625700

Now: what did you get?


Height Maps
-----------

  * home Height Map (ASTER 30m).png

    ASTER is the most recent public survey of elevation on Earth. It has high
    coverage and high (~30m) resolution. However, the instrument can get
    confused by high concentraions of clouds and mountains, creating gaps in
    the data that need to be repaired by hand.
    http://asterweb.jpl.nasa.gov/gdem.asp


  * home Height Map (SRTM3 v4.1).png

    SRTM data was originally provided by the Shuttle Radar Topography Mission
    but SRTM3 v4.1 is the result of considerable effort from CGIAR-CSI. It has
    a ~30m resolution in the US and a ~90m resolution elsewhere.
    http://www.cgiar-csi.org/data/srtm-90m-digital-elevation-database-v4-1


Elevation Adjustment
--------------------

The original elevation models for this area contained elevations ranging from
16 through 178 meters.

These elevation values need to get transformed into a grayscale images. This
section describes how that translation was done in this particular case.

(The game isn't yet released, so what follows is a guess, but I think it's a
pretty good guess. If this turns out to be wrong, please use the link above to
re-export this data once we figure out what the deal is.)

Cities: Skylines supports terrain from 0m to 1024m. It represents elevations as
16-bit unsigned integers, which can be read directly from 16-bit height maps.
Each level therefore corresponds to 1/64th of a meter: 0 is 0m, 64 is 1m, etc.
Cities: Skylines maps default to having sea level at 40m, with typical terrain
starting at 60m.

These heightmaps were adjusted such that 16m (actual) is equal to 40m in
game. All other elevations are relative to that.

Again, the lowest point in these height maps is now 40m in game; all other
terrain is even higher. You will likely want to adjust the water level as a
result.


Enjoy!
http://terrain.party/
