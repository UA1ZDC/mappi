SELECT * from meteo.insert_grib( 
@dt@ ,@hour@, @edition@, @center@, @subcenter@, @product@, @descr@, 
@levelValue@, @levelType@, @procType@, @timeRange@, @sign_dt@, @dt1@, @dt2@, @status@,
@data_type@, @discipline@,@category@,@defType@,
@stepLat@, @stepLon@, @fileid@, @tlgid@, @crc@, false, @dt_write@, false
);
