--
-- PostgreSQL database dump
--

-- Dumped from database version 13.6 (Debian 13.6-1.pgdg110+1)
-- Dumped by pg_dump version 14.2 (Debian 14.2-1.pgdg110+1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: global; Type: SCHEMA; Schema: -; Owner: postgres
--

CREATE SCHEMA global;


ALTER SCHEMA global OWNER TO postgres;

--
-- Name: check_fs_table(text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.check_fs_table(tablename_ text) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
DECLARE
  query_ TEXT;
BEGIN
  PERFORM 1 FROM pg_tables WHERE tablename = tablename_ LIMIT 1;
  IF NOT FOUND
  THEN
    query_ := format(
	  'CREATE TABLE meteo.%s (
		id bigint NOT NULL,
		filename text NOT NULL,
		filemd5 text NOT NULL,
		filesize integer NOT NULL,
		dt_write timestamp without time zone DEFAULT  timezone(''utc''::text, now()) NOT NULL
	   );
		CREATE SEQUENCE meteo.%s_id_seq
                START WITH 1
                INCREMENT BY 1
                NO MINVALUE
                NO MAXVALUE
                CACHE 1;
        ALTER SEQUENCE meteo.%s_id_seq OWNED BY meteo.%s.id;
        ALTER TABLE ONLY meteo.%s ALTER COLUMN id SET DEFAULT nextval(''meteo.%s_id_seq''::regclass);
        ALTER TABLE ONLY meteo.%s
               ADD CONSTRAINT %s_filemd5_uniq UNIQUE (filemd5);
        ALTER TABLE ONLY meteo.%s
               ADD CONSTRAINT %s_pkey PRIMARY KEY (id);
		CREATE UNIQUE INDEX %s_filemd5_idx
          ON meteo.%s USING btree
          (filemd5 COLLATE pg_catalog."default" ASC NULLS LAST)
          TABLESPACE pg_default;

        CREATE UNIQUE INDEX %s_id_idx
          ON meteo.%s USING btree
          (id ASC NULLS LAST)
          TABLESPACE pg_default;
	  ', tablename_,     	  tablename_, tablename_, 
							  tablename_, tablename_,
							  tablename_, tablename_,
							  tablename_, tablename_,
							  tablename_, tablename_,
							  tablename_, tablename_, 
							  tablename_, tablename_
	);
	EXECUTE query_;
  END IF;
  RETURN true;
END;
$$;


ALTER FUNCTION global.check_fs_table(tablename_ text) OWNER TO postgres;

--
-- Name: cleaner_remove_task_get_documents(text, text, timestamp without time zone, text, timestamp without time zone, text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.cleaner_remove_task_get_documents(collection_ text, dtfield_ text, enddt_ timestamp without time zone, dtwritefield text, storedt timestamp without time zone, gridfield text) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$
 DECLARE
   ref_ refcursor;
  mviews RECORD;
BEGIN   
  SELECT * FROM global.cleaner_remove_task_get_documents_curs( collection_, dtfield_, enddt_, dtWriteField, storeDt, gridField) into ref_;
  LOOP
    FETCH NEXT FROM ref_ INTO mviews;
    IF NOT FOUND THEN EXIT;END IF;  
    return  next mviews;
  END LOOP;
END 
$$;


ALTER FUNCTION global.cleaner_remove_task_get_documents(collection_ text, dtfield_ text, enddt_ timestamp without time zone, dtwritefield text, storedt timestamp without time zone, gridfield text) OWNER TO postgres;

--
-- Name: cleaner_remove_task_get_documents_curs(text, text, timestamp without time zone, text, timestamp without time zone, text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.cleaner_remove_task_get_documents_curs(collection_ text, dtfield_ text, enddt_ timestamp without time zone, dtwritefield text, storedt timestamp without time zone, gridfield text) RETURNS refcursor
    LANGUAGE plpgsql
    AS $$
 DECLARE
   tableName_ text;
   query_ text;
   ref refcursor := 'rescursor';
BEGIN   
  tableName_ := 'meteo.'||collection_;
  
  query_ := FORMAT( 'SELECT id, %s::text FROM %s WHERE %s < %L AND %s < %L ', 
                    gridField, tableName_, dtfield_, enddt_, dtWriteField, storeDt );
  raise notice '%s', query_;
  
  OPEN ref FOR EXECUTE query_;
  return ref;

END 
$$;


ALTER FUNCTION global.cleaner_remove_task_get_documents_curs(collection_ text, dtfield_ text, enddt_ timestamp without time zone, dtwritefield text, storedt timestamp without time zone, gridfield text) OWNER TO postgres;

--
-- Name: delete_data_expired(text, text, text, timestamp without time zone, text, timestamp without time zone, text, text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.delete_data_expired(msg_table text, msg_fs_field text, dt_field text, dt timestamp without time zone, dt_write_field text, dt_write timestamp without time zone, fs_table text, fs_field text) RETURNS record
    LANGUAGE plpgsql
    AS $_$
DECLARE
    msg            TEXT;
    msg_part_count INTEGER := 0;
    msg_rec_count  BIGINT := 0;

    fs             TEXT;
    fs_part_count  INTEGER := 0;
    fs_rec_count   BIGINT := 0;
	fs_key_field   TEXT;

    fs_table_avail TEXT;
    ts_interval    TEXT;
    query          TEXT;
BEGIN
    msg = 'meteo.' || msg_table;
	ts_interval := FORMAT('%s < %L', dt_field, dt);
   	IF dt_write_field IS NOT NULL
   	THEN
	    ts_interval := FORMAT('%s < %L AND %s < %L', dt_field, dt, dt_write_field, dt_write);
   	END IF;

	IF fs_table IS NOT NULL
    THEN
		fs = 'fs_' || fs_table;
		SELECT relname INTO fs_table_avail FROM pg_class WHERE relname=fs;
		IF fs_table_avail = fs
		THEN
			fs_key_field := 'id';
			IF fs_field IS NOT NULL
			THEN
				fs_key_field := fs_field;
			END IF;
		
			fs = 'meteo.' || fs;
	        query := FORMAT('DELETE FROM %s WHERE %s = ANY (SELECT %s FROM %s WHERE %s)',
				fs, fs_key_field, msg_fs_field, msg, ts_interval);
			RAISE NOTICE '%', query;
	        EXECUTE query;

    	    GET DIAGNOSTICS fs_rec_count = ROW_COUNT;
	        RAISE NOTICE 'table: %, delete records: %', fs, fs_rec_count;
	        IF 0 < fs_rec_count
	        THEN
	            EXECUTE FORMAT('SELECT * FROM global.drop_table_partition($1)') INTO fs_part_count USING fs_table;
    	        RAISE NOTICE 'table: %, drop partition: %', fs, fs_part_count;
	        END IF;
		END IF;
    END IF;

    query := FORMAT('DELETE FROM %s WHERE %s', msg, ts_interval);
	RAISE NOTICE '%', query;
    EXECUTE query;

    GET DIAGNOSTICS msg_rec_count = ROW_COUNT;
    RAISE NOTICE 'table: %, delete records: %', msg, msg_rec_count;
    IF 0 < msg_rec_count
    THEN
        EXECUTE FORMAT('SELECT * FROM global.drop_table_partition($1)') INTO msg_part_count USING msg_table;
        RAISE NOTICE 'table: %, drop partition: %', msg, msg_part_count;
    END IF;

    RETURN (msg_part_count, msg_rec_count, fs_part_count, fs_rec_count);
END;
$_$;


ALTER FUNCTION global.delete_data_expired(msg_table text, msg_fs_field text, dt_field text, dt timestamp without time zone, dt_write_field text, dt_write timestamp without time zone, fs_table text, fs_field text) OWNER TO postgres;

--
-- Name: drop_table_partition(text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.drop_table_partition(table_name text) RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE
	dst       TEXT;
    item      TEXT;
    chunks    TEXT[];
    count_rec INTEGER;
	res       INTEGER := 0;
BEGIN
	dst := 'meteo.' || table_name;
	-- список партиций
    chunks := ARRAY(
        SELECT ns.nspname || '.' || c.relname AS child
            FROM pg_inherits
            JOIN pg_class AS c ON (inhrelid=c.oid)
            JOIN pg_class AS p ON (inhparent=p.oid)
            JOIN pg_catalog.pg_namespace AS ns ON c.relnamespace = ns.oid
            JOIN pg_catalog.pg_namespace AS ps ON p.relnamespace = ps.oid
            WHERE ps.nspname || '.' || p.relname = dst ORDER BY child
    );

    IF 0 < ARRAY_LENGTH(chunks, 1)
    THEN
        FOREACH item IN ARRAY chunks
        LOOP
		    EXECUTE FORMAT('SELECT count(*) FROM %s', item) INTO count_rec;
			IF count_rec = 0
			THEN
				EXECUTE FORMAT('DROP TABLE %s', item);
				res := res + 1;
			END IF;
        END LOOP;
    ELSE
        RAISE NOTICE 'ОШИБКА - удаления партиции таблицы %', table_name;
        RETURN res;
    END IF;

    RETURN res;
END;
$$;


ALTER FUNCTION global.drop_table_partition(table_name text) OWNER TO postgres;

--
-- Name: drop_table_partition_not_empty(text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.drop_table_partition_not_empty(table_name text) RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE
    dst       TEXT;
    item      TEXT;
    chunks    TEXT[];
    count_rec INTEGER;
    res       INTEGER := 0;
BEGIN
    dst := 'meteo.' || table_name;
    -- список партиций
    chunks := ARRAY(
        SELECT ns.nspname || '.' || c.relname AS child
            FROM pg_inherits
            JOIN pg_class AS c ON (inhrelid=c.oid)
            JOIN pg_class AS p ON (inhparent=p.oid)
            JOIN pg_catalog.pg_namespace AS ns ON c.relnamespace = ns.oid
            JOIN pg_catalog.pg_namespace AS ps ON p.relnamespace = ps.oid
            WHERE ps.nspname || '.' || p.relname = dst ORDER BY child
    );
    raise notice '%',chunks;
    IF 0 < ARRAY_LENGTH(chunks, 1)
    THEN
        FOREACH item IN ARRAY chunks
        LOOP
            EXECUTE FORMAT('DROP TABLE %s', item);
            res := res + 1;
        END LOOP;
    END IF;

    RETURN res;
END;
$$;


ALTER FUNCTION global.drop_table_partition_not_empty(table_name text) OWNER TO postgres;

--
-- Name: f_create_index(character varying, character varying[], character varying, character varying); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.f_create_index(character varying, character varying[], character varying DEFAULT 'btree'::character varying, character varying DEFAULT NULL::character varying) RETURNS integer
    LANGUAGE plpgsql
    AS $_$
declare
    tableName alias for $1;
    columnNames alias for $2;
    indexMethod alias for $3;
    indexName alias for $4;

    q varchar;
    sColumns varchar;
begin
    if(tableName is null or trim(tableName) = '') then
        return -1;
    end if;
    
    if(ColumnNames is null or array_upper(columnNames, 1) = 0) then
        return -1;
    end if;

    sColumns = array_to_string(columnNames, ',');

    q = 'create index ';
    if(indexName is not null) then
        q = q || indexName || ' ';
    end if;

    q = q || 'on ' || tableName;

    if(indexMethod is not null) then
        q = q || ' using ' || indexMethod;
    end if;

    q = q || ' (' || sColumns || ')';

    execute q;

    return 1;
    
end
$_$;


ALTER FUNCTION global.f_create_index(character varying, character varying[], character varying, character varying) OWNER TO postgres;

--
-- Name: f_create_unique_constraint(text, text[]); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.f_create_unique_constraint(tbname text, fields text[]) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
DECLARE
  constraintName text;  
begin
  constraintName := 'ct_' || tbname || '_' || array_to_string(fields, '_');
  constraintName := replace(constraintName, '.', '_');
  constraintName := substring(constraintName from 1 for 63);
  
  
  EXECUTE FORMAT('ALTER TABLE %s ADD CONSTRAINT %s UNIQUE( %s )',
				 tbname, constraintName, array_to_string(fields, ','));
  return True;
end
$$;


ALTER FUNCTION global.f_create_unique_constraint(tbname text, fields text[]) OWNER TO postgres;

--
-- Name: f_is_constraint_exist(text, text[]); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.f_is_constraint_exist(tbname text, fields text[]) RETURNS boolean
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  constraintName text;
  iCount int4;
begin
  constraintName := 'ct_' || tbname || '_' || array_to_string(fields, '_');
  constraintName := replace(constraintName, '.', '_');
  constraintName := substring(constraintName from 1 for 63);
  
  SELECT COUNT(conname) INTO iCount FROM pg_catalog.pg_constraint con
  	INNER JOIN pg_catalog.pg_class rel
  	ON rel.oid = con.conrelid
  	INNER JOIN pg_catalog.pg_namespace nsp
  	ON nsp.oid = connamespace
  WHERE nsp.nspname||'.'||rel.relname = tbname AND conname = constraintName ;
  
  if( iCount = 1 ) then
  	return TRUE;
  else
  	return FALSE;
  end if;  
end
$$;


ALTER FUNCTION global.f_is_constraint_exist(tbname text, fields text[]) OWNER TO postgres;

--
-- Name: f_is_index_exist(text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.f_is_index_exist(text) RETURNS boolean
    LANGUAGE plpgsql STABLE
    AS $_$
declare
    iName alias for $1;
    iCount int4;
begin

    select count(pgi.indexname)  into iCount
    from
        pg_indexes pgi
    where
        pgi.schemaname||'.'||pgi.indexname = iName;

    if(iCount >= 1) then
        return TRUE;
    else
        return FALSE;
    end if;

    return FALSE;
    
    exception when others then
        return FALSE;

end
$_$;


ALTER FUNCTION global.f_is_index_exist(text) OWNER TO postgres;

--
-- Name: f_is_index_exist(text, text[]); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.f_is_index_exist(tbname text, fields text[]) RETURNS boolean
    LANGUAGE plpgsql STABLE
    AS $$
begin
  return global.f_is_index_exist(tbname || '_' || array_to_string(fields, '_') || '_idx' );
end
$$;


ALTER FUNCTION global.f_is_index_exist(tbname text, fields text[]) OWNER TO postgres;

--
-- Name: find_file_description(text, text, bigint); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.find_file_description(tablename_ text, filename_ text, id_ bigint) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
 query_ TEXT;
BEGIN
  PERFORM global.check_fs_table(tablename_);
  query_ = FORMAT('SELECT id, filename, filemd5, filesize, dt_write
    FROM meteo.%I WHERE filename = %L OR id = %L', tablename_, filename_, id_ );
	RETURN QUERY EXECUTE query_;
END;
$$;


ALTER FUNCTION global.find_file_description(tablename_ text, filename_ text, id_ bigint) OWNER TO postgres;

--
-- Name: find_file_list(text, text, bigint); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.find_file_list(tablename_ text, filename_ text, id_ bigint) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
 query_ TEXT;
BEGIN
  PERFORM global.check_fs_table(tablename_);
  query_ = FORMAT('SELECT id, filename, filemd5, filesize, dt_write
    FROM meteo.%I WHERE (%L IS NULL OR filename = %L) AND (%L IS NULL OR id = %L)', tablename_, filename_, filename_, id_, id_ );
	RETURN QUERY EXECUTE query_;
END;
$$;


ALTER FUNCTION global.find_file_list(tablename_ text, filename_ text, id_ bigint) OWNER TO postgres;

--
-- Name: parse_field_condition_object(jsonb, text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.parse_field_condition_object(filters jsonb, fieldname text) RETURNS text
    LANGUAGE plpgsql STABLE
    AS $_$ 
 DECLARE
   fieldConditions text[];
   fieldCondition text;
   fieldValue jsonb;
   fieldValueString text;
   resultValue text;
   fieldType text;
   
   conditionKey text[];
   caseSensetivity text;
   regex text;
 BEGIN  
  conditionKey := ARRAY(SELECT jsonb_object_keys(filters));  
  
  if 0 = array_length(conditionKey, 1)
  THEN
   raise notice 'NULL!';
    return NULL;
  END IF;
  
  IF conditionKey[1] = '$regex' AND conditionKey[2] = '$options'
  THEN  
    caseSensetivity := jsonb_extract_path_text(filters, '$options');
	regex := jsonb_extract_path_text(filters, '$regex');
    raise notice 'VALUE REGEX %', regex;

	if 'i' = caseSensetivity
	THEN
	  return format('( "%s" ~* ''%s'' )', fieldName, regex);
	ELSE
	  return format('( "%s" ~ ''%s'' )', fieldName, regex);
	END IF;	
  END IF;

  FOR fieldCondition IN SELECT jsonb_object_keys(filters)
  LOOP
    fieldValue:= jsonb_extract_path(filters, fieldCondition);
	fieldType := jsonb_typeof(fieldValue);
	IF fieldType = 'object'::text
	THEN
	    fieldValueString := global.parse_field_condition_value_object(fieldValue);	
	ELSE IF fieldType = 'array'::text
	THEN
		fieldValueString := global.parse_field_condition_value_array(fieldValue);	
	ELSE
	    fieldValueString := fieldValue;
    END IF;
	END IF;
	
	
	IF fieldName = '_id'::text
	THEN
	  fieldName = 'id';
	END IF;
	
	if fieldCondition = '$lte'::text
	THEN
		fieldConditions := fieldConditions || format('( "%s" <= %s )', fieldName, fieldValueString);
	END IF;
	if fieldCondition = '$gte'::text
	THEN
		fieldConditions := fieldConditions || format(' ( "%s" >= %s ) ', fieldName, fieldValueString);
	END IF;
	if fieldCondition = '$lt'::text
	THEN
		fieldConditions := fieldConditions || format(' ( "%s" < %s ) ', fieldName, fieldValueString);
	END IF;
	if fieldCondition = '$gt'::text
	THEN
		fieldConditions := fieldConditions || format(' ( "%s" > %s ) ', fieldName, fieldValueString);
	END IF;
	if fieldCondition = '$eq'::text
	THEN
		fieldConditions := fieldConditions || format(' ( "%s" = %s ) ', fieldName, fieldValueString);
	END IF;
	if fieldCondition = '$in'::text
	THEN
	  fieldConditions := fieldConditions || format(' ( "%s" IN ( %s ) ) ', fieldName, fieldValueString);
	END IF;
	if fieldCondition = '$ne'::text
	THEN
		fieldConditions := fieldConditions || format(' ( "%s" <> %s ) ', fieldName, fieldValueString);
	END IF;	
  END LOOP;
  
  raise notice 'COND THIS %', fieldConditions;

  resultValue := format('( %s )', array_to_string( fieldConditions, ' AND ' ));
  return resultValue;
 END;
$_$;


ALTER FUNCTION global.parse_field_condition_object(filters jsonb, fieldname text) OWNER TO postgres;

--
-- Name: parse_field_condition_value_array(jsonb); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.parse_field_condition_value_array(obj jsonb) RETURNS text
    LANGUAGE plpgsql STABLE
    AS $$ 
  DECLARE
    arrayValues text[];
	resultValue text;
	
	sValue jsonb;
	sType text;
	sValueText text;
	
	
  BEGIN
  	FOR sValue IN SELECT jsonb_array_elements(obj)
	LOOP
	  sType := jsonb_typeof(sValue);
	END LOOP;
	
	for sValueText IN SELECT jsonb_array_elements_text(obj)
	LOOP
	  IF sType = 'string'
	  THEN	    
	    arrayValues := arrayValues || format(' ''%s'' ', sValueText );
	  ELSE
	    arrayValues := arrayValues || format(' %s ', sValueText);
	  END IF;
	END LOOP;
	resultValue := array_to_string( arrayValues, ',' );
	return resultValue;
  END;
$$;


ALTER FUNCTION global.parse_field_condition_value_array(obj jsonb) OWNER TO postgres;

--
-- Name: parse_field_condition_value_object(jsonb); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.parse_field_condition_value_object(obj jsonb) RETURNS text
    LANGUAGE plpgsql STABLE
    AS $_$ 
  DECLARE
    typeKey text[];
	resultValue text;
	
	dtValue text;	
  BEGIN
  	
    typeKey := ARRAY(SELECT jsonb_object_keys(obj));
	
	IF '$date' = typeKey[1]
	THEN	  	     
		dtValue:= jsonb_extract_path_text(obj, '$date');
		resultValue :=  format('''%s''::TIMESTAMP WITHOUT TIME ZONE', dtValue);
	    return resultValue;
	END IF;	
	
	return NULL;
  END;
$_$;


ALTER FUNCTION global.parse_field_condition_value_object(obj jsonb) OWNER TO postgres;

--
-- Name: parse_json_condition(jsonb[]); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.parse_json_condition(filters jsonb[]) RETURNS text
    LANGUAGE plpgsql STABLE
    AS $$ 
DECLARE
  conditions text[];
 BEGIN
   FOR i IN array_lower(filters, 1) .. array_upper(filters, 1)
   LOOP
     conditions = conditions || global.parse_json_condition(filters[i]);
   END LOOP;
 
   return array_to_string( conditions, ' AND ' );
 END;
$$;


ALTER FUNCTION global.parse_json_condition(filters jsonb[]) OWNER TO postgres;

--
-- Name: parse_json_condition(jsonb); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.parse_json_condition(filters jsonb) RETURNS text
    LANGUAGE plpgsql STABLE
    AS $_$ 
 DECLARE
  conditionSqls text[];
  
  ckey text;
  
   fieldValue jsonb;
   fieldType text;
 BEGIN     
   raise notice 'parse_json_condition % %', filters, jsonb_object_keys(filters);
   FOR ckey IN SELECT jsonb_object_keys(filters)
   LOOP
     if '$and' = ckey or '$or' = ckey
	 THEN
	   conditionSqls := conditionSqls ||  global.parse_json_logical(jsonb_extract_path(filters, ckey), ckey);	  
	   continue;
     END IF;
	 
	 fieldValue:= jsonb_extract_path(filters, ckey);     
     fieldType := jsonb_typeof(fieldValue);
     IF fieldType = 'object'::text
     THEN       
       conditionSqls := conditionSqls || global.parse_field_condition_object(fieldValue, ckey);
	   continue;
	 END IF;
	 
	 IF '_id' = ckey
	 THEN
	   ckey = 'id';
	 END IF;
	 
	 conditionSqls := conditionSqls || format('"%s" = %s', ckey, fieldValue);
   END LOOP;
   
   raise notice 'parse_json_condition %', conditionSqls;

   return array_to_string( conditionSqls, ' AND ' );

 END;
$_$;


ALTER FUNCTION global.parse_json_condition(filters jsonb) OWNER TO postgres;

--
-- Name: parse_json_logical(jsonb, text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.parse_json_logical(inner_values jsonb, logical_parent text) RETURNS text
    LANGUAGE plpgsql STABLE
    AS $_$ 
 DECLARE
   conditionLogicalValues text[];
   arrayElement jsonb;
 BEGIN
   FOR arrayElement IN SELECT jsonb_array_elements(inner_values)
   LOOP
     raise notice 'ARRAY ELEM %', arrayElement;
     conditionLogicalValues := conditionLogicalValues || global.parse_json_condition(arrayElement);
   END LOOP;
   
   if '$and' = logical_parent
   THEN
     return format('( %s )', array_to_string( conditionLogicalValues, ' AND ' ));
   ELSE
     return format('( %s )', array_to_string( conditionLogicalValues, ' OR ' ));
   END IF;
   
 END;
$_$;


ALTER FUNCTION global.parse_json_logical(inner_values jsonb, logical_parent text) OWNER TO postgres;

--
-- Name: put_file_description(text, text, text, integer); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.put_file_description(tablename_ text, filename_ text, filemd5_ text, filesize_ integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
  err_context TEXT;
  idrec_ BIGINT;
  query_ TEXT;
  rec_ RECORD;
  dup_md5_ boolean;
  tablepart_ TEXT;
BEGIN
  PERFORM global.check_fs_table(tablename_);
  SELECT * FROM global.try_create_fs_part(tablename_)
    AS ( tablepart TEXT, id BIGINT ) LIMIT 1 INTO rec_;

  tablepart_ = rec_.tablepart;

--      raise notice 'part =%', rec_;
--      raise notice 'partition =%', rec_.tablepart;

  query_ = FORMAT('WITH descr AS (
          INSERT INTO meteo.%s ( id, filename, filemd5, filesize )
          VALUES ( %s, %L, %L, %L )
          ON CONFLICT (filemd5) DO NOTHING 
                                  RETURNING * )
      SELECT * FROM descr;', tablepart_, rec_.id, filename_, filemd5_, filesize_ );
  EXECUTE query_ INTO rec_;
  dup_md5_ := false;
  IF ( rec_.id IS NULL ) THEN
    query_ = FORMAT( 'SELECT * FROM meteo.%s WHERE filemd5 = %L LIMIT 1', tablepart_, filemd5_);
        EXECUTE query_ INTO rec_;
        dup_md5_ := TRUE;
  END IF;
  RETURN (rec_.id, rec_.filename, rec_.filemd5, rec_.filesize, dup_md5_);
  EXCEPTION
    WHEN OTHERS THEN
          GET STACKED DIAGNOSTICS err_context = PG_EXCEPTION_CONTEXT;
          RAISE INFO 'Error Name:%',SQLERRM;
          RAISE INFO 'Error State:%', SQLSTATE;
          RAISE INFO 'Error Context:%', err_context;
          RETURN idrec_;
END;
$$;


ALTER FUNCTION global.put_file_description(tablename_ text, filename_ text, filemd5_ text, filesize_ integer) OWNER TO postgres;

--
-- Name: remove_file_description(text, bigint); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.remove_file_description(tablename_ text, id_ bigint) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
DECLARE
  query_ TEXT;
  delcount_ INTEGER;
BEGIN
  delcount_ = 0;
  PERFORM global.check_fs_table(tablename_);
  IF id_ IS NULL THEN
    RETURN FALSE;
  END IF;
  query_ = FORMAT( 'WITH del AS (
				      DELETE FROM meteo.%I WHERE id=%L RETURNING id)
				    SELECT count(*) FROM del;', tablename_, id_ );
  EXECUTE query_ INTO delcount_;
  IF ( 0 >= delcount_ ) THEN
	RETURN FALSE;
  END IF;
  RETURN TRUE;
END;
$$;


ALTER FUNCTION global.remove_file_description(tablename_ text, id_ bigint) OWNER TO postgres;

--
-- Name: FUNCTION remove_file_description(tablename_ text, id_ bigint); Type: COMMENT; Schema: global; Owner: postgres
--

COMMENT ON FUNCTION global.remove_file_description(tablename_ text, id_ bigint) IS 'Удалить описание файла';


--
-- Name: rm_any_by_id(text, bigint); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.rm_any_by_id(collection_ text, id_ bigint) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
DECLARE
  tableName_ text;
  targetTables_ text[];
  query_ text;  
  exists_ bool;
  partition_ text;
BEGIN   
  tableName_ := 'meteo.'||collection_;
  
  --Удаляем запись из БД
  query_ := FORMAT('DELETE FROM %s WHERE id = %L',
                  tableName_, id_);
  EXECUTE query_;
  
  --Проверяем, появились ли пустые партиции
  targetTables_:= ARRAY(
		SELECT ns.nspname || '.'|| c.relname AS child 
			FROM pg_inherits
  			JOIN pg_class AS c ON (inhrelid=c.oid)
  			JOIN pg_class as p ON (inhparent=p.oid)
  			JOIN pg_catalog.pg_namespace AS ns ON c.relnamespace = ns.oid       
  			JOIN pg_catalog.pg_namespace AS ps ON p.relnamespace = ps.oid
			WHERE ps.nspname || '.' || p.relname = tableName_ ORDER BY child
	);
  
  FOREACH partition_ IN ARRAY targetTables_
	LOOP
    query_ := FORMAT('select exists (select 1 from %s)', partition_);
    EXECUTE query_ INTO exists_;
    IF exists_ = FALSE
    THEN
      query_ := FORMAT('DROP TABLE %s', partition_);
      EXECUTE query_;
    END IF;
  END LOOP;
  
  return TRUE;
END 
$$;


ALTER FUNCTION global.rm_any_by_id(collection_ text, id_ bigint) OWNER TO postgres;

--
-- Name: try_clean_table_gridfs(text, text, timestamp without time zone, text, timestamp without time zone, text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.try_clean_table_gridfs(tablename_ text, dtfieldname_ text, dt1_ timestamp without time zone, dtwritefieldname_ text, dt2_ timestamp without time zone, gridfsfield_ text) RETURNS record
    LANGUAGE plpgsql COST 1000
    AS $$
DECLARE
 targetTables_ text[];
 tableFullName text;
 ok_ real := 1;
 n_ bigint := 0;
 ret_ record;
 
 query_ text;
 count_ bigint;	
 
 gridfsFiles_ text[]; 
 
 partition_ text;
BEGIN
    --Получаем список партиций
    tableFullName := 'meteo.' || tableName_;

    targetTables_:= ARRAY(
		SELECT ns.nspname || '.'|| c.relname AS child 
			FROM pg_inherits
  			JOIN pg_class AS c ON (inhrelid=c.oid)
  			JOIN pg_class as p ON (inhparent=p.oid)
  			JOIN pg_catalog.pg_namespace AS ns ON c.relnamespace = ns.oid       
  			JOIN pg_catalog.pg_namespace AS ps ON p.relnamespace = ps.oid
			WHERE ps.nspname || '.' || p.relname = tableFullName ORDER BY child
	);
		
	--Если партиции существуют - проходимся по каждой из них
	IF array_length(targetTables_, 1) > 0
	THEN
    	FOREACH partition_ IN ARRAY targetTables_
		LOOP
		    query_ := FORMAT('SELECT COUNT(*) FROM %s WHERE %s > %L OR %s > %L ', 
							partition_, dtFieldName_, dt1_, dtWriteFieldName_, dt2_ );
              raise notice '%', query_;
							
			EXECUTE  query_ INTO count_;
			  
			IF count_ = 0
			THEN			
			  query_ := FORMAT('SELECT array_agg(%I) FROM %s', gridfsField_, partition_);
        
        EXECUTE query_ INTO gridfsFiles_;
        
				n_ := n_ + array_length(gridfsFiles_, 1);
        query_ := FORMAT('DROP TABLE %s', partition_);
				EXECUTE query_;        
        
	      SELECT ok_, n_, array_to_json(gridfsFiles_) INTO ret_;
	      return ret_;
			END IF;
			
    	END LOOP;
	ELSE
	  raise notice 'ОШИБКА - для удаления требуется наличие партицирования на таблицах, сделайте партицирование на таблице';
	  ok_ := 0;	  
	END IF;
	
	SELECT ok_, n_, array_to_json(gridfsFiles_) INTO ret_;
	return ret_;
END
$$;


ALTER FUNCTION global.try_clean_table_gridfs(tablename_ text, dtfieldname_ text, dt1_ timestamp without time zone, dtwritefieldname_ text, dt2_ timestamp without time zone, gridfsfield_ text) OWNER TO postgres;

--
-- Name: try_create_fs_part(text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.try_create_fs_part(tablemain_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
  tablepart_ TEXT;
  tablepart_only_ TEXT;
  id_ BIGINT;
  minval_ BIGINT;
  maxval_ BIGINT;
  query_ TEXT;
BEGIN
  query_ = FORMAT('SELECT nextval(''meteo.%s_id_seq''::regclass);', tablemain_ );
  EXECUTE query_ INTO id_; 
  minval_ := id_ - id_%100000;
  maxval_ := minval_ + 100000;
  tablepart_ = 'meteo.' || tablemain_ || '_' || CAST( minval_ AS TEXT ) || '_' || CAST( maxval_ AS TEXT );
  tablepart_only_ = tablemain_ || '_' || CAST( minval_ AS TEXT ) || '_' || CAST( maxval_ AS TEXT );
  PERFORM 1 FROM pg_tables WHERE tablename = tablepart_only_ LIMIT 1;
  IF NOT FOUND
  THEN
    query_ := format(
      -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
      'CREATE UNLOGGED TABLE %s
          (
		  CONSTRAINT %I_oninsert UNIQUE (filemd5),
          CONSTRAINT %I_id_check
                CHECK ( id BETWEEN %L::BIGINT
                                AND %L::BIGINT - 1)
          )
          INHERITS (%s)
          WITH (OIDS = FALSE, FILLFACTOR = 90)',
		 tablepart_, tablepart_only_, tablepart_only_,
         minval_, maxval_,
		 'meteo.' || tablemain_);
    EXECUTE query_  ;
	    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || tablepart_ || ' ADD CONSTRAINT pkey_' || tablepart_only_ || ' PRIMARY KEY (id)';

    -- создаём индексы
	IF ( meteo.f_is_index_exist(tablepart_only_ || '_id' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(tablepart_, ARRAY[ 'id'] );
	END IF;
	IF ( meteo.f_is_index_exist(tablepart_only_ || '_filemd5' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(tablepart_, ARRAY[ 'filemd5'] );
	END IF;
	IF ( meteo.f_is_index_exist(tablepart_only_ || '_filename' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(tablepart_, ARRAY[ 'filename'] );
	END IF;
  END IF;
--  raise notice 'partpart = % %', tablepart_only_, id_;
  RETURN (tablepart_only_, id_);
END;
$$;


ALTER FUNCTION global.try_create_fs_part(tablemain_ text) OWNER TO postgres;

--
-- Name: try_create_partition_by_dt(text, text, timestamp without time zone); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.try_create_partition_by_dt(table_name_ text, dt_field_name_ text, dt_ timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    table_main        TEXT  ;--самая главная таблица
    table_part_yyyy_mm_dd          TEXT       = '';--таблица по дням (партиция за сутки)
    rec_                RECORD;
    table_part_yyyy_mm_dd_only text;   
	query_ text;
	table_full_name TEXT;
BEGIN

  --rrr :=  clock_timestamp();
  --RAISE NOTICE 'start time operation: %', rrr;
  -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||table_name_; --самая главная таблица
  table_part_yyyy_mm_dd_only :=   table_name_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_name_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_full_name := 'meteo.'||table_part_yyyy_mm_dd;

  -- Проверяем партицию за cутки на существование --------------------------------

  PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
  -- Если её ещё нет, то 
  IF NOT FOUND
  THEN
  	-- Cоздаём партицию, наследуя мастер-таблицу --------------------------	
	query_ := format(
	  'CREATE TABLE %s ( CONSTRAINT %I_created_check CHECK ( %s BETWEEN %L::timestamp AND %L::timestamp + interval ''1 day'' - interval ''1 sec'') )
	   INHERITS (%s) WITH (OIDS = FALSE, FILLFACTOR = 90)',
			table_full_name,
			table_part_yyyy_mm_dd_only, 
			dt_field_name_, 
			date_trunc('day', dt_),
			date_trunc('day', dt_),
			table_main);	
    EXECUTE query_;
    -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
    FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                             FROM information_schema.role_table_grants
                             WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
      EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_full_name || ' TO ' || rec_.grantee;
    END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || table_full_name || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';
   END IF;
 
 RETURN table_full_name;
END;
$$;


ALTER FUNCTION global.try_create_partition_by_dt(table_name_ text, dt_field_name_ text, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: try_create_partition_by_year(text, text, smallint); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.try_create_partition_by_year(table_name_ text, dt_field_name_ text, year_ smallint) RETURNS text
    LANGUAGE plpgsql
    AS $$DECLARE
    table_main        TEXT  ;--самая главная таблица
    table_part_yyyy          TEXT       = '';--таблица по дням (партиция за год)
    rec_                RECORD;
    table_part_yyyy_only text;   
	query_ text;
	table_full_name TEXT;
BEGIN

  --rrr :=  clock_timestamp();
  --RAISE NOTICE 'start time operation: %', rrr;
  -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||table_name_; --самая главная таблица
  table_part_yyyy_only :=   table_name_  || '_' || year_::TEXT; --таблица по дням (партиция за год)
  table_part_yyyy :=   table_name_  || '_' || year_::TEXT; --таблица по дням (партиция за год)
  table_full_name := 'meteo.'||table_part_yyyy;

  -- Проверяем партицию за cутки на существование --------------------------------

  PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_only LIMIT 1;
  -- Если её ещё нет, то 
  IF NOT FOUND
  THEN
  	-- Cоздаём партицию, наследуя мастер-таблицу --------------------------	
	query_ := format(
	  'CREATE TABLE %s ( CONSTRAINT %I_created_check CHECK ( %s = %L ) )
	   INHERITS (%s) WITH (OIDS = FALSE, FILLFACTOR = 90)',
			table_full_name,
			table_part_yyyy_only, 
			dt_field_name_, 
			year_,			
			table_main);	
    EXECUTE query_;
    -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
    FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                             FROM information_schema.role_table_grants
                             WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
      EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_full_name || ' TO ' || rec_.grantee;
    END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    --EXECUTE 'ALTER TABLE ' || table_full_name || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';
	 for rec_ in
           select oid, conname
           from pg_constraint
           where contype = 'p' 
           and conrelid = table_main::regclass
       loop
           execute format(
               'alter table %s add constraint %s %s',
               table_full_name,			   
               replace(rec_.conname, table_name_, table_part_yyyy),
               pg_get_constraintdef(rec_.oid));
       end loop;
	
   END IF;
 
 RETURN table_full_name;
END;
$$;


ALTER FUNCTION global.try_create_partition_by_year(table_name_ text, dt_field_name_ text, year_ smallint) OWNER TO postgres;

--
-- Name: FUNCTION try_create_partition_by_year(table_name_ text, dt_field_name_ text, year_ smallint); Type: COMMENT; Schema: global; Owner: postgres
--

COMMENT ON FUNCTION global.try_create_partition_by_year(table_name_ text, dt_field_name_ text, year_ smallint) IS 'Функция создаёт дочернюю таблицу, отнаследованную от таблицы, на которой сработал триггер (table_master), с именем table_master_yyyy, если таковая не существует. Данные будут вставлены в дочернюю таблицу.
';


--
-- Name: try_create_partition_by_year(text, text, timestamp without time zone); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.try_create_partition_by_year(table_name_ text, dt_field_name_ text, dt_ timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$DECLARE
    table_main        TEXT  ;--самая главная таблица
    table_part_yyyy          TEXT       = '';--таблица по дням (партиция за год)
    rec_                RECORD;
    table_part_yyyy_only text;   
	query_ text;
	table_full_name TEXT;
BEGIN

  --rrr :=  clock_timestamp();
  --RAISE NOTICE 'start time operation: %', rrr;
  -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||table_name_; --самая главная таблица
  table_part_yyyy_only :=   table_name_  || '_' || to_char(dt_, 'YYYY'); --таблица по дням (партиция за год)
  table_part_yyyy :=   table_name_  || '_' || to_char(dt_, 'YYYY'); --таблица по дням (партиция за год)
  table_full_name := 'meteo.'||table_part_yyyy;

  -- Проверяем партицию за cутки на существование --------------------------------

  PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_only LIMIT 1;
  -- Если её ещё нет, то 
  IF NOT FOUND
  THEN
  	-- Cоздаём партицию, наследуя мастер-таблицу --------------------------	
	query_ := format(
	  'CREATE TABLE %s ( CONSTRAINT %I_created_check CHECK ( %s BETWEEN %L::timestamp AND %L::timestamp + interval ''1 year'' - interval ''1 sec'') )
	   INHERITS (%s) WITH (OIDS = FALSE, FILLFACTOR = 90)',
			table_full_name,
			table_part_yyyy_only, 
			dt_field_name_, 
			date_trunc('year', dt_),
			date_trunc('year', dt_),
			table_main);	
    EXECUTE query_;
    -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
    FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                             FROM information_schema.role_table_grants
                             WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
      EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_full_name || ' TO ' || rec_.grantee;
    END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    --EXECUTE 'ALTER TABLE ' || table_full_name || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';
	 for rec_ in
           select oid, conname
           from pg_constraint
           where contype = 'p' 
           and conrelid = table_main::regclass
       loop
           execute format(
               'alter table %s add constraint %s %s',
               table_full_name,			   
               replace(rec_.conname, table_name_, table_part_yyyy),
               pg_get_constraintdef(rec_.oid));
       end loop;
	
   END IF;
 
 RETURN table_full_name;
END;
$$;


ALTER FUNCTION global.try_create_partition_by_year(table_name_ text, dt_field_name_ text, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: FUNCTION try_create_partition_by_year(table_name_ text, dt_field_name_ text, dt_ timestamp without time zone); Type: COMMENT; Schema: global; Owner: postgres
--

COMMENT ON FUNCTION global.try_create_partition_by_year(table_name_ text, dt_field_name_ text, dt_ timestamp without time zone) IS 'Функция создаёт дочернюю таблицу, отнаследованную от таблицы, на которой сработал триггер (table_master), с именем table_master_yyyy, если таковая не существует. Данные будут вставлены в дочернюю таблицу.
';


--
-- Name: update_file_description(text, bigint, text, text, integer); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.update_file_description(tablename_ text, id_ bigint, filename_ text, filemd5_ text, filesize_ integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
DECLARE
  query_ TEXT;
BEGIN
  PERFORM global.check_fs_table(tablename_);
  query_ = FORMAT('UPDATE meteo.%I SET filename = %L,
				                       filemd5 = %L,
				                       filesize = %L WHERE id = %L;', tablename_, filename_, filemd5_, filesize_, id_ );

  EXECUTE query_;
  RETURN TRUE;
END;
$$;


ALTER FUNCTION global.update_file_description(tablename_ text, id_ bigint, filename_ text, filemd5_ text, filesize_ integer) OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

