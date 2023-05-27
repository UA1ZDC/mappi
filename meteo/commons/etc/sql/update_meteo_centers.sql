UPDATE meteo.centers
SET 
    priority            = @priority@,
    priority_value      = @priority_value@,
    forecast_count      = @forecast_count@,
    forecast_true_count = @forecast_true_count@
WHERE 
    ( @center@    IS NULL or center    = @center@     );