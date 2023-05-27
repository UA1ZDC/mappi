SELECT update_document as "ok" FROM meteo.update_document(
  @mapid@,
  @jobid@,
  @dt@,
  @hour@,
  @center@,
  @model@,
  @format@,
  @map_title@,
  @job_title@,
  @path@,
  @idfile@,
  @dt_write@
);
