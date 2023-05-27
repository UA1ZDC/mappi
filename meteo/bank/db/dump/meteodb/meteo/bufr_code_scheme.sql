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

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: bufr_code; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.bufr_code (
    id integer NOT NULL,
    bufr_code bigint NOT NULL,
    description text,
    code_values jsonb
);


ALTER TABLE meteo.bufr_code OWNER TO postgres;

--
-- Name: TABLE bufr_code; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.bufr_code IS 'Описание значений метео-параметров';


--
-- Name: bufr_code_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.bufr_code_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.bufr_code_id_seq OWNER TO postgres;

--
-- Name: bufr_code_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.bufr_code_id_seq OWNED BY meteo.bufr_code.id;


--
-- Name: bufr_code id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.bufr_code ALTER COLUMN id SET DEFAULT nextval('meteo.bufr_code_id_seq'::regclass);


--
-- Name: bufr_code bufr_code_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.bufr_code
    ADD CONSTRAINT bufr_code_pkey PRIMARY KEY (id);


--
-- Name: bufr_code_code_index; Type: INDEX; Schema: meteo; Owner: postgres
--

CREATE INDEX bufr_code_code_index ON meteo.bufr_code USING gin (code_values jsonb_path_ops);


--
-- Name: INDEX bufr_code_code_index; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON INDEX meteo.bufr_code_code_index IS 'Индекс на значения дескриптора';


--
-- Name: bufr_code_index; Type: INDEX; Schema: meteo; Owner: postgres
--

CREATE UNIQUE INDEX bufr_code_index ON meteo.bufr_code USING btree (bufr_code);


--
-- Name: INDEX bufr_code_index; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON INDEX meteo.bufr_code_index IS 'Индекс на значение дескриптора';


--
-- PostgreSQL database dump complete
--

