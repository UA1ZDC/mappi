SELECT * FROM meteo.pretreatment_insert(
    @session@,
    @instrument_id@,
    @channel_number@,
    @date_start@,
    @date_end@,
    @calibration@,
    @fpath@
)
