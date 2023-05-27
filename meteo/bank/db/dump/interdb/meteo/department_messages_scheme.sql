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
-- Name: department_messages; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.department_messages (
    id bigint NOT NULL,
    mac integer,
    dt timestamp without time zone,
    text text,
    from_user text,
    "from" text,
    "to" text,
    read_by text,
    files text[],
    parent text,
    delivered_to text
);


ALTER TABLE meteo.department_messages OWNER TO postgres;

--
-- Name: department_messages__id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.department_messages__id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.department_messages__id_seq OWNER TO postgres;

--
-- Name: department_messages__id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.department_messages__id_seq OWNED BY meteo.department_messages.id;


--
-- Name: department_messages id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.department_messages ALTER COLUMN id SET DEFAULT nextval('meteo.department_messages__id_seq'::regclass);


--
-- Name: department_messages department_messages_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.department_messages
    ADD CONSTRAINT department_messages_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

