SELECT station, station_type, MIN(dt), MAX(dt) FROM meteo.@tablename@ GROUP BY station, station_type
