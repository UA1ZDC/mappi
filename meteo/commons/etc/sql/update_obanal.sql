SELECT meteo.upsert_obanal( @dt@, 
                          @descr@,
                          @hour@,
                          @model@,
                          @level@,
                          @level_type@,
                          @center@,
                          @net_type@,
                          @forecast_start@, 
                          @forecast_end@, 
                          @fs_id@,
                          @count_points@, 
                          @time_range@,
                          @dates@,
                          @dt_write@) as ok;
