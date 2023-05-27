SELECT * FROM meteo.get_available_documents(
  @start_dt@,
  @end_dt@,
  @mapid@,
  @jobid@,
  @center@,
  @model@,
  @format@,
  @hour@,
  @map_title@,
  @job_title@,
  @limit@
)
AS (
  _id bigint,
  center integer,
  dt timestamp without time zone,
  format text,
  hour integer,
  job_title text,
  jobid text,
  map_title text,
  mapid text,
  model integer,
  path text,
  idfile bigint,
  dt_write timestamp without time zone
);
