SELECT frg.id, frg.title, frg.description, frg.is_active,
    s.name->>'ru' as station_name,
    jsonb_set( to_jsonb( s ), '{location}', jsonb_set(ST_AsGeoJson(s.location)::jsonb,'{alt}', to_jsonb(0)) , true )#-'{hydro_levels}'  as station,
    jsonb_set(ST_AsGeoJson(s.location)::jsonb,'{alt}', to_jsonb(0)) as station_coord,  
    fr.title as region_name,
    jsonb_set(ST_AsGeoJson(fr.region_center_point)::jsonb,'{alt}', to_jsonb(0)) as region_coord,
    frtrg.region_id,
    frtrg.station_id FROM meteo.forecast_regiongroup as frg
    LEFT JOIN meteo.forecast_region_to_regiongroup frtrg ON frg.id = frtrg.regiongroup_id
    LEFT JOIN meteo.stations        s  ON s.id  = frtrg.station_id
    LEFT JOIN meteo.forecast_region fr ON fr.id = frtrg.region_id
    WHERE (@id@ IS NULL or frg.id = @id@) 
       
