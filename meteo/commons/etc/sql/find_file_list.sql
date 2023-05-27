select * from
global.find_file_list( @tablename@, @filename@,
                             @id@ )
AS
(
  id bigint,
  filename text,
  filemd5 text,
  filesize int,
  dt_write timestamp without time zone
);
