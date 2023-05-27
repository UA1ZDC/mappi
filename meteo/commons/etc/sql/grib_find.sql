SELECT 
id as _id, fileid 
FROM meteo.grib WHERE 
  descr = @descr@ and
  center = @center@ and
  subcenter = @subcenter@ and
  dt = @dt@ and
  "levelType" = @levelType@ and
  level = @levelValue@ and
  "procType" = @procType@ and 
  dt1 = @dt1@ and
  dt2 = @dt2@ and
  hour = @hour@ and
  "timeRange" = @timeRange@ and
  "stepLat" = @stepLat@ and
  "stepLon" = @stepLon@ and
  error = FALSE 
  ORDER BY hour DESC;
