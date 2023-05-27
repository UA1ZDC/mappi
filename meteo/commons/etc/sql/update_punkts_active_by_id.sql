UPDATE meteo.punkts SET "is_active" = @is_active@ WHERE _id = @id@ RETURNING 1 as ok
