select * from
meteo.get_available_zond( @start_dt@,
                          @data_type@,
                          @level_type@ )
                     AS ( station_type SMALLINT,
                          station TEXT,
                          location jsonb,
                          station_info jsonb,
                          count BIGINT );
      
                                                      
 
