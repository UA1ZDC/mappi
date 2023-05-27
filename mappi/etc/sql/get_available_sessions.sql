SELECT * FROM sessions WHERE
       (@date_start@ IS NULL OR date_start >= @date_start@) AND
       (@date_end@   IS NULL OR date_end   <= @date_end@)   AND
       (@satellite_name@ IS NULL OR satellite_name = @satellite_name@) AND
       (@preprocessed@   IS NULL OR preprocessed = @preprocessed@)     AND	 
       (@site@ IS NULL OR site = @site@) AND
       (@thematic_done@  IS NULL OR thematic_done = @thematic_done@)
