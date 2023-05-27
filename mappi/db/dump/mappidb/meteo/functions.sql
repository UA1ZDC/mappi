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
-- Name: meteo; Type: SCHEMA; Schema: -; Owner: postgres
--

CREATE SCHEMA meteo;


ALTER SCHEMA meteo OWNER TO postgres;

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
        FOR item IN EXECUTE query
        LOOP
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
        FOR item IN EXECUTE query
        LOOP
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
-- PostgreSQL database dump complete
--

