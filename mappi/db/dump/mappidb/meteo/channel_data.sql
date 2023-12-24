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
-- Data for Name: channel; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (1, 1, 0.58, 0.63, 0.68, 'R0.6', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (2, 2, 0.725, 0.862, 1, 'R0.9', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (3, 3, 1.58, 1.61, 1.64, 'R1.6', '3A');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (4, 4, 3.55, 3.74, 3.93, 'T3.7', '3B');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (5, 5, 10.3, 10.8, 11.3, 'T11', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (6, 6, 11.5, 12, 12.5, 'T12', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (7, 1, 0.5, 0.6, 0.7, 'R0.6', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (8, 2, 0.7, 0.9, 1.1, 'R0.9', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (9, 3, 1.6, 1.7, 1.8, 'R1.6', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (10, 4, 3.5, 3.8, 4.1, 'T3.7', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (11, 5, 10.5, 11, 11.5, 'T11', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (12, 6, 11.5, 12, 12.5, 'T12', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (13, 1, 0.43, 0.455, 0.48, 'R0.46', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (14, 2, 0.48, 0.505, 0.53, 'R0.5', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (15, 3, 0.53, 0.555, 0.58, 'R0.56', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (16, 4, 0.58, 0.63, 0.68, 'R0.6', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (17, 5, 0.84, 0.865, 0.89, 'R0.9', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (18, 6, 1.325, 1.36, 1.395, 'R1.4', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (19, 7, 1.55, 1.6, 1.64, 'R1.6', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (20, 8, 3.55, 3.74, 3.93, 'T3.7', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (21, 9, 10.3, 10.8, 11.3, 'T11', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (22, 10, 11.5, 12, 12.5, 'T12', '10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (23, 1, 0.392, 0.412, 0.432, 'R0.41', 'M1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (24, 2, 0.427, 0.445, 0.463, 'R0.45', 'M2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (25, 3, 0.468, 0.488, 0.508, 'R0.49', 'M3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (26, 4, 0.535, 0.555, 0.575, 'R0.56', 'M4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (27, 5, 0.652, 0.672, 0.692, 'R0.67', 'M5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (28, 6, 0.731, 0.746, 0.761, 'R0.75', 'M6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (29, 7, 0.826, 0.865, 0.904, 'R0.86', 'M7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (30, 8, 1.22, 1.24, 1.26, 'R1.24', 'M8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (31, 9, 1.363, 1.378, 1.393, 'R1.38', 'M9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (32, 10, 1.55, 1.61, 1.67, 'R1.61', 'M10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (33, 11, 2.2, 2.25, 2.3, 'R2.25', 'M11');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (34, 12, 3.52, 3.7, 3.88, 'T3.7', 'M12');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (35, 13, 3.895, 4.05, 4.205, 'T4', 'M13');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (36, 14, 8.25, 8.55, 8.85, 'T8.5', 'M14');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (37, 15, 9.763, 10.763, 11.763, 'T10.7', 'M15');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (38, 16, 0.3, 0.7, 1.1, 'R0.7', 'DNB');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (39, 17, 0.3, 0.7, 1.1, 'R0.7', 'DNB_MSG');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (40, 18, 0.3, 0.7, 1.1, 'R0.7', 'DNB_LSG');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (41, 19, 0.56, 0.64, 0.72, 'R0.64', 'I1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (42, 20, 0.826, 0.865, 0.904, 'R0.87', 'I2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (43, 21, 1.55, 1.61, 1.67, 'R1.6', 'I3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (44, 22, 3.36, 3.74, 4.12, 'T3.7', 'I4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (45, 23, 9.55, 11.45, 13.35, 'T11.5', 'I5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (46, 1, 0.5, 0.57, 0.65, 'R0.57', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (47, 2, 0.65, 0.72, 0.8, 'R0.7', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (48, 3, 0.8, 0.86, 0.9, 'R0.86', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (49, 4, 3.5, 3.75, 4, 'T3.75', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (50, 5, 5.7, 6.35, 7, 'T6.4', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (51, 6, 7.5, 8, 8.5, 'T8', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (52, 7, 8.2, 8.7, 9.2, 'T8.7', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (53, 8, 9.2, 9.7, 10.2, 'T9.7', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (54, 9, 10.2, 10.7, 11.2, 'T11', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (55, 10, 11.2, 11.7, 12.5, 'T12', '10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (56, 1, 0, 0, 0, 'Tv89', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (57, 2, 0, 0, 0, 'Tv157', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (58, 3, 0, 0, 0, 'Th183.311', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (59, 4, 0, 0, 0, 'Th183.311', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (60, 5, 0, 0, 0, 'Th190.311', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (63, 3, 0, 50.3, 0, 'Tv50.3', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (64, 4, 0, 52.8, 0, 'Tv52.8', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (65, 5, 0, 53.596, 0, 'Th53.596', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (66, 6, 0, 54.4, 0, 'Th54.4', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (67, 7, 0, 54.94, 0, 'Tv54.94', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (68, 8, 0, 55.5, 0, 'Th55.5', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (69, 9, 0, 57.29, 0, 'Th57.29', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (71, 10, 0, 57.29, 0, 'Th57+/-217', '10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (72, 11, 0, 57.29, 0, 'Th57+/-322+/-48', '11');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (73, 12, 0, 57.29, 0, 'Th57+/-322+/-22', '12');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (74, 13, 0, 57.29, 0, 'Th57+/-322+/-10', '13');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (75, 14, 0, 57.29, 0, 'Th57+/-322+/-4.5', '14');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (76, 15, 0, 89, 0, 'Tv89', '15');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (61, 1, 0, 23.8, 0, 'Tv23.8', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (62, 2, 0, 31.4, 0, 'Tv31.4', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (77, 1, 0, 23.8, 0, 'Tv23.8', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (78, 2, 0, 31.4, 0, 'Tv31.4', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (79, 1, 0.62, 0.645, 0.67, 'VIS0.65', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (80, 2, 0.841, 0.8585, 0.876, 'NIR0.86', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (81, 3, 0.459, 0.469, 0.479, 'VIS0.47', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (82, 4, 0.545, 0.555, 0.565, 'VIS0.56', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (83, 5, 1.23, 1.24, 1.25, 'NIR1.24', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (84, 6, 1.628, 1.64, 1.652, 'SWIR1.64', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (85, 7, 2.105, 2.13, 2.155, 'SWIR2.13', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (86, 8, 0.405, 0.4125, 0.42, 'VIS0.41', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (87, 9, 0.438, 0.443, 0.448, 'VIS0.44', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (88, 10, 0.483, 0.488, 0.493, 'VIS0.49', '10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (89, 11, 0.526, 0.531, 0.536, 'VIS0.53', '11');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (90, 12, 0.546, 0.551, 0.556, 'VIS0.55', '12');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (91, 13, 0.662, 0.667, 0.672, 'VIS0.67', '13');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (92, 15, 0.673, 0.678, 0.683, 'VIS0.68', '15');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (93, 17, 0.743, 0.748, 0.753, 'VIS0.75', '17');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (94, 18, 0.862, 0.8695, 0.877, 'NIR0.87', '18');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (95, 19, 0.89, 0.905, 0.92, 'NIR0.91', '19');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (96, 20, 0.931, 0.936, 0.941, 'NIR0.94', '20');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (97, 21, 0.915, 0.94, 0.965, 'NIR0.94', '21');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (98, 22, 0.00366, 3.75, 3.84, 'MWIR3.84', '22');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (99, 23, 3.929, 3.959, 3.989, 'MWIR3.99', '23');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (100, 24, 3.929, 3.959, 3.989, 'MWIR3.99', '24');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (101, 25, 4.02, 4.05, 4.08, 'MWIR4.08', '25');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (102, 26, 4.433, 4.4655, 4.498, 'MWIR4.5', '26');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (103, 27, 4.482, 4.5155, 4.549, 'MWIR4.55', '27');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (104, 28, 1.36, 1.375, 1.39, 'SWIR1.39', '28');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (105, 29, 6.535, 6.715, 6.895, 'TIR6.9', '29');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (106, 30, 7.175, 7.325, 7.475, 'TIR7.48', '30');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (107, 31, 8.4, 8.55, 8.7, 'TIR8.7', '31');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (108, 32, 9.58, 9.73, 9.88, 'TIR9.88', '32');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (109, 33, 10.78, 11.03, 11.28, 'TIR11.28', '33');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (110, 34, 11.77, 12.02, 12.27, 'TIR12.27', '34');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (111, 35, 13.185, 13.335, 13.485, 'TIR13.49', '35');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (112, 36, 13.485, 13.635, 13.785, 'TIR13.79', '36');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (113, 37, 13.785, 13.935, 14.085, 'TIR14.09', '37');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (114, 38, 14.085, 14.235, 14.385, 'TIR14.39', '38');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (115, 14, 0.662, 0.667, 0.672, 'VIS0.67Hi', '14');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (116, 16, 0.673, 0.678, 0.683, 'VIS0.68Hi', '16');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (117, 1, 0.412, 0.412, 0.412, 'R0.41', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (118, 2, 0.443, 0.443, 0.443, 'R0.44', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (119, 3, 0.47, 0.47, 0.47, 'R0.47', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (120, 4, 0.49, 0.49, 0.49, 'R0.49', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (121, 5, 0.52, 0.52, 0.52, 'R0.52', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (122, 6, 0.55, 0.55, 0.55, 'R0.55', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (123, 7, 0.565, 0.565, 0.565, 'R0.56', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (124, 8, 0.65, 0.65, 0.65, 'R0.6', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (125, 9, 0.65, 0.65, 0.65, 'R0.65', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (126, 10, 0.685, 0.685, 0.685, 'R0.69', '10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (127, 11, 0.765, 0.765, 0.765, 'R0.76', '11');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (128, 12, 0.865, 0.865, 0.865, 'R0.86', '12');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (129, 13, 0.865, 0.865, 0.865, 'R0.87', '13');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (130, 14, 0.905, 0.905, 0.905, 'R0.9', '14');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (131, 15, 0.94, 0.94, 0.94, 'R0.94', '15');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (132, 16, 0.98, 0.98, 0.98, 'R0.98', '16');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (133, 17, 1.03, 1.03, 1.03, 'R1', '17');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (134, 18, 1.64, 1.64, 1.64, 'R1.64', '18');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (135, 19, 2.13, 2.13, 2.13, 'R2', '19');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (136, 20, 11.5, 11.5, 11.5, 'T11', '20');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (137, 1, 0.412, 0.412, 0.412, 'R0.41', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (138, 2, 0.443, 0.443, 0.443, 'R0.44', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (139, 3, 0.47, 0.47, 0.47, 'R0.47', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (140, 4, 0.49, 0.49, 0.49, 'R0.49', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (141, 5, 0.55, 0.55, 0.55, 'R0.52', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (142, 6, 0.555, 0.555, 0.555, 'R0.55', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (143, 7, 0.65, 0.65, 0.65, 'R0.56', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (144, 8, 0.67, 0.67, 0.67, 'R0.6', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (145, 9, 0.709, 0.709, 0.709, 'R0.65', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (146, 10, 0.746, 0.746, 0.746, 'R0.69', '10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (147, 11, 0.865, 0.865, 0.865, 'R0.86', '11');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (148, 12, 0.865, 0.865, 0.865, 'R0.87', '12');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (149, 13, 0.905, 0.905, 0.905, 'R0.9', '13');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (150, 14, 0.936, 0.936, 0.936, 'R0.93', '14');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (151, 15, 0.94, 0.94, 0.94, 'R0.94', '15');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (152, 16, 1.24, 1.24, 1.24, 'R1.24', '16');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (155, 17, 1.38, 1.38, 1.38, 'R1.38', '17');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (158, 18, 1.64, 1.64, 1.64, 'R1.64', '18');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (159, 19, 2.13, 2.13, 2.13, 'R2', '19');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (160, 20, 3.8, 3.8, 3.8, 'T3.8', '20');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (161, 21, 4.05, 4.05, 4.05, 'T4.05', '21');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (162, 22, 7.2, 7.2, 7.2, 'T3.8', '22');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (163, 23, 8.55, 8.55, 8.55, 'T8.55', '23');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (164, 24, 10.8, 10.8, 10.8, 'T10.8', '24');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (165, 25, 12, 12, 12, 'T12', '25');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (166, 1, 0.412, 0.412, 0.412, 'R0.41', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (167, 2, 0.443, 0.443, 0.443, 'R0.44', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (168, 3, 0.47, 0.47, 0.47, 'R0.47', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (169, 4, 0.49, 0.49, 0.49, 'R0.49', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (170, 5, 0.55, 0.55, 0.55, 'R0.52', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (171, 6, 0.555, 0.555, 0.555, 'R0.55', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (172, 7, 0.65, 0.65, 0.65, 'R0.65', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (173, 8, 0.67, 0.67, 0.67, 'R0.67', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (174, 9, 0.709, 0.709, 0.709, 'R0.7', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (175, 10, 0.746, 0.746, 0.746, 'R0.74', '10');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (176, 11, 0.865, 0.865, 0.865, 'R0.86', '11');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (177, 12, 0.865, 0.865, 0.865, 'R0.87', '12');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (178, 13, 0.905, 0.905, 0.905, 'R0.9', '13');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (179, 14, 0.936, 0.936, 0.936, 'R0.93', '14');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (180, 15, 0.94, 0.94, 0.94, 'R0.94', '15');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (181, 16, 1.03, 1.03, 1.03, 'R1', '16');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (182, 17, 1.38, 1.38, 1.38, 'R1.38', '17');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (183, 18, 1.64, 1.64, 1.64, 'R1.64', '18');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (184, 19, 2.13, 2.13, 2.13, 'R2', '19');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (185, 20, 3.8, 3.8, 3.8, 'T3.8', '20');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (186, 21, 4.05, 4.05, 4.05, 'T4.05', '21');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (187, 22, 7.2, 7.2, 7.2, 'T3.8', '22');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (188, 23, 8.55, 8.55, 8.55, 'T8.55', '23');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (189, 24, 10.8, 10.8, 10.8, 'T10.8', '24');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (190, 25, 12, 12, 12, 'T12', '25');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (191, 1, 0.70, 0.70, 0.70, 'R0.7', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (194, 4, 3.80, 3.80, 3.80, 'T3.8', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (195, 5, 4.05, 4.05, 4.05, 'T4.05', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (196, 6, 7.20, 7.20, 7.20, 'T3.8', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (197, 7, 8.55, 8.55, 8.55, 'T8.55', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (198, 8, 10.8, 10.8, 10.8, 'T10.8', '8');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (199, 9, 12.0, 12.0, 12.0, 'T12', '9');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (200, 1, 0.65, 0.65, 0.65, 'R0.65', '1');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (201, 2, 0.865, 0.865, 0.865, 'R0.86', '2');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (202, 3, 0.94, 0.94, 0.94, 'R0.94', '3');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (203, 4, 1.38, 1.38, 1.38, 'R1.38', '4');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (204, 5, 1.64, 1.64, 1.64, 'R1.64', '5');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (205, 6, 3.80, 3.80, 3.80, 'T3.8', '6');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (206, 7, 10.8, 10.8, 10.8, 'T10.8', '7');
INSERT INTO meteo.channel (id, number, lambda_min, lambda_center, lambda_max, alias, name) VALUES (207, 8, 12.0, 12.0, 12.0, 'T12', '8');

--
-- Name: channel_id_seq; Type: SEQUENCE SET; Schema: meteo; Owner: postgres
--

SELECT pg_catalog.setval('meteo.channel_id_seq', 115, true);


--
-- PostgreSQL database dump complete
--

