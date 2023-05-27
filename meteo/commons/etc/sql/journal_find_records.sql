select * from
meteo.find_journal_records(
 @filter@,
 @sort_column@,
 @sort_order@,
 @skip@,
 @limit@ )
AS (
  id bigint,
  priority integer,
  sender text,
  username text,
  file_name text,
  line_number integer,
  ip text,
  host text,
  dt timestamp without time zone,
  message text
);

