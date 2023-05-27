INSERT INTO meteo.thematic (session, type, them_name, date_start, date_end, instrument, projection, format, fpath)
       SELECT session_id, @type@, @them_name@, @date_start@, @date_end@, @instrument@, @projection@, @format@, @path@
       	      FROM meteo.pretreatment_view
       	      WHERE  satellite = @satellite@ and date_start=@date_start@ limit 1
