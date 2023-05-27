UPDATE meteo.processes
SET 
    file_name      = @file_name@,
    number_of_exec = @number_of_exec@,
    progress       = @progress@,
    is_finished    = @is_finished@,
    error          = @error@,
    last_val       = @last_val@,
    process_type   = @process_type@,
    stations       = @stations@,
    stationspdc    = @stationspdc@,
    step           = @step@,
    step_percent   = @step_percent@,
    hour_step      = @hour_step@,
    params         = @params@,
    paramspdc      = @paramspdc@
WHERE 
    ( @file_name@    IS NULL or file_name    = @file_name@     ) AND
    ( @is_finished@  IS NULL or is_finished  = @is_finished@   ) AND
    ( @process_type@ IS NULL or process_type = @process_type@  );