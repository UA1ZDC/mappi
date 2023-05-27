--
-- PostgreSQL database dump
--

-- Dumped from database version 9.6.4
-- Dumped by pg_dump version 9.6.4

SET statement_timeout = 0;
SET lock_timeout = 0;
 
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;
 

SET search_path = public, pg_catalog;

--
-- Data for Name: channels; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (7, 0.5, 0.600000024, 0.699999988, 1, 'R0.6');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (8, 0.699999988, 0.899999976, 1.10000002, 2, 'R0.9');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (9, 1.60000002, 1.70000005, 1.79999995, 3, 'R1.6');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (10, 3.5, 3.79999995, 4.0999999, 4, 'T3.7');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (11, 10.5, 11, 11.5, 5, 'T10.8');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (12, 11.5, 12, 12.5, 6, 'T12.0');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (13, 0.430000007, 0.455000013, 0.479999989, 1, 'R0.46');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (14, 0.479999989, 0.504999995, 0.529999971, 2, 'R0.5');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (15, 0.529999971, 0.555000007, 0.579999983, 3, 'R0.55');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (16, 0.579999983, 0.629999995, 0.680000007, 4, 'R0.63');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (17, 0.839999974, 0.86500001, 0.889999986, 5, 'R0.87');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (18, 1.32500005, 1.36000001, 1.39499998, 6, 'R1.4');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (19, 1.54999995, 1.60000002, 1.63999999, 7, 'R1.6');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (20, 3.54999995, 3.74000001, 3.93000007, 8, 'T3.7');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (21, 10.3000002, 10.8000002, 11.3000002, 9, 'T10.8');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (22, 11.5, 12, 12.5, 10, 'T12.0');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (1, 0.579999983, 0.629999995, 0.680000007, 1, 'R0.6');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (2, 0.725000024, 0.861999989, 1, 2, 'R0.9');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (3, 1.58000004, 1.61000001, 1.63999999, 3, 'R1.6');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (4, 3.54999995, 3.74000001, 3.93000007, 4, 'T3.7');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (6, 11.5, 12, 12.5, 6, 'T12.0');
INSERT INTO channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (5, 10.3000002, 10.8000002, 11.3000002, 5, 'T10.8');


--
-- Data for Name: instruments; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin) VALUES ('AVHRR/3', 2048, 55.3699989, 1.10000002, 0.0540999994, 0, 6, 4);
INSERT INTO instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin) VALUES ('MSU', 1568, 55.3699989, 1.10000002, 0.0540999994, 0, NULL, 3);
INSERT INTO instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin) VALUES ('MSU-MR', 1572, 55.3699989, 1.10000002, 0.0540999994, 0, NULL, 3);
INSERT INTO instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin) VALUES ('VIRR', 2048, 55.3699989, 1.10000002, 0.0540999994, 0, NULL, 4);


--
-- Data for Name: instr_ch; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO instr_ch (instr_name, channel_id) VALUES ('AVHRR/3', 1);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('AVHRR/3', 2);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('AVHRR/3', 3);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('AVHRR/3', 4);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('AVHRR/3', 5);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('AVHRR/3', 6);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('MSU-MR', 7);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('MSU-MR', 8);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('MSU-MR', 9);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('MSU-MR', 10);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('MSU-MR', 11);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('MSU-MR', 12);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 13);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 14);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 15);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 16);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 17);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 18);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 19);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 20);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 21);
INSERT INTO instr_ch (instr_name, channel_id) VALUES ('VIRR', 22);


--
-- Data for Name: preprocess; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: theme; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO theme (index, th_type, path, date_start, date_end, meta_path, topleft, topright, bottomleft, bottomright, session_id) VALUES (1, 'Псевдоцвета', '/home/maria/src.git/mappi/thematic/algs/test/201410310957_NOAA19s.0.bmp', '2017-11-09 00:00:00', '2017-11-09 10:00:00', '/home/maria/src.git/mappi/thematic/algs/test/201410310957_NOAA19s.meta', NULL, NULL, NULL, NULL, 3);
INSERT INTO theme (index, th_type, path, date_start, date_end, meta_path, topleft, topright, bottomleft, bottomright, session_id) VALUES (2, 'NDVI', '/home/viktor/src.git/mappi/thematic/algs/test/201410310957_NOAA19s.0.bmp', '2017-11-13 11:00:00', '2017-11-13 12:00:00', '/home/viktor/src.git/mappi/thematic/algs/test/201410310957_NOAA19s.meta', NULL, NULL, NULL, NULL, 7);


--
-- Data for Name: pre_theme; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Name: preprocess_index_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('preprocess_index_seq', 12, true);


--
-- Data for Name: satellites; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO satellites (name) VALUES ('NOAA19');


--
-- Data for Name: sat_instr; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: sites; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO sites (name, coords) VALUES ('Санкт-Петербург', NULL);


--
-- Data for Name: sessions; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO sessions (index, satellite_name, date_start, date_end, site, preprocessed, fpath, thematic_done) VALUES (1, 'NOAA19', '2017-11-07 11:00:00', '2017-11-07 11:50:00', 'Санкт-Петербург', true, NULL, false);


--
-- Name: sessions_index_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('sessions_index_seq', 19, true);


--
-- Data for Name: spatial_ref_sys; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: theme_ch; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Name: theme_index_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('theme_index_seq', 3, true);


--
-- PostgreSQL database dump complete
--

