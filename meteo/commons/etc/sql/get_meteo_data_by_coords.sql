select * from
meteo.get_meteo_data_by_coords( @start_dt@,
                                     @end_dt@,
                                     @level@,
                                     @level_type@,
                                     @data_type@,
                                     @station@,
                                     @station_type@,
                                     @descrname@,
                                     @coords@,
                                     @max_rast@
                                  )as 
(
  oid bigint, 
 	dt timestamp without time zone,
	station text,
	station_type smallint,
  station_info jsonb,
  data_type smallint,
	level real,
	level_type smallint,
	param jsonb,
	location jsonb,
	dist double precision 
	);
 
