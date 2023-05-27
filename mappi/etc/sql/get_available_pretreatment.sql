SELECT * FROM meteo.pretreatment_view
	WHERE
       (@date_start@ IS NULL OR date_start >= @date_start@) AND
       (@date_end@   IS NULL OR date_end   <= @date_end@)   AND
       (@session_id@ IS NULL OR session_id = @session_id@)  AND
       (@satellite@ IS NULL OR satellite = @satellite@) AND
       (@instrument_type@ IS NULL OR instrument_id = @instrument_type@)  AND
       (@channel@ IS NULL OR channel_number = ANY(ARRAY[@channel@]::integer[])) AND
       (@channel_alias@ IS NULL OR channel_alias = ANY(ARRAY[@channel_alias@])) AND
       (@calibration@ IS NULL OR calibration = @calibration@)

