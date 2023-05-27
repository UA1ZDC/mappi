select * from meteo.get_one_field(   
  @start_dt@,
  @end_dt@,
  @start_fdt@,
  @end_fdt@,
  @level@,
  @level_type@,
  @center@,
  @hour@,
  @model@,
  @descr@
);