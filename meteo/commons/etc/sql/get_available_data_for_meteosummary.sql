SELECT * from meteo.get_available_data_for_meteosummary(
  @dt@,
  @fdt@,
  ARRAY[1]::INTEGER[],
  ARRAY[20012,20012,20012,20003,
         20010,20013,20001,13055,11001,11002,12101,13003,10004,
         13022,12103,7700001,7700002,7700003]::INTEGER[]
);
