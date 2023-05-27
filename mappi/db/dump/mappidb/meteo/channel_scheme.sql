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
-- Name: channel id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.channel ALTER COLUMN id SET DEFAULT nextval('meteo.channel_id_seq'::regclass);


--
-- Name: channel channel_pk; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.channel
    ADD CONSTRAINT channel_pk PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

