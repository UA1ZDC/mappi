SELECT * FROM meteo.forecast_opr WHERE
       (@center@    IS NULL OR center = @center@) AND
       (@punkt_id@  IS NULL OR punkt_id = @punkt_id@) AND
       (@descrname@ IS NULL OR descrname = @descrname@) AND
       (@level@     IS NULL OR level = @level@) AND
       (@type_level@   IS NULL OR type_level = @type_level@) AND
       (@ftype_method@ IS NULL OR ftype_method = @ftype_method@)
