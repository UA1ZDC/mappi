SELECT * from
meteo.find_fax (
  @id@,
  @start_dt@, 
  @end_dt@,
  @t1@,
  @t2@,
  @a1@,
  @a2@,
  @ii@,
  @cccc@,
  @yygggg@
)
AS (
 id bigint,
 t1 text,
 t2 text,
 a1 text,
 a2 text,
 cccc text,
 dt timestamp without time zone,
 ii integer,
 image_path text,
 magic integer,
 yygggg text,
 _id bigint
);
