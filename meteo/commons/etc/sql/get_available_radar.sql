SELECT * FROM meteo.get_available_radar(
  @data_type@, 
  @h1@, 
  @h2@, 
  @product@, 
  @start_dt@, 
  @end_dt@        
)
AS (
    id integer,
    dt_write timestamp without time zone,
    dt timestamp without time zone,
    station text,
    h1 real,
    h2 real,
    product integer,
    count integer,
    fileid bigint,
    tlgid text,
    data_type integer,
    source text,
    ii integer
);
