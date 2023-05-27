select * from meteo.appointment_message_status_update(@id@, @login@) as (_id bigint, name text, owner bigint, type integer, users jsonb, currentuser bigint, dtstart timestamp without time zone);
