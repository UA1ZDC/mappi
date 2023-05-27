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
-- Name: check_tlg_dup(text, text, text[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.check_tlg_dup(md5_ text, source_ text, destination_ text[]) RETURNS bigint
    LANGUAGE plpgsql
    AS $$DECLARE
  idrec_ BIGINT;
  route_ JSONB;
BEGIN
  idrec_ := -1;
  route_ := json_build_object( 'src', source_, 'dst', destination_ );
  WITH updidq AS ( UPDATE meteo.msg SET dup = dup + 1, route = route || route_
				   WHERE md5 = md5_ RETURNING id )
  SELECT id FROM updidq INTO idrec_;
  IF ( idrec_ IS NULL ) THEN
    idrec_ := -1;
  END IF;
  RETURN idrec_;
END;$$;


ALTER FUNCTION meteo.check_tlg_dup(md5_ text, source_ text, destination_ text[]) OWNER TO postgres;

--
-- Name: FUNCTION check_tlg_dup(md5_ text, source_ text, destination_ text[]); Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON FUNCTION meteo.check_tlg_dup(md5_ text, source_ text, destination_ text[]) IS 'Проверка дубликатов телеграмм';


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
-- Name: find_all_typec_areas(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_all_typec_areas() RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
  SELECT * FROM meteo.find_all_typec_areas_curs() into ref_;
  LOOP
    FETCH NEXT FROM ref_ INTO mviews;
       IF NOT FOUND THEN EXIT;END IF;  
       return  next mviews;
  END LOOP;
  
END;
$$;


ALTER FUNCTION meteo.find_all_typec_areas() OWNER TO postgres;

--
-- Name: find_all_typec_areas_curs(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_all_typec_areas_curs() RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   ref refcursor := 'rescursor';
BEGIN  
  OPEN ref FOR SELECT * FROM meteo.typec_area;  
  return ref;
END;
$$;


ALTER FUNCTION meteo.find_all_typec_areas_curs() OWNER TO postgres;

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
-- Name: find_from_msg_viewer(jsonb[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_from_msg_viewer(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.find_from_msg_viewer_curs(filters, sort_column,sort_order,qskip,qslimit ) into ref_;
LOOP
        FETCH NEXT FROM ref_ INTO mviews;
        IF NOT FOUND THEN EXIT;END IF;  
        return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.find_from_msg_viewer(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: find_from_msg_viewer(text, text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_from_msg_viewer(filters text, sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS SETOF record
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
  ref_ refcursor;
  mviews RECORD;
BEGIN
 SELECT * FROM meteo.find_from_msg_viewer_curs(filters, sort_column,sort_order,qskip,qslimit ) into ref_;
LOOP
	FETCH NEXT FROM ref_ INTO mviews;
	IF NOT FOUND THEN EXIT;END IF;	
	return  next mviews;
END LOOP;
END;
$$;


ALTER FUNCTION meteo.find_from_msg_viewer(filters text, sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: find_from_msg_viewer_curs(jsonb[], text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_from_msg_viewer_curs(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   queryTemplate text := 'SELECT *, id FROM meteo.msg AS _id WHERE %s  ORDER BY %s %s LIMIT %s OFFSET %s';
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
        matchString := meteo.parse_json_condition(filters);
  ELSE
    matchString := 'TRUE';
  END IF;
    
  queryString:= format(queryTemplate, matchString, sort_column, sortOrderName, qslimit, qskip);

  OPEN ref FOR EXECUTE queryString;
  
  return ref;
END;
$$;


ALTER FUNCTION meteo.find_from_msg_viewer_curs(filters jsonb[], sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: find_from_msg_viewer_curs(text, text, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.find_from_msg_viewer_curs(filters text, sort_column text, sort_order integer, qskip integer, qslimit integer) RETURNS refcursor
    LANGUAGE plpgsql STABLE
    AS $$
DECLARE
   queryTemplate text := 'SELECT *, id AS _id FROM meteo.msg WHERE %s ORDER BY %s %s LIMIT %s OFFSET %s';
   sortOrderName text:= '';
   queryString text:='';
   ref refcursor := 'rescursor';
BEGIN 
  
  if 1 = sort_order THEN
    sortOrderName = 'ASC';
  ELSE
    sortOrderName = 'DESC';
  END IF;
  
  queryString:= format(queryTemplate, filters, sort_column, sortOrderName, qslimit, qskip);

  OPEN ref FOR EXECUTE queryString;
  
  return ref;
END;

$$;


ALTER FUNCTION meteo.find_from_msg_viewer_curs(filters text, sort_column text, sort_order integer, qskip integer, qslimit integer) OWNER TO postgres;

--
-- Name: get_statistic_by_dest(timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_statistic_by_dest(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
	RETURN query
	select date (dt) as dt,
    jsonb_array_elements(jsonb_array_elements(route)->'dst') as dst,
    T1,t2, count(dt) from meteo.msg where 
	route::jsonb->0->>'dst' is not null 
    and (dt_start_ IS NULL or dt >= dt_start_)  
    and (dt_end_ IS NULL or dt <= dt_end_)  
	group by date (dt),	dst, t1, t2 order by  dt, dst;
 
END;
$$;


ALTER FUNCTION meteo.get_statistic_by_dest(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_statistic_by_src(timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_statistic_by_src(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
	RETURN query select date (dt) as dt,
	jsonb_array_elements(route)->'src' as src,
	T1,t2, count(dt) from meteo.msg where 
	--route::jsonb->0->>'dst' is not null and
    (dt_start_ IS NULL or dt >= dt_start_)  
    and (dt_end_ IS NULL or dt <= dt_end_)  
	group by date (dt),	src, t1, t2 order by  dt, src;
 
END;
$$;


ALTER FUNCTION meteo.get_statistic_by_src(dt_start_ timestamp without time zone, dt_end_ timestamp without time zone) OWNER TO postgres;

--
-- Name: get_tlg(bigint[], text[], text[], text[], text[], text[], text[], integer[], text[], text[], boolean, timestamp without time zone, timestamp without time zone, timestamp without time zone, timestamp without time zone, integer, integer, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_tlg(id_ bigint[], t1_ text[], t2_ text[], a1_ text[], a2_ text[], cccc_ text[], bbb_ text[], ii_ integer[], yygggg_ text[], type_ text[], decoded_ boolean, start_msgdt_ timestamp without time zone, end_msgdt_ timestamp without time zone, start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, sort_ integer, limit_ integer, skip_ integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $_$DECLARE
  ref_ refcursor;
  rec_ RECORD;
  query_ TEXT;
  order_ TEXT; 
BEGIN
  order_ = 'ASC';
  IF ( 0 > sort_ ) THEN
    order_ = 'desc_';
  END IF;
  query_ = FORMAT( 'SELECT * FROM meteo.msg WHERE ( $1 IS NULL OR id = ANY($1) )
                          AND ( $2 IS NULL OR t1 = ANY($2) )
                          AND ( $3 IS NULL OR t2 = ANY($3) )
                          AND ( $4 IS NULL OR a1 = ANY($4) )
                          AND ( $5 IS NULL OR a2 = ANY($5) )
                          AND ( $6 IS NULL OR cccc = ANY($6) )
                          AND ( $7 IS NULL OR bbb = ANY($7) )
                          AND ( $8 IS NULL OR ii = ANY($8) )
                          AND ( $9 IS NULL OR yygggg = ANY($9) )
                          AND ( $10 IS NULL OR type = ANY($10) )
                          AND ( $11 IS NULL OR decoded = $11 )
                          AND ( $12 IS NULL OR $13 IS NULL OR ( msg_dt BETWEEN $12 AND $13 ) )
                          AND ( $14 IS NULL OR $15 IS NULL OR ( dt BETWEEN $14 AND $15 ) )
				 ORDER BY id %s LIMIT $16 OFFSET $17', order_ );
  RETURN QUERY EXECUTE query_ USING id_, t1_, t2_, a1_, a2_, cccc_, bbb_, ii_, yygggg_, type_, decoded_,
                                    start_msgdt_, end_msgdt_, start_dt_, end_dt_, limit_, skip_;

END;
$_$;


ALTER FUNCTION meteo.get_tlg(id_ bigint[], t1_ text[], t2_ text[], a1_ text[], a2_ text[], cccc_ text[], bbb_ text[], ii_ integer[], yygggg_ text[], type_ text[], decoded_ boolean, start_msgdt_ timestamp without time zone, end_msgdt_ timestamp without time zone, start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, sort_ integer, limit_ integer, skip_ integer) OWNER TO postgres;

--
-- Name: get_undecoded_fax_tlg(text[], timestamp without time zone, timestamp without time zone, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.get_undecoded_fax_tlg(type_ text[], start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, limit_ integer) RETURNS SETOF record
    LANGUAGE plpgsql
    AS $_$
DECLARE
  query_ TEXT;
BEGIN

	query_ = FORMAT( 'WITH foo AS (select array_agg(id) as id, t1, t2, a1, a2, ii, cccc, dddd, yygggg, external, 
							array_agg(file_id) as a,array_agg(bbb) as b
							FROM meteo.msg WHERE
								( $1 IS NULL OR $2 IS NULL OR ( dt BETWEEN $1 AND $2 ) )
                                AND ( $3 IS NULL OR type = ANY($3) )
								AND decoded = false 
							GROUP BY t1, t2, a1, a2, ii, cccc, dddd, yygggg, external  LIMIT $4)
	                SELECT unnest(id) as _id, t1, t2, a1, a2, ii, cccc, dddd, yygggg, external,
					unnest(a) as file_id, unnest(b) as bbb
					FROM foo ORDER BY yygggg, bbb ');

  RETURN QUERY EXECUTE query_ USING start_dt_, end_dt_, type_, limit_;

END;
$_$;


ALTER FUNCTION meteo.get_undecoded_fax_tlg(type_ text[], start_dt_ timestamp without time zone, end_dt_ timestamp without time zone, limit_ integer) OWNER TO postgres;

--
-- Name: insert_tlg(bigint, text, text, text, text, integer, text, text, text, text, timestamp without time zone, timestamp without time zone, text, boolean, integer, boolean, text, text, integer, bigint, integer, boolean, boolean, jsonb, integer); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.insert_tlg(id_ bigint, t1_ text, t2_ text, a1_ text, a2_ text, ii_ integer, cccc_ text, dddd_ text, bbb_ text, yygggg_ text, dt_ timestamp without time zone, msg_dt_ timestamp without time zone, md5_ text, external_ boolean, prior_ integer, bin_ boolean, format_ text, type_ text, size_ integer, file_id_ bigint, globalfile_id_ integer, addr_tlg_ boolean, confirm_tlg_ boolean, route_ jsonb, cor_number_ integer) RETURNS record
    LANGUAGE plpgsql
    AS $_$DECLARE
 table_name_ text;
 query_ text;
 dup_ INTEGER;
BEGIN
  SELECT meteo.try_create_msg_partition('msg',msg_dt_)  INTO table_name_;
  query_ :=  format('INSERT INTO %s (
    id, t1, t2, a1, a2, ii, cccc, dddd, bbb, yygggg, dt, msg_dt, md5, external,
    prior, bin, format, type, size, file_id, globalfile_id, addr_tlg, confirm_tlg, route, cor_number, dup
  )
  VALUES ( $1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21,$22,$23,$24,$25,$26)
  ON CONFLICT (md5) DO UPDATE SET dup = %s.dup + 1 RETURNING dup', table_name_, table_name_);
  EXECUTE query_ USING
    id_, t1_, t2_, a1_, a2_, ii_, cccc_, dddd_, bbb_, yygggg_, dt_, msg_dt_, md5_, external_,
    prior_, bin_, format_, type_, size_, file_id_, globalfile_id_, addr_tlg_, confirm_tlg_, 
    route_, cor_number_, 0 INTO dup_;
   RETURN (id_, dup_);
END;$_$;


ALTER FUNCTION meteo.insert_tlg(id_ bigint, t1_ text, t2_ text, a1_ text, a2_ text, ii_ integer, cccc_ text, dddd_ text, bbb_ text, yygggg_ text, dt_ timestamp without time zone, msg_dt_ timestamp without time zone, md5_ text, external_ boolean, prior_ integer, bin_ boolean, format_ text, type_ text, size_ integer, file_id_ bigint, globalfile_id_ integer, addr_tlg_ boolean, confirm_tlg_ boolean, route_ jsonb, cor_number_ integer) OWNER TO postgres;

--
-- Name: parse_field_condition_object(jsonb, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.parse_field_condition_object(filters jsonb, fieldname text) RETURNS text
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
   --raise notice 'NULL!';
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
	    fieldValueString := meteo.parse_field_condition_value_object(fieldValue);	
	ELSE IF fieldType = 'array'::text
	THEN
		fieldValueString := meteo.parse_field_condition_value_array(fieldValue);	
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
  
  resultValue := format('( %s )', array_to_string( fieldConditions, ' AND ' ));
  return resultValue;
 END;
$_$;


ALTER FUNCTION meteo.parse_field_condition_object(filters jsonb, fieldname text) OWNER TO postgres;

--
-- Name: parse_field_condition_value_array(jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.parse_field_condition_value_array(obj jsonb) RETURNS text
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


ALTER FUNCTION meteo.parse_field_condition_value_array(obj jsonb) OWNER TO postgres;

--
-- Name: parse_field_condition_value_object(jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.parse_field_condition_value_object(obj jsonb) RETURNS text
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


ALTER FUNCTION meteo.parse_field_condition_value_object(obj jsonb) OWNER TO postgres;

--
-- Name: parse_json_condition(jsonb[]); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.parse_json_condition(filters jsonb[]) RETURNS text
    LANGUAGE plpgsql STABLE
    AS $$ 
DECLARE
  conditions text[];
 BEGIN
   FOR i IN array_lower(filters, 1) .. array_upper(filters, 1)
   LOOP
     conditions = conditions || meteo.parse_json_condition(filters[i]);
   END LOOP;
 
   return array_to_string( conditions, ' AND ' );
 END;
$$;


ALTER FUNCTION meteo.parse_json_condition(filters jsonb[]) OWNER TO postgres;

--
-- Name: parse_json_condition(jsonb); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.parse_json_condition(filters jsonb) RETURNS text
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
	   conditionSqls := conditionSqls ||  meteo.parse_json_logical(jsonb_extract_path(filters, ckey), ckey);	  
	   continue;
     END IF;
	 
	 fieldValue:= jsonb_extract_path(filters, ckey);     
     fieldType := jsonb_typeof(fieldValue);
     IF fieldType = 'object'::text
     THEN       
       conditionSqls := conditionSqls || meteo.parse_field_condition_object(fieldValue, ckey);
	   continue;
	 END IF;
	 
	 IF '_id' = ckey
	 THEN
	   ckey = 'id';
	 END IF;
	 
	 conditionSqls := conditionSqls || format('"%s" = %s', ckey, fieldValue);
   END LOOP;

   return array_to_string( conditionSqls, ' AND ' );

 END;
$_$;


ALTER FUNCTION meteo.parse_json_condition(filters jsonb) OWNER TO postgres;

--
-- Name: parse_json_logical(jsonb, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.parse_json_logical(inner_values jsonb, logical_parent text) RETURNS text
    LANGUAGE plpgsql STABLE
    AS $_$
DECLARE
   conditionLogicalValues text[];
   arrayElement jsonb;
 BEGIN
   FOR arrayElement IN SELECT jsonb_array_elements(inner_values)
   LOOP
     conditionLogicalValues := conditionLogicalValues || meteo.parse_json_condition(arrayElement);
   END LOOP;
   
   if '$and' = logical_parent
   THEN
     return format('( %s )', array_to_string( conditionLogicalValues, ' AND ' ));
   ELSE
     return format('( %s )', array_to_string( conditionLogicalValues, ' OR ' ));
   END IF;
   
 END;
$_$;


ALTER FUNCTION meteo.parse_json_logical(inner_values jsonb, logical_parent text) OWNER TO postgres;

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
END;$$;


ALTER FUNCTION meteo.try_create_fs_part(tablemain_ text) OWNER TO postgres;

--
-- Name: try_create_msg_partition(text, timestamp without time zone); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.try_create_msg_partition(table_main_ text, dt_ timestamp without time zone) RETURNS text
    LANGUAGE plpgsql
    AS $$
DECLARE
    table_main        TEXT  ;--самая главная таблица
    table_part_yyyy_mm_dd          TEXT       = '';--таблица по дням (партиция за сутки)
    rec_                RECORD;
    table_part_yyyy_mm_dd_only text;
	query_ text;
BEGIN

--rrr :=  clock_timestamp();
 --RAISE NOTICE 'start time operation: %', rrr;
   -- Даём имя партиции --------------------------------------------------
  
  table_main := 'meteo.'||table_main_; --самая главная таблица
  table_part_yyyy_mm_dd_only :=   table_main_  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)
  table_part_yyyy_mm_dd :=   table_main  || '_' || to_char(dt_, 'YYYY_MM_DD'); --таблица по дням (партиция за сутки)

 
    -- Проверяем партицию за cутки на существование --------------------------------

      PERFORM 1 FROM pg_tables WHERE tablename = table_part_yyyy_mm_dd_only LIMIT 1;
      -- Если её ещё нет, то 
      IF NOT FOUND
      THEN
          query_ := format(
          -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
            'CREATE UNLOGGED TABLE %s
          (
		  CONSTRAINT %I_oninsert UNIQUE (md5),
           CONSTRAINT %I_created_check
                CHECK ( msg_dt BETWEEN %L::timestamp
                                AND %L::timestamp + interval ''1 day'' - interval ''1 sec'')
          )
          INHERITS (%s)
          WITH (OIDS = FALSE, FILLFACTOR = 90)',
			 table_part_yyyy_mm_dd,table_part_yyyy_mm_dd_only,table_part_yyyy_mm_dd_only,
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
	IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_id' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 'id'] );
	END IF;
	IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_md5' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 'md5'] );
	END IF;
	IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_msg_dt' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 'msg_dt'] );
	END IF;
	IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_dt' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 'dt'] );
	END IF;
	IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_t1_t2_a1_a2_cccc_dddd' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 't1','t2','a1','a2','cccc','dddd'] );
	END IF;
	IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_decoded' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 'decoded'] );
	END IF;
	IF ( meteo.f_is_index_exist(table_part_yyyy_mm_dd_only || '_cccc_dddd_addr_tlg' ) = FALSE ) THEN
      PERFORM meteo.f_create_index(table_part_yyyy_mm_dd, ARRAY[ 'cccc','dddd', 'addr_tlg'] );
	END IF;

 END IF;

  -- Вставляем данные в партицию ----------------------------------------
  -- EXECUTE 'INSERT INTO ' || table_part_yyyy_mm_dd || ' SELECT ((' || quote_literal(NEW) || ')::' || table_main || ').*';
  --raise notice 'query = %','INSERT INTO ' || table_part_hh || ' SELECT ((' || quote_literal(NEW) || ')::' || table_part_yyyy_mm_dd || ').*';

  RETURN table_part_yyyy_mm_dd;
END;
$$;


ALTER FUNCTION meteo.try_create_msg_partition(table_main_ text, dt_ timestamp without time zone) OWNER TO postgres;

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
-- Name: update_tlg_set_decoded(bigint[], boolean); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.update_tlg_set_decoded(id_ bigint[], decoded_ boolean) RETURNS record
    LANGUAGE plpgsql
    AS $$DECLARE
  count_ INTEGER;
  allcount_ INTEGER;
  ok_ INTEGER;
BEGIN
  WITH q AS (
    UPDATE meteo.msg SET decoded = decoded_ WHERE id = ANY(id_)
	RETURNING 1
  )
  SELECT INTO count_ count(*) FROM q;
  ok_ = 1;
  SELECT INTO allcount_ array_length( id_, 1 );
  RETURN (count_, allcount_, ok_);
END;$$;


ALTER FUNCTION meteo.update_tlg_set_decoded(id_ bigint[], decoded_ boolean) OWNER TO postgres;

--
-- PostgreSQL database dump complete
--

