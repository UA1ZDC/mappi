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

--
-- Name: esimo_elem_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.esimo_elem_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.esimo_elem_id_seq OWNER TO postgres;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: esimo_elem; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.esimo_elem (
    id bigint DEFAULT nextval('meteo.esimo_elem_id_seq'::regclass) NOT NULL,
    esimo_name text NOT NULL,
    full_title_ru text,
    short_title_ru text,
    unit text,
    full_title_eng text,
    description text,
    obj_size text,
    obj_type text,
    bufr_descr integer
);


ALTER TABLE meteo.esimo_elem OWNER TO postgres;

--
-- Name: TABLE esimo_elem; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.esimo_elem IS 'Словарь системных элементов ЕСИМО';


--
-- Name: COLUMN esimo_elem.esimo_name; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.esimo_elem.esimo_name IS 'Единица данных: название';


--
-- Name: COLUMN esimo_elem.full_title_ru; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.esimo_elem.full_title_ru IS 'Объект: собственное название  на русском языке';


--
-- Name: COLUMN esimo_elem.short_title_ru; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.esimo_elem.short_title_ru IS 'Объект: краткое название  на русском языке';


--
-- Name: COLUMN esimo_elem.unit; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.esimo_elem.unit IS 'Единица измерения';


--
-- Name: COLUMN esimo_elem.full_title_eng; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.esimo_elem.full_title_eng IS 'Объект: собственное название  на английском языке';


--
-- Name: COLUMN esimo_elem.description; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.esimo_elem.description IS 'Описание';


--
-- Name: COLUMN esimo_elem.obj_size; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.esimo_elem.obj_size IS 'Объект: размер';


--
-- Name: COLUMN esimo_elem.obj_type; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON COLUMN meteo.esimo_elem.obj_type IS 'Формат: идентификатор';


--
-- Name: esimo_elem esimo_elem_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.esimo_elem
    ADD CONSTRAINT esimo_elem_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

