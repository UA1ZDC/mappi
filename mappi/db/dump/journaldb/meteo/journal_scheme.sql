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
-- Name: journal_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.journal_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.journal_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: journal; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.journal (
    id bigint DEFAULT nextval('meteo.journal_seq'::regclass) NOT NULL,
    priority integer,
    sender text,
    username text,
    file_name text,
    line_number integer,
    ip text,
    host text,
    dt timestamp without time zone,
    message text
);


ALTER TABLE meteo.journal OWNER TO postgres;

--
-- Name: journal _id; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.journal
    ADD CONSTRAINT _id PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

