CREATE OR REPLACE FUNCTION drop_inherited(_table_master text, _datetime timestamp without time zone) RETURNS integer AS
$BODY$DECLARE
  rec_        RECORD;
  ok_         BOOLEAN  = FALSE;
  drop_count_ INTEGER  = 0;

    --- NB: Наследуемые таблицы должны создаваться триггерной функцией insert_into_inherited(),
    ---     которая обеспечивает для них название _table_master_yyyy_mm_dd

BEGIN
    --- Находим все таблицы, наследуемые от table_master:
  FOR rec_ IN EXECUTE 'SELECT c.relname FROM pg_inherits 
                           JOIN pg_class c ON inhrelid = c.oid
                           JOIN pg_class p ON inhparent = p.oid WHERE p.relname=''' || _table_master || ''''
  LOOP
    --- Для каждой записи выделяем дату из названия таблицы и сравниваем в заданной:
    SELECT INTO ok_ (reverse(split_part(reverse(rec_.relname), '_', 3)) || '-' ||
                     reverse(split_part(reverse(rec_.relname), '_', 2)) || '-' ||
                     reverse(split_part(reverse(rec_.relname), '_', 1)))::date < _datetime;
    
    --- Если дата таблицы меньше заданной, удаляем таблицу:
    IF ok_ = TRUE
    THEN
      EXECUTE 'DROP TABLE ' || rec_.relname || ' CASCADE';
      drop_count_ = drop_count_ + 1;
    END IF;
  END LOOP;

  RETURN drop_count_;
END;$BODY$
LANGUAGE plpgsql VOLATILE NOT LEAKPROOF
COST 100;
