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
-- Name: msg; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.msg (
    id bigint NOT NULL,
    t1 text NOT NULL,
    t2 text NOT NULL,
    a1 text,
    a2 text,
    ii integer NOT NULL,
    cccc text NOT NULL,
    dddd text,
    bbb text,
    yygggg text NOT NULL,
    dt timestamp without time zone NOT NULL,
    msg_dt timestamp without time zone NOT NULL,
    md5 text NOT NULL,
    decoded boolean DEFAULT false,
    external boolean NOT NULL,
    prior integer,
    bin boolean NOT NULL,
    format text NOT NULL,
    type text NOT NULL,
    size integer NOT NULL,
    file_id bigint NOT NULL,
    globalfile_id integer,
    addr_tlg boolean,
    confirm_tlg boolean,
    route jsonb,
    dup integer,
    cor_number integer,
    confirm boolean
);


ALTER TABLE meteo.msg OWNER TO postgres;

--
-- Name: TABLE msg; Type: COMMENT; Schema: meteo; Owner: postgres
--

COMMENT ON TABLE meteo.msg IS 'Журал телеграмм';


--
-- Name: msg_id_seq; Type: SEQUENCE; Schema: meteo; Owner: postgres
--

CREATE SEQUENCE meteo.msg_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE meteo.msg_id_seq OWNER TO postgres;

--
-- Name: msg_id_seq; Type: SEQUENCE OWNED BY; Schema: meteo; Owner: postgres
--

ALTER SEQUENCE meteo.msg_id_seq OWNED BY meteo.msg.id;


--
-- Name: msg id; Type: DEFAULT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.msg ALTER COLUMN id SET DEFAULT nextval('meteo.msg_id_seq'::regclass);


--
-- Name: msg msg_pkey; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.msg
    ADD CONSTRAINT msg_pkey PRIMARY KEY (id);


--
-- PostgreSQL database dump complete
--

