--
-- PostgreSQL database dump
--

-- Dumped from database version 13.6 (Debian 13.6-1.pgdg110+1)
-- Dumped by pg_dump version 14.2 (Debian 14.2-1.pgdg110+1)

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

--
-- Data for Name: instrument_channel; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (8, 1, 1);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (9, 1, 2);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (10, 1, 3);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (11, 1, 4);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (12, 1, 5);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (13, 1, 6);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (14, 4, 7);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (15, 4, 8);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (16, 4, 9);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (17, 4, 10);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (18, 4, 11);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (19, 4, 12);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (20, 3, 13);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (21, 3, 14);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (22, 3, 15);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (23, 3, 16);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (24, 3, 17);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (25, 3, 18);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (26, 3, 19);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (27, 3, 20);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (28, 3, 21);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (29, 3, 22);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (30, 2, 7);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (31, 2, 8);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (32, 2, 9);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (33, 2, 10);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (34, 2, 11);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (35, 2, 12);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (36, 10, 23);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (37, 10, 24);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (38, 10, 25);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (39, 10, 26);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (40, 10, 27);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (41, 10, 28);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (42, 10, 29);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (43, 10, 30);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (44, 10, 31);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (45, 10, 32);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (46, 10, 33);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (47, 10, 34);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (48, 10, 35);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (49, 10, 36);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (50, 10, 37);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (51, 10, 38);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (52, 10, 39);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (53, 10, 40);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (54, 10, 41);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (55, 10, 42);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (56, 10, 43);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (57, 10, 44);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (58, 10, 45);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (59, 9, 46);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (60, 9, 47);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (61, 9, 48);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (62, 9, 49);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (63, 9, 50);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (64, 9, 51);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (65, 9, 52);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (66, 9, 53);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (67, 9, 54);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (68, 9, 55);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (70, 6, 56);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (71, 6, 57);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (72, 6, 58);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (73, 6, 59);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (74, 6, 60);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (76, 7, 61);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (77, 7, 62);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (78, 7, 63);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (79, 7, 64);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (80, 7, 65);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (81, 7, 66);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (82, 7, 67);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (83, 7, 68);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (84, 7, 69);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (85, 7, 71);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (86, 7, 72);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (87, 7, 73);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (88, 7, 74);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (89, 7, 75);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (90, 7, 76);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (91, 8, 77);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (92, 8, 78);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (93, 11 ,79);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (94, 11 ,80);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (95, 11 ,81);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (96, 11 ,82);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (97, 11 ,83);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (98, 11 ,84);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (99, 11 ,85);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (100, 11 ,86);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (101, 11 ,87);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (102, 11 ,88);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (103, 11 ,89);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (104, 11 ,90);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (105, 11 ,91);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (106, 11 ,92);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (107, 11 ,93);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (108, 11 ,94);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (109, 11 ,95);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (110, 11 ,96);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (111, 11 ,97);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (112, 11 ,98);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (113, 11 ,99);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (114, 11 ,100);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (115, 11 ,101);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (116, 11 ,102);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (117, 11 ,103);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (118, 11 ,104);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (119, 11 ,105);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (120, 11 ,106);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (121, 11 ,107);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (122, 11 ,108);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (123, 11 ,109);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (124, 11 ,110);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (125, 11 ,111);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (126, 11 ,112);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (127, 11 ,113);
INSERT INTO meteo.instrument_channel (id, instrument, channel) VALUES (128, 11 ,114);

--
-- Name: instrument_channel_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.instrument_channel_id_seq', 129, true);


--
-- PostgreSQL database dump complete
--

