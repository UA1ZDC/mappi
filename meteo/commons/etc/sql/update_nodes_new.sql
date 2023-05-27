INSERT INTO meteo.nodes(id, name, station, host, port, location, alt, country, cccc, status, quality, station_type)
  VALUES (DEFAULT,
    @name@,
    @station@,
    @host@,
    @port@,
    @location@,
    @alt@,
    @country@,
    @cccc@,
    @status@,
    @quality@,
    @type@ )
  ON CONFLICT(station, host, port) DO UPDATE SET
    name = EXCLUDED.name,
    location = EXCLUDED.location,
    alt = EXCLUDED.alt,
    country = EXCLUDED.country,
    cccc = EXCLUDED.cccc,
    status = EXCLUDED.status,
    quality = EXCLUDED.quality,
    station_type = EXCLUDED.station_type;
