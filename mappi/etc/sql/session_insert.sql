SELECT * FROM meteo.session_insert(
    @satellite@,
    @date_start@,
    @date_end@,
    @date_start_offset@,
    @revol@,
    @direction@,
    @elevat_max@,
    @site@,
    @fpath@,
    @tle@
)
