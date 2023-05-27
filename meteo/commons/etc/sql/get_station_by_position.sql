SELECT * from meteo.get_station_by_position( @coordinates@,
                           @maxdistance@,
                           @stationtype@ ) as 
(
	dist jsonb,
	_id integer,  	
	station text,
	index jsonb,
	station_type smallint,
	name jsonb,
	location jsonb,
	country integer,
	hydro_level jsonb,
	countryinfo jsonb,
	airstrip_direction double precision
);
