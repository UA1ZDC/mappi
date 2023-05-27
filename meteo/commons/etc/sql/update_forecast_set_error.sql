UPDATE meteo.forecast SET error = TRUE WHERE id = ANY(ARRAY[@id@])
