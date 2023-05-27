SELECT * from meteo.get_stations( @id@,
                                  @station@,
                                  @stationtype@,
                                  @stationindex@,
                                  @cccc@,
                                  @name@,
                                  @country@,
                                  @skip@,
                                  @limit@,
                                  @query@,
                                  @geometry@

) as
(
  _id                integer,
	station            text,
	index              jsonb,
	station_type       smallint,
	name               jsonb,
	location           jsonb,
	country            integer,
	hydro_level        jsonb,
	countryinfo        jsonb,
	airstrip_direction double precision
);
