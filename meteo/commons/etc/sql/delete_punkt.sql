WITH deleted AS (DELETE FROM meteo.punkts WHERE _id = ANY(@id@) RETURNING _id) SELECT count(*) AS n FROM deleted
