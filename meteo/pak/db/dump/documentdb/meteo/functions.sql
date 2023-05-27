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
-- Name: find_fax(bigint, timestamp without time zone, timestamp without time zone, text, text, text, text, integer, text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_fax(_id bigint, start_dt timestamp without time zone, end_dt timestamp without time zone, t1 text, t2 text, a1 text, a2 text, ii integer, cccc text, yygggg text) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$ 
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
	SELECT * FROM meteo.find_fax_curs( _id, start_dt, end_dt, t1, t2, a1, a2, ii, cccc, yygggg) into ref_;
	LOOP
		FETCH NEXT FROM ref_ INTO mviews;
		IF NOT FOUND THEN EXIT;END IF;	
		return  next mviews;
	END LOOP;
END;
$$;


ALTER FUNCTION meteo.find_fax(_id bigint, start_dt timestamp without time zone, end_dt timestamp without time zone, t1 text, t2 text, a1 text, a2 text, ii integer, cccc text, yygggg text) OWNER TO postgres;

--
-- Name: find_fax_curs(bigint, timestamp without time zone, timestamp without time zone, text, text, text, text, integer, text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_fax_curs(id_ bigint, start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, t1_ text, t2_ text, a1_ text, a2_ text, ii_ integer, cccc_ text, yygggg_ text) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$ 
DECLARE 
   ref refcursor := 'rescursor';
BEGIN
  OPEN ref FOR SELECT *, id as _id FROM meteo.fax
  WHERE ( start_dt_ is NULL or start_dt_ <= "dt"  )
  AND ( end_dt_ is NULL or "dt" <= end_dt_  )
  AND ( id_ is NULL or id_ = "id" )
  AND ( t1_ is NULL or t1_ = "t1" )
  AND ( t2_ is NULL or t2_ = "t2" )
  AND ( a1_ is NULL or a1_ = "a1" )  
  AND ( a2_ is NULL or a2_ = "a2" )
  AND ( ii_ is NULL or ii_ = "ii" )
  AND ( cccc_ is Null or cccc_ = "cccc" )
  AND ( yygggg_ is Null or yygggg_ = "yygggg");
  return ref;
END;
$$;


ALTER FUNCTION meteo.find_fax_curs(id_ bigint, start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, t1_ text, t2_ text, a1_ text, a2_ text, ii_ integer, cccc_ text, yygggg_ text) OWNER TO postgres;

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
-- Name: find_image_satelite_by_dt(timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_image_satelite_by_dt(dt_start timestamp without time zone, dt_end timestamp without time zone) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$ 
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
	SELECT * FROM meteo.find_image_satelite_by_dt_curs(dt_start, dt_end) into ref_;
	LOOP
		FETCH NEXT FROM ref_ INTO mviews;
		IF NOT FOUND THEN EXIT;END IF;	
		return  next mviews;
	END LOOP;

END;
$$;


ALTER FUNCTION meteo.find_image_satelite_by_dt(dt_start timestamp without time zone, dt_end timestamp without time zone) OWNER TO postgres;

--
-- Name: find_image_satelite_by_dt_curs(timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_image_satelite_by_dt_curs(dt_start timestamp without time zone, dt_end timestamp without time zone) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$ 
DECLARE 
   ref refcursor := 'rescursor';
BEGIN
  OPEN ref FOR SELECT * FROM meteo.image_satelite WHERE dt_start <= "dt" AND dt <= "dt";
  return ref;
END;
$$;


ALTER FUNCTION meteo.find_image_satelite_by_dt_curs(dt_start timestamp without time zone, dt_end timestamp without time zone) OWNER TO postgres;

--
-- Name: get_available_documents(timestamp without time zone, timestamp without time zone, text, text, integer, integer, text, integer, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_documents(start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, mapid_ text, jobid_ text, center_ integer, model_ integer, format_ text, hour_ integer, map_title_ text, job_title_ text, limit_ integer) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
    SELECT * FROM meteo.get_available_documents_curs( start_dt_, end_dt_, mapid_, jobid_, center_, model_, format_, hour_, map_title_, job_title_, limit_ ) into ref_;
	LOOP
		FETCH NEXT FROM ref_ INTO mviews;
		IF NOT FOUND THEN EXIT;END IF;	
		return  next mviews;
	END LOOP;	
END;$$;


ALTER FUNCTION meteo.get_available_documents(start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, mapid_ text, jobid_ text, center_ integer, model_ integer, format_ text, hour_ integer, map_title_ text, job_title_ text, limit_ integer) OWNER TO postgres;

--
-- Name: get_available_documents_curs(timestamp without time zone, timestamp without time zone, text, text, integer, integer, text, integer, text, text, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_available_documents_curs(start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, mapid_ text, jobid_ text, center_ integer, model_ integer, format_ text, hour_ integer, map_title_ text, job_title_ text, limit_ integer) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  ref refcursor := 'rescursor';  
BEGIN
    IF limit_ is NULL
	THEN	
		OPEN ref FOR SELECT * FROM meteo.documents WHERE
		( start_dt_ IS NULL OR start_dt_ <= "dt" ) AND
		( end_dt_ IS NULL OR "dt" <= end_dt_ ) AND
		( mapid_ IS NULL OR mapid_ = "mapid" ) AND
		( jobid_ IS NULL OR jobid_ = "jobid" ) AND
		( center_ IS NULL OR center_ = "center" ) AND
		( model_ IS NULL OR model_ = "model" ) AND
		( format_ IS NULL OR format_ = "format" ) AND
		( hour_ IS NULL OR hour_ = "hour" ) AND
		( map_title_ IS NULL OR map_title_ = "map_title" ) AND
		( job_title_ IS NULL OR job_title_ = "job_title" ) 
		ORDER BY "dt" DESC;
	ELSE 
	  OPEN ref FOR SELECT * FROM meteo.documents WHERE
		( start_dt_ IS NULL OR start_dt_ <= "dt" ) AND
		( end_dt_ IS NULL OR "dt" <= end_dt_ ) AND
		( mapid_ IS NULL OR mapid_ = "mapid" ) AND
		( jobid_ IS NULL OR jobid_ = "jobid" ) AND
		( center_ IS NULL OR center_ = "center" ) AND
		( model_ IS NULL OR model_ = "model" ) AND
		( format_ IS NULL OR format_ = "format" ) AND
		( hour_ IS NULL OR hour_ = "hour" ) AND
		( map_title_ IS NULL OR map_title_ = "map_title" ) AND
		( job_title_ IS NULL OR job_title_ = "job_title" )
		ORDER BY "dt" DESC
		LIMIT limit_;
	END IF;
	return ref;
	
END;$$;


ALTER FUNCTION meteo.get_available_documents_curs(start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, mapid_ text, jobid_ text, center_ integer, model_ integer, format_ text, hour_ integer, map_title_ text, job_title_ text, limit_ integer) OWNER TO postgres;

--
-- Name: insert_fax(text, text, text, text, text, timestamp without time zone, integer, text, integer, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_fax(t1 text, t2 text, a1 text, a2 text, cccc text, dt timestamp without time zone, ii integer, image_path text, magic integer, yygggg text) RETURNS real
    LANGUAGE plpgsql
    AS $$ 
DECLARE
  resultId bigint;
  tableName text;
BEGIN
    select meteo.try_create_fax_partition(dt) into tableName;	
	EXECUTE format('INSERT INTO %s ( "t1", "t2", "a1", "a2", "cccc", "dt", "ii", "image_path", "magic", "yygggg")
		    VALUES (''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'') RETURNING "id"',
		   tableName, t1, t2, a1, a2, cccc, dt, ii, image_path, magic, yygggg) INTO resultId;
	
	IF resultId = Null
	THEN
		return 0;
	ELSE
        return 1;
	END IF;		
END;
$$;


ALTER FUNCTION meteo.insert_fax(t1 text, t2 text, a1 text, a2 text, cccc text, dt timestamp without time zone, ii integer, image_path text, magic integer, yygggg text) OWNER TO postgres;

--
-- Name: insert_image_satelite(text, text, text, text, integer, text, text, bigint, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_image_satelite(t1 text, t2 text, a1 text, a2 text, ii integer, cccc text, yygggg text, fileid bigint, dt timestamp without time zone, dt_write timestamp without time zone) RETURNS real
    LANGUAGE plpgsql
    AS $$ 
DECLARE
  resultId bigint;
  tableName text;
BEGIN
    tableName := meteo.try_create_image_satelite_partition(dt);
	
	EXECUTE FORMAT( 'INSERT INTO %s ("t1", "t2", "a1", "a2", "ii", "cccc", "yygggg", "fileid", "dt", "dt_write")
					 VALUES (''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'', ''%s'') RETURNING "id"',
				   tableName, t1, t2, a1, a2, ii, cccc, yygggg, fileid, dt, dt_write ) INTO resultId;
	IF resultId = Null
	THEN
		return 0;
	ELSE
        return 1;
	END IF;		
END;
$$;


ALTER FUNCTION meteo.insert_image_satelite(t1 text, t2 text, a1 text, a2 text, ii integer, cccc text, yygggg text, fileid bigint, dt timestamp without time zone, dt_write timestamp without time zone) OWNER TO postgres;

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
-- Name: remove_file_description(text, text, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.remove_file_description(tablename_ text, filename_ text DEFAULT NULL::text, filemd5_ text DEFAULT NULL::text) RETURNS boolean
    LANGUAGE plpgsql
    AS $$DECLARE
  query_ TEXT;
BEGIN
  PERFORM meteo.check_fs_table(tablename_);
  IF filename_ IS NULL AND filemd5_ IS NULL THEN
    RETURN FALSE;
  END IF;
  query_ = FORMAT( 'DELETE FROM meteo.%I WHERE  %L = filename OR %L = filemd5', tablename_, filename_, filemd5_ );
  EXECUTE query_;
  RETURN TRUE;
END;$$;


ALTER FUNCTION meteo.remove_file_description(tablename_ text, filename_ text, filemd5_ text) OWNER TO postgres;

--
-- Name: try_create_documents_partition(timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_documents_partition(dt timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE  
  table_part text;
BEGIN
  table_part := global.try_create_partition_by_dt( 'documents', 'dt', dt);
  
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  END IF;
  	
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'mapid', 'jobid', 'dt', 'hour', 'center', 'model', 'format', 'map_title', 'job_title' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'mapid', 'jobid', 'dt', 'hour', 'center', 'model', 'format', 'map_title', 'job_title' ] );
  END IF;
  
  IF ( global.f_is_constraint_exist( table_part, ARRAY[ 'mapid', 'jobid', 'dt', 'hour', 'center', 'model', 'format', 'map_title', 'job_title' ] ) = FALSE ) THEN
  	PERFORM global.f_create_unique_constraint( table_part, ARRAY[ 'mapid', 'jobid', 'dt', 'hour', 'center', 'model', 'format', 'map_title', 'job_title' ] );
  END IF;
 
 RETURN table_part;
END;
$$;


ALTER FUNCTION meteo.try_create_documents_partition(dt timestamp without time zone) OWNER TO postgres;

--
-- Name: try_create_fax_partition(timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_fax_partition(dt timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE  
  table_part text;
BEGIN

  table_part := global.try_create_partition_by_dt( 'fax', 'dt', dt);
  IF ( global.f_is_index_exist(table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  END IF;
 
 RETURN table_part;
END;
$$;


ALTER FUNCTION meteo.try_create_fax_partition(dt timestamp without time zone) OWNER TO postgres;

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
  RETURN (tablepart_only_, id_);
END;$$;


ALTER FUNCTION meteo.try_create_fs_part(tablemain_ text) OWNER TO postgres;

--
-- Name: try_create_image_satelite_partition(timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_image_satelite_partition(dt timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE  
  table_part text;
BEGIN

  table_part := global.try_create_partition_by_dt( 'image_satelite', 'dt', dt);
  IF ( global.f_is_index_exist( table_part, ARRAY[ 'dt' ] ) = FALSE ) THEN
    PERFORM global.f_create_index( table_part, ARRAY[ 'dt' ] );
  END IF;
 
 RETURN table_part;
END;
$$;


ALTER FUNCTION meteo.try_create_image_satelite_partition(dt timestamp without time zone) OWNER TO postgres;

--
-- Name: update_document(text, text, timestamp without time zone, integer, integer, integer, text, text, text, text, text, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_document(mapid_ text, jobid_ text, dt_ timestamp without time zone, hour_ integer, center_ integer, model_ integer, format_ text, map_title_ text, job_title_ text, path_ text, idfile_ text, dt_write_ timestamp without time zone) RETURNS real
    LANGUAGE plpgsql
    AS $$ 
DECLARE
  resultId bigint;
  tableName text;  
BEGIN
    tableName := meteo.try_create_documents_partition(dt_);
	
	
	EXECUTE FORMAT( 'INSERT INTO %s ("center", "dt", "format", "hour", "job_title", "jobid", "map_title", "mapid", "model", "path", "idfile", "dt_write" )
					 VALUES (%L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L, %L) 
				     ON CONFLICT ( "mapid", "jobid", "dt", "hour", "center", "model", "format", "map_title", "job_title")
				     DO UPDATE SET "path" = %L, "idfile" = %L, "dt_write" = %L
				     RETURNING "id"', 
				     tableName,
				     center_, dt_, format_, hour_, job_title_, jobid_, map_title_, mapid_, model_, path_, idfile_, dt_write_,
				     path_, idfile_, dt_write_) INTO resultId;
	IF resultId = Null
	THEN
		return 0;
	ELSE
        return 1;
	END IF;		
END;
$$;


ALTER FUNCTION meteo.update_document(mapid_ text, jobid_ text, dt_ timestamp without time zone, hour_ integer, center_ integer, model_ integer, format_ text, map_title_ text, job_title_ text, path_ text, idfile_ text, dt_write_ timestamp without time zone) OWNER TO postgres;

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

