select * from
meteo.get_available_data_obanal(   
  @start_dt@,
  @end_dt@,
  @start_fdt@,
  @end_fdt@,
  @level@,
  @level_type@,
  @center@,
  @hour@,
  @model@,
  @descr@,
  @net_type@
);