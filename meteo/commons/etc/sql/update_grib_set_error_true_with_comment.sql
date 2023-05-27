UPDATE meteo.grib SET  analysed = True, error = True, comment = @comment@ WHERE id = @id@;
