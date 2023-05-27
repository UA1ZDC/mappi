select * from meteo.conference_message_status_update(
    @id@,
    @login@ ) 
as (id_ bigint, mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, delivery_ jsonb, unread_ jsonb, total_ integer, files_ jsonb, parent_ text) ;
