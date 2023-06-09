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

--
-- Data for Name: typec_area; Type: TABLE DATA; Schema: meteo; Owner: postgres
--

INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (1, 'AK', 'Alaska', 'Аляска');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (2, 'AH', 'Afghanistan', 'Афганистан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (3, 'MA', 'Mauritius', 'Маврикий');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (4, 'AY', 'Armenia', 'Армения');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (5, 'AG', 'Argentina', 'Аргентина');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (6, 'AB', 'Albania', 'Албания');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (7, 'AI', 'Ascension Island', 'остров Вознесения');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (8, 'LC', 'Saint Lucia', 'Сент-Люсия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (9, 'AU', 'Australia', 'Австралия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (10, 'LT', 'Lithuania', 'Литва');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (11, 'BC', 'Botswana', 'Ботсвана');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (12, 'LV', 'Latvia', 'Латвия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (13, 'AT', 'Antigua and Barbuda', 'Антигуа и Барбуда');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (14, 'LI', 'Liberia', 'Либерия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (15, 'AL', 'Algeria', 'Алжир');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (16, 'AJ', 'Azerbaijan', 'Азербайджан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (17, 'MN', 'St. Maarten, St.Eustatius and Saba', 'Св. Мартена, Св. Евстатия и Саба');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (18, 'MI', 'Mali', 'Мали');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (19, 'BJ', 'Benin', 'Бенин');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (20, 'BK', 'Banks Islands', 'остров Банкс');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (21, 'MD', 'Madeira Island', 'Мадейра');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (22, 'BR', 'Barbados', 'Барбадос');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (23, 'MS', 'Malaysia', 'Малайзия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (24, 'CG', 'Congo', 'Конго');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (25, 'BU', 'Bulgaria', 'Болгария');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (26, 'MY', 'Mariana Islands', 'Марианские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (27, 'BH', 'Belize', 'Белиз');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (28, 'LS', 'Lesotho', 'Лесото');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (29, 'MO', 'Mongolia', 'Монголия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (30, 'MF', 'Saint-Martin, Saint-Bartholomew, Guadeloupe and other French islands in the vicinity', 'Сан-Марино, св. Варфоломея, Гваделупа и др. острова по соседству с Францией ');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (31, 'MR', 'Martinique', 'Мартиника');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (32, 'MW', 'Malawi', 'Малави');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (33, 'MJ', 'Macedonia, The Republic of', 'Республика Македония');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (34, 'BX', 'Belgium, Luxembourg', 'Бельгия, Люксембург');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (35, 'BA', 'Bahamas, The', 'Багамские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (36, 'BM', 'Myanmar', 'Мьянма');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (37, 'BO', 'Bolivia', 'Боливия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (38, 'MV', 'Maldives', 'Мальдивы');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (39, 'LJ', 'Slovenia', 'Словения');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (40, 'AN', 'Angola', 'Ангола');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (41, 'MB', 'Marion Island', 'остров Мэрион');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (42, 'MC', 'Morocco', 'Марокко');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (43, 'BW', 'Bangladesh', 'Бангладеш');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (44, 'LN', 'Southern Line Islands', 'острова Южной линии');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (45, 'BD', 'Brunei', 'Бруней');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (46, 'BY', 'Belarus', 'Белорусия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (47, 'CD', 'Chad', 'Чад');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (48, 'LY', 'Libya', 'Ливия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (49, 'ML', 'Malta', 'Мальта');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (50, 'BN', 'Bahrain', 'Бахрейн');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (51, 'MX', 'Mexico', 'Мексика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (52, 'BZ', 'Brazil', 'Бразилия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (53, 'MZ', 'Mozambique', 'Мозамбик');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (54, 'NI', 'Nigeria', 'Нигерия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (55, 'AZ', 'Azores Islands', 'Азорские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (56, 'BE', 'Bermuda', 'Бермудские отсрова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (57, 'BI', 'Burundi', 'Бурунди');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (58, 'MK', 'Serbia and Montenegro', 'Сербия и Черногория');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (59, 'MT', 'Mauritania', 'Мавритания');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (60, 'MH', 'Marshall Islands', 'Маршалловы острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (61, 'NG', 'Papua New Guinea', 'Папуа Новая Гвинея');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (62, 'MG', 'Madagascar', 'Мадагаскар');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (63, 'NC', 'New Caledonia', 'Новая Каледония');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (64, 'CE', 'Central African Republic', 'Центральная Африканская Республика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (65, 'CM', 'Cameroon', 'Камерун');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (66, 'FG', 'French Guiana', 'Французская Гвиана');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (67, 'RW', 'Rwanda', 'Руанда');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (68, 'SW', 'Switzerland', 'Швейцария');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (69, 'HK', 'Hong Kong, China', 'Гон-Конг, Китай');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (70, 'IL', 'Iceland', 'Исландия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (71, 'TP', 'Sao Tome and Principe', 'Сан-Томе и Принсипи');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (72, 'UZ', 'Uzbekistan', 'Узбекистан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (73, 'IO', 'Indian Ocean area', 'Индийский океан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (74, 'NO', 'Norway', 'Норвегия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (75, 'DO', 'Dominica', 'Доминика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (76, 'PT', 'Pitcairn Islands', 'Питкэрн');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (77, 'EQ', 'Ecuador', 'Эквадор');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (78, 'FP', 'Saint Pierre and Miquelon', 'Сен-Пьер и Микелон');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (79, 'GC', 'Cayman Islands', 'Каймановы острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (80, 'HO', 'Honduras', 'Гондурас');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (81, 'IN', 'India', 'Индия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (82, 'KA', 'Caroline Islands', 'Каролинские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (83, 'VI', 'Virgin Islands', 'Виргинские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (84, 'FW', 'Wallis and Futuna Islands', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (85, 'SE', 'Southern Ocean area', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (86, 'CU', 'Cuba', 'Куба');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (87, 'SB', 'Sri Lanka', 'Шри-Ланка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (88, 'GI', 'Gibraltar', 'Гибралтар');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (89, 'GM', 'Guam Island', 'остров Гуам');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (90, 'GQ', 'Equatorial Guinea', 'Экваторияльная Гвинея');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (91, 'SY', 'Syria', 'Сирия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (92, 'TD', 'Trinidad and Tobago', 'Тринидад и Тобаго');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (93, 'KI', 'Christmas Islands', 'остров Рождества');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (94, 'NM', 'Namibia', 'Намибия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (95, 'CZ', 'Czech Republic', 'Чешская Республика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (96, 'OS', 'Austria', 'Австрия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (97, 'PH', 'Philippines', 'Филиппинские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (98, 'PM', 'Panama', 'Панама');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (99, 'PY', 'Paraguay', 'Парагваи');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (100, 'SZ', 'Spitzbergen Islands', 'острова Шпицберген');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (101, 'TH', 'Thailand', 'Тайланд');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (102, 'IR', 'Iran, Islamic Republic of', 'Иран, Исламская Республика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (103, 'AF', 'Africa', 'Африка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (104, 'AO', 'West Africa', 'Западная Африка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (105, 'OC', 'Oceania', 'Океания');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (106, 'FE', 'Far East', 'Ближний Восток');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (107, 'GX', 'Gulf of Mexico area', 'Мексиканский залив');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (108, 'NW', 'Nauru Island', 'остров Науру');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (109, 'OR', 'South Orkney Islands', 'острова Южной Оркни');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (110, 'DL', 'Germany', 'Германия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (111, 'ET', 'Ethiopia', 'Эфиопия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (112, 'QT', 'Qatar', 'Катар');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (113, 'FA', 'Faeroe Islands', 'Фарерские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (114, 'SX', 'Santa Cruz Islands', 'острова Санта-Крус');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (115, 'HA', 'Haiti', 'Гаити');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (116, 'HE', 'Saint Helena', 'остров Святой Елены');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (117, 'TG', 'Togo', 'Того');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (118, 'TU', 'Turkey', 'Турция');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (119, 'TV', 'Tuvalu', 'Тувалу');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (120, 'KK', 'Cocos Islands', 'Кокосовые острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (121, 'YG', 'Yougoslavia', 'Югославия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (122, 'GE', 'Gough Island', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (123, 'EN', 'Northern Europe', 'Северная Европа');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (124, 'CS', 'Costa Rica', 'Коста Рика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (125, 'CV', 'Cape Verde Islands', 'острова Зелёного Мыса');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (126, 'DJ', 'Djibouti', 'Джибути');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (127, 'PK', 'Pakistan', 'Пакистан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (128, 'EO', 'Estonia', 'Эстония');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (129, 'RS', 'Russian Federation (asia area)', 'Российская Федерация (Азиатская часть)');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (130, 'GG', 'Georgia', 'Грузия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (131, 'SO', 'Solomon Islands', 'Соломоновы острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (132, 'GR', 'Greece', 'Греция');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (133, 'SU', 'Sudan', 'Судан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (134, 'TK', 'Tokelau Islands', 'острова Токелау');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (135, 'TM', 'East Timor', 'Восточный Тимор');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (136, 'VN', 'Venezuela', 'Венесуэлла');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (137, 'KW', 'Kuwait', 'Кувейт');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (138, 'ZA', 'South Africa', 'Южная Африка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (139, 'NA', 'North America', 'Северная Африка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (140, 'NT', 'North Atlantic area', 'Северо-Атлантическое побережье');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (141, 'MP', 'Central Mediterranean area', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (142, 'NR', 'Niger', 'Нигерия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (143, 'NU', 'Netherlands Antilles (Aruba, Bonaire, Cura?ao)', 'Нидерланды Антильские острова (Аруба, Бонайре, Курасао)');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (144, 'OM', 'Oman', 'Оман');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (145, 'PU', 'Puerto Rico', 'Пуэрто Рико');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (146, 'ES', 'El Salvador', 'Сальвадор');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (147, 'FI', 'Finland', 'Финляндия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (148, 'SC', 'Seychelles Islands', 'Сейшельские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (149, 'SQ', 'Slovakia', 'Словакия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (150, 'SR', 'Singapore', 'Сингапур');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (151, 'GU', 'Guatemala', 'Гватемала');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (152, 'UK', 'United Kingdom of Great Britain and Northern Ireland', 'Объединённое Королевство Британия И Северная Ирландия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (153, 'JP', 'Japan', 'Япония');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (154, 'KY', 'Kyrgyzstan', 'Киргистан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (155, 'PQ', 'Western North Pacific', 'Северо-Западный Тихий океан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (156, 'EC', 'East China Sea area', 'Восточно-Китайское море');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (157, 'EM', 'Middle Europe', 'Средняя Европа');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (158, 'EU', 'Europe', 'Европа');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (159, 'EW', 'Western Europe', 'Западная Европа');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (160, 'XT', 'Tropical belt', 'Тропический пояс');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (161, 'NL', 'Netherlands', 'Нидерланды');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (162, 'NP', 'Nepal', 'Непал');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (163, 'DN', 'Denmark', 'Дания');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (164, 'QB', 'Bosnia and Herzegovina', 'Босния и Герцоговина');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (165, 'RM', 'Republic of Moldova', 'Республика Молдова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (166, 'SK', 'Sarawak', 'Саравак');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (167, 'TS', 'Tunisia', 'Тунис');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (168, 'IY', 'Italy', 'Италия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (169, 'UG', 'Uganda', 'Уганда');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (170, 'ZW', 'Zimbabwe', 'Зимбабве');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (171, 'GA', 'Gulf of Alaska area', 'залив Аляски');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (172, 'XW', 'Western hemisphere', 'Западное полушарие');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (173, 'BV', 'Bouvet Island', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (174, 'SS', 'South China Sea area', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (175, 'NZ', 'New Zealand', 'Новая Зеландия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (176, 'ER', 'United Arab Emirates', 'Объединённые Арабские Эмираты');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (177, 'FJ', 'Fiji Islands', 'острова Фиджи');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (178, 'SD', 'Saudi Arabia', 'Саудовская Аравия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (179, 'HW', 'Hawaiian Islands', 'Гавайские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (180, 'UY', 'Uruguay', 'Уругвай');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (181, 'AM', 'Central Africa', 'Центральная Африка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (182, 'EA', 'East Africa', 'Восточная Африка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (183, 'EE', 'Eastern Europe', 'Востоная Европа');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (184, 'XS', 'Southern hemisphere', 'Южное полушарие');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (185, 'DR', 'Dominican Republic', 'Доминиканская Республика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (186, 'GB', 'Gambia, The', 'Гамбия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (187, 'GD', 'Grenada', 'Гренада');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (188, 'SL', 'Sierra Leone', 'Сьерра-Леоне');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (189, 'GO', 'Gabon', 'Габон');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (190, 'IE', 'Ireland', 'Ирландия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (191, 'OH', 'Sea of Okhotsk', 'Охотское море');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (192, 'PN', 'North Pacific area', 'Северное побережье Тихого океана');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (193, 'XX', 'For use when other designators are not appropriate', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (194, 'RE', 'Reunion and associated Islands', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (195, 'CO', 'Colombia', 'Колумбия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (196, 'PI', 'Phoenix Islands', 'Финикийские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (197, 'SG', 'Senegal', 'Сенегал');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (198, 'ID', 'Indonesia', 'Индонезия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (199, 'IQ', 'Iraq', 'Ирак');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (200, 'TR', 'Turkmenistan', 'Туркменистан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (201, 'IV', 'Cote dIvoire', 'Кот де Вуар');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (202, 'YE', 'Yemen, Republic of', 'Республика Йемен');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (203, 'PZ', 'Eastern Pacific area', 'Восточное побережье Тихого океана');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (204, 'CR', 'Canary Islands (Spain)', 'Канарские отсрова (Испания)');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (205, 'PO', 'Portugal', 'Португалия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (206, 'PR', 'Peru', 'Перу');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (207, 'SI', 'Somalia', 'Сомали');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (208, 'GH', 'Ghana', 'Гана');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (209, 'JD', 'Jordan', 'Иордан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (210, 'JM', 'Jamaica', 'Ямайка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (211, 'KR', 'Korea, North', 'Северная Корея');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (212, 'KU', 'Cook Islands', 'острова Кука');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (213, 'KZ', 'Kazakhstan', 'Казахстан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (214, 'AC', 'Arctic', 'Арктика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (215, 'AE', 'South-East Asia', 'Юго-Восточная Азия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (216, 'BQ', 'Baltic Sea area', 'Балтийское море');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (217, 'CA', 'Caribbean and Central America', 'Карибская и Центральная Америка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (218, 'PW', 'Western Pacific area', 'Западное побережье Тихого океана');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (219, 'MU', 'Macau', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (220, 'TC', 'Tristan da Cunha', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (221, 'TI', 'Turks and Caicos Islands', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (222, 'CI', 'China', 'Китай');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (223, 'SP', 'Spain', 'Испания');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (224, 'TN', 'Tanzania, United Republic of', 'Объединённая Республика Танзания');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (225, 'TO', 'Tonga', 'Тонга');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (226, 'KN', 'Kenya', 'Кения');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (227, 'ZM', 'Western Samoa', 'Западная Саома');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (228, 'LB', 'Lebanon', 'Ливан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (229, 'PE', 'Persian Gulf area', 'Персидский залив');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (230, 'XE', 'Eastern hemisphere', 'Восточное полушарие');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (231, 'MM', 'Mediterranean area', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (232, 'SJ', 'Sea of Japan area', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (233, 'CH', 'Chile', 'Чили');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (234, 'CN', 'Canada', 'Канада');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (235, 'CT', 'Canton Island', 'остров Кантон');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (236, 'PL', 'Poland', 'Польша');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (237, 'EG', 'Egypt', 'Египет');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (238, 'GL', 'Greenland', 'Гренландия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (239, 'SN', 'Sweden', 'Швеция');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (240, 'SV', 'Swaziland', 'Свазиленд');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (241, 'HV', 'Burkina Faso', 'Буркина-Фасо');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (242, 'AA', 'Antarctica', 'Антарктика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (243, 'AP', 'Southern Africa', 'Южная Африка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (244, 'AS', 'Asia', 'Азия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (245, 'NK', 'Nicaragua', 'Никорагуа');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (246, 'NV', 'Vanuatu', 'Вануату');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (247, 'CY', 'Cyprus', 'Кипр');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (248, 'PF', 'French Polynesia Islands', 'острова Франции Полинезии');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (249, 'RA', 'Russian Federation (europe area)', 'Российская Федерация (Европеиская часть)');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (250, 'RH', 'Croatia', 'Хорватия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (251, 'FR', 'France', 'Франция');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (252, 'SM', 'Suriname', 'Суринам');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (253, 'GN', 'Guinea', 'Гвинея');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (254, 'IC', 'Comoros', 'Коморские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (255, 'IS', 'Israel', 'Израэль');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (256, 'VG', 'Saint Vincent and the Grenadines', 'Сент-Винсент и Гренадины');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (257, 'KO', 'Korea, South', 'Южная Корея');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (258, 'VS', 'Vietnam', 'Вьетнам');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (259, 'AW', 'Near East', 'Ближний Восток');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (260, 'PA', 'Pacific area', 'Тихий океан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (261, 'SA', 'South America', 'Южная Америка');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (262, 'FK', 'Falkland Islands, Islas Malvinas', 'Фолклендские острова, Мальвинские острова');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (263, 'RO', 'Romania', 'Румыния');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (264, 'GW', 'Guinea-Bissau', 'Гвинея-Бисау');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (265, 'GY', 'Guyana', 'Гвинея');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (266, 'TA', 'Tajikistan', 'Таджикистан');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (267, 'HU', 'Hungary', 'Венгрия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (268, 'UR', 'Ukraine', 'Украина');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (269, 'US', 'United States of America', 'Соединённые Штаты Америки');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (270, 'KB', 'Kiribati', 'Кирибати');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (271, 'KP', 'Cambodia', 'Камбоджия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (272, 'ZB', 'Zambia', 'Замбия');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (273, 'LA', 'Laos', 'Лаос');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (274, 'ZR', 'Zaire', 'Заир');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (275, 'AX', 'Arabian Sea area', 'Арабское море');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (276, 'PS', 'South Pacific area', 'Южное побережье Тихого океана');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (277, 'ST', 'South Atlantic area', 'Южная Атлантика');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (278, 'XN', 'Northern hemisphere', 'Северное полушарие');
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (279, 'MQ', 'Western Mediterranean area', NULL);
INSERT INTO meteo.typec_area (id, name_smb, name_lat, name_cyr) VALUES (280, 'ME', 'Eastern Mediterranean area', NULL);


--
-- PostgreSQL database dump complete
--

