SELECT * FROM meteo.dopusk_opr WHERE
       descriptor = @descriptor@ AND
       (level IS NULL OR level && ARRAY[@level@]) AND
       type_level && ARRAY[@type_level@] AND
       hour_min < @hour@ AND hour_max >= @hour@
