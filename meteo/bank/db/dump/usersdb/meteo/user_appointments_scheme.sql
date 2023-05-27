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
-- Name: user_appointments; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.user_appointments (
    id bigint NOT NULL,
    name text,
    owner bigint,
    type integer,
    users bigint[],
    "currentUser" bigint,
    "dtStart" timestamp without time zone
);


ALTER TABLE meteo.user_appointments OWNER TO postgres;

--
-- Name: user_appointments__id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.user_appointments__id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.user_appointments__id_seq OWNER TO postgres;

--
-- Name: user_appointments__id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.user_appointments__id_seq OWNED BY meteo.user_appointments.id;


--
-- Name: user_appointments id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.user_appointments ALTER COLUMN id SET DEFAULT nextval('meteo.user_appointments__id_seq'::regclass);


--
-- Name: user_appointments uni_name; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.user_appointments
    ADD CONSTRAINT uni_name UNIQUE (name);


--
-- Name: user_appointments user_appointments_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.user_appointments
    ADD CONSTRAINT user_appointments_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

