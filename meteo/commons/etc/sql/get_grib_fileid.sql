SELECT descr, fileid FROM meteo.grib
       WHERE
       dt = @dt@ AND
       (@descr@ IS NULL or descr = ANY (@descr@)) AND
       "levelType" = @levelType@ AND
       "level" = @levelValue@ AND
       center = @center@ AND
       hour = @hour@ AND
       "procType" = @procType@ AND
       ("stepLat" = @steplat@ OR "stepLon" = @steplon@)
       and (@forecast_beg@ IS NULL or "dt1" = @forecast_beg@)
       and (@forecast_end@ IS NULL or "dt2" = @forecast_end@)
       and error != TRUE
