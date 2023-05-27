INSERT INTO meteo.mlmodel(
    "descr",
    "net_type",
    "hour",
    "model",
    "level_type",
    "level",
    "center",
    "fs_id",
    "mlmodel_type",
    "dt_write",
    "location"
 )
VALUES ( 
    @descr@,
    @net_type@,
    @hour@,
    @model@,
    @level_type@,
    @level@,
    @center@,
    @fs_id@,
    @mlmodel_type@,
    @dt_write@,
    @location@
)
ON CONFLICT ON CONSTRAINT mlmodel_uniq_data DO 
UPDATE SET  descr        = @descr@,
            net_type     = @net_type@,
            hour         = @hour@,
            model        = @model@,
            level_type   = @level_type@,
            level        = @level@,
            center       = @center@,
            fs_id        = @fs_id@,
            mlmodel_type = @mlmodel_type@,
            location     = @location@ ;
