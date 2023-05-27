SELECT * from
meteo.find_forecast_records(
 @filter@,
 @sort_column@,
 @sort_order@,
 @skip@,
 @limit@ )
AS (
  id bigint,
  dt timestamp without time zone,
  dt_beg timestamp without time zone,
  dt_end timestamp without time zone,
  station text,
  level integer,
  level_type smallint,
  center bigint,
  hour integer,
  descrname text,
  code text,
  method_title text,
  quality integer,
  observ jsonb,
  field jsonb
)

