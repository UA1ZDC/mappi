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
-- Name: users; Type: TABLE; Schema: meteo; Owner: postgres
--

CREATE TABLE meteo.users (
    id bigint NOT NULL,
    username text NOT NULL,
    role integer DEFAULT 3 NOT NULL,
    rank integer DEFAULT 1 NOT NULL,
    gender integer,
    department integer,
    permissions jsonb,
    birthdate text,
    block boolean DEFAULT false,
    avatar text
);
ALTER TABLE ONLY meteo.users ALTER COLUMN permissions SET STORAGE PLAIN;


ALTER TABLE meteo.users OWNER TO postgres;

--
-- Name: users users_username_unique; Type: CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.users
    ADD CONSTRAINT users_username_unique UNIQUE (username);


--
-- Name: users department_id; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.users
    ADD CONSTRAINT department_id FOREIGN KEY (department) REFERENCES meteo.departments(id) NOT VALID;


--
-- Name: users rank_id; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.users
    ADD CONSTRAINT rank_id FOREIGN KEY (rank) REFERENCES meteo.ranks(id);


--
-- Name: users role_id; Type: FK CONSTRAINT; Schema: meteo; Owner: postgres
--

ALTER TABLE ONLY meteo.users
    ADD CONSTRAINT role_id FOREIGN KEY (role) REFERENCES meteo.roles(id);


--
-- PostgreSQL database dump complete
--

