DELETE FROM meteo.report
WHERE dt = @dt@ AND ST_Within( meteo.report.location, ST_SetSRID( @location@ ,4326 ) );