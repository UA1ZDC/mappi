--
-- PostgreSQL database dump
--

-- Dumped from database version 9.6.4
-- Dumped by pg_dump version 9.6.4

SET statement_timeout = 0;
SET lock_timeout = 0;
 
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;
 

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


--
-- Name: postgis; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS postgis WITH SCHEMA public;


--
-- Name: EXTENSION postgis; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION postgis IS 'PostGIS geometry, geography, and raster spatial types and functions';


SET search_path = public, pg_catalog;

--
-- Name: insert_into_inherited(); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION insert_into_inherited() RETURNS trigger
    LANGUAGE plpgsql
    AS $$
DECLARE
    table_master    TEXT       = TG_RELNAME;
    table_part      TEXT       = '';
    rec_            RECORD;		
BEGIN
        -- Даём имя партиции --------------------------------------------------
    table_part := table_master  || '_' || date_part( 'year', NEW.date_start )::TEXT
                                || '_' || date_part( 'month', NEW.date_start )::TEXT
                                || '_' || date_part( 'day', NEW.date_start )::TEXT;
 
        -- Проверяем партицию на существование --------------------------------
    PERFORM 1 FROM pg_class WHERE relname = table_part LIMIT 1;
 
        -- Если её ещё нет, то создаём ----------------------------------------
    IF NOT FOUND
    THEN
        -- Cоздаём партицию, наследуя мастер-таблицу --------------------------
      EXECUTE  'CREATE TABLE ' || table_part || '
      (
        CONSTRAINT ' || table_part || '_created_check CHECK (date_start BETWEEN ''' || date_trunc('day', NEW.date_start) ||
                                                             ''' AND ''' || date_trunc('day', NEW.date_start) + interval '1 day' - interval '1 sec' || ''')
      )
      INHERITS (' || table_master || ')
      WITH (OIDS = FALSE)';

        -- Получаем привелегии базовой таблицы и копируем их на текущую партицию -------------------------
      FOR rec_ IN EXECUTE 'SELECT grantee, string_agg(privilege_type, '', '') AS priv FROM information_schema.role_table_grants 
                           WHERE table_name = ''' || table_master || ''' GROUP BY grantee' LOOP
        EXECUTE 'GRANT ' || rec_.priv || ' ON ' || table_part || ' TO ' || rec_.grantee;
      END LOOP;

        -- Создаём индексы для текущей партиции -------------------------------
      PERFORM 1 FROM information_schema.columns WHERE table_name = table_master AND column_name = 'forecast_time';
      IF FOUND
      THEN
        EXECUTE '
            CREATE INDEX ' || table_part || '_date_index
            ON ' || table_part || '
            USING btree
            (forecast_time)';
      END IF;

      PERFORM 1 FROM information_schema.columns WHERE table_name = table_master AND column_name = 'findex';
      IF FOUND
      THEN
        EXECUTE '
            CREATE INDEX ' || table_part || '_id_index
            ON ' || table_part || '
            USING btree
            (findex)';

        -- Создаём первичный ключ для текущей партиции ------------------------
        EXECUTE '
            ALTER TABLE ' || table_part || '
            ADD CONSTRAINT pkey_' || table_part || ' PRIMARY KEY (findex)';

      END IF;
      IF ( table_master = 'obanal' ) THEN
        EXECUTE 'CREATE TRIGGER tr_ai_' || table_part || '
           AFTER INSERT OR UPDATE ON ' || table_part || ' FOR EACH ROW
           EXECUTE PROCEDURE new_obanal_result()';
      END IF;

    END IF;
 
        -- Вставляем данные в партицию ----------------------------------------
    EXECUTE '
        INSERT INTO ' || table_part || ' 
            SELECT ((' || quote_literal(NEW) || ')::' || table_master || ').*';
 
    RETURN NULL;

END;
$$;


ALTER FUNCTION public.insert_into_inherited() OWNER TO postgres;

--
-- Name: update_preprocess(bigint, text, integer, boolean, text); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION update_preprocess(_session_id bigint, _instrument_name text, _channel integer, _calibration boolean, _path text) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
  declare
	id_ bigint;
	query_ text;
	suffix_ text;
	date_start_ timestamp without time zone;
	date_end_ timestamp without time zone;
	channel_id_ bigint;
begin
	select sessions.date_start into date_start_ from sessions where
	sessions.index = _session_id;

	select sessions.date_end into date_end_ from sessions where
	sessions.index = _session_id;

	select channels.id into channel_id_ from channels, instruments, instr_ch where 
	instr_ch.instr_name = _instrument_name
	and channels.id = instr_ch.channel_id
	and channels.channel = _channel;

	select index into id_ from preprocess where
	session_id = _session_id and
	date_start = date_start_ AND
	date_end = date_end_ AND
	calibration = _calibration and
	channel_id = channel_id_;

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
			''' || date_start_ || ''',
			''' || date_end_ || ''',
			''' || _calibration || ''',
			''' || channel_id_ || ''',
			''' || _path || ''');';
	end if;
	execute query_;

	return true;
end;
$$;


ALTER FUNCTION public.update_preprocess(_session_id bigint, _instrument_name text, _channel integer, _calibration boolean, _path text) OWNER TO postgres;

--
-- Name: update_sessions(text, timestamp without time zone, timestamp without time zone, text, text, boolean); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION update_sessions(_satellite_name text, _date_start timestamp without time zone, _date_end timestamp without time zone, _site text, _fpath text, _preprocessed boolean) RETURNS bigint
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
				preprocessed, '
				|| suffix_ || '
				)
			values (
			''' || _satellite_name	 || ''',
			''' || _date_start || ''',
			''' || _date_end || ''',
			''' || _site || ''',
			''' || _preprocessed || ''',
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


ALTER FUNCTION public.update_sessions(_satellite_name text, _date_start timestamp without time zone, _date_end timestamp without time zone, _site text, _fpath text, _preprocessed boolean) OWNER TO postgres;

--
-- Name: update_theme(text, text, text, bigint); Type: FUNCTION; Schema: public; Owner: postgres
--

CREATE FUNCTION update_theme(_type text, _path text, _meta text, _session_id bigint) RETURNS boolean
    LANGUAGE plpgsql
    AS $$
declare 
	id_ bigint;
	query_ text;
	suffix_ text;
	dts_ timestamp without time zone;
	dte_ timestamp without time zone;
begin
	select index into id_ from theme where
	th_type = _type and
	session_id = _session_id and
	path = _path;

	if ( id_ is not null ) then
	query_ = 'UPDATE theme SET ';
	suffix_ = ' path = ''' || _path ''' AND meta_path = ''' || _meta ''' ';
	query_ = query_ || suffix_ || '
		WHERE index = ' || id_ || ' ';
	else
	select date_start into dts_ from sessions where index = _session_id;
	select date_end into dte_ from sessions where index = _session_id;
	
	query_ = ' INSERT INTO theme(
				th_type,
				date_start,
				date_end,
				path,
				meta_path,
				session_id
				)
			values (
				''' || _type || ''',
				''' || dts_ || ''',
				''' || dte_ || ''',
				''' || _path || ''',
				''' || _meta || ''',
				''' || _session_id || '''
				);';
	end if;
	execute query_;

	return true;
end;
$$;


ALTER FUNCTION public.update_theme(_type text, _path text, _meta text, _session_id bigint) OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: channels; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE channels (
    id bigint NOT NULL,
    lambda_min real,
    lambda_center real,
    lambda_max real,
    channel integer,
    alias text
);


ALTER TABLE channels OWNER TO postgres;

--
-- Name: COLUMN channels.channel; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN channels.channel IS 'Номер канала в приборе';


--
-- Name: COLUMN channels.alias; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN channels.alias IS 'Условное обозначение канала для тематической обработки';


--
-- Name: instr_ch; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE instr_ch (
    instr_name text,
    channel_id bigint
);


ALTER TABLE instr_ch OWNER TO postgres;

--
-- Name: instruments; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE instruments (
    name text NOT NULL,
    samples integer,
    scan_angle real,
    fov real,
    fovstep real,
    direction integer,
    velocity real,
    rt_thin integer
);


ALTER TABLE instruments OWNER TO postgres;

--
-- Name: pre_theme; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE pre_theme (
    id bigint NOT NULL,
    theme_id bigint,
    pre_id bigint
);


ALTER TABLE pre_theme OWNER TO postgres;

--
-- Name: preprocess_index_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE preprocess_index_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE preprocess_index_seq OWNER TO postgres;

--
-- Name: preprocess; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE preprocess (
    index bigint DEFAULT nextval('preprocess_index_seq'::regclass) NOT NULL,
    session_id bigint,
    date_start timestamp without time zone,
    date_end timestamp without time zone,
    calibration boolean,
    path text,
    channel_id bigint
);


ALTER TABLE preprocess OWNER TO postgres;

--
-- Name: sessions_index_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE sessions_index_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE sessions_index_seq OWNER TO postgres;

--
-- Name: sessions; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE sessions (
    index bigint DEFAULT nextval('sessions_index_seq'::regclass) NOT NULL,
    satellite_name text,
    date_start timestamp without time zone,
    date_end timestamp without time zone,
    site text,
    preprocessed boolean,
    fpath text,
    thematic_done boolean DEFAULT false
);


ALTER TABLE sessions OWNER TO postgres;

--
-- Name: COLUMN sessions.thematic_done; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN sessions.thematic_done IS 'Флаг, указывающий на то, что все тематические обработки завершены.';


--
-- Name: pretreatment; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW pretreatment AS
 SELECT preprocess.date_start,
    preprocess.date_end,
    preprocess.calibration,
    preprocess.path,
    channels.channel,
    channels.alias,
    instr_ch.instr_name,
    preprocess.session_id,
    sessions.satellite_name
   FROM preprocess,
    channels,
    instruments,
    instr_ch,
    sessions
  WHERE ((channels.id = preprocess.channel_id) AND (instr_ch.instr_name = instruments.name) AND (channels.id = instr_ch.channel_id) AND (preprocess.session_id = sessions.index));


ALTER TABLE pretreatment OWNER TO postgres;

--
-- Name: COLUMN pretreatment.channel; Type: COMMENT; Schema: public; Owner: postgres
--

COMMENT ON COLUMN pretreatment.channel IS 'Номер канала в приборе';


--
-- Name: sat_instr; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE sat_instr (
    id bigint NOT NULL,
    sat_name text,
    instr_name text
);


ALTER TABLE sat_instr OWNER TO postgres;

--
-- Name: satellites; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE satellites (
    name text NOT NULL
);


ALTER TABLE satellites OWNER TO postgres;

--
-- Name: sites; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE sites (
    name text NOT NULL,
    coords geometry(Point)
);


ALTER TABLE sites OWNER TO postgres;

--
-- Name: theme_index_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

CREATE SEQUENCE theme_index_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE theme_index_seq OWNER TO postgres;

--
-- Name: theme; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE theme (
    index bigint DEFAULT nextval('theme_index_seq'::regclass) NOT NULL,
    th_type text,
    path text,
    date_start timestamp without time zone,
    date_end timestamp without time zone,
    meta_path text,
    topleft geometry(Point),
    topright geometry(Point),
    bottomleft geometry(Point),
    bottomright geometry(Point),
    session_id bigint
);


ALTER TABLE theme OWNER TO postgres;

--
-- Name: test_view; Type: VIEW; Schema: public; Owner: postgres
--

CREATE VIEW test_view AS
 SELECT sessions.fpath,
    theme.th_type
   FROM sessions,
    theme
  WHERE (sessions.index = theme.session_id);


ALTER TABLE test_view OWNER TO postgres;

--
-- Name: theme_ch; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE theme_ch (
    id bigint NOT NULL,
    theme_id bigint,
    channel_id bigint
);


ALTER TABLE theme_ch OWNER TO postgres;

--
-- Name: channels channels_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY channels
    ADD CONSTRAINT channels_pkey PRIMARY KEY (id);


--
-- Name: instruments instruments_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY instruments
    ADD CONSTRAINT instruments_pkey PRIMARY KEY (name);


--
-- Name: pre_theme pre_theme_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY pre_theme
    ADD CONSTRAINT pre_theme_pkey PRIMARY KEY (id);


--
-- Name: preprocess preprocess_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY preprocess
    ADD CONSTRAINT preprocess_pkey PRIMARY KEY (index);


--
-- Name: sat_instr sat_instr_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sat_instr
    ADD CONSTRAINT sat_instr_pkey PRIMARY KEY (id);


--
-- Name: satellites satellites_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY satellites
    ADD CONSTRAINT satellites_pkey PRIMARY KEY (name);


--
-- Name: sessions sessions_fpath_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sessions
    ADD CONSTRAINT sessions_fpath_key UNIQUE (fpath);


--
-- Name: sessions sessions_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sessions
    ADD CONSTRAINT sessions_pkey PRIMARY KEY (index);


--
-- Name: sites sites_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sites
    ADD CONSTRAINT sites_pkey PRIMARY KEY (name);


--
-- Name: theme_ch theme_ch_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY theme_ch
    ADD CONSTRAINT theme_ch_pkey PRIMARY KEY (id);


--
-- Name: theme theme_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY theme
    ADD CONSTRAINT theme_pkey PRIMARY KEY (index);


--
-- Name: fki_sessions_satellite_name_fkey; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX fki_sessions_satellite_name_fkey ON sessions USING btree (satellite_name);


--
-- Name: preprocess tr_prep_insert_into_inherited; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER tr_prep_insert_into_inherited BEFORE INSERT ON preprocess FOR EACH ROW EXECUTE PROCEDURE insert_into_inherited();


--
-- Name: sessions tr_ses_insert_into_inherited; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER tr_ses_insert_into_inherited BEFORE INSERT ON sessions FOR EACH ROW EXECUTE PROCEDURE insert_into_inherited();


--
-- Name: theme tr_them_insert_into_inherited; Type: TRIGGER; Schema: public; Owner: postgres
--

CREATE TRIGGER tr_them_insert_into_inherited BEFORE INSERT ON theme FOR EACH ROW EXECUTE PROCEDURE insert_into_inherited();


--
-- Name: instr_ch instr_ch_channel_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY instr_ch
    ADD CONSTRAINT instr_ch_channel_id_fkey FOREIGN KEY (channel_id) REFERENCES channels(id);


--
-- Name: instr_ch instr_ch_instr_name_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY instr_ch
    ADD CONSTRAINT instr_ch_instr_name_fkey FOREIGN KEY (instr_name) REFERENCES instruments(name);


--
-- Name: pre_theme pre_theme_pre_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY pre_theme
    ADD CONSTRAINT pre_theme_pre_id_fkey FOREIGN KEY (pre_id) REFERENCES preprocess(index);


--
-- Name: pre_theme pre_theme_theme_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY pre_theme
    ADD CONSTRAINT pre_theme_theme_id_fkey FOREIGN KEY (theme_id) REFERENCES theme(index);


--
-- Name: preprocess preprocess_channel_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY preprocess
    ADD CONSTRAINT preprocess_channel_id_fkey FOREIGN KEY (channel_id) REFERENCES channels(id);


--
-- Name: sat_instr sat_instr_instr_name_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sat_instr
    ADD CONSTRAINT sat_instr_instr_name_fkey FOREIGN KEY (instr_name) REFERENCES instruments(name);


--
-- Name: sat_instr sat_instr_sat_name_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sat_instr
    ADD CONSTRAINT sat_instr_sat_name_fkey FOREIGN KEY (sat_name) REFERENCES satellites(name);


--
-- Name: sessions sessions_satellite_name_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sessions
    ADD CONSTRAINT sessions_satellite_name_fkey FOREIGN KEY (satellite_name) REFERENCES satellites(name);


--
-- Name: sessions sessions_site_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY sessions
    ADD CONSTRAINT sessions_site_fkey FOREIGN KEY (site) REFERENCES sites(name);


--
-- Name: theme_ch theme_ch_channel_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY theme_ch
    ADD CONSTRAINT theme_ch_channel_id_fkey FOREIGN KEY (channel_id) REFERENCES channels(id);


--
-- Name: theme_ch theme_ch_theme_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY theme_ch
    ADD CONSTRAINT theme_ch_theme_id_fkey FOREIGN KEY (theme_id) REFERENCES theme(index);


--
-- PostgreSQL database dump complete
--

