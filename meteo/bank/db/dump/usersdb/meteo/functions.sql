--
-- PostgreSQL database dump
--

-- Dumped from database version 9.6.10
-- Dumped by pg_dump version 9.6.10

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: meteo; Type: SCHEMA; Schema: -; Owner: postgres
--

CREATE SCHEMA meteo;


ALTER SCHEMA meteo OWNER TO postgres;

--
-- Name: check_fs_table(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.check_fs_table(tablename_ text) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
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
END;$$;


ALTER FUNCTION meteo.check_fs_table(tablename_ text) OWNER TO postgres;

--
-- Name: f_create_index(character varying, character varying[], character varying, character varying); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.f_create_index(character varying, character varying[], character varying DEFAULT 'btree'::character varying, character varying DEFAULT NULL::character varying) RETURNS integer
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


ALTER FUNCTION meteo.f_create_index(character varying, character varying[], character varying, character varying) OWNER TO postgres;

--
-- Name: f_is_index_exist(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.f_is_index_exist(text) RETURNS boolean
    LANGUAGE plpgsql
    AS $_$
declare
    iName alias for $1;

    iCount int4;
begin

    select count(pgi.indexname)  into iCount
    from
        pg_indexes pgi
    where
        pgi.indexname = iName;

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


ALTER FUNCTION meteo.f_is_index_exist(text) OWNER TO postgres;

--
-- Name: find_appointments_by_user(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_appointments_by_user(user_ bigint) RETURNS TABLE(_id bigint, name text, owner bigint, type integer, users bigint[], currentuser bigint, dtstart timestamp without time zone)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM meteo.user_appointments AS appointments 
        WHERE ( "currentUser" = user_  );
END
$$;


ALTER FUNCTION meteo.find_appointments_by_user(user_ bigint) OWNER TO postgres;

--
-- Name: find_file_description(text, text, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_file_description(tablename_ text, filename_ text, id_ bigint) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
 query_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  query_ = FORMAT('SELECT id, filename, filemd5, filesize, dt_write
    FROM meteo.%I WHERE filename = %L OR id = %L', tablename_, filename_, id_ );
	RETURN QUERY EXECUTE query_;
END;$$;


ALTER FUNCTION meteo.find_file_description(tablename_ text, filename_ text, id_ bigint) OWNER TO postgres;

--
-- Name: find_positions(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_positions() RETURNS TABLE(_id bigint, name text, parent bigint, current text)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT ranks.id as _id,
               ranks.name,
               ranks.parent,
               ranks.current
        FROM meteo.positions AS ranks;
END
$$;


ALTER FUNCTION meteo.find_positions() OWNER TO postgres;

--
-- Name: find_user_appointments(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_user_appointments() RETURNS TABLE(_id bigint, name text, owner bigint, type integer, users json, "currentUser" bigint, "dtStart" timestamp without time zone)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT 
		appointments.id as _id,
		appointments.name,
		appointments.owner,
		appointments.type,
		array_to_json(appointments.users),
		appointments."currentUser",
		appointments."dtStart"
        FROM 	meteo.user_appointments AS appointments;
END
$$;


ALTER FUNCTION meteo.find_user_appointments() OWNER TO postgres;

--
-- Name: find_user_settings(text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_user_settings(login_ text, type_ text) RETURNS TABLE(_id bigint, user_login text, settings_type text, settings text[])
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM 	meteo.user_settings AS settings
        WHERE ( login_ IS NULL OR login_  = settings.user_login) AND ( type_ IS NULL OR type_ = settings.settings_type) ;
END
$$;


ALTER FUNCTION meteo.find_user_settings(login_ text, type_ text) OWNER TO postgres;

--
-- Name: find_users(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_users() RETURNS TABLE(_id bigint, login text, role_id integer, rank_id integer, gender integer, department bigint, permissions jsonb)
    LANGUAGE plpgsql
    AS $$
BEGIN
RETURN QUERY
		SELECT 
		users.id as _id,
		users.username,
		users.role,
		users.rank,
		users.gender,
		users.department,
		users.permissions
        FROM meteo.users AS users;
END
$$;


ALTER FUNCTION meteo.find_users() OWNER TO postgres;

--
-- Name: insert_user_appointments(text, bigint, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_user_appointments(name_ text, owner_ bigint, type_ integer) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
ok real;
n real;
"nModified" real;
upserted record;
ret_val record;
 BEGIN
	INSERT INTO meteo.user_appointments(name, owner,type)
	                   VALUES (name_,owner_,type_)
	        ON CONFLICT(name) DO NOTHING
	          RETURNING * INTO upserted;
IF upserted IS NULL THEN 
  ok := 0;
  n := 0;
  "nModified" := 1;
ELSE
  ok := 1;
  n := 1;
  "nModified" := 0;
END IF;
SELECT ok, n, "nModified", to_jsonb(upserted) INTO ret_val;
return ret_val;
END;
$$;


ALTER FUNCTION meteo.insert_user_appointments(name_ text, owner_ bigint, type_ integer) OWNER TO postgres;

--
-- Name: load_custom_settings(text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.load_custom_settings(login_ text, name_ text) RETURNS bigint
    LANGUAGE plpgsql STABLE
    AS $$ 
DECLARE
  fileid_ BIGINT;
  query_ TEXT;
BEGIN

  query_ = FORMAT('SELECT file_id
    FROM meteo.custom_settings WHERE login = %L AND name = %L', login_, name_ );
  EXECUTE query_ INTO fileid_;
  RETURN fileid_;

END;
$$;


ALTER FUNCTION meteo.load_custom_settings(login_ text, name_ text) OWNER TO postgres;

--
-- Name: put_file_description(text, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.put_file_description(tablename_ text, filename_ text, filemd5_ text, filesize_ integer) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE
  err_context TEXT;
  idrec_ BIGINT;
  query_ TEXT;
  rec_ RECORD;
  dup_md5_ boolean;
  tablepart_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  SELECT * FROM meteo.try_create_fs_part(tablename_)
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
END;$$;


ALTER FUNCTION meteo.put_file_description(tablename_ text, filename_ text, filemd5_ text, filesize_ integer) OWNER TO postgres;

--
-- Name: put_user(integer, text, integer, integer, integer, integer, integer, text, boolean, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.put_user(id_ integer, username_ text, role_ integer, rank_ integer, gender_ integer, department_ integer, contact_visibility_ integer, birthdate_ text, block_ boolean, avatar_ text) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE 
	ret_id_ bigint;
	permissions_ jsonb;
BEGIN
	IF (contact_visibility_ IS NOT NULL) THEN
    	permissions_ := jsonb_set('{}', '{contact_visibility}', to_jsonb(contact_visibility_));
	END IF;

	INSERT INTO meteo.users(id, username, role, rank, gender, department, permissions, birthdate, block, avatar)
	VALUES (id_, username_, role_, rank_, gender_, department_, permissions_, birthdate_, block_, avatar_)
	ON CONFLICT (username) DO UPDATE SET
	    id = id_,
		role = role_,
		rank = rank_,
	  	gender = gender_,
	  	department = department_,
	  	permissions = permissions_,
		birthdate = birthdate_,
		block = block_,
		avatar = avatar_
	RETURNING id INTO ret_id_;
	
    RETURN ret_id_;
END;
$$;


ALTER FUNCTION meteo.put_user(id_ integer, username_ text, role_ integer, rank_ integer, gender_ integer, department_ integer, contact_visibility_ integer, birthdate_ text, block_ boolean, avatar_ text) OWNER TO postgres;

--
-- Name: remove_file_description(text, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.remove_file_description(tablename_ text, id_ bigint) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
  query_ TEXT;
  delcount_ INTEGER;
BEGIN
  delcount_ = 0;
  PERFORM meteo.check_fs_table(tablename_);
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
END;$$;


ALTER FUNCTION meteo.remove_file_description(tablename_ text, id_ bigint) OWNER TO postgres;

--
-- Name: FUNCTION remove_file_description(tablename_ text, id_ bigint); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.remove_file_description(tablename_ text, id_ bigint) IS 'Удалить описание файла';


--
-- Name: save_custom_settings(text, text, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.save_custom_settings(login_ text, name_ text, fileid_ bigint) RETURNS boolean
    LANGUAGE plpgsql
    AS $$ 
DECLARE  
BEGIN	
	INSERT INTO meteo.custom_settings ( login, name, file_id )
    VALUES ( login_, name_, fileid_ )
    ON CONFLICT ( login, name ) DO UPDATE SET file_id = fileid_;
	return true;
END;
$$;


ALTER FUNCTION meteo.save_custom_settings(login_ text, name_ text, fileid_ bigint) OWNER TO postgres;

--
-- Name: set_appointments_for_user(bigint, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.set_appointments_for_user(id_ bigint, users_ bigint) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
ok real;
ret_id bigint;
ret_val record;
 BEGIN
	UPDATE meteo.user_appointments AS appointments
	SET users = users || users_
	WHERE (appointments.id = id_ ) 
	RETURNING id INTO ret_id;
	UPDATE meteo.user_appointments AS appointments
	SET users = remove_array(users, users_)
	WHERE (appointments.id != id_ ) 
	RETURNING id INTO ret_id;
ok := 1;
SELECT ok INTO ret_val;
return ret_val;
END;
$$;


ALTER FUNCTION meteo.set_appointments_for_user(id_ bigint, users_ bigint) OWNER TO postgres;

--
-- Name: set_user_appointments_current_user(bigint, bigint, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.set_user_appointments_current_user(id_ bigint, user_ bigint, dt_ timestamp without time zone) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
ok real;
n integer;
"nModified" integer;
ret_id bigint;
ret_val record;
 BEGIN
	UPDATE meteo.user_appointments AS appointments
	SET "currentUser" = user_, "dtStart" = dt_
	WHERE (appointments.id = id_ AND ( user_ = SOME(appointments.users )) )
	RETURNING id INTO ret_id;
IF ret_id IS NULL THEN 
  ok := 0;
  n := 0;
  "nModified" := 0;
ELSE
  ok := 1;
  n := 1;
  "nModified" = 1;
END IF;
SELECT ok, n, "nModified" INTO ret_val;
return ret_val;
END;
$$;


ALTER FUNCTION meteo.set_user_appointments_current_user(id_ bigint, user_ bigint, dt_ timestamp without time zone) OWNER TO postgres;

--
-- Name: set_user_appointments_users(bigint, bigint[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.set_user_appointments_users(id_ bigint, users_ bigint[]) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
ok real;
n integer;
ret_id bigint;
ret_val record;
 BEGIN
	UPDATE meteo.user_appointments AS appointments
	SET users = users_
	WHERE appointments.id = id_ 
	RETURNING id INTO ret_id;
IF ret_id IS NULL THEN 
  ok := 0;
  n := 0;
ELSE
  ok := 1;
  n := 1;
END IF;
SELECT ok, n INTO ret_val;
return ret_val;
END;
$$;


ALTER FUNCTION meteo.set_user_appointments_users(id_ bigint, users_ bigint[]) OWNER TO postgres;

--
-- Name: try_create_fs_part(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_fs_part(tablemain_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE
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
      'CREATE TABLE %s
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
END;$$;


ALTER FUNCTION meteo.try_create_fs_part(tablemain_ text) OWNER TO postgres;

--
-- Name: unset_user_appointments_current_user(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.unset_user_appointments_current_user(id_ bigint) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
ok real;
n integer;
ret_id bigint;
ret_val record;
 BEGIN
	UPDATE meteo.user_appointments AS appointments
	SET "currentUser" = NULL, "dtStart" = NULL
	WHERE appointments.id = id_ 
	RETURNING id INTO ret_id;
IF ret_id IS NULL THEN 
  ok := 0;
  n := 0;
ELSE
  ok := 1;
  n := 1;
END IF;
SELECT ok, n INTO ret_val;
return ret_val;
END;
$$;


ALTER FUNCTION meteo.unset_user_appointments_current_user(id_ bigint) OWNER TO postgres;

--
-- Name: update_file_description(text, bigint, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_file_description(tablename_ text, id_ bigint, filename_ text, filemd5_ text, filesize_ integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
  query_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  query_ = FORMAT('UPDATE meteo.%I SET filename = %L,
				                       filemd5 = %L,
				                       filesize = %L WHERE id = %L;', tablename_, filename_, filemd5_, filesize_, id_ );

  EXECUTE query_;
  RETURN TRUE;
END;$$;


ALTER FUNCTION meteo.update_file_description(tablename_ text, id_ bigint, filename_ text, filemd5_ text, filesize_ integer) OWNER TO postgres;

--
-- Name: user_appointment_remove_from_all(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.user_appointment_remove_from_all(users_ bigint) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
test record;
result record;
 BEGIN
	UPDATE meteo.user_appointments
	SET users = array_remove(users, users_)
	WHERE ( users_ = ANY(users) )
	RETURNING * into test;
  ok := 1;
SELECT ok INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.user_appointment_remove_from_all(users_ bigint) OWNER TO postgres;

--
-- Name: user_position_find_by_current(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.user_position_find_by_current(login_ text) RETURNS TABLE(_id bigint, name text, parent bigint, current text)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM 	meteo.positions AS positions
        WHERE ( login_ IS NULL OR login_  = positions.current)  ;
END
$$;


ALTER FUNCTION meteo.user_position_find_by_current(login_ text) OWNER TO postgres;

--
-- Name: user_position_insert(text, bigint, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.user_position_insert(name_ text, parent_ bigint, current_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE 
ret_id_ bigint;
ret_val_ record;
ok real;
 BEGIN

	INSERT INTO meteo.positions(name, parent, current)
	                   VALUES (name_, parent_, current_)
	        ON CONFLICT DO NOTHING
	          RETURNING id INTO ret_id_;
IF ret_id_ IS NULL THEN
  ok := 0;
ELSE
  ok :=1;
END IF;
select ok into ret_val_;	          
RETURN ret_val_;
END;
$$;


ALTER FUNCTION meteo.user_position_insert(name_ text, parent_ bigint, current_ text) OWNER TO postgres;

--
-- Name: user_position_remove(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.user_position_remove(id_ bigint) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE 
ret_id_ bigint;
ret_val_ record;
n integer;
ok real;
 BEGIN
	DELETE FROM meteo.positions 
	WHERE meteo.positions.id = id_
	RETURNING id INTO ret_id_;
IF ret_id_ IS NULL THEN
  ok := 0;
  n := 0;
ELSE
  ok := 1;
  n := 1;
END IF;
select ok, n into ret_val_;
return ret_val_;
END;
$$;


ALTER FUNCTION meteo.user_position_remove(id_ bigint) OWNER TO postgres;

--
-- Name: user_position_update(bigint, text, bigint, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.user_position_update(id_ bigint, name_ text, parent_ bigint, current_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE 
ret_id_ bigint;
ret_val_ record;
lastErr jsonb;
n integer;
updates boolean;
ok real;
 BEGIN
	UPDATE meteo.positions SET name = name_, parent = parent_, current = current_
	WHERE meteo.positions.id = id_
	RETURNING id INTO ret_id_;
IF ret_id_ IS NULL THEN
  ok := 0;
  updates := false;
  n := 0;
ELSE
  ok := 1;
  updates := true;
  n := 1;
END IF;
lastErr := jsonb_set('{}', '{n}', to_jsonb(n));
lastErr := jsonb_set(lastErr, '{updatedExisting}', to_jsonb(updates));
select ok, lastErr into ret_val_;
return ret_val_;
END;
$$;


ALTER FUNCTION meteo.user_position_update(id_ bigint, name_ text, parent_ bigint, current_ text) OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

