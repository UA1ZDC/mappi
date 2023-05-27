SELECT insert_radar as "ok" FROM meteo.insert_radar(
  @collection@,
  @dt_write@,
  @dt@,
  @station@,
  @h1@,
  @h2@,
  @product@,
  @count@,
  @fileid@,
  @tlgid@,
  @data_type@,
  @source@,
  @ii@
);
