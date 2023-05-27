SELECT * FROM meteo.get_station_type( @type@ ) as
(type smallint,
	description text,
	max_data_type integer,
	min_data_type integer);
