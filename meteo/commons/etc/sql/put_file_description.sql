SELECT * FROM global.put_file_description( @tablename@, @filename@, @md5@, @size@ )
                              AS ( id BIGINT, name TEXT, md5 TEXT, size INTEGER, dup BOOLEAN ) ;
