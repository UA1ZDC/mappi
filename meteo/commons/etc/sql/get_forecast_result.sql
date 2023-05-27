SELECT id, dt, dt_beg, dt_end, level, level_type, center, hour, model, station_info,
              param || jsonb_build_object('real', field || observ) AS param
       FROM meteo.forecast
       WHERE station = @station@ AND station_type = @station_type@ AND
       	     (@start_dt@ IS NULL OR dt_beg >= @start_dt@) AND
	     (@end_dt@   IS NULL OR dt_end <= @end_dt@)    AND
	     (@opravd@   IS NULL OR flag_opravd != @opravd@) AND
	     (@method@   IS NULL OR param ->>'method' =  @method@)
       ORDER BY dt_end 
