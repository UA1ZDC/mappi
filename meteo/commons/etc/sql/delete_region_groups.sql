-- начало транзакции
BEGIN;

DELETE FROM meteo.forecast_regiongroup 
    WHERE id = @id@;

DELETE FROM meteo.forecast_region_to_regiongroup 
    WHERE regiongroup_id = @id@;

COMMIT;