SELECT center, subcenter, dt, "procType", 
       "levelType", "level", descr,
	     dt1, dt2, "timeRange", hour FROM meteo.grib
	  WHERE 
	  dt BETWEEN @start_dt@ AND @end_dt@  AND
    (@descr@ IS NULL or descr = ANY (@descr@)) AND
    "levelType" = @levelType@ AND
    "level" = @levelValue@ AND
    center = @center@ AND
    hour = @hour@ AND
    "procType" = @procType@ AND
    analysed != true and
    error != TRUE 
    ORDER BY hour DESC limit 1;
