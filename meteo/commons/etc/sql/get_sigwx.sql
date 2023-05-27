SELECT * FROM meteo.sigwx WHERE 
                    ( @dt@ IS NULL OR dt = @dt@ )
                AND ( @start_dt@ IS NULL OR dt_beg >= @start_dt@ )
                AND ( @end_dt@ IS NULL OR dt_end <= @end_dt@ )
                AND ( @center@ IS NULL OR center = @center@ )
                AND ( @level_lo@ IS NULL OR level_lo = @level_lo@ )
                AND ( @level_hi@ IS NULL OR level_hi = @level_hi@ );
