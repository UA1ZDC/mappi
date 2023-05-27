SELECT * from meteo.conference_message_insert(@mac@, @dt@, @from@, @to@, @text@, @delivery@,
                                                    @unread@, @total@, @files@, @parent@) as (ok real, n integer);

