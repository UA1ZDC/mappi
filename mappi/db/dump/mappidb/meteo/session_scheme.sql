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
-- Name: session id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.session ALTER COLUMN id SET DEFAULT nextval('meteo.session_id_seq'::regclass);


--
-- Name: session session_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.session
    ADD CONSTRAINT session_pkey PRIMARY KEY (id);


--
-- Name: session $1; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.session
    ADD CONSTRAINT "$1" FOREIGN KEY (satellite) REFERENCES meteo.satellite(id) NOT VALID;


--
-- Name: session $2; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.session
    ADD CONSTRAINT "$2" FOREIGN KEY (site) REFERENCES meteo.site(id) NOT VALID;


--
-- PostgreSQL database dump complete
--

