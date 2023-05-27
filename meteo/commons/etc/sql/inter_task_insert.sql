select * from meteo.inter_task_insert(
       @name@,
       @text@,
       @dtStart@,
       @dtEnd@,
       @labor@,        
       @basics@,
       @owner@,
       @parent@,
       @idfile@,
       @control@,
       @stage@
       ) as (ok real, n integer, value jsonb, "lastErrorObject" jsonb) ;
