SELECT * from meteo.get_available_hours_obanal(
  @dt@,
  @center@,
  @model@,
  @level@,
  @level_type@,
  @descr@,
  @min_level_count@
) as ( dt timestamp without time zone, center integer, hour integer, level integer[] );
