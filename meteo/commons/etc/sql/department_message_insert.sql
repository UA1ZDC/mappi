select * from meteo.department_message_insert(@mac@, @dt@, @from@, @to@, @text@, @files@, @parent@, @user@) 
as (_id bigint, mac integer, dt timestamp without time zone, text text, from_user text, "from" text, "to" text, read_by text, files jsonb, parent text, delivered_to text);
