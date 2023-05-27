--
-- PostgreSQL database dump
--

-- Dumped from database version 11.13 (Debian 11.13-1.pgdg100+1)
-- Dumped by pg_dump version 11.13 (Debian 11.13-1.pgdg100+1)

SET statement_timeout = 0;
SET lock_timeout = 0;
 
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
 

--
-- Name: global; Type: SCHEMA; Schema: -; Owner: postgres
--

CREATE SCHEMA global;


ALTER SCHEMA global OWNER TO postgres;

--
-- Name: meteo; Type: SCHEMA; Schema: -; Owner: postgres
--

CREATE SCHEMA meteo;


ALTER SCHEMA meteo OWNER TO postgres;

--
-- Name: postgis; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS postgis WITH SCHEMA public;


--
-- Name: EXTENSION postgis; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION postgis IS 'PostGIS geometry, geography, and raster spatial types and functions';


--
-- Name: delete_data_expired(text, text, text, timestamp without time zone, text, timestamp without time zone, text, text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.delete_data_expired(msg_table text, text, dt_field text, dt timestamp without time zone, dt_write_field text, dt_write timestamp without time zone, fs_table text, fs_field text) RETURNS record
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


ALTER FUNCTION global.delete_data_expired(msg_table text, text, dt_field text, dt timestamp without time zone, dt_write_field text, dt_write timestamp without time zone, fs_table text, fs_field text) OWNER TO postgres;

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
        RAISE NOTICE 'ОШИБКА - удаления партиции таблицы %', dst;
        RETURN res;
    END IF;

    RETURN res;
END;
$$;


ALTER FUNCTION global.drop_table_partition(table_name text) OWNER TO postgres;

--
-- Name: partitioning_range_day(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.partitioning_range_day() RETURNS trigger
    LANGUAGE plpgsql
    AS $_$
DECLARE
    master TEXT = TG_TABLE_NAME;
	children TEXT;
	item RECORD;
	query TEXT;
BEGIN
    children := FORMAT('%s_%s_%s_%s',
		master,
		date_part('year', NEW.date_start)::TEXT,
        date_part('month', NEW.date_start)::TEXT,
		date_part('day', NEW.date_start)::TEXT);

	PERFORM 1 FROM pg_class WHERE relname = children LIMIT 1;
	IF NOT FOUND THEN
		-- создание партиции
		query := FORMAT('CREATE TABLE %s (CONSTRAINT %s_check CHECK (date_start BETWEEN ''%s'' AND ''%s'')) INHERITS (%s) WITH (OIDS = FALSE)',
			TG_TABLE_SCHEMA || '.' || children,
			master,
			date_trunc('day', NEW.date_start),
			date_trunc('day', NEW.date_start) + interval '1 day' - interval '1 sec',
			TG_TABLE_SCHEMA || '.' || master);
		EXECUTE query;

        -- привелегии базовой таблицы копируются в партицию 
      	query := FORMAT('SELECT grantee, string_agg(privilege_type, '', '') AS priv FROM information_schema.role_table_grants WHERE table_name = ''%s'' GROUP BY grantee',
			TG_TABLE_SCHEMA || '.' || master);
		FOR item IN EXECUTE query LOOP
			query := FORMAT('GRANT %s ON %s TO %s',
				item.priv,
				TG_TABLE_SCHEMA || '.' || children,
				item.grantee);
            EXECUTE query;
        END LOOP;
	END IF;

	EXECUTE FORMAT('INSERT INTO %s SELECT $1.*', TG_TABLE_SCHEMA || '.' || children) USING NEW;

	RETURN NULL;
END;
$_$;


ALTER FUNCTION meteo.partitioning_range_day() OWNER TO postgres;

--
-- Name: pretreatment_insert(bigint, integer, integer, timestamp without time zone, timestamp without time zone, boolean, character varying); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.pretreatment_insert(session bigint, instrument_id integer, channel_number integer, date_start_ timestamp without time zone, date_end timestamp without time zone, calibration boolean, fpath character varying) RETURNS integer
    LANGUAGE plpgsql
    AS $_$
DECLARE
	istrument_channel_id INTEGER;
	rows INTEGER;
	query TEXT;
BEGIN

	IF session <= 0
	THEN
		SELECT id INTO session FROM meteo.session WHERE meteo.session.date_start = date_start ORDER BY id DESC LIMIT 1;
	END IF;

	EXECUTE FORMAT('SELECT id FROM meteo.instrument_channel_view WHERE instrument_id = $1 AND channel_number = $2')
	    INTO istrument_channel_id USING instrument_id, channel_number;
	IF istrument_channel_id IS NULL
	THEN
		RAISE NOTICE 'ОШИБКА - прибор [%] не имеет канала [%]', instrument_id, channel_number;
		RETURN 0;
	END IF;

	query := FORMAT('INSERT INTO meteo.pretreatment VALUES(%s, %s, %L, %L, %L, %L)',
		session, istrument_channel_id, date_start_, date_end, calibration, fpath);
	EXECUTE query;

	RETURN 1;
END
$_$;


ALTER FUNCTION meteo.pretreatment_insert(session bigint, instrument_id integer, channel_number integer, date_start_ timestamp without time zone, date_end timestamp without time zone, calibration boolean, fpath character varying) OWNER TO postgres;

--
-- Name: pretreatment_partitioning_range_day(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.pretreatment_partitioning_range_day() RETURNS trigger
    LANGUAGE plpgsql
    AS $_$
DECLARE
    master TEXT = TG_TABLE_NAME;
	children TEXT;
	item RECORD;
	query TEXT;
BEGIN
    children := FORMAT('%s_%s_%s_%s',
		master,
		date_part('year', NEW.date_start)::TEXT,
        date_part('month', NEW.date_start)::TEXT,
		date_part('day', NEW.date_start)::TEXT);

	PERFORM 1 FROM pg_class WHERE relname = children LIMIT 1;
	IF NOT FOUND THEN
		-- создание партиции
		query := FORMAT('CREATE TABLE %s (CONSTRAINT %s_check CHECK (date_start BETWEEN ''%s'' AND ''%s'')) INHERITS (%s) WITH (OIDS = FALSE)',
			TG_TABLE_SCHEMA || '.' || children,
			master,
			date_trunc('day', NEW.date_start),
			date_trunc('day', NEW.date_start) + interval '1 day' - interval '1 sec',
			TG_TABLE_SCHEMA || '.' || master);
		EXECUTE query;

        -- привелегии базовой таблицы копируются в партицию 
      	query := FORMAT('SELECT grantee, string_agg(privilege_type, '', '') AS priv FROM information_schema.role_table_grants WHERE table_name = ''%s'' GROUP BY grantee',
			TG_TABLE_SCHEMA || '.' || master);
		FOR item IN EXECUTE query LOOP
			query := FORMAT('GRANT %s ON %s TO %s',
				item.priv,
				TG_TABLE_SCHEMA || '.' || children,
				item.grantee);
            EXECUTE query;
        END LOOP;

		-- внешнии ключи
		query := FORMAT('ALTER TABLE %s ADD FOREIGN KEY (session) REFERENCES meteo.session (id) ON UPDATE NO ACTION ON DELETE SET NULL NOT VALID', TG_TABLE_SCHEMA || '.' || children);
		EXECUTE query;
		
		query := FORMAT('ALTER TABLE %s ADD FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel (id)', TG_TABLE_SCHEMA || '.' || children);
		EXECUTE query;
	END IF;

	EXECUTE FORMAT('INSERT INTO %s SELECT $1.*', TG_TABLE_SCHEMA || '.' || children) USING NEW;

	RETURN NULL;
END;
$_$;


ALTER FUNCTION meteo.pretreatment_partitioning_range_day() OWNER TO postgres;

--
-- Name: session_insert(character varying, timestamp without time zone, timestamp without time zone, integer, integer, integer, real, character varying, character varying, text); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.session_insert(satellite_name character varying, date_start timestamp without time zone, date_end timestamp without time zone, date_start_offset integer, revol integer, direction integer, elevat_max real, site_name character varying, fpath character varying, tle text) RETURNS bigint
    LANGUAGE plpgsql
    AS $_$
DECLARE
	satellite_id INTEGER;
	site_id INTEGER;
	session_id BIGINT;
	query TEXT;
BEGIN
	EXECUTE FORMAT('SELECT id FROM meteo.satellite WHERE name = $1') INTO satellite_id USING satellite_name;
	RAISE NOTICE '% = %', satellite_name, satellite_id;
	IF satellite_id IS NULL
	THEN
		RAISE NOTICE 'ОШИБКА - спутник [%] не найден', satellite_name;
		RETURN -1;
	END IF;

	EXECUTE FORMAT('SELECT id FROM meteo.site WHERE name = $1') INTO site_id USING site_name;
	RAISE NOTICE '% = %', site_name, site_id;
	IF site_id IS NULL
	THEN
		RAISE NOTICE 'ОШИБКА - пункт приема [%] не найден', site_name;
		RETURN -1;
	END IF;

	query := FORMAT('INSERT INTO meteo.session VALUES(DEFAULT, %s, %L, %L, %s, %s, %s, %s, %s, %L, %L) RETURNING id',
		satellite_id, date_start, date_end, date_start_offset, revol, direction, elevat_max, site_id, fpath, tle);
	EXECUTE query INTO session_id;

	RETURN session_id;
END
$_$;


ALTER FUNCTION meteo.session_insert(satellite_name character varying, date_start timestamp without time zone, date_end timestamp without time zone, date_start_offset integer, revol integer, direction integer, elevat_max real, site_name character varying, fpath character varying, tle text) OWNER TO postgres;

--
-- Name: thematic_partitioning_range_day(); Type: FUNCTION; Schema: meteo; Owner: postgres
--

CREATE FUNCTION meteo.thematic_partitioning_range_day() RETURNS trigger
    LANGUAGE plpgsql
    AS $_$
DECLARE
    master TEXT = TG_TABLE_NAME;
	children TEXT;
	item RECORD;
	query TEXT;
BEGIN
    children := FORMAT('%s_%s_%s_%s',
		master,
		date_part('year', NEW.date_start)::TEXT,
        date_part('month', NEW.date_start)::TEXT,
		date_part('day', NEW.date_start)::TEXT);

	PERFORM 1 FROM pg_class WHERE relname = children LIMIT 1;
	IF NOT FOUND THEN
		-- создание партиции
		query := FORMAT('CREATE TABLE %s (CONSTRAINT %s_check CHECK (date_start BETWEEN ''%s'' AND ''%s'')) INHERITS (%s) WITH (OIDS = FALSE)',
			TG_TABLE_SCHEMA || '.' || children,
			master,
			date_trunc('day', NEW.date_start),
			date_trunc('day', NEW.date_start) + interval '1 day' - interval '1 sec',
			TG_TABLE_SCHEMA || '.' || master);
		EXECUTE query;

        -- привелегии базовой таблицы копируются в партицию 
      	query := FORMAT('SELECT grantee, string_agg(privilege_type, '', '') AS priv FROM information_schema.role_table_grants WHERE table_name = ''%s'' GROUP BY grantee',
			TG_TABLE_SCHEMA || '.' || master);
		FOR item IN EXECUTE query LOOP
			query := FORMAT('GRANT %s ON %s TO %s',
				item.priv,
				TG_TABLE_SCHEMA || '.' || children,
				item.grantee);
            EXECUTE query;
        END LOOP;

		-- внешнии ключи
		query := FORMAT('ALTER TABLE %s ADD FOREIGN KEY (session) REFERENCES meteo.session (id) ON UPDATE NO ACTION ON DELETE SET NULL NOT VALID', TG_TABLE_SCHEMA || '.' || children);
		EXECUTE query;
		
		query := FORMAT('ALTER TABLE %s ADD FOREIGN KEY (type) REFERENCES meteo.thematic_type (id)', TG_TABLE_SCHEMA || '.' || children);
		EXECUTE query;
	END IF;

	EXECUTE FORMAT('INSERT INTO %s SELECT $1.*', TG_TABLE_SCHEMA || '.' || children) USING NEW;

	RETURN NULL;
END;
$_$;


ALTER FUNCTION meteo.thematic_partitioning_range_day() OWNER TO postgres;

--
-- Name: clear_all(timestamp without time zone); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.clear_all(_datetime timestamp without time zone) RETURNS void
    LANGUAGE plpgsql
    AS $$DECLARE
BEGIN
  --perform * from clear_sessions(_datetime);
  DELETE FROM public.sessions where date_start <= _datetime;
  perform * from clear_preprocess(_datetime);
  perform * from clear_theme(_datetime);

END;
$$;


ALTER FUNCTION public.clear_all(_datetime timestamp without time zone) OWNER TO postgres;

--
-- Name: clear_preprocess(timestamp without time zone); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.clear_preprocess(_datetime timestamp without time zone) RETURNS timestamp without time zone
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
  PERFORM drop_inherited('preprocess', _datetime);
  RETURN _datetime;
END;
$$;


ALTER FUNCTION public.clear_preprocess(_datetime timestamp without time zone) OWNER TO postgres;

--
-- Name: clear_sessions(timestamp without time zone); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.clear_sessions(_datetime timestamp without time zone) RETURNS timestamp without time zone
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
  PERFORM drop_inherited('sessions', _datetime);
  RETURN _datetime;
END;
$$;


ALTER FUNCTION public.clear_sessions(_datetime timestamp without time zone) OWNER TO postgres;

--
-- Name: clear_theme(timestamp without time zone); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.clear_theme(_datetime timestamp without time zone) RETURNS timestamp without time zone
    LANGUAGE plpgsql
    AS $$
DECLARE
BEGIN
  PERFORM drop_inherited('theme', _datetime);
  RETURN _datetime;
END;
$$;


ALTER FUNCTION public.clear_theme(_datetime timestamp without time zone) OWNER TO postgres;

--
-- Name: drop_inherited(text, timestamp without time zone); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.drop_inherited(_table_master text, _datetime timestamp without time zone) RETURNS integer
    LANGUAGE plpgsql
    AS $$DECLARE
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
END;$$;


ALTER FUNCTION public.drop_inherited(_table_master text, _datetime timestamp without time zone) OWNER TO postgres;

--
-- Name: partitioing_range_month(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.partitioing_range_month() RETURNS trigger
    LANGUAGE plpgsql
    AS $_$
DECLARE
    parent TEXT = TG_RELNAME;
    children TEXT;
    item RECORD;		
	query TEXT;
BEGIN
    children := parent || '_' || date_part('year', NEW.date_start)::TEXT || '_' || date_part('month', NEW.date_start)::TEXT;
    RAISE NOTICE '%', children;
    --PERFORM 1 FROM pg_class WHERE relname = children LIMIT 1;
    --IF NOT FOUND
    --THEN
	    -- создание партиции
	    --query := FORMAT('CREATE TABLE %s CONSTRAINT %s_check CHECK (date_start = BETWEEN %s AND %s) INHERITS (%s) WITH (OIDS = FALSE)',
		 --   children,
		--	children,
		--	date_trunc('month', NEW.date_start),
		--	parent);
		--RAISE NOTICE 's', query;
		--EXECUTE query;

        -- привелегии базовой таблицы копируются в партицию 
      --  query := FORMAT('SELECT grantee, string_agg(privilege_type, '', '') AS priv FROM information_schema.role_table_grants WHERE table_name = %s GROUP BY grantee', parent);
        --RAISE NOTICE 's', query;
		--FOR item IN EXECUTE query LOOP
		  --  query := FORMAT('GRANT %s ON %s TO %s';
			--	item.priv,
				--children
				--item.grantee);
			--RAISE NOTICE 's', query;
            --EXECUTE query
        --END LOOP;

		-- создание индекса для партиции
        --PERFORM 1 FROM information_schema.columns WHERE table_name = parent AND column_name = 'forecast_time';
        --IF FOUND
        --THEN
		--    query := FORMAT('CREATE INDEX %s_index ON %s USING btree (forecast_time)',
		--	   	children,
		--		children);
		--	RAISE NOTICE 's', query;
        --    EXECUTE query
		--END IF;

        --PERFORM 1 FROM information_schema.columns WHERE table_name = parent AND column_name = 'findex';
        --IF FOUND
        --THEN
        --    EXECUTE 'CREATE INDEX ' || table_part || '_id_index ON ' || table_part || ' USING btree (findex)';
        --    EXECUTE 'ALTER TABLE ' || table_part || ' ADD CONSTRAINT pkey_' || table_part || ' PRIMARY KEY (findex)';
        --END IF;
    --END IF;
 
    -- INSERT INTO measurement_y2006m02 VALUES (NEW.*);
    -- EXECUTE 'INSERT INTO ' || table_part || ' SELECT ((' || quote_literal(NEW) || ')::' || table_master || ').*';
	--EXECUTE INSERT children VALUES (NEW.*);
	query := FORMAT('INSERT INTO %s SELECT $1.* USING NEW', children);
	RAISE NOTICE '%', query;
	EXECUTE query; 

    RETURN NULL;
END;
$_$;


ALTER FUNCTION public.partitioing_range_month() OWNER TO postgres;

--
-- Name: update_preprocess(bigint, integer, integer, boolean, text, timestamp without time zone, timestamp without time zone); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.update_preprocess(_session_id bigint, _instrument_type integer, _channel integer, _calibration boolean, _path text, _date_start timestamp without time zone, _date_end timestamp without time zone) RETURNS boolean
    LANGUAGE plpgsql
    AS $$  declare
	id_ bigint;
	query_ text;
	suffix_ text;
	channel_id_ bigint;
begin
	select channels.id into channel_id_ from channels, instruments, instr_ch where 
	instr_ch.instr_type = _instrument_type
	and channels.id = instr_ch.channel_id
	and channels.channel = _channel;

	select index into id_ from preprocess where
	session_id = _session_id and
	date_start = _date_start AND
	date_end = _date_end AND
	calibration = _calibration and
	channel_id = channel_id_;

	update sessions set date_start = _date_start, date_end = _date_end, preprocessed = true
	where index = _session_id;

	if (id_ is not null) then
	query_ = 'UPDATE preprocess SET ';
	suffix_ = ' path = ''' || _path || ''' ';
	query_ = query_ || suffix_ || '
		WHERE index = ' || id_ || ' ';
	else
	suffix_ = ' path ';
	query_ = ' INSERT INTO preprocess(
				session_id,
				date_start,
				date_end,
				calibration,
				channel_id, '
				|| suffix_ || '
				)
			values (
			''' || _session_id || ''',
			''' || _date_start || ''',
			''' || _date_end || ''',
			''' || _calibration || ''',
			''' || channel_id_ || ''',
			''' || _path || ''');';
	end if;
	execute query_;

	return true;
end;
$$;


ALTER FUNCTION public.update_preprocess(_session_id bigint, _instrument_type integer, _channel integer, _calibration boolean, _path text, _date_start timestamp without time zone, _date_end timestamp without time zone) OWNER TO postgres;

--
-- Name: update_sessions(text, timestamp without time zone, timestamp without time zone, text, text, boolean, text); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.update_sessions(_satellite_name text, _date_start timestamp without time zone, _date_end timestamp without time zone, _site text, _fpath text, _preprocessed boolean, _tle text) RETURNS bigint
    LANGUAGE plpgsql
    AS $$
DECLARE
	id_ BIGINT;
	query_ TEXT;
	suffix_ TEXT;
BEGIN
	SELECT index INTO id_ FROM sessions WHERE
	satellite_name = _satellite_name AND
	date_start = _date_start AND
	date_end = _date_end AND
	site = _site AND
	preprocessed = _preprocessed;

	IF (id_ IS NOT NULL ) THEN 
	query_ = 'UPDATE sessions SET ';
	suffix_ = ' fpath = ''' || _fpath || ''' ';
	query_ = query_ || suffix_ || '
		WHERE index = ' || id_ || ' ';
	ELSE 
	suffix_ = ' fpath ';
	query_ = ' INSERT INTO sessions(
				satellite_name,
				date_start,
				date_end,
				site,
				preprocessed,
				tle, '
				|| suffix_ || '
				)
			values (
			''' || _satellite_name	 || ''',
			''' || _date_start || ''',
			''' || _date_end || ''',
			''' || _site || ''',
			''' || _preprocessed || ''',
			''' || _tle || ''',
			''' || _fpath || ''');';
	END IF;
	EXECUTE query_;

	SELECT index into id_ from sessions where
	satellite_name = _satellite_name and
	date_start = _date_start and
	date_end = _date_end and
	site = _site and
	fpath = _fpath;

	RETURN id_;
END;
$$;


ALTER FUNCTION public.update_sessions(_satellite_name text, _date_start timestamp without time zone, _date_end timestamp without time zone, _site text, _fpath text, _preprocessed boolean, _tle text) OWNER TO postgres;

--
-- Name: update_thematic(text, text, bigint, integer, integer, timestamp without time zone, timestamp without time zone, integer); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.update_thematic(_name text, _path text, _session_id bigint, _instr_type integer, _channel_number integer, _dts timestamp without time zone, _dte timestamp without time zone, _type integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$declare 
	id_ bigint;
	query_ text;
	suffix_ text;
	channel_id bigint;
begin
	select index into id_ from theme where
		th_name = _name and
		session_id = _session_id and
		path = _path;
		
	channel_id = NULL;

	if (_channel_number is not null) then
		select id into channel_id from channels, instr_ch where
		instr_ch.instr_type = _instr_type AND instr_ch.channel_id = channels.id AND channels.channel = _channel_number;	
	end if;

	if ( id_ is not null ) then
	query_ = 'UPDATE theme SET ';
	suffix_ = ' path = ''' || _path || ''' ';
	query_ = query_ || suffix_ || ' WHERE index = ' || id_ || ' ';
	else	
	query_ = ' INSERT INTO theme(
				th_name,
				date_start,
				date_end,
				path,
				session_id,
				instr_type,
				channel_id,
				type
				)
			values (
				''' || _name || ''',
				''' || _dts || ''',
				''' || _dte || ''',
				''' || _path || ''',
				''' || _session_id || ''',
				''' || _instr_type || ''',
				''' || channel_id || ''',
				''' || _type || '''
				);';
	end if;
	execute query_;

	return true;
end;
$$;


ALTER FUNCTION public.update_thematic(_name text, _path text, _session_id bigint, _instr_type integer, _channel_number integer, _dts timestamp without time zone, _dte timestamp without time zone, _type integer) OWNER TO postgres;

--
-- Name: update_theme(text, text, bigint, integer, timestamp without time zone, timestamp without time zone, integer); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION public.update_theme(_name text, _path text, _session_id bigint, _instr_type integer, _dts timestamp without time zone, _dte timestamp without time zone, _type integer) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
declare 
	id_ bigint;
	query_ text;
	suffix_ text;
	channel_id bigint;
begin
	select index into id_ from theme where
	th_name = _name and
	session_id = _session_id and
	path = _path;

	if ( id_ is not null ) then
	query_ = 'UPDATE theme SET ';
	suffix_ = ' path = ''' || _path || ''' ';
	query_ = query_ || suffix_ || ' WHERE index = ' || id_ || ' ';
	else	
	query_ = ' INSERT INTO theme(
				th_name,
				date_start,
				date_end,
				path,
				session_id,
				instr_type,
				type
				)
			values (
				''' || _name || ''',
				''' || _dts || ''',
				''' || _dte || ''',
				''' || _path || ''',
				''' || _session_id || ''',
				''' || _instr_type || ''',
				''' || _type || '''
				);';
	end if;
	execute query_;

	return true;
end;
$$;


ALTER FUNCTION public.update_theme(_name text, _path text, _session_id bigint, _instr_type integer, _dts timestamp without time zone, _dte timestamp without time zone, _type integer) OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: channel; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.channel (
    id integer NOT NULL,
    number integer NOT NULL,
    lambda_min real,
    lambda_center real,
    lambda_max real,
    alias character varying,
    name character varying
);


ALTER TABLE meteo.channel OWNER TO postgres;

--
-- Name: channel_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.channel_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.channel_id_seq OWNER TO postgres;

--
-- Name: channel_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.channel_id_seq OWNED BY meteo.channel.id;


--
-- Name: instrument; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.instrument (
    id integer NOT NULL,
    name character varying NOT NULL
);


ALTER TABLE meteo.instrument OWNER TO postgres;

--
-- Name: instrument_channel; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.instrument_channel (
    id integer NOT NULL,
    instrument integer NOT NULL,
    channel integer NOT NULL
);


ALTER TABLE meteo.instrument_channel OWNER TO postgres;

--
-- Name: instrument_channel_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.instrument_channel_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.instrument_channel_id_seq OWNER TO postgres;

--
-- Name: instrument_channel_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.instrument_channel_id_seq OWNED BY meteo.instrument_channel.id;


--
-- Name: instrument_channel_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.instrument_channel_view AS
 SELECT instrument_channel.id,
    instrument.id AS instrument_id,
    instrument.name AS instrument_name,
    channel.number AS channel_number,
    channel.alias AS channel_alias
   FROM ((meteo.instrument_channel
     LEFT JOIN meteo.instrument ON ((instrument_channel.instrument = instrument.id)))
     LEFT JOIN meteo.channel ON ((instrument_channel.channel = channel.id)));


ALTER TABLE meteo.instrument_channel_view OWNER TO postgres;

--
-- Name: instrument_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.instrument_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.instrument_id_seq OWNER TO postgres;

--
-- Name: instrument_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.instrument_id_seq OWNED BY meteo.instrument.id;


--
-- Name: pretreatment; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.pretreatment (
    session bigint,
    instrument_channel integer NOT NULL,
    date_start timestamp without time zone NOT NULL,
    date_end timestamp without time zone NOT NULL,
    calibration boolean,
    fpath character varying NOT NULL
);


ALTER TABLE meteo.pretreatment OWNER TO postgres;

--
-- Name: pretreatment_2021_10_19; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.pretreatment_2021_10_19 (
    CONSTRAINT pretreatment_check CHECK (((date_start >= '2021-10-19 00:00:00'::timestamp without time zone) AND (date_start <= '2021-10-19 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.pretreatment);


ALTER TABLE meteo.pretreatment_2021_10_19 OWNER TO postgres;

--
-- Name: pretreatment_2021_10_20; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.pretreatment_2021_10_20 (
    CONSTRAINT pretreatment_check CHECK (((date_start >= '2021-10-20 00:00:00'::timestamp without time zone) AND (date_start <= '2021-10-20 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.pretreatment);


ALTER TABLE meteo.pretreatment_2021_10_20 OWNER TO postgres;

--
-- Name: pretreatment_2021_10_21; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.pretreatment_2021_10_21 (
    CONSTRAINT pretreatment_check CHECK (((date_start >= '2021-10-21 00:00:00'::timestamp without time zone) AND (date_start <= '2021-10-21 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.pretreatment);


ALTER TABLE meteo.pretreatment_2021_10_21 OWNER TO postgres;

--
-- Name: pretreatment_2021_10_31; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.pretreatment_2021_10_31 (
    CONSTRAINT pretreatment_check CHECK (((date_start >= '2021-10-31 00:00:00'::timestamp without time zone) AND (date_start <= '2021-10-31 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.pretreatment);


ALTER TABLE meteo.pretreatment_2021_10_31 OWNER TO postgres;

--
-- Name: pretreatment_2021_3_1; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.pretreatment_2021_3_1 (
    CONSTRAINT pretreatment_check CHECK (((date_start >= '2021-03-01 00:00:00'::timestamp without time zone) AND (date_start <= '2021-03-01 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.pretreatment);


ALTER TABLE meteo.pretreatment_2021_3_1 OWNER TO postgres;

--
-- Name: pretreatment_2021_7_12; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.pretreatment_2021_7_12 (
    CONSTRAINT pretreatment_check CHECK (((date_start >= '2021-07-12 00:00:00'::timestamp without time zone) AND (date_start <= '2021-07-12 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.pretreatment);


ALTER TABLE meteo.pretreatment_2021_7_12 OWNER TO postgres;

--
-- Name: satellite; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.satellite (
    id integer NOT NULL,
    name character varying NOT NULL
);


ALTER TABLE meteo.satellite OWNER TO postgres;

--
-- Name: session; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.session (
    id bigint NOT NULL,
    satellite integer NOT NULL,
    date_start timestamp without time zone NOT NULL,
    date_end timestamp without time zone NOT NULL,
    date_start_offset integer NOT NULL,
    revol integer NOT NULL,
    direction integer NOT NULL,
    elevat_max real,
    site integer,
    fpath character varying NOT NULL,
    tle character varying
);


ALTER TABLE meteo.session OWNER TO postgres;

--
-- Name: site; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.site (
    id integer NOT NULL,
    name character varying NOT NULL,
    coord public.geometry NOT NULL
);


ALTER TABLE meteo.site OWNER TO postgres;

--
-- Name: site_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.site_view AS
 SELECT site.id,
    site.name,
    public.st_y(site.coord) AS latitude,
    public.st_x(site.coord) AS longitude
   FROM meteo.site;


ALTER TABLE meteo.site_view OWNER TO postgres;

--
-- Name: session_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.session_view AS
 SELECT session.id,
    satellite.name AS satellite,
    session.date_start,
    session.date_end,
    session.date_start_offset,
    session.revol,
    session.direction,
    session.elevat_max,
    site_view.name AS site,
    site_view.latitude,
    site_view.longitude,
    session.fpath,
    session.tle
   FROM ((meteo.session
     LEFT JOIN meteo.site_view ON ((session.site = site_view.id)))
     LEFT JOIN meteo.satellite ON ((session.satellite = satellite.id)));


ALTER TABLE meteo.session_view OWNER TO postgres;

--
-- Name: pretreatment_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.pretreatment_view AS
 SELECT session_view.id AS session_id,
    session_view.satellite,
    session_view.site,
    session_view.latitude,
    session_view.longitude,
    instrument_channel_view.instrument_id,
    instrument_channel_view.instrument_name,
    instrument_channel_view.channel_number,
    instrument_channel_view.channel_alias,
    pretreatment.date_start,
    pretreatment.date_end,
    pretreatment.calibration,
    pretreatment.fpath
   FROM ((meteo.pretreatment
     LEFT JOIN meteo.instrument_channel_view ON ((pretreatment.instrument_channel = instrument_channel_view.id)))
     LEFT JOIN meteo.session_view ON ((pretreatment.session = session_view.id)));


ALTER TABLE meteo.pretreatment_view OWNER TO postgres;

--
-- Name: raw_pretreatment_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.raw_pretreatment_view AS
 SELECT session_view.id AS session_id,
    session_view.satellite,
    session_view.date_start,
    session_view.date_end,
    session_view.date_start_offset,
    session_view.site,
    session_view.latitude,
    session_view.longitude,
    session_view.fpath,
    session_view.tle
   FROM (meteo.session_view
     LEFT JOIN meteo.pretreatment ON ((session_view.id = pretreatment.session)))
  WHERE (pretreatment.session IS NULL);


ALTER TABLE meteo.raw_pretreatment_view OWNER TO postgres;

--
-- Name: thematic; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic (
    session bigint NOT NULL,
    type integer NOT NULL,
    date_start timestamp without time zone NOT NULL,
    date_end timestamp without time zone NOT NULL,
    fpath character varying NOT NULL,
    instrument integer NOT NULL
);


ALTER TABLE meteo.thematic OWNER TO postgres;

--
-- Name: raw_thematic_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.raw_thematic_view AS
 SELECT session_view.id AS session_id,
    session_view.satellite,
    session_view.date_start,
    session_view.date_end,
    session_view.date_start_offset,
    session_view.site,
    session_view.latitude,
    session_view.longitude,
    session_view.fpath,
    session_view.tle
   FROM (meteo.session_view
     LEFT JOIN meteo.thematic ON ((session_view.id = thematic.session)))
  WHERE (thematic.session IS NULL);


ALTER TABLE meteo.raw_thematic_view OWNER TO postgres;

--
-- Name: satellite_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.satellite_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.satellite_id_seq OWNER TO postgres;

--
-- Name: satellite_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.satellite_id_seq OWNED BY meteo.satellite.id;


--
-- Name: session_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.session_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.session_id_seq OWNER TO postgres;

--
-- Name: session_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.session_id_seq OWNED BY meteo.session.id;


--
-- Name: site_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.site_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.site_id_seq OWNER TO postgres;

--
-- Name: site_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.site_id_seq OWNED BY meteo.site.id;


--
-- Name: thematic_2021_10_19; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic_2021_10_19 (
    CONSTRAINT thematic_check CHECK (((date_start >= '2021-10-19 00:00:00'::timestamp without time zone) AND (date_start <= '2021-10-19 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.thematic);


ALTER TABLE meteo.thematic_2021_10_19 OWNER TO postgres;

--
-- Name: thematic_2021_10_20; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic_2021_10_20 (
    CONSTRAINT thematic_check CHECK (((date_start >= '2021-10-20 00:00:00'::timestamp without time zone) AND (date_start <= '2021-10-20 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.thematic);


ALTER TABLE meteo.thematic_2021_10_20 OWNER TO postgres;

--
-- Name: thematic_2021_10_21; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic_2021_10_21 (
    CONSTRAINT thematic_check CHECK (((date_start >= '2021-10-21 00:00:00'::timestamp without time zone) AND (date_start <= '2021-10-21 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.thematic);


ALTER TABLE meteo.thematic_2021_10_21 OWNER TO postgres;

--
-- Name: thematic_2021_10_31; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic_2021_10_31 (
    CONSTRAINT thematic_check CHECK (((date_start >= '2021-10-31 00:00:00'::timestamp without time zone) AND (date_start <= '2021-10-31 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.thematic);


ALTER TABLE meteo.thematic_2021_10_31 OWNER TO postgres;

--
-- Name: thematic_2021_3_1; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic_2021_3_1 (
    CONSTRAINT thematic_check CHECK (((date_start >= '2021-03-01 00:00:00'::timestamp without time zone) AND (date_start <= '2021-03-01 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.thematic);


ALTER TABLE meteo.thematic_2021_3_1 OWNER TO postgres;

--
-- Name: thematic_2021_7_12; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic_2021_7_12 (
    CONSTRAINT thematic_check CHECK (((date_start >= '2021-07-12 00:00:00'::timestamp without time zone) AND (date_start <= '2021-07-12 23:59:59'::timestamp without time zone)))
)
INHERITS (meteo.thematic);


ALTER TABLE meteo.thematic_2021_7_12 OWNER TO postgres;

--
-- Name: thematic_type; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic_type (
    id integer NOT NULL,
    name character varying NOT NULL,
    title character varying
);


ALTER TABLE meteo.thematic_type OWNER TO postgres;

--
-- Name: thematic_type_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.thematic_type_id_seq
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.thematic_type_id_seq OWNER TO postgres;

--
-- Name: thematic_type_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.thematic_type_id_seq OWNED BY meteo.thematic_type.id;


--
-- Name: thematic_view; Type: VIEW; Schema: meteo; Owner: postgres
--

CREATE VIEW meteo.thematic_view AS
 SELECT session_view.id AS session_id,
    session_view.satellite,
    session_view.site,
    session_view.latitude,
    session_view.longitude,
    thematic.type,
    thematic_type.title AS name,
    thematic.instrument,
    instrument.name AS instr_name,
    thematic.date_start,
    thematic.date_end,
    thematic.fpath
   FROM (((meteo.thematic
     LEFT JOIN meteo.thematic_type ON ((thematic.type = thematic_type.id)))
     LEFT JOIN meteo.session_view ON ((thematic.session = session_view.id)))
     LEFT JOIN meteo.instrument ON ((thematic.instrument = instrument.id)));


ALTER TABLE meteo.thematic_view OWNER TO postgres;

--
-- Name: channels; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.channels (
    id bigint NOT NULL,
    lambda_min real,
    lambda_center real,
    lambda_max real,
    channel integer,
    alias text
);


ALTER TABLE public.channels OWNER TO postgres;

--
-- Name: COLUMN channels.channel; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.channels.channel IS 'Номер канала в приборе';


--
-- Name: COLUMN channels.alias; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.channels.alias IS 'Условное обозначение канала для тематической обработки';


--
-- Name: instr_ch; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.instr_ch (
    channel_id bigint,
    instr_type smallint
);


ALTER TABLE public.instr_ch OWNER TO postgres;

--
-- Name: instruments; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.instruments (
    name text NOT NULL,
    samples integer,
    scan_angle real,
    fov real,
    fovstep real,
    direction integer,
    velocity real,
    rt_thin integer,
    type smallint NOT NULL
);


ALTER TABLE public.instruments OWNER TO postgres;

--
-- Name: pre_theme; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.pre_theme (
    id bigint NOT NULL,
    theme_id bigint,
    pre_id bigint
);


ALTER TABLE public.pre_theme OWNER TO postgres;

--
-- Name: preprocess_index_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.preprocess_index_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.preprocess_index_seq OWNER TO postgres;

--
-- Name: preprocess; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.preprocess (
    index bigint DEFAULT nextval('public.preprocess_index_seq'::regclass) NOT NULL,
    session_id bigint,
    date_start timestamp without time zone,
    date_end timestamp without time zone,
    calibration boolean,
    path text,
    channel_id bigint
);


ALTER TABLE public.preprocess OWNER TO postgres;

--
-- Name: sessions_index_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.sessions_index_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.sessions_index_seq OWNER TO postgres;

--
-- Name: sessions; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.sessions (
    index bigint DEFAULT nextval('public.sessions_index_seq'::regclass) NOT NULL,
    satellite_name text,
    date_start timestamp without time zone,
    date_end timestamp without time zone,
    site text,
    preprocessed boolean,
    fpath text,
    thematic_done boolean DEFAULT false,
    tle text
);


ALTER TABLE public.sessions OWNER TO postgres;

--
-- Name: COLUMN sessions.thematic_done; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.sessions.thematic_done IS 'Флаг, указывающий на то, что все тематические обработки завершены.';


--
-- Name: pretreatment; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.pretreatment AS
 SELECT DISTINCT sessions.date_start,
    sessions.date_end,
    preprocess.calibration,
    preprocess.path,
    channels.channel,
    channels.alias,
    instruments.name AS instr_name,
    instr_ch.instr_type,
    preprocess.session_id,
    sessions.satellite_name
   FROM public.preprocess,
    public.channels,
    public.instruments,
    public.instr_ch,
    public.sessions
  WHERE ((channels.id = preprocess.channel_id) AND (instr_ch.instr_type = instruments.type) AND (channels.id = instr_ch.channel_id) AND (preprocess.session_id = sessions.index));


ALTER TABLE public.pretreatment OWNER TO postgres;

--
-- Name: COLUMN pretreatment.channel; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN public.pretreatment.channel IS 'Номер канала в приборе';


--
-- Name: sat_instr; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.sat_instr (
    id bigint NOT NULL,
    sat_name text,
    instr_type smallint
);


ALTER TABLE public.sat_instr OWNER TO postgres;

--
-- Name: satellites; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.satellites (
    name text NOT NULL
);


ALTER TABLE public.satellites OWNER TO postgres;

--
-- Name: sites; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.sites (
    name text NOT NULL,
    coords public.geometry(Point)
);


ALTER TABLE public.sites OWNER TO postgres;

--
-- Name: theme_index_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE public.theme_index_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.theme_index_seq OWNER TO postgres;

--
-- Name: theme; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.theme (
    index bigint DEFAULT nextval('public.theme_index_seq'::regclass) NOT NULL,
    th_name text,
    path text,
    date_start timestamp without time zone,
    date_end timestamp without time zone,
    session_id bigint,
    instr_type integer,
    type integer,
    channel_id bigint
);


ALTER TABLE public.theme OWNER TO postgres;

--
-- Name: theme_types; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.theme_types (
    index integer NOT NULL,
    name text,
    title text
);


ALTER TABLE public.theme_types OWNER TO postgres;

--
-- Name: thematics; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW public.thematics AS
 SELECT DISTINCT theme.path,
    sessions.date_start,
    sessions.date_end,
    theme.session_id,
    theme.instr_type,
    theme.type,
    channels.channel,
    channels.alias,
    instruments.name AS instr_name,
    sessions.satellite_name,
    theme_types.name,
    theme_types.title
   FROM public.theme,
    public.channels,
    public.instruments,
    public.instr_ch,
    public.sessions,
    public.theme_types
  WHERE ((instr_ch.instr_type = theme.instr_type) AND (instr_ch.channel_id = channels.id) AND (channels.id = theme.channel_id) AND (instr_ch.instr_type = instruments.type) AND (sessions.index = theme.session_id) AND (theme.type = theme_types.index));


ALTER TABLE public.thematics OWNER TO postgres;

--
-- Name: theme_ch; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.theme_ch (
    id bigint NOT NULL,
    theme_id bigint,
    channel_id bigint
);


ALTER TABLE public.theme_ch OWNER TO postgres;

--
-- Name: channel id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.channel ALTER COLUMN id SET DEFAULT nextval('meteo.channel_id_seq'::regclass);


--
-- Name: instrument id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.instrument ALTER COLUMN id SET DEFAULT nextval('meteo.instrument_id_seq'::regclass);


--
-- Name: instrument_channel id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.instrument_channel ALTER COLUMN id SET DEFAULT nextval('meteo.instrument_channel_id_seq'::regclass);


--
-- Name: satellite id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.satellite ALTER COLUMN id SET DEFAULT nextval('meteo.satellite_id_seq'::regclass);


--
-- Name: session id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.session ALTER COLUMN id SET DEFAULT nextval('meteo.session_id_seq'::regclass);


--
-- Name: site id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.site ALTER COLUMN id SET DEFAULT nextval('meteo.site_id_seq'::regclass);


--
-- Name: thematic_type id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_type ALTER COLUMN id SET DEFAULT nextval('meteo.thematic_type_id_seq'::regclass);


--
-- Name: channel channel_pk; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.channel
    ADD CONSTRAINT channel_pk PRIMARY KEY (id);


--
-- Name: instrument_channel instrument_channel_pk; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.instrument_channel
    ADD CONSTRAINT instrument_channel_pk PRIMARY KEY (id);


--
-- Name: instrument_channel instrument_channel_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.instrument_channel
    ADD CONSTRAINT instrument_channel_unique UNIQUE (instrument, channel);


--
-- Name: instrument instrument_pk; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.instrument
    ADD CONSTRAINT instrument_pk PRIMARY KEY (id);


--
-- Name: instrument instrument_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.instrument
    ADD CONSTRAINT instrument_unique UNIQUE (name);


--
-- Name: pretreatment pretreatment_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment
    ADD CONSTRAINT pretreatment_unique UNIQUE (session, instrument_channel);


--
-- Name: satellite satellite_pk; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.satellite
    ADD CONSTRAINT satellite_pk PRIMARY KEY (id);


--
-- Name: satellite satellite_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.satellite
    ADD CONSTRAINT satellite_unique UNIQUE (name);


--
-- Name: session session_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.session
    ADD CONSTRAINT session_pkey PRIMARY KEY (id);


--
-- Name: site site_pk; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.site
    ADD CONSTRAINT site_pk PRIMARY KEY (id);


--
-- Name: site site_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.site
    ADD CONSTRAINT site_unique UNIQUE (name);


--
-- Name: thematic_type thematic_type_pk; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_type
    ADD CONSTRAINT thematic_type_pk PRIMARY KEY (id);


--
-- Name: thematic_type thematic_type_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_type
    ADD CONSTRAINT thematic_type_unique UNIQUE (name);


--
-- Name: channels channels_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.channels
    ADD CONSTRAINT channels_pkey PRIMARY KEY (id);


--
-- Name: instruments instruments_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.instruments
    ADD CONSTRAINT instruments_pkey PRIMARY KEY (type);


--
-- Name: pre_theme pre_theme_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.pre_theme
    ADD CONSTRAINT pre_theme_pkey PRIMARY KEY (id);


--
-- Name: preprocess preprocess_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.preprocess
    ADD CONSTRAINT preprocess_pkey PRIMARY KEY (index);


--
-- Name: sat_instr sat_instr_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sat_instr
    ADD CONSTRAINT sat_instr_pkey PRIMARY KEY (id);


--
-- Name: satellites satellites_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.satellites
    ADD CONSTRAINT satellites_pkey PRIMARY KEY (name);


--
-- Name: sessions sessions_fpath_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sessions
    ADD CONSTRAINT sessions_fpath_key UNIQUE (fpath);


--
-- Name: sessions sessions_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sessions
    ADD CONSTRAINT sessions_pkey PRIMARY KEY (index);


--
-- Name: sites sites_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sites
    ADD CONSTRAINT sites_pkey PRIMARY KEY (name);


--
-- Name: theme_ch theme_ch_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.theme_ch
    ADD CONSTRAINT theme_ch_pkey PRIMARY KEY (id);


--
-- Name: theme theme_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.theme
    ADD CONSTRAINT theme_pkey PRIMARY KEY (index);


--
-- Name: theme_types theme_types_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.theme_types
    ADD CONSTRAINT theme_types_pkey PRIMARY KEY (index);


--
-- Name: fki_sessions_satellite_name_fkey; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX fki_sessions_satellite_name_fkey ON public.sessions USING btree (satellite_name);


--
-- Name: pretreatment partitioning; Type: TRIGGER; Schema: meteo; Owner: postgres
--

CREATE TRIGGER partitioning BEFORE INSERT ON meteo.pretreatment FOR EACH ROW EXECUTE PROCEDURE meteo.pretreatment_partitioning_range_day();


--
-- Name: thematic partitioning; Type: TRIGGER; Schema: meteo; Owner: postgres
--

CREATE TRIGGER partitioning BEFORE INSERT ON meteo.thematic FOR EACH ROW EXECUTE PROCEDURE meteo.thematic_partitioning_range_day();


--
-- Name: preprocess tr_prep_insert_into_inherited; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER tr_prep_insert_into_inherited BEFORE INSERT ON public.preprocess FOR EACH ROW EXECUTE PROCEDURE public.partitioing_range_month();


--
-- Name: sessions tr_sessions_insert_into_inherited; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER tr_sessions_insert_into_inherited BEFORE INSERT ON public.sessions FOR EACH ROW EXECUTE PROCEDURE public.partitioing_range_month();

ALTER TABLE public.sessions DISABLE TRIGGER tr_sessions_insert_into_inherited;


--
-- Name: theme tr_them_insert_into_inherited; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER tr_them_insert_into_inherited BEFORE INSERT ON public.theme FOR EACH ROW EXECUTE PROCEDURE public.partitioing_range_month();


--
-- Name: instrument_channel $1; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.instrument_channel
    ADD CONSTRAINT "$1" FOREIGN KEY (instrument) REFERENCES meteo.instrument(id);


--
-- Name: session $1; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.session
    ADD CONSTRAINT "$1" FOREIGN KEY (satellite) REFERENCES meteo.satellite(id) NOT VALID;


--
-- Name: thematic $1; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic
    ADD CONSTRAINT "$1" FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE CASCADE NOT VALID;


--
-- Name: pretreatment $1; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment
    ADD CONSTRAINT "$1" FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE CASCADE NOT VALID;


--
-- Name: thematic $2; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic
    ADD CONSTRAINT "$2" FOREIGN KEY (type) REFERENCES meteo.thematic_type(id) NOT VALID;


--
-- Name: instrument_channel $2; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.instrument_channel
    ADD CONSTRAINT "$2" FOREIGN KEY (channel) REFERENCES meteo.channel(id);


--
-- Name: pretreatment $2; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment
    ADD CONSTRAINT "$2" FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel(id);


--
-- Name: session $2; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.session
    ADD CONSTRAINT "$2" FOREIGN KEY (site) REFERENCES meteo.site(id) NOT VALID;


--
-- Name: pretreatment_2021_10_19 pretreatment_2021_10_19_instrument_channel_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_10_19
    ADD CONSTRAINT pretreatment_2021_10_19_instrument_channel_fkey FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel(id);


--
-- Name: pretreatment_2021_10_19 pretreatment_2021_10_19_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_10_19
    ADD CONSTRAINT pretreatment_2021_10_19_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: pretreatment_2021_10_20 pretreatment_2021_10_20_instrument_channel_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_10_20
    ADD CONSTRAINT pretreatment_2021_10_20_instrument_channel_fkey FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel(id);


--
-- Name: pretreatment_2021_10_20 pretreatment_2021_10_20_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_10_20
    ADD CONSTRAINT pretreatment_2021_10_20_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: pretreatment_2021_10_21 pretreatment_2021_10_21_instrument_channel_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_10_21
    ADD CONSTRAINT pretreatment_2021_10_21_instrument_channel_fkey FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel(id);


--
-- Name: pretreatment_2021_10_21 pretreatment_2021_10_21_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_10_21
    ADD CONSTRAINT pretreatment_2021_10_21_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: pretreatment_2021_10_31 pretreatment_2021_10_31_instrument_channel_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_10_31
    ADD CONSTRAINT pretreatment_2021_10_31_instrument_channel_fkey FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel(id);


--
-- Name: pretreatment_2021_10_31 pretreatment_2021_10_31_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_10_31
    ADD CONSTRAINT pretreatment_2021_10_31_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: pretreatment_2021_3_1 pretreatment_2021_3_1_instrument_channel_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_3_1
    ADD CONSTRAINT pretreatment_2021_3_1_instrument_channel_fkey FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel(id);


--
-- Name: pretreatment_2021_3_1 pretreatment_2021_3_1_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_3_1
    ADD CONSTRAINT pretreatment_2021_3_1_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: pretreatment_2021_7_12 pretreatment_2021_7_12_instrument_channel_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_7_12
    ADD CONSTRAINT pretreatment_2021_7_12_instrument_channel_fkey FOREIGN KEY (instrument_channel) REFERENCES meteo.instrument_channel(id);


--
-- Name: pretreatment_2021_7_12 pretreatment_2021_7_12_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.pretreatment_2021_7_12
    ADD CONSTRAINT pretreatment_2021_7_12_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: thematic_2021_10_19 thematic_2021_10_19_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_10_19
    ADD CONSTRAINT thematic_2021_10_19_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: thematic_2021_10_19 thematic_2021_10_19_type_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_10_19
    ADD CONSTRAINT thematic_2021_10_19_type_fkey FOREIGN KEY (type) REFERENCES meteo.thematic_type(id);


--
-- Name: thematic_2021_10_20 thematic_2021_10_20_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_10_20
    ADD CONSTRAINT thematic_2021_10_20_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: thematic_2021_10_20 thematic_2021_10_20_type_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_10_20
    ADD CONSTRAINT thematic_2021_10_20_type_fkey FOREIGN KEY (type) REFERENCES meteo.thematic_type(id);


--
-- Name: thematic_2021_10_21 thematic_2021_10_21_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_10_21
    ADD CONSTRAINT thematic_2021_10_21_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: thematic_2021_10_21 thematic_2021_10_21_type_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_10_21
    ADD CONSTRAINT thematic_2021_10_21_type_fkey FOREIGN KEY (type) REFERENCES meteo.thematic_type(id);


--
-- Name: thematic_2021_10_31 thematic_2021_10_31_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_10_31
    ADD CONSTRAINT thematic_2021_10_31_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: thematic_2021_10_31 thematic_2021_10_31_type_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_10_31
    ADD CONSTRAINT thematic_2021_10_31_type_fkey FOREIGN KEY (type) REFERENCES meteo.thematic_type(id);


--
-- Name: thematic_2021_3_1 thematic_2021_3_1_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_3_1
    ADD CONSTRAINT thematic_2021_3_1_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: thematic_2021_3_1 thematic_2021_3_1_type_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_3_1
    ADD CONSTRAINT thematic_2021_3_1_type_fkey FOREIGN KEY (type) REFERENCES meteo.thematic_type(id);


--
-- Name: thematic_2021_7_12 thematic_2021_7_12_session_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_7_12
    ADD CONSTRAINT thematic_2021_7_12_session_fkey FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE SET NULL NOT VALID;


--
-- Name: thematic_2021_7_12 thematic_2021_7_12_type_fkey; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic_2021_7_12
    ADD CONSTRAINT thematic_2021_7_12_type_fkey FOREIGN KEY (type) REFERENCES meteo.thematic_type(id);


--
-- Name: instr_ch instr_ch_channel_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.instr_ch
    ADD CONSTRAINT instr_ch_channel_id_fkey FOREIGN KEY (channel_id) REFERENCES public.channels(id);


--
-- Name: instr_ch instr_ch_instr_type_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.instr_ch
    ADD CONSTRAINT instr_ch_instr_type_fkey FOREIGN KEY (instr_type) REFERENCES public.instruments(type);


--
-- Name: pre_theme pre_theme_pre_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.pre_theme
    ADD CONSTRAINT pre_theme_pre_id_fkey FOREIGN KEY (pre_id) REFERENCES public.preprocess(index);


--
-- Name: pre_theme pre_theme_theme_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.pre_theme
    ADD CONSTRAINT pre_theme_theme_id_fkey FOREIGN KEY (theme_id) REFERENCES public.theme(index);


--
-- Name: preprocess preprocess_channel_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.preprocess
    ADD CONSTRAINT preprocess_channel_id_fkey FOREIGN KEY (channel_id) REFERENCES public.channels(id);


--
-- Name: sat_instr sat_instr_instr_type_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sat_instr
    ADD CONSTRAINT sat_instr_instr_type_fkey FOREIGN KEY (instr_type) REFERENCES public.instruments(type);


--
-- Name: sat_instr sat_instr_sat_name_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sat_instr
    ADD CONSTRAINT sat_instr_sat_name_fkey FOREIGN KEY (sat_name) REFERENCES public.satellites(name);


--
-- Name: sessions sessions_satellite_name_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sessions
    ADD CONSTRAINT sessions_satellite_name_fkey FOREIGN KEY (satellite_name) REFERENCES public.satellites(name);


--
-- Name: sessions sessions_site_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sessions
    ADD CONSTRAINT sessions_site_fkey FOREIGN KEY (site) REFERENCES public.sites(name);


--
-- Name: theme_ch theme_ch_channel_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.theme_ch
    ADD CONSTRAINT theme_ch_channel_id_fkey FOREIGN KEY (channel_id) REFERENCES public.channels(id);


--
-- Name: theme_ch theme_ch_theme_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.theme_ch
    ADD CONSTRAINT theme_ch_theme_id_fkey FOREIGN KEY (theme_id) REFERENCES public.theme(index);


--
-- Name: theme theme_channels_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.theme
    ADD CONSTRAINT theme_channels_fkey FOREIGN KEY (channel_id) REFERENCES public.channels(id);


--
-- Name: theme theme_theme_types_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.theme
    ADD CONSTRAINT theme_theme_types_fkey FOREIGN KEY (type) REFERENCES public.theme_types(index);


--
-- PostgreSQL database dump complete
--

