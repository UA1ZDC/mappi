SELECT *
FROM meteo.find_image_satelite_by_dt( @start_dt@, @end_dt@)
AS
( 
  _id bigint, 
  t1 text,
  t2 text,
  a1 text,
  a2 text,
  ii int,
  cccc text,
  yygggg text,
  dt timestamp without time zone,
  dt_write timestamp without time zone,
  fileid bigint
);
