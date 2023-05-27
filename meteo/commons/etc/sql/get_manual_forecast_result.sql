SELECT * from meteo.get_manual_forecast_result(
  @id@,
  @stations@,
  @stations_types@,
  @region_id@,
  @user_id@,
  @dt@,
  @dt_start@,
  @dt_end@,
  @forecast_hour@,
  @from_dt@,
  @to_dt@,
  @only_last@
) ;

