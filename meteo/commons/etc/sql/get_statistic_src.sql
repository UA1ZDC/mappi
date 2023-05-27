SELECT * FROM meteo.get_statistic_by_src( @dt_start@, @dt_end@)
         AS ( dt date, src jsonb, T1 text, T2 text, count bigint);
