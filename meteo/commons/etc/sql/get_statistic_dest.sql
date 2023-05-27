SELECT * FROM meteo.get_statistic_by_dest( @dt_start@, @dt_end@)
         AS ( dt date, dest jsonb, T1 text, T2 text, count bigint);
