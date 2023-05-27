SELECT * FROM meteo.get_available_sigwx( @date_start@, @date_end@ )
                             AS ( "_id.dt" timestamp without time zone,
                                  "_id.dt_beg" timestamp without time zone,
                                  "_id.dt_end" timestamp without time zone,
                                  "_id.center" smallint,
                                  "_id.level_lo" integer, 
                                  "_id.level_hi" integer,
                                  "count" bigint, 
                                  "fileid" jsonb );
