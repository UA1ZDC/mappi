SELECT meteo.insert_manual_forecast(
    @id@,
    @region_id@,
    @user_id@,
    @dt@,
    @dt_start@,
    @dt_end@,
    @forecast_hour@,
    @forecast_code@,
    @forecast_text@,
    @forecast_params@,
    @station_id@
) as last_id
