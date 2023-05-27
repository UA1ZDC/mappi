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

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: thematic; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.thematic (
    session bigint NOT NULL,
    type integer NOT NULL,
    them_name text NOT NULL,
    date_start timestamp without time zone NOT NULL,
    date_end timestamp without time zone NOT NULL,
    fpath character varying NOT NULL,
    instrument integer NOT NULL,
    projection character varying(10) NOT NULL,
    format character varying(5) NOT NULL
);


ALTER TABLE meteo.thematic OWNER TO postgres;

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
-- Name: thematic partitioning; Type: TRIGGER; Schema: meteo; Owner: postgres
--

CREATE TRIGGER partitioning BEFORE INSERT ON meteo.thematic FOR EACH ROW EXECUTE FUNCTION meteo.thematic_partitioning_range_day();


--
-- Name: thematic $1; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.thematic
    ADD CONSTRAINT "$1" FOREIGN KEY (session) REFERENCES meteo.session(id) ON DELETE CASCADE;


--
-- PostgreSQL database dump complete
--

