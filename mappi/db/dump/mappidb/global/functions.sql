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
-- Name: delete_data_expired(text, text, text, timestamp without time zone, text, timestamp without time zone, text, text); Type: FUNCTION; Schema: global; Owner: postgres
--

CREATE FUNCTION global.delete_data_expired(msg_table text, text, dt_field text, dt timestamp without time zone, dt_write_field text, dt_write timestamp without time zone, fs_table text, fs_field text) RETURNS record
    LANGUAGE plpgsql
    AS $_$
DECLARE
    msg TEXT;
    msg_part_count INTEGER := 0;
    msg_rec_count BIGINT := 0;

    fs TEXT;
    fs_part_count INTEGER := 0;
    fs_rec_count BIGINT := 0;
    fs_key_field TEXT;

    fs_table_avail TEXT;
    ts_interval TEXT;
    query TEXT;
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
    dst TEXT;
    item TEXT;
    chunks TEXT[];
    count_rec INTEGER;
    res INTEGER := 0;
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
-- PostgreSQL database dump complete
--

