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
-- Name: custom_settings; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.custom_settings (
    id integer NOT NULL,
    login text NOT NULL,
    name text NOT NULL,
    file_id bigint NOT NULL
);


ALTER TABLE meteo.custom_settings OWNER TO postgres;

--
-- Name: custom_settings_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.custom_settings_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.custom_settings_id_seq OWNER TO postgres;

--
-- Name: custom_settings_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.custom_settings_id_seq OWNED BY meteo.custom_settings.id;


--
-- Name: custom_settings id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.custom_settings ALTER COLUMN id SET DEFAULT nextval('meteo.custom_settings_id_seq'::regclass);


--
-- Name: custom_settings custom_settings_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.custom_settings
    ADD CONSTRAINT custom_settings_pkey PRIMARY KEY (id);


--
-- Name: login_name_idx; Type: INDEX; Schema: meteo; Owner: postgres
--

CREATE UNIQUE INDEX login_name_idx ON meteo.custom_settings USING btree (login, name);


--
-- PostgreSQL database dump complete
--

