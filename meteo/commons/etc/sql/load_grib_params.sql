SELECT 
    grib1 as grib1_parameter,
    grib2 as parameter,
    grb2discipline as discipline,
    grb2category as category,
    fullname as name,
    unit as unit,
    bufr_descr as bufr_descr,
    unit_en as unit_en
FROM meteo.parameters;
