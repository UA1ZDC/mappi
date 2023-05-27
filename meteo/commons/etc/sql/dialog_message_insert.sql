select * from meteo.dialog_message_insert(
             @mac@,
             @dt@,
             @from@ ,
             @to@,
             @text@,
             @files@,
             @status@,
             @urls@,
             @system@ ) as (ok real, n integer) ;
