SELECT *,ST_AsGeoJson(location)::jsonb as location_json  FROM meteo.mlmodel
WHERE 
    ( @descr@         IS NULL or descr         = @descr@        ) AND
    ( @level_type@    IS NULL or level_type    = @level_type@   ) AND
    ( @level@         IS NULL or level         = @level@        ) AND
    ( @hour@          IS NULL or hour          = @hour@         ) AND
    ( @center@        IS NULL or center        = @center@       ) AND
    ( @model@         IS NULL or model         = @model@        ) AND
    ( @mlmodel_type@  IS NULL or mlmodel_type  = @mlmodel_type@ ) ;