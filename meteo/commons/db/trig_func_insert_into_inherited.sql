CREATE OR REPLACE FUNCTION insert_into_inherited() RETURNS trigger AS
$BODY$DECLARE
    table_master    TEXT       = TG_RELNAME;
    table_part      TEXT       = '';		
BEGIN
        -- Даём имя партиции --------------------------------------------------
    table_part := table_master  || '_' || date_part( 'year', NEW.dt )::TEXT
                                || '_' || date_part( 'month', NEW.dt )::TEXT
                                || '_' || date_part( 'day', NEW.dt )::TEXT;
 
        -- Проверяем партицию на существование --------------------------------
    PERFORM 1 FROM pg_class WHERE relname = table_part LIMIT 1;
 
        -- Если её ещё нет, то создаём ----------------------------------------
    IF NOT FOUND
    THEN
        -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
      EXECUTE  'CREATE TABLE ' || table_part || '
      (
        CONSTRAINT ' || table_part || '_created_check CHECK (dt BETWEEN ''' || date_trunc('day', NEW.dt) ||
                                                             ''' AND ''' || date_trunc('day', NEW.dt) + interval '1 day' - interval '1 sec' || ''')
      )
      INHERITS (' || table_master || ')
      WITH (OIDS = FALSE)';

        -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
      FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv FROM information_schema.role_table_grants
                           WHERE table_name = ''' || table_master || ''' GROUP BY grantee' LOOP
        EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part || ' TO ' || rec_.grantee;
      END LOOP;

        -- Создаём индексы для текущей партиции -------------------------------
      PERFORM 1 FROM information_schema.columns WHERE table_name = table_master AND column_name = 'dt';
      IF FOUND
      THEN
        EXECUTE '
            CREATE INDEX ' || table_part || '_dt_index
            ON ' || table_part || '
            USING btree
            (dt)';
      END IF;

      PERFORM 1 FROM information_schema.columns WHERE table_name = table_master AND column_name = 'id';
      IF FOUND
      THEN
        EXECUTE '
            CREATE INDEX ' || table_part || '_id_index
            ON ' || table_part || '
            USING btree
            (id)';

        -- Создаём первичный ключ для текущей партиции ------------------------
        EXECUTE '
            ALTER TABLE ' || table_part || '
            ADD CONSTRAINT pkey_' || table_part || ' PRIMARY KEY (id)';

      END IF;

    END IF;
 
        -- Вставляем данные в партицию ----------------------------------------
    EXECUTE '
        INSERT INTO ' || table_part || ' 
            SELECT ((' || quote_literal(NEW) || ')::' || table_master || ').*';
 
    RETURN NULL;

END;$BODY$
LANGUAGE plpgsql VOLATILE NOT LEAKPROOF
COST 100;
