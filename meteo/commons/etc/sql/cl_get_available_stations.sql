SELECT station, type, cltype, dtbegin as "dtBegin", dtend as "dtEnd", utc, jsonb_set(ST_AsGeoJson(location)::jsonb,'{alt}', to_jsonb(0)) as location, name_ru, name_en, country from meteo.stations

