SELECT * FROM meteo.get_available_documents(
  NULL,
  NULL,
  NULL,
  @jobid@,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
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
