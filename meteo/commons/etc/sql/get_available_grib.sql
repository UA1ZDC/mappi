select * from
meteo.get_available_grib(
  @start_dt@,
  @end_dt@,
  @level@,
  @level_type@,
  @center@,
  @hour@,
  @model@,
  @descr@,
  @step_lat@,
  @step_lon@,
  @forecast_beg@,
  @forecast_end@
);
