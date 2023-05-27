--
-- PostgreSQL database dump
--

-- Dumped from database version 11.13 (Debian 11.13-1.pgdg100+1)
-- Dumped by pg_dump version 11.13 (Debian 11.13-1.pgdg100+1)

SET statement_timeout = 0;
SET lock_timeout = 0;
 
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
 

--
-- Data for Name: channel; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (23, 1, 0.39199999, 0.412, 0.432000011, 'R0.41', 'M1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (24, 2, 0.426999986, 0.444999993, 0.463, 'R0.45', 'M2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (25, 3, 0.467999995, 0.488000005, 0.508000016, 'R0.49', 'M3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (26, 4, 0.535000026, 0.555000007, 0.574999988, 'R0.56', 'M4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (27, 5, 0.65200001, 0.671999991, 0.691999972, 'R0.67', 'M5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (28, 6, 0.731000006, 0.745999992, 0.760999978, 'R0.75', 'M6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (29, 7, 0.825999975, 0.86500001, 0.903999984, 'R0.86', 'M7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (30, 8, 1.22000003, 1.24000001, 1.25999999, 'R1.24', 'M8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (31, 9, 1.36300004, 1.37800002, 1.39300001, 'R1.38', 'M9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (32, 10, 1.54999995, 1.61000001, 1.66999996, 'R1.61', 'M10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (33, 11, 2.20000005, 2.25, 2.29999995, 'R2.25', 'M11');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (34, 12, 3.51999998, 3.70000005, 3.88000011, 'T3.7', 'M12');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (35, 13, 3.89499998, 4.05000019, 4.20499992, 'T4', 'M13');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (36, 14, 8.25, 8.55000019, 8.85000038, 'T8.5', 'M14');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (37, 15, 9.76299953, 10.7629995, 11.7629995, 'T10.7', 'M15');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (38, 16, 0.300000012, 0.699999988, 1.10000002, 'R0.7', 'DNB');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (39, 17, 0.300000012, 0.699999988, 1.10000002, 'R0.7', 'DNB_MSG');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (40, 18, 0.300000012, 0.699999988, 1.10000002, 'R0.7', 'DNB_LSG');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (41, 19, 0.560000002, 0.639999986, 0.720000029, 'R0.64', 'I1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (42, 20, 0.825999975, 0.86500001, 0.903999984, 'R0.87', 'I2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (43, 21, 1.54999995, 1.61000001, 1.66999996, 'R1.6', 'I3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (44, 22, 3.3599999, 3.74000001, 4.11999989, 'T3.7', 'I4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (45, 23, 9.55000019, 11.4499998, 13.3500004, 'T11.5', 'I5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (46, 1, 0.5, 0.569999993, 0.649999976, 'R0.57', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (47, 2, 0.649999976, 0.720000029, 0.800000012, 'R0.7', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (48, 3, 0.800000012, 0.860000014, 0.899999976, 'R0.86', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (49, 4, 3.5, 3.75, 4, 'T3.75', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (50, 5, 5.69999981, 6.3499999, 7, 'T6.4', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (51, 6, 7.5, 8, 8.5, 'T8', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (52, 7, 8.19999981, 8.69999981, 9.19999981, 'T8.7', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (53, 8, 9.19999981, 9.69999981, 10.1999998, 'T9.7', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (54, 9, 10.1999998, 10.6999998, 11.1999998, 'T11', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (55, 10, 11.1999998, 11.6999998, 12.5, 'T12', '10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (1, 1, 0.579999983, 0.629999995, 0.680000007, 'R0.6', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (2, 2, 0.725000024, 0.861999989, 1, 'R0.9', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (3, 3, 1.58000004, 1.61000001, 1.63999999, 'R1.6', '3A');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (4, 4, 3.54999995, 3.74000001, 3.93000007, 'T3.7', '3B');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (5, 5, 10.3000002, 10.8000002, 11.3000002, 'T11', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (6, 6, 11.5, 12, 12.5, 'T12', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (7, 1, 0.5, 0.600000024, 0.699999988, 'R0.6', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (8, 2, 0.699999988, 0.899999976, 1.10000002, 'R0.9', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (9, 3, 1.60000002, 1.70000005, 1.79999995, 'R1.6', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (10, 4, 3.5, 3.79999995, 4.0999999, 'T3.7', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (11, 5, 10.5, 11, 11.5, 'T11', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (12, 6, 11.5, 12, 12.5, 'T12', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (13, 1, 0.430000007, 0.455000013, 0.479999989, 'R0.46', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (14, 2, 0.479999989, 0.504999995, 0.529999971, 'R0.5', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (15, 3, 0.529999971, 0.555000007, 0.579999983, 'R0.56', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (16, 4, 0.579999983, 0.629999995, 0.680000007, 'R0.6', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (17, 5, 0.839999974, 0.86500001, 0.889999986, 'R0.9', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (18, 6, 1.32500005, 1.36000001, 1.39499998, 'R1.4', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (19, 7, 1.54999995, 1.60000002, 1.63999999, 'R1.6', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (20, 8, 3.54999995, 3.74000001, 3.93000007, 'T3.7', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (21, 9, 10.3000002, 10.8000002, 11.3000002, 'T11', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (22, 10, 11.5, 12, 12.5, 'T12', '10');


--
-- Data for Name: instrument; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.instrument (id, name) VALUES (1, 'AVHRR/3');
INSERT INTO meteo.instrument (id, name) VALUES (3, 'VIRR');
INSERT INTO meteo.instrument (id, name) VALUES (2, 'MSU-MR(h)');
INSERT INTO meteo.instrument (id, name) VALUES (4, 'MSU-MR(l)');
INSERT INTO meteo.instrument (id, name) VALUES (5, 'MTVZA-GY');
INSERT INTO meteo.instrument (id, name) VALUES (6, 'MHS');
INSERT INTO meteo.instrument (id, name) VALUES (9, 'MSU-GS');
INSERT INTO meteo.instrument (id, name) VALUES (7, 'AMSU-A1');
INSERT INTO meteo.instrument (id, name) VALUES (8, 'AMSU-A2');
INSERT INTO meteo.instrument (id, name) VALUES (10, 'VIIRS');


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


--
-- Data for Name: satellite; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.satellite (id, name) VALUES (1, 'NOAA 19');
INSERT INTO meteo.satellite (id, name) VALUES (2, 'NOAA 18');
INSERT INTO meteo.satellite (id, name) VALUES (3, 'NOAA 15');
INSERT INTO meteo.satellite (id, name) VALUES (4, 'METEOR-M 2');
INSERT INTO meteo.satellite (id, name) VALUES (5, 'METOP-A');
INSERT INTO meteo.satellite (id, name) VALUES (6, 'FENGYUN 3A');
INSERT INTO meteo.satellite (id, name) VALUES (7, 'FENGYUN 3B');
INSERT INTO meteo.satellite (id, name) VALUES (8, 'FENGYUN 3C');
INSERT INTO meteo.satellite (id, name) VALUES (10, 'NOAA 20');
INSERT INTO meteo.satellite (id, name) VALUES (9, 'ELEKTRO-L 2');


--
-- Data for Name: site; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.site (id, name, coord) VALUES (2, 'Санкт-Петербург', '010100000052B81E85EB513E40D7A3703D0AF74D40');


--
-- Data for Name: session; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (154, 1, '2021-10-19 15:57:07', '2021-10-19 16:09:23', 0, 65448, 1, 86.3671036, 2, '20211019T155707_20211019T160923_NOAA19_65448_ASC.raw', 'AAAH5UBWkaYGeeyrPwOLXlOujwU/+7L79kyyoz/+cbX7H4OPP1aMnt9TuA5AGNpTrOnCfD+yYu3yUlDxP6+N+zqZBg4AAAAAAAD0fA==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (155, 4, '2021-10-19 16:16:08', '2021-10-19 16:28:10', 0, 37770, 1, 79.2086029, 2, '20211019T161608_20211019T162810_METEOR-M2_37770_ASC.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (156, 5, '2021-10-19 16:47:22', '2021-10-19 16:59:22', 0, 77841, 1, 88.7437973, 2, '20211019T164722_20211019T165922_METOP-A_77841_ASC.raw', 'AAAH5UBWlO33adljPvOFuzfGYkc/+4BmGeWckkAChnlBW8yyPyR/6cUrF9xAABSsI5koK0ARAv6HP9+xP6/BtWTDjm8AAAAAAAEk0w==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (157, 7, '2021-10-20 04:40:37', '2021-10-20 04:52:48', 0, 56701, 0, 69.4044037, 2, '20211020T044037_20211020T045248_FENGYUN3B_56701_DSС.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (158, 1, '2021-10-20 05:49:41', '2021-10-20 06:01:50', 0, 65456, 0, 69.7957001, 2, '20211020T054941_20211020T060150_NOAA19_65456_DSС.raw', 'AAAH5UBWkaYGeeyrPwOLXlOujwU/+7L79kyyoz/+cbX7H4OPP1aMnt9TuA5AGNpTrOnCfD+yYu3yUlDxP6+N+zqZBg4AAAAAAAD0fA==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (159, 4, '2021-10-20 06:03:54', '2021-10-20 06:15:58', 0, 37778, 0, 89.5446014, 2, '20211020T060354_20211020T061558_METEOR-M2_37778_DSС.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (160, 5, '2021-10-20 06:34:40', '2021-10-20 06:46:38', 0, 77849, 0, 76.8908997, 2, '20211020T063440_20211020T064638_METOP-A_77849_DSС.raw', 'AAAH5UBWlO33adljPvOFuzfGYkc/+4BmGeWckkAChnlBW8yyPyR/6cUrF9xAABSsI5koK0ARAv6HP9+xP6/BtWTDjm8AAAAAAAEk0w==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (161, 3, '2021-10-20 06:39:41', '2021-10-20 06:51:30', 418, 21906, 0, 71.2764969, 2, '20211020T063941_20211020T065130_NOAA15_21906_DSС.raw', 'AAAH5UBWjP9VdahOPv630RIr0ZU/+49KOJbXyEAAq7wzgXYeP1JDZALe15FABCIjPuPDs0AOKLiN2oIJP6/bit6KDNgAAAAAAABKQQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (162, 8, '2021-10-20 07:20:27', '2021-10-20 07:32:30', 0, 41796, 0, 61.8129005, 2, '20211020T072027_20211020T073230_FENGYUN3C_41796_DSС.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (163, 7, '2021-10-20 14:34:54', '2021-10-20 14:47:02', 0, 56707, 1, 67.2057037, 2, '20211020T143454_20211020T144702_FENGYUN3B_56707_ASC.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (164, 1, '2021-10-20 15:45:22', '2021-10-20 15:57:35', 0, 65462, 1, 74.7786026, 2, '20211020T154522_20211020T155735_NOAA19_65462_ASC.raw', 'AAAH5UBWkaYGeeyrPwOLXlOujwU/+7L79kyyoz/+cbX7H4OPP1aMnt9TuA5AGNpTrOnCfD+yYu3yUlDxP6+N+zqZBg4AAAAAAAD0fA==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (165, 4, '2021-10-20 15:56:11', '2021-10-20 16:08:12', 95, 37784, 1, 80.2649994, 2, '20211020T155611_20211020T160812_METEOR-M2_37784_ASC.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (166, 5, '2021-10-20 16:26:41', '2021-10-20 16:38:34', 0, 77855, 1, 68.3582001, 2, '20211020T162641_20211020T163834_METOP-A_77855_ASC.raw', 'AAAH5UBWlO33adljPvOFuzfGYkc/+4BmGeWckkAChnlBW8yyPyR/6cUrF9xAABSsI5koK0ARAv6HP9+xP6/BtWTDjm8AAAAAAAEk0w==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (167, 3, '2021-10-20 16:29:44', '2021-10-20 16:41:37', 531, 21912, 1, 79.0921021, 2, '20211020T162944_20211020T164137_NOAA15_21912_ASC.raw', 'AAAH5UBWjP9VdahOPv630RIr0ZU/+49KOJbXyEAAq7wzgXYeP1JDZALe15FABCIjPuPDs0AOKLiN2oIJP6/bit6KDNgAAAAAAABKQQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (168, 8, '2021-10-20 17:14:42', '2021-10-20 17:26:55', 0, 41802, 1, 66.6682968, 2, '20211020T171442_20211020T172655_FENGYUN3C_41802_ASC.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (169, 6, '2021-10-21 02:48:09', '2021-10-21 03:00:05', 0, 69442, 0, 62.9225006, 2, '20211021T024809_20211021T030005_FENGYUN3A_69442_DSС.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (170, 7, '2021-10-21 04:25:19', '2021-10-21 04:37:35', 0, 56715, 0, 83.9159012, 2, '20211021T042519_20211021T043735_FENGYUN3B_56715_DSС.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (171, 4, '2021-10-21 05:43:57', '2021-10-21 05:55:57', 0, 37792, 0, 69.4588013, 2, '20211021T054357_20211021T055557_METEOR-M2_37792_DSС.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (172, 3, '2021-10-21 06:14:27', '2021-10-21 06:26:20', 0, 21920, 0, 82.9047012, 2, '20211021T061427_20211021T062620_NOAA15_21920_DSС.raw', 'AAAH5UBWjP9VdahOPv630RIr0ZU/+49KOJbXyEAAq7wzgXYeP1JDZALe15FABCIjPuPDs0AOKLiN2oIJP6/bit6KDNgAAAAAAABKQQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (173, 8, '2021-10-21 07:05:15', '2021-10-21 07:17:25', 0, 41810, 0, 74.9459991, 2, '20211021T070515_20211021T071725_FENGYUN3C_41810_DSС.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (174, 6, '2021-10-21 12:41:17', '2021-10-21 12:53:18', 0, 69448, 1, 69.8247986, 2, '20211021T124117_20211021T125318_FENGYUN3A_69448_ASC.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (175, 7, '2021-10-21 14:19:32', '2021-10-21 14:31:44', 0, 56721, 1, 82.2342987, 2, '20211021T141932_20211021T143144_FENGYUN3B_56721_ASC.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (176, 1, '2021-10-21 15:33:39', '2021-10-21 15:45:46', 0, 65476, 1, 64.3238983, 2, '20211021T153339_20211021T154546_NOAA19_65476_ASC.raw', 'AAAH5UBWkaYGeeyrPwOLXlOujwU/+7L79kyyoz/+cbX7H4OPP1aMnt9TuA5AGNpTrOnCfD+yYu3yUlDxP6+N+zqZBg4AAAAAAAD0fA==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (177, 4, '2021-10-21 15:36:22', '2021-10-21 15:48:13', 575, 37798, 1, 62.0638008, 2, '20211021T153622_20211021T154813_METEOR-M2_37798_ASC.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (178, 3, '2021-10-21 16:04:33', '2021-10-21 16:16:23', 0, 21926, 1, 74.759201, 2, '20211021T160433_20211021T161623_NOAA15_21926_ASC.raw', 'AAAH5UBWjP9VdahOPv630RIr0ZU/+49KOJbXyEAAq7wzgXYeP1JDZALe15FABCIjPuPDs0AOKLiN2oIJP6/bit6KDNgAAAAAAABKQQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (179, 8, '2021-10-21 16:59:28', '2021-10-21 17:11:45', 0, 41816, 1, 81.2313995, 2, '20211021T165928_20211021T171145_FENGYUN3C_41816_ASC.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (180, 6, '2021-10-22 02:30:10', '2021-10-22 02:42:15', 0, 69456, 0, 79.1551971, 2, '20211022T023010_20211022T024215_FENGYUN3A_69456_DSС.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (181, 7, '2021-10-22 04:10:00', '2021-10-22 04:22:17', 0, 56729, 0, 80.5598984, 2, '20211022T041000_20211022T042217_FENGYUN3B_56729_DSС.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (182, 8, '2021-10-22 06:50:04', '2021-10-22 07:02:17', 0, 41824, 0, 89.8644028, 2, '20211022T065004_20211022T070217_FENGYUN3C_41824_DSС.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (183, 2, '2021-10-22 09:21:38', '2021-10-22 09:33:54', 0, 84651, 0, 65.3487015, 2, '20211022T092138_20211022T093354_NOAA18_84651_DSС.raw', 'AAAH5UBWj/Imw7koPxZJzASDev0/+6WHscQqT0AFnGF4VckuP1i4yQnJrV5AA5pJqIvF50AOsc+qYRewP6+Owcr+YB4AAAAAAAE/VQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (184, 6, '2021-10-22 12:23:18', '2021-10-22 12:35:22', 0, 69462, 1, 87.8852005, 2, '20211022T122318_20211022T123522_FENGYUN3A_69462_ASC.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (185, 7, '2021-10-22 14:04:14', '2021-10-22 14:16:26', 0, 56735, 1, 82.1417999, 2, '20211022T140414_20211022T141626_FENGYUN3B_56735_ASC.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (186, 7, '2021-10-20 04:40:38', '2021-10-20 04:52:49', 0, 56701, 0, 69.4044037, 2, '20211020T044038_20211020T045249_FENGYUN3B_56701_DSС.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (187, 1, '2021-10-20 05:49:41', '2021-10-20 06:01:50', 0, 65456, 0, 69.7957001, 2, '20211020T054941_20211020T060150_NOAA19_65456_DSС.raw', 'AAAH5UBWkaYGeeyrPwOLXlOujwU/+7L79kyyoz/+cbX7H4OPP1aMnt9TuA5AGNpTrOnCfD+yYu3yUlDxP6+N+zqZBg4AAAAAAAD0fA==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (188, 4, '2021-10-20 06:03:54', '2021-10-20 06:15:58', 0, 37778, 0, 89.5446014, 2, '20211020T060354_20211020T061558_METEOR-M2_37778_DSС.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (189, 5, '2021-10-20 06:34:40', '2021-10-20 06:46:38', 0, 77849, 0, 76.8908997, 2, '20211020T063440_20211020T064638_METOP-A_77849_DSС.raw', 'AAAH5UBWlO33adljPvOFuzfGYkc/+4BmGeWckkAChnlBW8yyPyR/6cUrF9xAABSsI5koK0ARAv6HP9+xP6/BtWTDjm8AAAAAAAEk0w==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (190, 3, '2021-10-20 06:39:41', '2021-10-20 06:51:30', 418, 21906, 0, 71.2764969, 2, '20211020T063941_20211020T065130_NOAA15_21906_DSС.raw', 'AAAH5UBWjP9VdahOPv630RIr0ZU/+49KOJbXyEAAq7wzgXYeP1JDZALe15FABCIjPuPDs0AOKLiN2oIJP6/bit6KDNgAAAAAAABKQQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (191, 8, '2021-10-20 07:20:27', '2021-10-20 07:32:30', 0, 41796, 0, 61.8129005, 2, '20211020T072027_20211020T073230_FENGYUN3C_41796_DSС.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (192, 7, '2021-10-20 14:34:54', '2021-10-20 14:47:02', 0, 56707, 1, 67.2057037, 2, '20211020T143454_20211020T144702_FENGYUN3B_56707_ASC.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (193, 1, '2021-10-20 15:45:22', '2021-10-20 15:57:35', 0, 65462, 1, 74.7786026, 2, '20211020T154522_20211020T155735_NOAA19_65462_ASC.raw', 'AAAH5UBWkaYGeeyrPwOLXlOujwU/+7L79kyyoz/+cbX7H4OPP1aMnt9TuA5AGNpTrOnCfD+yYu3yUlDxP6+N+zqZBg4AAAAAAAD0fA==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (194, 4, '2021-10-20 15:56:11', '2021-10-20 16:08:12', 95, 37784, 1, 80.2649994, 2, '20211020T155611_20211020T160812_METEOR-M2_37784_ASC.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (195, 5, '2021-10-20 16:26:41', '2021-10-20 16:38:34', 0, 77855, 1, 68.3582001, 2, '20211020T162641_20211020T163834_METOP-A_77855_ASC.raw', 'AAAH5UBWlO33adljPvOFuzfGYkc/+4BmGeWckkAChnlBW8yyPyR/6cUrF9xAABSsI5koK0ARAv6HP9+xP6/BtWTDjm8AAAAAAAEk0w==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (196, 3, '2021-10-20 16:29:44', '2021-10-20 16:41:37', 531, 21912, 1, 79.0921021, 2, '20211020T162944_20211020T164137_NOAA15_21912_ASC.raw', 'AAAH5UBWjP9VdahOPv630RIr0ZU/+49KOJbXyEAAq7wzgXYeP1JDZALe15FABCIjPuPDs0AOKLiN2oIJP6/bit6KDNgAAAAAAABKQQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (197, 8, '2021-10-20 17:14:42', '2021-10-20 17:26:55', 0, 41802, 1, 66.6682968, 2, '20211020T171442_20211020T172655_FENGYUN3C_41802_ASC.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (198, 6, '2021-10-21 02:48:09', '2021-10-21 03:00:05', 0, 69442, 0, 62.9225006, 2, '20211021T024809_20211021T030005_FENGYUN3A_69442_DSС.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (199, 7, '2021-10-21 04:25:19', '2021-10-21 04:37:35', 0, 56715, 0, 83.9159012, 2, '20211021T042519_20211021T043735_FENGYUN3B_56715_DSС.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (200, 4, '2021-10-21 05:43:57', '2021-10-21 05:55:57', 0, 37792, 0, 69.4588013, 2, '20211021T054357_20211021T055557_METEOR-M2_37792_DSС.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (201, 3, '2021-10-21 06:14:27', '2021-10-21 06:26:20', 0, 21920, 0, 82.9047012, 2, '20211021T061427_20211021T062620_NOAA15_21920_DSС.raw', 'AAAH5UBWjP9VdahOPv630RIr0ZU/+49KOJbXyEAAq7wzgXYeP1JDZALe15FABCIjPuPDs0AOKLiN2oIJP6/bit6KDNgAAAAAAABKQQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (202, 8, '2021-10-21 07:05:15', '2021-10-21 07:17:25', 0, 41810, 0, 74.9459991, 2, '20211021T070515_20211021T071725_FENGYUN3C_41810_DSС.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (203, 6, '2021-10-21 12:41:17', '2021-10-21 12:53:18', 0, 69448, 1, 69.8247986, 2, '20211021T124117_20211021T125318_FENGYUN3A_69448_ASC.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (204, 7, '2021-10-21 14:19:32', '2021-10-21 14:31:44', 0, 56721, 1, 82.2342987, 2, '20211021T141932_20211021T143144_FENGYUN3B_56721_ASC.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (205, 1, '2021-10-21 15:33:39', '2021-10-21 15:45:46', 0, 65476, 1, 64.3238983, 2, '20211021T153339_20211021T154546_NOAA19_65476_ASC.raw', 'AAAH5UBWkaYGeeyrPwOLXlOujwU/+7L79kyyoz/+cbX7H4OPP1aMnt9TuA5AGNpTrOnCfD+yYu3yUlDxP6+N+zqZBg4AAAAAAAD0fA==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (206, 4, '2021-10-21 15:36:22', '2021-10-21 15:48:13', 575, 37798, 1, 62.0638008, 2, '20211021T153622_20211021T154813_METEOR-M2_37798_ASC.raw', 'AAAH5UBWlR09J6i7PvVCQqvdTzc/+39/gdth70ABGv41dPoMP0Pj4pMHryFABgeOfBLRlkAMQZGDaxGDP6+9BTwsKCUAAAAAAACITg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (207, 3, '2021-10-21 16:04:33', '2021-10-21 16:16:23', 0, 21926, 1, 74.759201, 2, '20211021T160433_20211021T161623_NOAA15_21926_ASC.raw', 'AAAH5UBWjP9VdahOPv630RIr0ZU/+49KOJbXyEAAq7wzgXYeP1JDZALe15FABCIjPuPDs0AOKLiN2oIJP6/bit6KDNgAAAAAAABKQQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (208, 8, '2021-10-21 16:59:28', '2021-10-21 17:11:45', 0, 41816, 1, 81.2313995, 2, '20211021T165928_20211021T171145_FENGYUN3C_41816_ASC.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (209, 6, '2021-10-22 02:30:10', '2021-10-22 02:42:15', 0, 69456, 0, 79.1551971, 2, '20211022T023010_20211022T024215_FENGYUN3A_69456_DSС.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (210, 7, '2021-10-22 04:10:00', '2021-10-22 04:22:17', 0, 56729, 0, 80.5598984, 2, '20211022T041000_20211022T042217_FENGYUN3B_56729_DSС.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (211, 8, '2021-10-22 06:50:04', '2021-10-22 07:02:17', 0, 41824, 0, 89.8644028, 2, '20211022T065004_20211022T070217_FENGYUN3C_41824_DSС.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (212, 2, '2021-10-22 09:21:38', '2021-10-22 09:33:54', 0, 84651, 0, 65.3487015, 2, '20211022T092138_20211022T093354_NOAA18_84651_DSС.raw', 'AAAH5UBWj/Imw7koPxZJzASDev0/+6WHscQqT0AFnGF4VckuP1i4yQnJrV5AA5pJqIvF50AOsc+qYRewP6+Owcr+YB4AAAAAAAE/VQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (213, 6, '2021-10-22 12:23:18', '2021-10-22 12:35:22', 0, 69462, 1, 87.8852005, 2, '20211022T122318_20211022T123522_FENGYUN3A_69462_ASC.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (214, 7, '2021-10-22 14:04:14', '2021-10-22 14:16:26', 0, 56735, 1, 82.1417999, 2, '20211022T140414_20211022T141626_FENGYUN3B_56735_ASC.raw', 'AAAH5UBWjzoPex0tPwNoVm9nL9Q/+6rZS+gvPD/3Ifynpk7bP1wAEMb3oLZACAA3dzjSU0AKSPRtksyxP6+iIUm/7bAAAAAAAADSQg==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (215, 8, '2021-10-22 16:44:18', '2021-10-22 16:56:34', 0, 41830, 1, 83.4834976, 2, '20211022T164418_20211022T165634_FENGYUN3C_41830_ASC.raw', 'AAAH5UBWlRmmREzhPv7H7D1Vdf4/+4JWD6I3JEACypd2QNgpP1QMkd66NRs//l+5bFtiZEARjoYqN37VP6+hdQfg/LUAAAAAAACYCQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (216, 2, '2021-10-22 19:17:17', '2021-10-22 19:29:34', 0, 84656, 1, 62.8462982, 2, '20211022T191717_20211022T192934_NOAA18_84656_ASC.raw', 'AAAH5UBWj/Imw7koPxZJzASDev0/+6WHscQqT0AFnGF4VckuP1i4yQnJrV5AA5pJqIvF50AOsc+qYRewP6+Owcr+YB4AAAAAAAE/VQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (217, 10, '2021-10-21 10:17:24', '2021-10-21 10:29:27', 0, 20334, 1, 87.893898, 2, '20211021T101724_20211021T102927_NOAA20_20334_ASC.raw', 'AAAH5UBWig2yT/PePv0uSlFuF/I/+5D6IIl+gT/gw4XkuDuhPxcYSnmg1c4/+uNJxAHE5UASa3EmhklWP6+2fBVvftcAAAAAAABEGQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (218, 10, '2021-10-21 10:17:23', '2021-10-21 10:29:26', 0, 20334, 1, 87.893898, 2, '20211021T101723_20211021T102926_NOAA20_20334_ASC.raw', 'AAAH5UBWig2yT/PePv0uSlFuF/I/+5D6IIl+gT/gw4XkuDuhPxcYSnmg1c4/+uNJxAHE5UASa3EmhklWP6+2fBVvftcAAAAAAABEGQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (219, 6, '2021-10-21 12:41:16', '2021-10-21 12:53:17', 0, 69448, 1, 69.8247986, 2, '20211021T124116_20211021T125317_FENGYUN3A_69448_ASC.raw', 'AAAH5UBWkTicHgNLPvgfFtfEB+g/+32RYKDkRj/y45+X50P8P0nT5pyBq2ZADsGwVzXodUADhGXcRfv0P6+xZs5zh6QAAAAAAAED9Q==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (220, 10, '2021-10-22 00:05:49', '2021-10-22 00:17:51', 0, 20342, 0, 77.9121017, 2, '20211022T000549_20211022T001751_NOAA20_20342_DSС.raw', 'AAAH5UBWig2yT/PePv0uSlFuF/I/+5D6IIl+gT/gw4XkuDuhPxcYSnmg1c4/+uNJxAHE5UASa3EmhklWP6+2fBVvftcAAAAAAABEGQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (221, 10, '2021-10-22 09:58:37', '2021-10-22 10:10:35', 0, 20348, 1, 73.1738968, 2, '20211022T095837_20211022T101035_NOAA20_20348_ASC.raw', 'AAAH5UBWig2yT/PePv0uSlFuF/I/+5D6IIl+gT/gw4XkuDuhPxcYSnmg1c4/+uNJxAHE5UASa3EmhklWP6+2fBVvftcAAAAAAABEGQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (222, 10, '2021-10-22 23:47:00', '2021-10-22 23:58:55', 0, 20356, 0, 60.1651993, 2, '20211022T234700_20211022T235855_NOAA20_20356_DSС.raw', 'AAAH5UBWig2yT/PePv0uSlFuF/I/+5D6IIl+gT/gw4XkuDuhPxcYSnmg1c4/+uNJxAHE5UASa3EmhklWP6+2fBVvftcAAAAAAABEGQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (223, 10, '2021-10-22 00:05:50', '2021-10-22 00:17:52', 0, 20342, 0, 77.9121017, 2, '20211022T000550_20211022T001752_NOAA20_20342_DSС.raw', 'AAAH5UBWig2yT/PePv0uSlFuF/I/+5D6IIl+gT/gw4XkuDuhPxcYSnmg1c4/+uNJxAHE5UASa3EmhklWP6+2fBVvftcAAAAAAABEGQ==');
INSERT INTO meteo.session (id, satellite, date_start, date_end, date_start_offset, revol, direction, elevat_max, site, fpath, tle) VALUES (224, 10, '2021-10-22 09:58:38', '2021-10-22 10:10:36', 0, 20348, 1, 73.1738968, 2, '20211022T095838_20211022T101036_NOAA20_20348_ASC.raw', 'AAAH5UBWig2yT/PePv0uSlFuF/I/+5D6IIl+gT/gw4XkuDuhPxcYSnmg1c4/+uNJxAHE5UASa3EmhklWP6+2fBVvftcAAAAAAABEGQ==');


--
-- Data for Name: pretreatment; Type: TABLE DATA; Schema: meteo; Owner: postgres
--



--
-- Data for Name: pretreatment_2021_10_19; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.pretreatment_2021_10_19 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (155, 30, '2021-10-19 07:51:33', '2021-10-19 07:58:43', false, '/home/maria/satdata//pretreatment//20211019_0751/202110190751_202110190758_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_19 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (155, 31, '2021-10-19 07:51:33', '2021-10-19 07:58:43', false, '/home/maria/satdata//pretreatment//20211019_0751/202110190751_202110190758_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_19 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (155, 32, '2021-10-19 07:51:33', '2021-10-19 07:58:43', false, '/home/maria/satdata//pretreatment//20211019_0751/202110190751_202110190758_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_19 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (155, 33, '2021-10-19 07:51:33', '2021-10-19 07:58:43', false, '/home/maria/satdata//pretreatment//20211019_0751/202110190751_202110190758_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_19 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (155, 34, '2021-10-19 07:51:33', '2021-10-19 07:58:43', false, '/home/maria/satdata//pretreatment//20211019_0751/202110190751_202110190758_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_19 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (155, 35, '2021-10-19 07:51:33', '2021-10-19 07:58:43', false, '/home/maria/satdata//pretreatment//20211019_0751/202110190751_202110190758_METEOR-M2_2_06.po');


--
-- Data for Name: pretreatment_2021_10_20; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (159, 30, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (159, 31, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (159, 32, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (159, 33, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (159, 34, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (159, 35, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_06.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (165, 30, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (165, 31, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (165, 32, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (165, 33, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (165, 34, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (165, 35, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_06.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (188, 30, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (188, 31, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (188, 32, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (188, 33, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (188, 34, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (188, 35, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_06.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (194, 30, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (194, 31, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (194, 32, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (194, 33, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (194, 34, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_20 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (194, 35, '2021-10-20 07:51:33', '2021-10-20 07:58:43', false, '/home/maria/satdata//pretreatment//20211020_0751/202110200751_202110200758_METEOR-M2_2_06.po');


--
-- Data for Name: pretreatment_2021_10_21; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (171, 30, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (171, 31, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (171, 32, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (171, 33, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (171, 34, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (171, 35, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_06.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (177, 30, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (177, 31, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (177, 32, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (177, 33, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (177, 34, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (177, 35, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_06.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (200, 30, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (200, 31, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (200, 32, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (200, 33, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (200, 34, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (200, 35, '2021-10-21 07:51:33', '2021-10-21 07:58:43', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210758_METEOR-M2_2_06.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (206, 30, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_01.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (206, 31, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_02.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (206, 32, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_03.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (206, 33, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_04.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (206, 34, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_05.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (206, 35, '2021-10-21 07:51:33', '2021-10-21 07:53:49', false, '/home/maria/satdata//pretreatment//20211021_0751/202110210751_202110210753_METEOR-M2_2_06.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 20, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_01.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 21, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_02.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 22, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_03.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 23, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_04.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 24, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_05.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 25, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_06.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 26, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_07.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 27, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_08.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 28, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_09.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (219, 29, '2021-10-21 12:41:16', '2021-10-21 13:06:02', false, '/home/maria/satdata//pretreatment//20211021_1241/202110211241_202110211306_FENGYUN3A_3_10.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 36, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_01.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 37, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_02.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 38, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_03.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 39, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_04.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 40, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_05.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 41, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_06.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 43, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_08.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 44, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_09.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 45, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_10.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 46, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_11.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 47, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_12.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 48, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_13.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 49, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_14.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 50, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_15.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 51, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_16.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 52, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_17.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 55, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_20.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 56, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_21.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 57, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_22.po');
INSERT INTO meteo.pretreatment_2021_10_21 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (224, 58, '2021-10-21 10:17:23', '2021-10-21 10:29:26', false, '/home/maria/satdata//pretreatment//20211021_1017/202110211017_NOAA20_10_23.po');


--
-- Data for Name: pretreatment_2021_10_31; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (183, 8, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_01.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (183, 9, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_02.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (183, 11, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_04.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (183, 12, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_05.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (183, 13, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_06.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (212, 8, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_01.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (212, 9, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_02.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (212, 11, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_04.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (212, 12, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_05.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (212, 13, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_06.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (216, 8, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_01.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (216, 9, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_02.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (216, 11, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_04.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (216, 12, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_05.po');
INSERT INTO meteo.pretreatment_2021_10_31 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (216, 13, '2021-10-31 09:57:54', '2021-10-31 10:05:34', true, '/home/maria/satdata//pretreatment//20211031_0957/202110310957_202110311005_NOAA18_1_06.po');


--
-- Data for Name: pretreatment_2021_3_1; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_01.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (161, 9, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_02.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (161, 11, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_04.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_05.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (161, 13, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_06.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (167, 8, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_01.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (167, 9, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_02.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (167, 11, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_04.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (167, 12, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_05.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (167, 13, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_06.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (172, 8, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_01.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (172, 9, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_02.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (172, 11, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_04.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (172, 12, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_05.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (172, 13, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_06.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (178, 8, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_01.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (178, 9, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_02.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (178, 11, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_04.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (178, 12, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_05.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (178, 13, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_06.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (190, 8, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_01.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (190, 9, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_02.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (190, 11, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_04.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (190, 12, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_05.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (190, 13, '2021-03-01 12:45:01', '2021-03-01 12:48:03', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011248_NOAA15_1_06.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (196, 8, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_01.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (196, 9, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_02.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (196, 11, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_04.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (196, 12, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_05.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (196, 13, '2021-03-01 12:45:01', '2021-03-01 12:46:55', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011246_NOAA15_1_06.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (201, 8, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_01.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (201, 9, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_02.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (201, 11, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_04.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (201, 12, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_05.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (201, 13, '2021-03-01 12:45:01', '2021-03-01 12:52:26', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_06.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (207, 8, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_01.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (207, 9, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_02.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (207, 11, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_04.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (207, 12, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_05.po');
INSERT INTO meteo.pretreatment_2021_3_1 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (207, 13, '2021-03-01 12:45:01', '2021-03-01 12:52:25', true, '/home/maria/satdata//pretreatment//20210301_1245/202103011245_202103011252_NOAA15_1_06.po');


--
-- Data for Name: pretreatment_2021_7_12; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (154, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:16', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_01.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (154, 9, '2021-07-12 13:41:37', '2021-07-12 13:49:16', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_02.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (154, 11, '2021-07-12 13:41:37', '2021-07-12 13:49:16', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_04.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (154, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:16', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_05.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (154, 13, '2021-07-12 13:41:37', '2021-07-12 13:49:16', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_06.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (158, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_01.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (158, 9, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_02.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (158, 11, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_04.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (158, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_05.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (158, 13, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_06.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (164, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_01.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (164, 9, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_02.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (164, 11, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_04.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (164, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_05.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (164, 13, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_06.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (176, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_01.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (176, 9, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_02.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (176, 11, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_04.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (176, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_05.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (176, 13, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_06.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (187, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_01.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (187, 9, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_02.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (187, 11, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_04.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (187, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_05.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (187, 13, '2021-07-12 13:41:37', '2021-07-12 13:49:12', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_06.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (193, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_01.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (193, 9, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_02.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (193, 11, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_04.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (193, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_05.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (193, 13, '2021-07-12 13:41:37', '2021-07-12 13:49:14', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_06.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (205, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_01.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (205, 9, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_02.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (205, 11, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_04.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (205, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_05.po');
INSERT INTO meteo.pretreatment_2021_7_12 (session, instrument_channel, date_start, date_end, calibration, fpath) VALUES (205, 13, '2021-07-12 13:41:37', '2021-07-12 13:49:11', true, '/home/maria/satdata//pretreatment//20210712_1341/202107121341_202107121349_NOAA19_1_06.po');


--
-- Data for Name: thematic_type; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.thematic_type (id, name, title) VALUES (1, 'Grayscale', 'Оттенки серого');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (2, 'FalseColor', 'Псевдоцвета');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (3, 'Airmass', 'Воздушные массы');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (4, 'Dust', 'Пыль');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (5, 'Ash', 'Пепел');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (6, 'Microphysics', 'Микрофизика (24ч)');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (7, 'DayMicrophysics', 'Микрофизика (день)');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (8, 'NightMicrophysics', 'Микрофизика (ночь)');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (9, 'SevereStorm', 'Шторм');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (10, 'Snow', 'Снег');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (11, 'Natural', 'Естественные цвета');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (12, 'Clouds', 'Облака');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (13, 'Fog', 'Туман');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (14, 'TrueColor', 'Настоящий цвет');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (15, 'CloudPhase', 'Фаза облаков');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (16, 'CloudType', 'Тип облаков');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (17, 'NDVI', 'Вегетационный индекс');
INSERT INTO meteo.thematic_type (id, name, title) VALUES (18, 'Index8', 'Пользовательский');


--
-- Data for Name: thematic; Type: TABLE DATA; Schema: meteo; Owner: postgres
--



--
-- Data for Name: thematic_2021_10_19; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 1, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 1, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 1, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 1, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 1, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 1, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 17, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 2, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 7, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 8, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 12, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_19 (session, type, date_start, date_end, fpath, instrument) VALUES (155, 11, '2021-10-19 07:51:33', '2021-10-19 07:58:43', '/home/maria/satdata//thematics/2021-10-19/202110195107_METEOR-M2_rgb.11.to', 2);


--
-- Data for Name: thematic_2021_10_20; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 17, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 2, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 7, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 8, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 12, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 11, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.11.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 1, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 17, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 2, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 7, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 8, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 12, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_20 (session, type, date_start, date_end, fpath, instrument) VALUES (159, 11, '2021-10-20 07:51:33', '2021-10-20 07:58:43', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_rgb.11.to', 2);


--
-- Data for Name: thematic_2021_10_21; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 17, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 2, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 7, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 8, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 12, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 11, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.11.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 17, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 2, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 7, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 8, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 12, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 11, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.11.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 17, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 2, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 7, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 8, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 12, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 11, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.11.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-20/202110205107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 17, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 2, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 7, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 8, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 12, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 11, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.11.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 17, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 2, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 7, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 8, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 12, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 11, '2021-10-21 07:51:33', '2021-10-21 07:58:43', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.11.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.1.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.3.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.4.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.5.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 1, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_2.6.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 17, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_ndvi.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 2, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.2.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 7, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.7.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 8, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.8.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 12, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.12.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (171, 11, '2021-10-21 07:51:33', '2021-10-21 07:53:49', '/home/maria/satdata//thematics/2021-10-21/202110215107_METEOR-M2_rgb.11.to', 2);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.1.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.2.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.3.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.4.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.5.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.6.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.7.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.8.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.9.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 1, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_3.10.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 17, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_ndvi.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 2, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_rgb.2.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 7, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_rgb.7.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 8, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_rgb.8.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 12, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_rgb.12.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 11, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_rgb.11.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (219, 14, '2021-10-21 12:41:16', '2021-10-21 13:06:02', '/home/maria/satdata//thematics/2021-10-21/202110214112_FENGYUN3A_rgb.14.to', 3);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.1.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.2.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.3.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.4.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.5.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.6.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.8.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.9.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.10.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.11.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.12.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.13.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.14.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.15.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.16.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.17.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.20.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.21.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.22.to', 10);
INSERT INTO meteo.thematic_2021_10_21 (session, type, date_start, date_end, fpath, instrument) VALUES (224, 1, '2021-10-21 10:17:23', '2021-10-21 10:29:26', '/home/maria/satdata//thematics/2021-10-21/202110211710_NOAA20_10.23.to', 10);


--
-- Data for Name: thematic_2021_10_31; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.1.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.2.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.4.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.5.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.6.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 17, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 2, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 8, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 12, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.1.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.2.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.4.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.5.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.6.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 17, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 2, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 8, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 12, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.1.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.2.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.4.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.5.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 1, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_1.6.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 17, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 2, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 8, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_10_31 (session, type, date_start, date_end, fpath, instrument) VALUES (183, 12, '2021-10-31 09:57:54', '2021-10-31 10:05:34', '/home/maria/satdata//thematics/2021-10-31/202110315709_NOAA18_rgb.12.to', 1);


--
-- Data for Name: thematic_2021_3_1; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.1.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.4.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.5.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.6.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 17, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 2, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.1.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.4.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.5.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.6.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 17, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 2, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.1.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.4.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.5.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.6.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 17, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 2, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.1.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.4.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.5.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.6.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 17, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 2, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.1.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.4.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.5.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.6.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 17, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 2, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:48:03', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.1.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.4.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.5.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.6.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 17, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 2, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:46:55', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.1.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.4.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.5.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.6.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 17, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 2, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:52:26', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.1.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.4.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.5.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 1, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_1.6.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 17, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 2, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 8, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_3_1 (session, type, date_start, date_end, fpath, instrument) VALUES (161, 12, '2021-03-01 12:45:01', '2021-03-01 12:52:25', '/home/maria/satdata//thematics/2021-03-01/202103014512_NOAA15_rgb.12.to', 1);


--
-- Data for Name: thematic_2021_7_12; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.1.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.4.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.5.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.6.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 17, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 2, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:16', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.1.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.4.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.5.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.6.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 17, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 2, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.1.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.4.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.5.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.6.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 17, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 2, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.1.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.4.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.5.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.6.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 17, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 2, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.1.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.4.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.5.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.6.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 17, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 2, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:12', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.1.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.4.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.5.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.6.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 17, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 2, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:14', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.12.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.1.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.4.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.5.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 1, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_1.6.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 17, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_ndvi.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 2, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.2.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 8, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.8.to', 1);
INSERT INTO meteo.thematic_2021_7_12 (session, type, date_start, date_end, fpath, instrument) VALUES (154, 12, '2021-07-12 13:41:37', '2021-07-12 13:49:11', '/home/maria/satdata//thematics/2021-07-12/202107124113_NOAA19_rgb.12.to', 1);


--
-- Data for Name: channels; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (7, 0.5, 0.600000024, 0.699999988, 1, 'R0.6');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (8, 0.699999988, 0.899999976, 1.10000002, 2, 'R0.9');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (9, 1.60000002, 1.70000005, 1.79999995, 3, 'R1.6');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (10, 3.5, 3.79999995, 4.0999999, 4, 'T3.7');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (11, 10.5, 11, 11.5, 5, 'T10.8');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (12, 11.5, 12, 12.5, 6, 'T12.0');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (13, 0.430000007, 0.455000013, 0.479999989, 1, 'R0.46');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (14, 0.479999989, 0.504999995, 0.529999971, 2, 'R0.5');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (15, 0.529999971, 0.555000007, 0.579999983, 3, 'R0.55');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (16, 0.579999983, 0.629999995, 0.680000007, 4, 'R0.63');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (17, 0.839999974, 0.86500001, 0.889999986, 5, 'R0.87');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (18, 1.32500005, 1.36000001, 1.39499998, 6, 'R1.4');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (19, 1.54999995, 1.60000002, 1.63999999, 7, 'R1.6');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (20, 3.54999995, 3.74000001, 3.93000007, 8, 'T3.7');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (21, 10.3000002, 10.8000002, 11.3000002, 9, 'T10.8');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (22, 11.5, 12, 12.5, 10, 'T12.0');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (1, 0.579999983, 0.629999995, 0.680000007, 1, 'R0.6');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (2, 0.725000024, 0.861999989, 1, 2, 'R0.9');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (3, 1.58000004, 1.61000001, 1.63999999, 3, 'R1.6');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (4, 3.54999995, 3.74000001, 3.93000007, 4, 'T3.7');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (6, 11.5, 12, 12.5, 6, 'T12.0');
INSERT INTO public.channels (id, lambda_min, lambda_center, lambda_max, channel, alias) VALUES (5, 10.3000002, 10.8000002, 11.3000002, 5, 'T10.8');


--
-- Data for Name: instruments; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin, type) VALUES ('AVHRR/3', 2048, 55.3699989, 1.10000002, 0.0540999994, 0, 6, 4, 1);
INSERT INTO public.instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin, type) VALUES ('MSU', 1568, 55.3699989, 1.10000002, 0.0540999994, 0, NULL, 3, 2);
INSERT INTO public.instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin, type) VALUES ('MSU-MR', 1572, 55.3699989, 1.10000002, 0.0540999994, 0, NULL, 3, 4);
INSERT INTO public.instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin, type) VALUES ('VIRR', 2048, 55.3699989, 1.10000002, 0.0540999994, 0, NULL, 4, 3);
INSERT INTO public.instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin, type) VALUES ('MHS', 90, 55.3699989, 1.10000002, 0.0540999994, 0, 6, 1, 5);
INSERT INTO public.instruments (name, samples, scan_angle, fov, fovstep, direction, velocity, rt_thin, type) VALUES ('AVHRR/3', 2048, 55.3699989, 1.10000002, 0.0540999994, 0, 6, 4, 10);


--
-- Data for Name: instr_ch; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (1, 1);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (2, 1);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (3, 1);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (4, 1);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (5, 1);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (6, 1);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (7, 4);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (8, 4);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (9, 4);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (10, 4);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (11, 4);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (12, 4);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (13, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (14, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (15, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (16, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (17, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (18, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (19, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (20, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (21, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (22, 3);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (7, 2);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (8, 2);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (9, 2);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (10, 2);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (11, 2);
INSERT INTO public.instr_ch (channel_id, instr_type) VALUES (12, 2);


--
-- Data for Name: preprocess; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: theme_types; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.theme_types (index, name, title) VALUES (1, 'grayscale', 'Оттенки серого');
INSERT INTO public.theme_types (index, name, title) VALUES (4, 'index16', NULL);
INSERT INTO public.theme_types (index, name, title) VALUES (5, 'index24', NULL);
INSERT INTO public.theme_types (index, name, title) VALUES (6, 'index32', NULL);
INSERT INTO public.theme_types (index, name, title) VALUES (3, 'NDVI', 'NDVI');
INSERT INTO public.theme_types (index, name, title) VALUES (2, 'FalseColor', 'Псевдоцвета');
INSERT INTO public.theme_types (index, name, title) VALUES (7, 'DayMicrophysics', 'Микрофизика (день)');
INSERT INTO public.theme_types (index, name, title) VALUES (8, 'Clouds', 'Облака');
INSERT INTO public.theme_types (index, name, title) VALUES (9, 'Natural', 'Естественные цвета');
INSERT INTO public.theme_types (index, name, title) VALUES (10, 'NightMicrophysics', 'Микрофизика (ночь)');


--
-- Data for Name: theme; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: pre_theme; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: satellites; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.satellites (name) VALUES ('NOAA 19');
INSERT INTO public.satellites (name) VALUES ('NOAA 18');
INSERT INTO public.satellites (name) VALUES ('NOAA 15');
INSERT INTO public.satellites (name) VALUES ('METEOR-M 2');
INSERT INTO public.satellites (name) VALUES ('FENGYUN 3A');
INSERT INTO public.satellites (name) VALUES ('FENFYUN 3B');
INSERT INTO public.satellites (name) VALUES ('FENGYUN 3C');
INSERT INTO public.satellites (name) VALUES ('METOP-A');


--
-- Data for Name: sat_instr; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: sites; Type: TABLE DATA; Schema: public; Owner: postgres
--

INSERT INTO public.sites (name, coords) VALUES ('Санкт-Петербург', NULL);


--
-- Data for Name: sessions; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: spatial_ref_sys; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Data for Name: theme_ch; Type: TABLE DATA; Schema: public; Owner: postgres
--



--
-- Name: channel_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.channel_id_seq', 23, true);


--
-- Name: instrument_channel_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.instrument_channel_id_seq', 69, true);


--
-- Name: instrument_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.instrument_id_seq', 10, true);


--
-- Name: satellite_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.satellite_id_seq', 9, true);


--
-- Name: session_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.session_id_seq', 224, true);


--
-- Name: site_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.site_id_seq', 2, true);


--
-- Name: thematic_type_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.thematic_type_id_seq', 10, true);


--
-- Name: preprocess_index_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.preprocess_index_seq', 409, true);


--
-- Name: sessions_index_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.sessions_index_seq', 173, true);


--
-- Name: theme_index_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.theme_index_seq', 198, true);


--
-- PostgreSQL database dump complete
--

