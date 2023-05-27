--
-- PostgreSQL database dump
--

-- Dumped from database version 9.6.10
-- Dumped by pg_dump version 11.7 (Debian 11.7-0+deb10u1)

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

SET default_with_oids = false;

--
-- Name: fs; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.fs (
    id bigint NOT NULL,
    filename text NOT NULL,
    filemd5 text NOT NULL,
    filesize integer NOT NULL,
    dt_write timestamp without time zone DEFAULT timezone('utc'::text, now()) NOT NULL
);


ALTER TABLE meteo.fs OWNER TO postgres;

--
-- Name: TABLE fs; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.fs IS 'Описание файлов, на которые могут ссылаться другие файлы';


--
-- Name: fs_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.fs_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.fs_id_seq OWNER TO postgres;

--
-- Name: fs_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.fs_id_seq OWNED BY meteo.fs.id;


--
-- Name: fs id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.fs ALTER COLUMN id SET DEFAULT nextval('meteo.fs_id_seq'::regclass);


--
-- Name: fs filename_uniq; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.fs
    ADD CONSTRAINT filename_uniq UNIQUE (filename);


--
-- Name: fs fs_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.fs
    ADD CONSTRAINT fs_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

