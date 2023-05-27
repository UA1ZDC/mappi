SELECT * FROM meteo.thematic_view WHERE 
       (@date_start@ IS NULL OR date_start >= @date_start@) AND
       (@date_start@ IS NULL OR date_start <= @date_end@)   AND
       (@satellite_name@ IS NULL OR satellite = @satellite_name@) AND
       (@type@  IS NULL OR type = @type@) AND
       (@instr_type@ IS NULL OR instrument = @instrument_type@) AND
       (@session_id@ IS NULL OR session_id = @session_id@) AND
       (@projection@ IS NULL OR projection = @projection@) AND
       (@format@     IS NULL OR format = @format@)
