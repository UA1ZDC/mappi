SELECT 
center, subcenter,dt,"procType","levelType",level as "levelValue",descr,
dt1,dt2,"timeRange", hour,"stepLat","stepLon"
FROM meteo.grib WHERE analysed = false and error = FALSE and descr >0  ORDER BY hour DESC LIMIT 1;
