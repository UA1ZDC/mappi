select * from meteo.appointment_message_insert( @mac@, @dt@, @from@, @to@, @text@, @files@, @parent@, @created@) as (_id bigint, name text, owner bigint, type integer, users jsonb, currentuser bigint, dtstart timestamp without time zone);

