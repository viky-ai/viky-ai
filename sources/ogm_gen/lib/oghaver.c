/*
 *	Haversine distance
 *	Copyright (c) 2006 by Patrick Constant
 *	Dev : December 2006
 *	Version 1.0
*/
#include <loggen.h>
#include <math.h>


#define DOgPi           3.141592653589793238               


/*
 *  Haversine distance is described in http://www.movable-type.co.uk/scripts/LatLong.html
 *  This script calculates great-circle distances between the two points - that is, 
 *  the shortest distance over the earth’s surface - using the ‘Haversine’ formula.
 *  It assumes a spherical earth, ignoring ellipsoidal effects - which is accurate enough - for most purposes.. - 
 *  giving an ‘as-the-crow-flies’ distance between the two points (ignoring any hills!).
 *  Formula is:
 *    R = earth’s radius (mean radius = 6,371km)
 *    Dlat = lat2- lat1
 *    Dlong = long2- long1
 *    a = sin²(Dlat/2) + cos(lat1).cos(lat2).sin²(Dlong/2)
 *    c = 2.atan2(sqrt(a), sqrt(1-a))
 *    d = R.c 
*/




PUBLIC(int) OgHaversineDistance(double radius, double vlat1, double vlong1, double vlat2, double vlong2, double *pdistance, int dimension)
{
double Dlat,lat1,lat2;
double Dlong,long1,long2;
double a1,a2,a3,a4,a,c1,c2,c;
double pi=DOgPi;

if (dimension == DOgHaversineDimensionDecimalDegree) {
  lat1=vlat1*pi/180; lat2=vlat2*pi/180;
  long1=vlong1*pi/180; long2=vlong2*pi/180;
  }
/** by default, we are in radian **/
else {
  lat1=vlat1; lat2=vlat2;
  long1=vlong1; long2=vlong2;
  }
Dlat = lat2 - lat1; if (Dlat < 0) Dlat = -Dlat;
Dlong = long2- long1; if (Dlong < 0) Dlong = -Dlong;

a1 = sin(Dlat/2); a1 = a1*a1;
a2 = cos(lat1);
a3 = cos(lat2);
a4 = sin(Dlong/2); a4 = a4*a4;
a = a1 + a2*a3*a4;

c1 = sqrt(a);
c2 = sqrt(1-a);
c =  2*atan2(c1,c2);

*pdistance = radius * c;

DONE;
}






