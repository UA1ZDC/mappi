SELECT id,station, name, country, cccc, host,port, quality, status, station_type,
ST_AsGeoJson(location)::jsonb||jsonb_build_object('alt',alt) as location
FROM meteo.nodes;
