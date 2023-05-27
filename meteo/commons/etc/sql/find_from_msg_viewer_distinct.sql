select distinct on (globalfile_id) * from (	select * from
meteo.find_from_msg_viewer(
 @filter@,
 @sort_column@,
 @sort_order@,
 @skip@,
 @limit@ )
AS (
  id bigint,
  t1 text,
  t2 text,
  a1 text,
  a2 text,
  ii integer,
  cccc text,
  dddd text,
  bbb text,
  yygggg text,
  dt timestamp,
  msg_dt timestamp,
  md5 text,
  decoded boolean,
  external boolean,
  prior integer,
  bin boolean,
  format text,
  type text,
  size integer,
  file_id bigint,
  globalfile_id integer,
  addr_tlg boolean,
  confirm_tlg boolean,
  route jsonb,
  dup integer,
  cor_number integer,
  confirm boolean,
  _id bigint
) ) as foo;

