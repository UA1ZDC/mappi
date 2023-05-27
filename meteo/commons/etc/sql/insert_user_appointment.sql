select * from meteo.insert_user_appointments(@name@, @owner@, @type@) as (ok real, n real, "nModified" real, upserted jsonb) ;
