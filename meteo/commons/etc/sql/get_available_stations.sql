select * from
meteo.get_available_stations(@date_start@,
                      @date_end@,
                      @data_type@)
                 AS ( station_type smallint, stations jsonb );
      
                                                      
 
