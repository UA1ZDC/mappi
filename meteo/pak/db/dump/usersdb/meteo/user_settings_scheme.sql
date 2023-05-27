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
-- Name: user_settings; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.user_settings (
    _id bigint NOT NULL,
    user_login text,
    settings_type text,
    settings jsonb[]
);


ALTER TABLE meteo.user_settings OWNER TO postgres;

--
-- Name: user_settings user_settings_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.user_settings
    ADD CONSTRAINT user_settings_pkey PRIMARY KEY (_id);


--
-- PostgreSQL database dump complete
--

