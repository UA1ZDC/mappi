SELECT * FROM meteo.processes
WHERE 
    ( @process_type@   IS NULL or process_type   = @process_type@  ) AND
    ( @is_finished@    IS NULL or is_finished    = @is_finished@   ) ;