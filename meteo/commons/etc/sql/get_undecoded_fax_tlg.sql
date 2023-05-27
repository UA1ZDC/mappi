SELECT * FROM meteo.get_undecoded_fax_tlg( @type@,@start_dt@, @end_dt@, @limit@ )
AS (         _id bigint,
              t1 text,
              t2 text,
              a1 text,
              a2 text,
              ii integer,
              cccc text,
              dddd text,
              yygggg text,
              external boolean,
              file_id bigint,
              bbb text
       );
