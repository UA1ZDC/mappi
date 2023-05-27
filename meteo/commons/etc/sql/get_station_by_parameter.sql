select * from meteo.get_station_by_parameter(@paramname@, @paramvalue@) as 
(_id integer,
station text,
"index" jsonb,
station_type smallint,
"name" jsonb,
coord jsonb,
country integer,
hydro_levels real,
countryinfo jsonb);
