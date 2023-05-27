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
-- Name: find_journal_records(jsonb[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_journal_records(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.find_journal_records_curs(filters, sort_column,sort_order,qskip,qslimit ) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;	
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.find_journal_records(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: find_journal_records_curs(jsonb[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_journal_records_curs(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   queryTemplate text := 'SELECT * FROM meteo.journal WHERE %s  ORDER BY %s %s LIMIT %s OFFSET %s';
   sortOrderName text:= '';
   queryString text:='';
   matchString text:= '';
   ref refcursor := 'rescursor';
BEGIN   
  if 1 = sort_order THEN
    sortOrderName = 'ASC';
  ELSE
    sortOrderName = 'DESC';
  END IF;
  
  IF array_length(filters, 1) <> 0
  THEN
        matchString := global.parse_json_condition(filters);
  ELSE
    matchString := 'TRUE';
  END IF;
    
  queryString:= format(queryTemplate, matchString, sort_column, sortOrderName, qslimit, qskip);
  
  OPEN ref FOR EXECUTE queryString;  
  return ref;
END;

$$;


ALTER FUNCTION meteo.find_journal_records_curs(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: insert_journal(integer, text, text, text, integer, text, text, timestamp without time zone, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_journal(priority integer, sender text, username text, filename text, linenumber integer, ip text, host text, dt timestamp without time zone, messagetext text) RETURNS real
    LANGUAGE plpgsql
    AS $$
DECLARE 
  insertedId bigint;
  tableName text;
BEGIN
	tableName := meteo.try_create_journal_partition(dt);
	
	EXECUTE FORMAT('INSERT INTO %s("priority", "sender", "username", "file_name", "line_number", "ip", "host", "dt", "message")
				   VALUES ( %L, %L, %L, %L, %L, %L, %L, %L, %L) RETURNING "id" ',
				   tableName, 
				   priority,
				   sender,
				   username,
				   filename,
				   linenumber, 
				   ip,
				   host,
				   dt,
				   messagetext ) INTO insertedId;
	
	IF insertedId = Null
	THEN
		return 0;
	ELSE
        return 1;
	END IF;	
END
$$;


ALTER FUNCTION meteo.insert_journal(priority integer, sender text, username text, filename text, linenumber integer, ip text, host text, dt timestamp without time zone, messagetext text) OWNER TO postgres;

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
-- Name: try_create_journal_partition(timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_journal_partition(dt timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE  
  table_part text;
BEGIN

  table_part := global.try_create_partition_by_dt( 'journal', 'dt', dt);
  
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  END IF;
  
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'priority', 'host', 'ip', 'username', 'sender' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'priority', 'host', 'ip', 'username', 'sender' ] );
  END IF;
  
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt', 'priority', 'host', 'ip', 'sender', 'username' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'dt', 'priority', 'host', 'ip', 'sender', 'username' ] );
  END IF;
  
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt', 'priority', 'host', 'ip', 'sender', 'username' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'dt', 'priority', 'host', 'ip', 'sender', 'username' ] );
  END IF;
  
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'host', 'ip', 'sender', 'username' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'host', 'ip', 'sender', 'username' ] );
  END IF;
  
 RETURN table_part;
END;
$$;


ALTER FUNCTION meteo.try_create_journal_partition(dt timestamp without time zone) OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

