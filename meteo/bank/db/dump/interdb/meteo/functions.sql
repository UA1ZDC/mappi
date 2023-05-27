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
-- Name: appointment_last_msg_dt(text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.appointment_last_msg_dt(to_ text, from_ text) RETURNS TABLE(_id bigint, mac integer, dt timestamp without time zone, "from" text, "to" text, text text, files text[], parent text, created_by text, read_by text)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM meteo.appointment_messages AS messages
        WHERE ( ( from_ = messages.from AND to_ = messages.to ) OR ( to_ = messages.from AND from_ = messages.to ) )
        ORDER BY dt DESC
        LIMIT 1;
END
$$;


ALTER FUNCTION meteo.appointment_last_msg_dt(to_ text, from_ text) OWNER TO postgres;

--
-- Name: appointment_message_find(integer, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.appointment_message_find(mac_ integer, from_ text, to_ text, skip_ integer DEFAULT 0) RETURNS TABLE(_id bigint, mac integer, dt timestamp without time zone, "from" text, "to" text, text text, files text[], parent text, created_by text, read_by text)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM meteo.appointment_messages AS messages
        WHERE (mac_ >= messages.mac)
          AND ( ( from_ = messages.from AND to_ = messages.to ) OR ( to_ = messages.from AND from_ = messages.to ) )
        ORDER BY dt DESC
        LIMIT 50
        OFFSET skip_;
END
$$;


ALTER FUNCTION meteo.appointment_message_find(mac_ integer, from_ text, to_ text, skip_ integer) OWNER TO postgres;

--
-- Name: appointment_message_insert(integer, timestamp without time zone, text, text, text, text[], text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.appointment_message_insert(mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, files_ text[], parent_ text, created_by_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
res record;
 BEGIN
	INSERT INTO meteo.appointment_messages(mac, dt, "from", "to", "text", files, parent, created_by)
	                   VALUES (mac_, dt_, from_, to_, text_, files_, parent_, created_by_)
	        ON CONFLICT DO NOTHING 
	 RETURNING id,  
				appointment_messages.name, 
				appointment_messages.owner, 
				appointment_messages.type, 
				appointment_messages.currentuser, 
				to_jsonb(appointment_messages.users), 
				appointment_messages.dtstart into res;
	--PERFORM res;
	return res;
END;
$$;


ALTER FUNCTION meteo.appointment_message_insert(mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, files_ text[], parent_ text, created_by_ text) OWNER TO postgres;

--
-- Name: appointment_message_status_update(bigint, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.appointment_message_status_update(id_ bigint, login_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
res record;
 BEGIN
	UPDATE meteo.appointment_messages 
	SET  read_by = login_
	WHERE ( id_ = id ) 
	RETURNING id as _id, 
				appointment_messages.name, 
				appointment_messages.owner, 
				appointment_messages.type, 
				appointment_messages.currentuser, 
				to_jsonb(appointment_messages.users), 
				appointment_messages.dtstart into res;
	--PERFORM res;
	return res;
END;
$$;


ALTER FUNCTION meteo.appointment_message_status_update(id_ bigint, login_ text) OWNER TO postgres;

--
-- Name: appointment_unread_message_stats(integer, text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.appointment_unread_message_stats(mac_ integer, from_ text, to_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE 
ok real;
n integer;
result record;
BEGIN
        SELECT COUNT(*)
        FROM 	meteo.appointment_messages AS appointment
        WHERE ( mac_ IS NULL OR mac_  >= appointment.mac )
           AND ( to_ IS NULL OR to_ = appointment.to )
           AND ( from_ IS NULL OR from_ = appointment.from) 
           AND ( appointment.read_by IS NULL ) 
        into n;
if n = NULL
THEN
  ok := 0;
ELSE
  ok := 1;
END IF;
SELECT ok, n INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.appointment_unread_message_stats(mac_ integer, from_ text, to_ text) OWNER TO postgres;

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
-- Name: conference_create(text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_create(name_ text, owner_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
members_ text[];
ok real;
id_ bigint;
result record;
nModified integer;
n integer;
upserted_ jsonb;
exists_ text;
 BEGIN
 members_ = members_ || owner_;
 nModified := 1;
 --exists_ := NULL:
        select name from meteo.conferences where (name = name_) into exists_;
	INSERT INTO meteo.conferences(name, owner, members)
	                   VALUES (name_, owner_, members_)
	        ON CONFLICT(name) DO NOTHING  
	 RETURNING id INTO id_;
IF id_ = NULL
THEN
  ok := 0;
  n := 0;
ELSE
  ok := 1;
  n := 1;
END IF;

IF exists_ is NULL THEN
  upserted_ := jsonb_set('{}', '{_id}', to_jsonb(id_));
ELSE
  nModified := 0;
END IF;

SELECT ok, n, nModified, upserted_ INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.conference_create(name_ text, owner_ text) OWNER TO postgres;

--
-- Name: conference_find_by_memberid(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_find_by_memberid(login_ text) RETURNS TABLE(_id bigint, name text, owner text, members json)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT id, conferences.name, conferences.owner, array_to_json(conferences.members)
        FROM meteo.conferences AS conferences
        WHERE ( login_ = ANY( conferences.members ) );

END
$$;


ALTER FUNCTION meteo.conference_find_by_memberid(login_ text) OWNER TO postgres;

--
-- Name: conference_last_msg_dt(text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_last_msg_dt(to_ text) RETURNS TABLE(_id bigint, mac integer, dt timestamp without time zone, "from" text, "to" text, text text, delivery text[], total integer, files text[], parent text, unread text[])
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM meteo.conference_messages AS messages
        WHERE ( to_ = messages.to)
        ORDER BY dt DESC
        LIMIT 1;
END
$$;


ALTER FUNCTION meteo.conference_last_msg_dt(to_ text) OWNER TO postgres;

--
-- Name: conference_member_add(bigint, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_member_add(id_ bigint, members_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
n real;
"nModified" real;
result record;
test record;
 BEGIN
	UPDATE meteo.conferences
	SET members = members || members_
	WHERE id_ = id
	RETURNING * into test;
IF test.id is NULL
THEN
  ok := 0;
  n := 0;
  "nModified" := 0;
ELSE
  ok := 1;
  n := 1;
  "nModified" := 1;
END IF;
SELECT ok, n, "nModified" INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.conference_member_add(id_ bigint, members_ text) OWNER TO postgres;

--
-- Name: conference_member_remove(bigint, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_member_remove(id_ bigint, members_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
n real;
"nModified" real;
result record;
test record;
 BEGIN
	UPDATE meteo.conferences
	SET members = array_remove( members, members_ )
	WHERE id_ = id
	RETURNING * into test;
IF test.id is NULL
THEN
  ok := 0;
  n := 0;
  "nModified" := 0;
ELSE
  ok := 1;
  n := 1;
  "nModified" := 1;
END IF;
SELECT ok, n, "nModified" INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.conference_member_remove(id_ bigint, members_ text) OWNER TO postgres;

--
-- Name: conference_message_insert(integer, timestamp without time zone, text, text, text, text[], text[], integer, text[], text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_message_insert(mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, delivery_ text[], unread_ text[], total_ integer, files_ text[], parent_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
n integer;
result record;
 BEGIN
	INSERT INTO meteo.conference_messages(mac, dt, "from", "to", "text", delivery, unread, total, files, parent)
	                   VALUES (mac_, dt_, from_, to_, text_, delivery_, unread_, total_, files_, parent_)
	        ON CONFLICT DO NOTHING 
	 RETURNING id INTO n;
IF n = NULL
THEN
  ok := 0;
ELSE
  ok := 1;
END IF;
SELECT ok, n INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.conference_message_insert(mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, delivery_ text[], unread_ text[], total_ integer, files_ text[], parent_ text) OWNER TO postgres;

--
-- Name: conference_message_status_update(bigint, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_message_status_update(id_ bigint, login_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
res record;
 BEGIN
	UPDATE meteo.conference_messages 
	SET delivery = array_remove(delivery, login_), unread =  array_remove(unread, login_)
	WHERE id_ = id
	RETURNING id as _id, conference_messages.mac, 
				conference_messages.dt, 
				conference_messages."from", 
				conference_messages."to", 
				conference_messages.text, 
				to_jsonb(conference_messages.delivery), 
				to_jsonb(conference_messages.unread), 
				conference_messages.total, 
				to_jsonb(conference_messages.files), 
				conference_messages.parent into res;
return res;
END;
$$;


ALTER FUNCTION meteo.conference_message_status_update(id_ bigint, login_ text) OWNER TO postgres;

--
-- Name: conference_rename(bigint, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_rename(id_ bigint, name_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
n real;
"nModified" real;
result record;
test record;
 BEGIN
	UPDATE meteo.conferences
	SET name = name_
	WHERE id_ = id
	RETURNING * into test;
IF test.id IS NULL
THEN
  ok := 0;
  n := 0;
  "nModified" := 0;
ELSE
  ok := 1;
  n := 1;
  "nModified" := 1;
END IF;
SELECT ok, n, "nModified" INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.conference_rename(id_ bigint, name_ text) OWNER TO postgres;

--
-- Name: conference_unread_message_stats(integer, text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.conference_unread_message_stats(mac_ integer, to_ text, user_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE 
ok real;
n integer;
result record;
BEGIN
        SELECT COUNT(*)
        FROM 	meteo.conference_messages AS conference
        WHERE ( mac_ IS NULL OR mac_  >= conference.mac )
           AND ( to_ IS NULL OR to_ = conference.to )
           AND ( user_ IS NULL OR user_ = ANY(conference.unread) ) 
        into n;
if n = NULL
THEN
  ok := 0;
ELSE
  ok := 1;
END IF;
SELECT ok, n INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.conference_unread_message_stats(mac_ integer, to_ text, user_ text) OWNER TO postgres;

--
-- Name: department_last_msg_dt(text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.department_last_msg_dt(from_ text, to_ text) RETURNS TABLE(_id bigint, mac integer, dt timestamp without time zone, text text, from_user text, "from" text, "to" text, read_by text, files text[], parent text, delivered_to text)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM meteo.department_messages AS messages
        WHERE ( ( from_ = messages.from AND to_ = messages.to ) OR ( to_ = messages.from AND from_ = messages.to ) )
        ORDER BY dt DESC
        LIMIT 1;
END
$$;


ALTER FUNCTION meteo.department_last_msg_dt(from_ text, to_ text) OWNER TO postgres;

--
-- Name: department_message_insert(integer, timestamp without time zone, text, text, text, text[], text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.department_message_insert(mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, files_ text[], parent_ text, from_user_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE 
res record;
 BEGIN
	INSERT INTO meteo.department_messages(mac, dt, "from", "to", "text", files, parent, from_user)
	                   VALUES (mac_, dt_, from_, to_, text_, files_, parent_, from_user_)
	        ON CONFLICT DO NOTHING 
	 RETURNING id, department_messages.mac, 
				department_messages.dt, 
				department_messages.text, 
				department_messages.from_user, 
				department_messages."from", 
				department_messages."to", 
				department_messages.read_by, 
				to_jsonb(department_messages.files), 
				department_messages.parent, 
				department_messages.delivered_to into res;
	--PERFORM res;
	return res;
END;
$$;


ALTER FUNCTION meteo.department_message_insert(mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, files_ text[], parent_ text, from_user_ text) OWNER TO postgres;

--
-- Name: department_message_list_get(integer, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.department_message_list_get(mac_ integer, from_ text, to_ text, skip_ integer DEFAULT 0) RETURNS TABLE(_id bigint, mac integer, dt timestamp without time zone, text text, from_user text, "from" text, "to" text, read_by text, files jsonb, parent text, delivered_to text)
    LANGUAGE plpgsql
    AS $$
BEGIN
RETURN QUERY
        SELECT 
		messages.id,
		messages.mac, 
		messages.dt, 
		messages.text, 
		messages.from_user, 
		messages."from", 
		messages."to", 
		messages.read_by, 
		to_jsonb(messages.files), 
		messages.parent, 
		messages.delivered_to
        FROM meteo.department_messages AS messages
        WHERE (mac_ >= messages.mac)
          AND ( ( from_ = messages.from AND to_ = messages.to ) OR ( to_ = messages.from AND from_ = messages.to ) )
        ORDER BY dt DESC
        LIMIT 50
        OFFSET skip_;
END
$$;


ALTER FUNCTION meteo.department_message_list_get(mac_ integer, from_ text, to_ text, skip_ integer) OWNER TO postgres;

--
-- Name: department_message_status_update(bigint, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.department_message_status_update(id_ bigint, to_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
res record;
 BEGIN
	UPDATE meteo.department_messages 
	SET delivered_to = to_, read_by = to_
	WHERE ( ( id_ = id ) AND ( delivered_to is NULL ) AND ( read_by is NULL ) )
	RETURNING id, department_messages.mac, 
				department_messages.dt, 
				department_messages.text, 
				department_messages.from_user, 
				department_messages."from", 
				department_messages."to", 
				department_messages.read_by, 
				department_messages.files, 
				department_messages.parent, 
				department_messages.delivered_to into res;
	--PERFORM res;
	return res;
END;
$$;


ALTER FUNCTION meteo.department_message_status_update(id_ bigint, to_ text) OWNER TO postgres;

--
-- Name: department_unread_message_stats(integer, text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.department_unread_message_stats(mac_ integer, from_ text, to_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE 
ok real;
n integer;
result record;
BEGIN
        SELECT COUNT(*)
        FROM 	meteo.department_messages AS department
        WHERE ( mac_ IS NULL OR mac_  >= department.mac )
           AND ( from_ IS NULL OR from_ = department.from )
           AND ( to_ IS NULL OR to_ = department.to) 
           AND ( department.read_by IS NULL)
        into n;
if n = NULL
THEN
  ok := 0;
ELSE
  ok := 1;
END IF;
SELECT ok, n INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.department_unread_message_stats(mac_ integer, from_ text, to_ text) OWNER TO postgres;

--
-- Name: dialog_message_find(integer, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.dialog_message_find(mac_ integer, from_ text, to_ text, skip_ integer DEFAULT 0) RETURNS TABLE(_id bigint, mac integer, dt timestamp without time zone, "from" text, "to" text, status integer, system boolean, text text, files text[], urls text[])
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM meteo.dialogs_messages AS messages
        WHERE (mac_ >= messages.mac)
          AND ( ( from_ = messages.from AND to_ = messages.to ) OR ( to_ = messages.from AND from_ = messages.to ) )
        ORDER BY dt DESC
        LIMIT 50
        OFFSET skip_;
END
$$;


ALTER FUNCTION meteo.dialog_message_find(mac_ integer, from_ text, to_ text, skip_ integer) OWNER TO postgres;

--
-- Name: dialog_message_insert(integer, timestamp without time zone, text, text, text, text[], integer, text[], boolean); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.dialog_message_insert(mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, files_ text[], status_ integer, urls_ text[], system_ boolean) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
n integer;
result record;
query_ text;
 BEGIN
	 INSERT INTO meteo.dialogs_messages(mac, dt, "from", "to", status, system, "text", files, urls)
	                        VALUES (mac_, dt_, from_, to_, status_, system_, text_, files_, urls_)
	                        ON CONFLICT DO NOTHING 
	                        RETURNING id into n;
IF n = NULL
THEN
  ok := 0;
ELSE
  ok := 1;
END IF;
SELECT ok, n INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.dialog_message_insert(mac_ integer, dt_ timestamp without time zone, from_ text, to_ text, text_ text, files_ text[], status_ integer, urls_ text[], system_ boolean) OWNER TO postgres;

--
-- Name: dialog_message_status_update(bigint, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.dialog_message_status_update(id_ bigint, status_ integer) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
result record;
test record;
 BEGIN
	UPDATE meteo.dialogs_messages 
	SET status = status_
	WHERE ( id_ = id ) 
	RETURNING id as _id, mac, dt, "from", "to", status, system, text, files, urls into test;
IF test._id IS NULL
THEN
  ok := 0;
ELSE
  ok := 1;
END IF;
SELECT ok, to_jsonb(test) INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.dialog_message_status_update(id_ bigint, status_ integer) OWNER TO postgres;

--
-- Name: dialog_unread_message_stats(integer, text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.dialog_unread_message_stats(mac_ integer, from_ text, to_ text) RETURNS record
    LANGUAGE plpgsql
    AS $$
DECLARE
ok real;
n integer; 
ret_count integer;
result record;
BEGIN
        SELECT COUNT(*)
        FROM 	meteo.dialogs_messages AS dialogs
        WHERE ( mac_ IS NULL OR mac_  >= dialogs.mac )
           AND ( from_ IS NULL OR from_ = dialogs.from )
           AND ( to_ IS NULL OR to_ = dialogs.to) 
           AND (  4 != dialogs.status ) into ret_count;
n := 0;	
IF ret_count is NULL THEN
  ok := 0;
ELSE
  ok := 1;
  n := ret_count;
END IF;
SELECT ok, n INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.dialog_unread_message_stats(mac_ integer, from_ text, to_ text) OWNER TO postgres;

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
-- Name: find_conference_by_id(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_conference_by_id(id_ bigint) RETURNS TABLE(_id bigint, name text, owner text, members json)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT id, conferences.name, conferences.owner, array_to_json(conferences.members)
        FROM meteo.conferences AS conferences
        WHERE (id_ = conferences.id);
END
$$;


ALTER FUNCTION meteo.find_conference_by_id(id_ bigint) OWNER TO postgres;

--
-- Name: find_conference_message_list(integer, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_conference_message_list(mac_ integer, to_ text, skip_ integer DEFAULT 0) RETURNS TABLE(_id bigint, mac integer, dt timestamp without time zone, "from" text, "to" text, text text, delivery json, total integer, files json, parent text, unread json)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT id as _id,
		messages.mac,
		messages.dt,
		messages.from,
		messages.to,
		messages.text,
		array_to_json( messages.delivery ),
		messages.total,
		array_to_json( messages.files ),
		messages.parent,
		array_to_json(messages.unread)
        FROM meteo.conference_messages AS messages
        WHERE (mac_ >= messages.mac)
          AND ( to_ = messages.to ) 
        ORDER BY dt DESC
        LIMIT 50
        OFFSET skip_;
END
$$;


ALTER FUNCTION meteo.find_conference_message_list(mac_ integer, to_ text, skip_ integer) OWNER TO postgres;

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
-- Name: find_file_list(text, text, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_file_list(tablename_ text, filename_ text, id_ bigint) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$DECLARE
 query_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  query_ = FORMAT('SELECT id, filename, filemd5, filesize, dt_write
    FROM meteo.%I WHERE (%L IS NULL OR filename = %L) AND (%L IS NULL OR id = %L)', tablename_, filename_, filename_, id_, id_ );
	RETURN QUERY EXECUTE query_;
END;$$;


ALTER FUNCTION meteo.find_file_list(tablename_ text, filename_ text, id_ bigint) OWNER TO postgres;

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
-- Name: inter_file_up_down_load_end(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.inter_file_up_down_load_end(id_ bigint) RETURNS void
    LANGUAGE plpgsql
    AS $$BEGIN
        DELETE
        FROM meteo.up_down_loads 
        WHERE ( id_ = id);
END
$$;


ALTER FUNCTION meteo.inter_file_up_down_load_end(id_ bigint) OWNER TO postgres;

--
-- Name: inter_file_up_down_load_find(bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.inter_file_up_down_load_find(id_ bigint) RETURNS TABLE(upload_id bigint, gridfs_id text, dt_write timestamp without time zone, file_id text, _id bigint, chunk_id integer, next_chunk integer)
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM meteo.up_down_loads AS loads
        WHERE ( id_ = loads.id);
END
$$;


ALTER FUNCTION meteo.inter_file_up_down_load_find(id_ bigint) OWNER TO postgres;

--
-- Name: inter_file_up_down_load_start(timestamp without time zone, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.inter_file_up_down_load_start(dt_write_ timestamp without time zone, file_id_ bigint) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
n bigint;
result record;
 BEGIN
	INSERT INTO meteo.up_down_loads(dt_write, file_id, next_chunk)
	                   VALUES (dt_write_, file_id_, 0)
	        ON CONFLICT DO NOTHING 
	 RETURNING id INTO n;
IF n = NULL
THEN
  ok := 0;
ELSE
  ok := 1;
END IF;
SELECT ok, n INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.inter_file_up_down_load_start(dt_write_ timestamp without time zone, file_id_ bigint) OWNER TO postgres;

--
-- Name: inter_file_up_down_load_update(bigint, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.inter_file_up_down_load_update(id_ bigint, next_chunk_ integer) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE
 ret_id_ bigint;
BEGIN
	UPDATE meteo.up_down_loads SET next_chunk = next_chunk_
	WHERE (meteo.up_down_loads.id = id_) RETURNING id INTO ret_id_;
RETURN ret_id_;
END
$$;


ALTER FUNCTION meteo.inter_file_up_down_load_update(id_ bigint, next_chunk_ integer) OWNER TO postgres;

--
-- Name: inter_task_find(bigint, text, text, timestamp without time zone, timestamp without time zone, text, text, text, text, boolean); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.inter_task_find(id_ bigint, name_ text, text_ text, date_start_ timestamp without time zone, date_end_ timestamp without time zone, labor_ text, basics_ text, parent_ text, owner_ text, control_ boolean) RETURNS TABLE(_id bigint, name text, text text, date_start timestamp without time zone, date_end timestamp without time zone, labor text, basics text, owner text, parent text, control boolean, stage jsonb, idfile text[])
    LANGUAGE plpgsql
    AS $$BEGIN
RETURN QUERY
        SELECT *
        FROM meteo.tasks AS tasks
        WHERE ( ( (id_ IS NULL) OR (tasks.id = id_) )
            AND ( (name_ IS NULL) OR (tasks.name = name_) )
            AND ( (text_ IS NULL) OR (tasks.text = text_) )
            AND ( (date_start_ IS NULL) OR (tasks.date_start >= date_start_) ) 
            AND ( (date_end_ IS NULL) OR (tasks.date_end <= date_end_) )
            AND ( (labor_ IS NULL) OR (tasks.labor = labor_) )
            AND ( (basics_ IS NULL) OR (tasks.basics = basics_) )
            AND ( (owner_ IS NULL) OR (tasks.owner = owner_) )
            AND ( (parent_ IS NULL) OR (tasks.parent = parent_) )
            AND ( (control_ IS NULL) OR (tasks.control = control_) )
            );

END
$$;


ALTER FUNCTION meteo.inter_task_find(id_ bigint, name_ text, text_ text, date_start_ timestamp without time zone, date_end_ timestamp without time zone, labor_ text, basics_ text, parent_ text, owner_ text, control_ boolean) OWNER TO postgres;

--
-- Name: inter_task_insert(text, text, timestamp without time zone, timestamp without time zone, text, text, text, text, text[], boolean, jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.inter_task_insert(name_ text, text_ text, date_start_ timestamp without time zone, date_end_ timestamp without time zone, labor_ text, basics_ text, owner_ text, parent_ text, idfile_ text[], control_ boolean, stage_ jsonb) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
n integer;
result record;
val record;
"lastErrorObject" jsonb;
 BEGIN
	INSERT INTO meteo.tasks( "name", "text", date_start, date_end, labor, basics, "owner", parent, idfile, control, stage )
	                   VALUES (name_, text_, date_start_, date_end_, labor_, basics_, owner_, parent_, idfile_, control_, stage_ )
	        ON CONFLICT DO NOTHING 
	 RETURNING *, id as _id INTO val;
IF val._id is NULL
THEN
  ok := 0;
ELSE
  ok := 1;
  "lastErrorObject" := jsonb_set('{}', '{upserted}', to_jsonb(val._id));
  "lastErrorObject"  := jsonb_set("lastErrorObject", '{n}', to_jsonb(1));
END IF;
SELECT ok, n, to_jsonb(val), "lastErrorObject"  INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.inter_task_insert(name_ text, text_ text, date_start_ timestamp without time zone, date_end_ timestamp without time zone, labor_ text, basics_ text, owner_ text, parent_ text, idfile_ text[], control_ boolean, stage_ jsonb) OWNER TO postgres;

--
-- Name: inter_task_update(bigint, text, text, timestamp without time zone, timestamp without time zone, text, text, text, text, text[], boolean, jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.inter_task_update(id_ bigint, name_ text, text_ text, date_start_ timestamp without time zone, date_end_ timestamp without time zone, labor_ text, basics_ text, owner_ text, parent_ text, idfile_ text[], control_ boolean, stage_ jsonb) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE 
ok real;
n integer;
result record;
val record;
"lastErrorObject" jsonb;
 BEGIN
	UPDATE meteo.tasks
	SET "name" = name_, "text" = text_, date_start = date_start_, date_end = date_end_, labor = labor_, basics = basics_, "owner" = owner_,
	     parent = parent_, idfile = idfile_, control = control_, stage = stage_ 
	WHERE id = id_
	RETURNING *, id as _id INTO val;
IF val._id is NULL
THEN
  ok := 0;
ELSE
  ok := 1;
  "lastErrorObject" := jsonb_set('{}', '{updatedExisting}', to_jsonb(true));
END IF;
SELECT ok, "lastErrorObject"  INTO result;
return result;
END;
$$;


ALTER FUNCTION meteo.inter_task_update(id_ bigint, name_ text, text_ text, date_start_ timestamp without time zone, date_end_ timestamp without time zone, labor_ text, basics_ text, owner_ text, parent_ text, idfile_ text[], control_ boolean, stage_ jsonb) OWNER TO postgres;

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
-- Name: modify_user(text, integer, bigint, text, integer, integer, bigint); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.modify_user(login_ text, gender_ integer, department_ bigint, avatar_ text, contact_visibility_ integer, role_id_ integer, rank_id_ bigint) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE 
ret_id_ bigint;
permissions_ jsonb;
 BEGIN
    IF (role_id_ IS NULL) THEN role_id_ := 3; END IF;
    IF (rank_id_ IS NULL) THEN rank_id_ := 1; END IF;
 	IF (contact_visibility_ IS NOT NULL) THEN
      permissions_ := jsonb_set('{}', '{contact_visibility}', to_jsonb(contact_visibility_));
	END IF;

	INSERT INTO meteo.users(id, login, role, rank, gender, department, permission)
	VALUES (id_, login_, role_, rank_, gender_, department_, permission_)
	ON CONFLICT(login_) DO UPDATE SET
	  role = role_,
	  rank = rank_,
	  gender = gender_,
	  department = department_,
	  permissions = permissions_
	RETURNING id INTO ret_id_;
RETURN ret_id_;
END;
$$;


ALTER FUNCTION meteo.modify_user(login_ text, gender_ integer, department_ bigint, avatar_ text, contact_visibility_ integer, role_id_ integer, rank_id_ bigint) OWNER TO postgres;

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
-- Name: try_create_department_messages_partition(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_department_messages_partition() RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    dt_ timestamp without time zone = now();
    table_main  TEXT  ;--самая главная таблица
    table_part_yyyy_mm_dd TEXT = '';--таблица по дням (партиция за сутки)
    rec_  RECORD;
    table_part_yyyy_mm_dd_only text;
    query_ text;
BEGIN

--rrr :=  clock_timestamp();
 --RAISE NOTICE 'start time operation: %', rrr;
   -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.department_messages'; --самая главная таблица;
  table_part_yyyy_mm_dd_only :=   table_main_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_main  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)

 
    -- Проверяем партицию за cутки на существование --------------------------------

      PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
      -- Если её ещё нет, то 
      IF NOT FOUND
      THEN
          query_ := format(
          -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
            'CREATE TABLE %s
          (
		     CONSTRAINT %I_created_check
             CHECK ( dt BETWEEN %L::timestamp
                               AND %L::timestamp + interval ''1 day'' - interval ''1 sec'')
          )
          INHERITS (%s)
          WITH (OIDS = FALSE, FILLFACTOR = 90)',
			 table_part_yyyy_mm_dd,table_part_yyyy_mm_dd_only,
			  date_trunc('day', dt_),date_trunc('day', dt_),
			  table_main);
          EXECUTE query_  ;
          -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
          FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                                FROM information_schema.role_table_grants
                                WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
            EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part_yyyy_mm_dd || ' TO ' || rec_.grantee;
          END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || table_part_yyyy_mm_dd || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';

    -- создаём индексы
    /* IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only ||
		'_dt_descr_hour_model_level_level_type_center_forecast_start_forecast_end' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, 
	  ARRAY[ 'dt', 'descr', 'hour', 'model', 'level', 'level_type', 'center', 'forecast_start', 'forecast_end']);
    END IF;*/
 END IF;
 RETURN table_part_yyyy_mm_dd;
END;
$$;


ALTER FUNCTION meteo.try_create_department_messages_partition() OWNER TO postgres;

--
-- Name: try_create_dialogs_messages_partition(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_dialogs_messages_partition() RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    dt_ timestamp without time zone = now();
    table_main_  TEXT  ;--самая главная таблица
    table_part_yyyy_mm_dd TEXT = '';--таблица по дням (партиция за сутки)
    rec_  RECORD;
    table_part_yyyy_mm_dd_only text;
    query_ text;
BEGIN

--rrr :=  clock_timestamp();
 --RAISE NOTICE 'start time operation: %', rrr;
   -- Даём имя партиции --------------------------------------------------
  
  table_main_ := 'meteo.dialogs_messages'; --самая главная таблица;
  table_part_yyyy_mm_dd_only :=   table_main_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_main_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)

 
    -- Проверяем партицию за cутки на существование --------------------------------

      PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
      -- Если её ещё нет, то 
      IF NOT FOUND
      THEN
          query_ := format(
          -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
            'CREATE TABLE %s
          (
		     CONSTRAINT %I_created_check
             CHECK ( dt BETWEEN %L::timestamp
                               AND %L::timestamp + interval ''1 day'' - interval ''1 sec'')
          )
          INHERITS (%s)
          WITH (OIDS = FALSE, FILLFACTOR = 90)',
			 table_part_yyyy_mm_dd,table_part_yyyy_mm_dd_only,
			  date_trunc('day', dt_),date_trunc('day', dt_),
			  table_main_);
          EXECUTE query_  ;
          -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
          FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv
                                FROM information_schema.role_table_grants
                                WHERE table_name = ''' || table_main || ''' GROUP BY grantee' LOOP
            EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part_yyyy_mm_dd || ' TO ' || rec_.grantee;
          END LOOP;

    -- Создаём первичный ключ для текущей партиции ------------------------
    EXECUTE 'ALTER TABLE ' || table_part_yyyy_mm_dd || ' ADD CONSTRAINT pkey_' || table_part_yyyy_mm_dd_only || ' PRIMARY KEY (id)';

    -- создаём индексы
    /* IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only ||
		'_dt_descr_hour_model_level_level_type_center_forecast_start_forecast_end' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, 
	  ARRAY[ 'dt', 'descr', 'hour', 'model', 'level', 'level_type', 'center', 'forecast_start', 'forecast_end']);
    END IF;*/
 END IF;
 RETURN table_part_yyyy_mm_dd;
END;
$$;


ALTER FUNCTION meteo.try_create_dialogs_messages_partition() OWNER TO postgres;

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
-- PostgreSQL database dump complete
--

