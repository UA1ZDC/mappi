SELECT * FROM meteo.insert_tlg( 
  @id@, @t1@, @t2@, @a1@, @a2@, @ii@, @cccc@, @dddd@, @bbb@, @yygggg@, @dt@, @msg_dt@, @md5@, @external@,
  @prior@, @bin@, @format@, @type@, @size@, @file_id@, @globalfile_id@, @addr_tlg@, @confirm_tlg@,
  jsonb_build_array( jsonb_build_object( 'src', @src@, 'dst', @dst@ ) ), @cor_number@
) AS ( id BIGINT, dup INTEGER );
