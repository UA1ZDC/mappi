SELECT dt_beg, dt_end, level, level_type, station, t.station_type, jsonb_agg(descrname) AS descrname, fi AS lat, la AS lon
FROM (
       SELECT dt_beg, dt_end, level, level_type, station, station_type, descrname
       FROM meteo.forecast
       WHERE dt_end >= @dt@
       GROUP BY level, level_type, station, station_type, descrname, dt_beg, dt_end) t 
LEFT JOIN meteo.punkts ON
          t.station = meteo.punkts.station_id AND t.station_type = meteo.punkts.station_type
GROUP BY dt_beg, dt_end, level, level_type, station, t.station_type, fi, la
