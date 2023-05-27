SELECT * from
meteo.find_forecast_accuracy_records(
 @filter@,
 @sort_column@,
 @sort_order@,
 @skip@,
 @limit@ )
AS (
  id bigint,
  punkt_id text,
  type_level integer,
  level integer,
  descrname text,
  center integer,
  hour integer,
  ftype_method text,
  opr integer,
  not_opr integer,
  total integer
)
