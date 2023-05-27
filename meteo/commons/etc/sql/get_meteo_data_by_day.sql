select * from
meteo.get_last_meteo_data( @start_dt@,
                                     @end_dt@,
                                     @level@,
                                     @level_type@,
                                     @data_type@,
                                     @station@,
                                     @station_type@,
                                     @descrname@,
                                     @location@,
                                     @max_distace@,
                                     @country@
                                  )
as
(
  oid bigint,
  dt timestamp without time zone,
  station text,
  station_info jsonb,
  station_type smallint,
  data_type smallint,
  level real,
  level_type smallint,
  param jsonb,
  location jsonb,
  dist double precision
  );
--      "station_info.country": @country@,
