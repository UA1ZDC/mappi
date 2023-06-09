#!/usr/bin/python3
# coding: utf-8

# Приращение направления и скорости среднего ветра в слоя 0-14, 0-16, ..., 0-30 км к среднему ветру слоя 0-12 км

# Наименования констант:

# winter - значения для зимы
# spring - значения для весны
# summer - значения для лета
# autumn - значения для осени

# 10 значения при скорости ветра в слое 0-12 меньшей или равной 10 м/с
# 11 значения при скорости ветра в слое 0-12 большей или равной 11 м/с

# Значения определяются в зависимости от направления ветра в слове 0-12 км
# sz - северо-запад
# ssz - севернее северо-запада
# s - север
# ssv - севернее северо-востока
# sv - северо-восток
# vsv - восточнее северо-востока
# v - восток
# vyv - восточнее юго-востока
# yv - юго-восток
# yyv - южнее юго-востока
# y - юг
# yyz - южнее юго-запада
# yz - юго-апад
# zyz - западнее юго-запада
# z - запад
# zsz - западнее северо-запада

# dd - направление ветра
# ff - скорость ветра

layers = [14000, 16000, 18000, 20000, 22000, 24000, 26000, 28000, 30000] # верхние границы слоев в метрах

# значения приращения ветра для соответствующих слоев
winter_10_sz_dd = [0, -2, -3, -3, -7, -12, -14,-10, -10]
winter_10_sz_ff = [2, 2, 2, 2, 2, 3, 4, 4, 4]
winter_10_ssz_dd = [-6, -10, -13, -15, -20, -22, -26, -27, -29]
winter_10_ssz_ff = [1, 1, 1, 1, 1, 2, 3, 3, 4]
winter_10_s_dd = [-12, -18, -23, -26, -32, -32, -39, -44, -48]
winter_10_s_ff = [0, 0, 0, 0, 1, 2, 2, 2, 3]
winter_10_ssv_dd = [-20, -27, -33, -37, -46, -48, -53, -57, -59]
winter_10_ssv_ff = [0, 0, 0, 0, 0, 1, 1, 1, 2]
winter_10_sv_dd = [-28, -36, -43, -48, -61, -65, -67, -71, -71]
winter_10_sv_ff = [0, -1, -1, -1, -1, 0, 0, 1, 1]
winter_10_vsv_dd = [-25, -32, -39, -49, -58, -63, -66, -69, -82]
winter_10_vsv_ff = [0, -1,-2, -2, -2, -2, -1, 0, 0]
winter_10_v_dd = [-22, -28, -35, -51, -55, -62, -66, -68, -93]
winter_10_v_ff = [-1, -2, -3, -3, -3, -3, -2, -2, -1]
winter_10_vyv_dd = [-2, -2, -2, -8, -4, -2, 1, 2, -8]
winter_10_vyv_ff = [-1, -2, -3, -3, -3, -3, -2, -2, -1]
winter_10_yv_dd = [18, 23, 30, 36, 47, 59, 68, 72, 78]
winter_10_yv_ff = [-2, -3, -3, -4, -4, -4, -3, -2, -1]
winter_10_yyv_dd = [16, 21, 26, 32, 40, 50, 56, 60 ,64]
winter_10_yyv_ff = [-1, -1, -1, -2, -2, -2, -1, 0, 0]
winter_10_y_dd = [15, 19, 23, 28, 34, 40, 45, 48, 50]
winter_10_y_ff = [0, 0, 0, 0, 0, 0, 1, 1, 2]
winter_10_yyz_dd = [10, 15, 22, 22, 26, 31, 34, 35, 37]
winter_10_yyz_ff = [0, 0, 0, 0, 1, 1, 2, 2, 3]
winter_10_yz_dd = [6, 10, 12, 15, 18, 22, 23, 23, 25]
winter_10_yz_ff = [1, 1, 1, 1, 2, 2, 3, 3, 4]
winter_10_zyz_dd = [4, 7, 8, 10, 11, 13, 13, 15, 15]
winter_10_zyz_ff = [1, 1, 2, 2, 2, 3, 3, 4, 4]
winter_10_z_dd = [3, 4, 4, 4, 4, 4, 2, 8, 5]
winter_10_z_ff = [2, 2, 2, 3, 3, 4, 4, 5, 5]
winter_10_zsz_dd = [1, 1, 0, 0, -1, -4, -6, -1, -2]
winter_10_zsz_ff = [2, 2, 3, 3, 3, 3, 4, 4, 4]

winter_11_sz_dd = [-2, -4, -6, -9, -11, -11, -16, -19, -22]
winter_11_sz_ff = [0, 0, 0, 0, 0, 0, 1, 1, 2]
winter_11_ssz_dd = [-1, -4, -7, -10, -15, -16, -23, -28, -29]
winter_11_ssz_ff = [0, 0, 0, -1, -1, -1, 0, 0, 1]
winter_11_s_dd = [0, -4, -8, -12, -18, -22, -30, -37, -36]
winter_11_s_ff = [0, -1, -1, -2, -2, -2, -1, -1, 0]
winter_11_ssv_dd = [-5, -10, -14, -18, -23, -26, -31, -36, -42]
winter_11_ssv_ff = [0, 0, -1, -2, -2, -2, -2, -2, -2]
winter_11_sv_dd = [-10, -15, -20, -24, -29, -30, -32, -35, -47]
winter_11_sv_ff = [0, 0, -1, -2, -2, -3,- 4, -3, -3]
winter_11_vsv_dd = [-5, -12, -20, -32, -42, -50, -53, -57, -63]
winter_11_vsv_ff = [-3, -3, -4, -5, -5, -5, -6, -5, -4]
winter_11_v_dd = [0, -10, -20, -40, -55, -70, -75, -80, -80]
winter_11_v_ff = [-6, -7, -8, -8, -9, -8, -3, -7, -5]
winter_11_vyv_dd = [6, 3, 2, -6, -8, -9, -4, -1, 1]
winter_11_vyv_ff = [-4, -5, -6, -6, -7, -7, -7, -6, -4]
winter_11_yv_dd = [13, 16, 23, 28, 38,51, 66, 78, 82]
winter_11_yv_ff = [-2, -3, -4, -4, -5, -5, -5, -5, -3]
winter_11_yyv_dd = [12, 16, 25, 29, 37, 46, 56, 65, 68]
winter_11_yyv_ff = [-2, -3, -4, -4, -4, -4, -3, -3, -2]
winter_11_y_dd = [12, 17, 26, 30, 36, 42, 45, 52, 53]
winter_11_y_ff = [-2, -3, -4, -4, -4, -4, -3, -3, -2]
winter_11_yyz_dd = [6, 10, 15, 18, 24, 27, 30, 35, 36]
winter_11_yyz_ff = [-1, -2, -2, -2, -3, -3, -1, 0, 0]
winter_11_yz_dd = [0, 2, 5, 7, 12, 12, 16, 18, 18]
winter_11_yz_ff = [0, -1, -1, -1, -2, -2, 0, 0, 0]
winter_11_zyz_dd = [2, 3, 5, 6, 8, 8, 10, 10, 10]
winter_11_zyz_ff = [0, 0, 0, 0, 0, 0, 1, 1, 2]
winter_11_z_dd = [3, 4, 5, 5, 5, 4, 4, 3, 3]
winter_11_z_ff = [0, 0, 0, 1, 1, 2, 2, 3, 4]
winter_11_zsz_dd = [0, 0, 0, -2, -3, -3, -6, -8, -9]
winter_11_zsz_ff = [0, 0, 0, 0, 0, 1, 1, 2, 3]

spring_10_sz_dd = [-5, -5, -5, -7, -7, -5, -4, -2, 0]
spring_10_sz_ff = [1, 1, 0, 0, 0, 0, 0, 0, 0]
spring_10_ssz_dd = [-6, -7, -9, -12, -13, -12, -11, -10, -9]
spring_10_ssz_ff = [1, 1, 0, 0, 0, 0, 0, 0, 0]
spring_10_s_dd = [-8, -10, -13, -16, -18, -18, -17, -17, -19]
spring_10_s_ff = [1, 1, 1, 1, 1, 1, 1, 1, 1]
spring_10_ssv_dd = [-14, -18, -20, -25, -28, -28, -28, -30, -32]
spring_10_ssv_ff = [0, 0, 0, 0, 0, 0, 0, 0, 0]
spring_10_sv_dd = [-21, -25, -28, -35, -38, -38, -40, -43, -44]
spring_10_sv_ff = [0, 0, -1, -1, -1, -1, -1, -1, -1]
spring_10_vsv_dd = [-20, -22, -25, -29, -30, -30, -32, -34, -35]
spring_10_vsv_ff = [0, 0, 0, -1, -1, -1, -1, -1, -1]
spring_10_v_dd = [-19, -20, -23, -24, -22, -23, -25, -26, -25]
spring_10_v_ff = [1, 0, 0, -2, -2, -2, -2, -2, -2]
spring_10_vyv_dd = [5, 7, 7, 7, 8, 9, 9, 9, 9]
spring_10_vyv_ff = [0, -1, -1, -2, -3, -3, -3, -3, -3]
spring_10_yv_dd = [28, 34, 38, 38, 39, 41, 43, 44, 44]
spring_10_yv_ff = [-2, -2, -3, -3, -4, -4, -4, -4, -4]
spring_10_yyv_dd = [21, 26, 30, 31, 32, 34, 34, 33, 33]
spring_10_yyv_ff = [-1, -1, -1, -2, -2, -3, -3, -4, -4]
spring_10_y_dd = [15, 18, 23, 24, 26, 27, 24, 22, 21]
spring_10_y_ff = [0, 0, 0, -1, -1, -2, -2, -3, -3]
spring_10_yyz_dd = [12, 15, 18, 19, 22, 22, 21, 20, 20]
spring_10_yyz_ff = [0, 0, 0, 0, 0, -1, -1, -2, -2]
spring_10_yz_dd = [8, 12, 13, 15, 18, 18, 18, 18, 18]
spring_10_yz_ff = [1, 1, 0, 0, 0, -1, -1, -1, -1]
spring_10_zyz_dd = [4, 6, 8, 9, 12, 12, 12, 13]
spring_10_zyz_ff = [1, 1, 0, 0, 0, 0, 0, 0, 0]
spring_10_z_dd = [0, 1, 3, 3, 5, 6, 7, 7, 9]
spring_10_z_ff = [1, 1, 1, 1, 0, 0, 0, 0, 0]
spring_10_zsz_dd = [-2, -2, -1, -2, -1, 0, 1, 2, 4]
spring_10_zsz_ff = [1, 1, 0, 0, 0, 0, 0, 0, 0]

spring_11_sz_dd = [-2, -2, -2, -3, -2, -2, 0, 0, 0]
spring_11_sz_ff = [0, 0, -1, -1, -1, -2, -3, -3, -3]
spring_11_ssz_dd = [-5, -6, -7, -8, -8, -9, -7, -7, -7]
spring_11_ssz_ff = [0, 0, -1, -1, -1, -2, -2, -3, -3]
spring_11_s_dd = [-8, -11, -13, -14, -14, -16, -15, -15, -14]
spring_11_s_ff = [-1, -1, -2, -2, -2, -2, -2, -3, -3]
spring_11_ssv_dd = [-10, -11, -14, -5, -15, -17, -16, -16, -16]
spring_11_ssv_ff = [-1, -2, -3, -3, -4, -4, -4, -6, -6]
spring_11_sv_dd = [-13, -12, -16, -16, -17, -18, -17, -18, -19]
spring_11_sv_ff = [-2, -3, -4, -5, -6, -6, -7, -9, -9]
spring_11_vsv_dd = [0, 1, -1, -1, -3, -3, -4, -5, -5]
spring_11_vsv_ff = [-1, -3, -4, -5, -6, -6, -7, -8, -8]
spring_11_v_dd = [13, 14, 13, 13, 11, 11, 10, 8, 8]
spring_11_v_ff = [-1, -3, -4, -5, -6, -6, -7, -7, -8]
spring_11_vyv_dd = [10, 12, 12, 12, 11, 11, 10, 8, 6]
spring_11_vyv_ff = [0, -1, -2, -3, -4, -4, -5, -5, -6]
spring_11_yv_dd = [8, 10, 11, 11, 11, 10, 9, 8, 5]
spring_11_yv_ff = [1, 0 ,-1, -2, -3, -3, -4, -4, -5]
spring_11_yyv_dd = [6, 8, 10, 10, 11, 11, 11, 11, 10]
spring_11_yyv_ff = [0, -1, -2, -3, -3, -4, -5, -5, -6]
spring_11_y_dd = [4, 6, 9, 10, 12, 12, 13, 14, 14]
spring_11_y_ff = [-1, -2, -3, -4, -4, -5, -6, -6, -7]
spring_11_yyz_dd = [4, 6, 8, 9, 10, 11, 12, 12, 13]
spring_11_yyz_ff = [0, -1, -2, -2, -3, -3, -4, -4, -5]
spring_11_yz_dd = [5, 6, 7, 8, 9, 10, 11, 11, 12]
spring_11_yz_ff = [0, 0, -1, -1, -2, -2, -3, -3, -4]
spring_11_zyz_dd = [3, 4, 5, 6, 6, 8, 9, 10, 11]
spring_11_zyz_ff = [0, 0, 0, -1, -1, -2, -2, -2, -3]
spring_11_z_dd = [2, 2, 3, 4, 4, 6, 7, 10, 10]
spring_11_z_ff = [1, 0, 0, -1, -1, -2, -2, -2, -3]
spring_11_zsz_dd = [0, 0, 0, 0, 1, 2, 3, 5, 5]
spring_11_zsz_ff = [0, 0, 0, -1, 0, -2, -2, -2, -3]

summer_10_sz_dd = [-17, -18, -20, -20, -20, -16, -10, -5, 2]
summer_10_sz_ff = [0, 0, -1, -2, -3, -4, -5, -6, -6]
summer_10_ssz_dd = [-11, -12, -12, -12, -8, -4, 0, 4, 12]
summer_10_ssz_ff = [0, -1, -2, -2, -3, -4, -5, -5, -5]
summer_10_s_dd = [-5, -6, -4, -3, 4, 8 ,10, 13, 22]
summer_10_s_ff = [-1, -2, -3, -3, -4, -4, -4, -5, -5]
summer_10_ssv_dd = [0, -5, -4, -3, -2, 6, 9, 11, 18]
summer_10_ssv_ff = [0, -1, -2, -2, -3, -3, -3, -4, -4]
summer_10_sv_dd = [-5, -5, -3, -2, 0, 4, 8, 10, 14]
summer_10_sv_ff = [0, -1, -2, -2, -2, -3, -3, -3, -3]
summer_10_vsv_dd = [0, -1, 1, 1, 2, 3, 5, 6, 10]
summer_10_vsv_ff = [0, -1, -2, -2, -2, -2, -2, -2, -2]
summer_10_v_dd = [4, 3, 4, 3, 4, 2, 3, 3, 3]
summer_10_v_ff = [-1, -2, -2, -2, -2, -1, -1, 0, 0]
summer_10_vyv_dd = [6, 6, 8, 6, 6, 3, 2, 1, 0]
summer_10_vyv_ff = [-1, -1, -1, -1, -1, -1, -1, 0, 0]
summer_10_yv_dd = [9, 10, 12, 10, 3, 4, 2, -1, -2]
summer_10_yv_ff = [-1, -1, -1, -1, -1, -1, -1, 0, 0]
summer_10_yyv_dd = [7, 8, 8, 6, 2, -3, -7, -10, -12]
summer_10_yyv_ff = [0, 0, -1, -1, -1, -1, -2, -2, -2]
summer_10_y_dd = [5, 5, -5, 1, -5, -10, -17, -20, -23]
summer_10_y_ff = [0, 0, -1, -2, -2, -2, -3, -3, -3]
summer_10_yyz_dd = [3, 3, 3, -1, -2, -8, -14, -18, -21]
summer_10_yyz_ff = [0, 0, 0, -1, -2, -2, -3, -3, -4]
summer_10_yz_dd = [1, 2, 1, 0, -3, -6, -12, -16, -20]
summer_10_yz_ff = [0, 0, 0, -2, -2, -3, -4, -4, -5]
summer_10_zyz_dd = [0, 0, -1, -3, -5, -7, -12, -17, -21]
summer_10_zyz_ff = [0, 0, 0, -1, -2, -3, -4, -4, -5]
summer_10_z_dd = [-1, -2, -3, -6, -8, -9, -13, -18, -22]
summer_10_z_ff = [1, 1, 0, -1, -2, -3, -4, -5, -5]
summer_10_zsz_dd = [-9, -10, -11, -13, -14, -13, -12, -12, -10]
summer_10_zsz_ff = [0, 0, 0, -1, -2, -3, -4, -5, -6]

summer_11_sz_dd = [-5, -4, -4, -4, -2, -1, 3, 9, 12]
summer_11_sz_ff = [0, 0, -2, -3, -4, -5, -6, -7, -5]
summer_11_ssz_dd = [-4, -5, -5, -5, -3, -1, 2, 6, 9]
summer_11_ssz_ff = [0, -1, -2, -3, -4, -5, -6, -6, -6]
summer_11_s_dd = [-3, -6, -7, -6, -5, -2, 1, 4, 6]
summer_11_s_ff = [-1, -2, -3, -3, -4, -5, -6, -6, -6]
summer_11_ssv_dd = [-2, -4, -5, -3, -2, 0 ,2, 5, 7]
summer_11_ssv_ff = [-1, -2, -3, -3, -4, -5, -5, -5, -5]
summer_11_sv_dd = [-2, -3, -3, 1, 1, 3, 4, 6, 8]
summer_11_sv_ff = [-2, -3, -3, -4, -4, -5, -5, -5, -5]
summer_11_vsv_dd = [-3, -3, -3, 0, 1, 2, 3, 4, 5]
summer_11_vsv_ff = [-2, -3, -3, -4, -4, -4, -4, -4, -4]
summer_11_v_dd = [-4, -3, -2, 0, 1, 2, 2, 3, 3]
summer_11_v_ff = [-2, -3, -4, -4, -4, -4, -4, -4, -4]
summer_11_vyv_dd = [3, 3, 4, 4, 2, 2, 0, 0, 0]
summer_11_vyv_ff = [-1, -2, -3, -3, -3, -3, -4, -4, -4]
summer_11_yv_dd = [10, 10, 10, 8, 4, -2, -1, -2, -3]
summer_11_yv_ff = [-1, -2, -2, -3, -3, -3, -4, -4, -3]
summer_11_yyv_dd = [7, 6, 5, 3, 0, -2, -5, -6, -8]
summer_11_yyv_ff = [-1, -1, -2, -3, -3, -3, -4, -5, -5]
summer_11_y_dd = [4, 2, 1, -2, -4, -7, -9, -12, -14]
summer_11_y_ff = [-1, -1, -2, -3, -3, -3, -4, -5, -5]
summer_11_yyz_dd = [2, 1, 0, -1, -4, -6, -8, -11, -14]
summer_11_yyz_ff = [0, 0, -1, -2, -4, -4 ,-5, -6, -7]
summer_11_yz_dd = [0, 0, 0, -1, -4, -6, -8, -11, -14]
summer_11_yz_ff = [0, 0, -1, -2, -4, -4, -6, -7, -7]
summer_11_zyz_dd = [-1, -2, -1, -1, -5, -4, -5, -7, -8]
summer_11_zyz_ff = [0, -1, -1, -2, -4, -4, -6, -7, -8]
summer_11_z_dd = [-2, -2, -2, -1, -7, -3, -3, -3, -3]
summer_11_z_ff = [-1, -2, -2, -3, -4, -5, -7, -8, -9]
summer_11_zsz_dd = [-3, -3, -3, -3, -4, -2, 0, 3, 4]
summer_11_zsz_ff = [0, -1, -2, -3, -4, -5, -6, -7, -7]

autumn_10_sz_dd = [4, 4, 3, 1, 0, 0, -2, -3, 2]
autumn_10_sz_ff = [0, 0, 1, 1, 1, 2, 2, 2, 2]
autumn_10_ssz_dd = [-1, -2, -3, -5, -7, -8, -12, -1, -11]
autumn_10_ssz_ff = [0, 0, 0, 0, 0, 0, 0, 0, 0]
autumn_10_s_dd = [-6, -8, -9, -12, -15, -17, -23, -25, -25]
autumn_10_s_ff = [1, 0, 0, 0, 0, -1, -1, -1, 0]
autumn_10_ssv_dd = [-4, -6, -10, -13, -16, -18, -23, -25, -25]
autumn_10_ssv_ff = [1, 0, 0, -1, -1, -2, -2, -2, -2]
autumn_10_sv_dd = [-2, -5, -12, -15, -18, -20, -23, -26, -17]
autumn_10_sv_ff = [1, 0, -1, -2, -2, -3, -3, -3, -3]
autumn_10_vsv_dd = [-3, -6, -11, -15, -20, -22, -30, -32, -27]
autumn_10_vsv_ff = [0, 0, -1, -2, -2, -2, -3, -3, -3]
autumn_10_v_dd = [-5, -8, -10, -15, -21, -25, -38, -38, -38]
autumn_10_v_ff = [0, -1, -2, -2, -2, -2, -3, -3, -3]
autumn_10_vyv_dd = [3, 4, 4, 4, 4, 3, 0, 2, 5]
autumn_10_vyv_ff = [0, -1, -1, -1, -1, -1, -2, -2, -1]
autumn_10_yv_dd = [12, 17, 19, 23, 30, 32, 38, 42, 49]
autumn_10_yv_ff = [0, -1, -1, -1, -1, -1, -2, -1, 0]
autumn_10_yyv_dd = [8, 12, 16, 20, 26, 30, 38, 39, 45]
autumn_10_yyv_ff = [0, 0, -1, -1, -1, -1, -2, -1, 0]
autumn_10_y_dd = [4, 8, 14, 18, 23, 28, 28, 37, 41]
autumn_10_y_ff = [0, 0, -1, -1, -2, -2, -2, -1, 0]
autumn_10_yyz_dd = [4, 10, 15, 18, 21, 24, 25, 30, 32]
autumn_10_yyz_ff = [0, 0, 0, 0, -1, -1, -1, 0, 0]
autumn_10_yz_dd = [5, 12, 17, 19, 19, 21, 22, 23, 25]
autumn_10_yz_ff = [0, 0, 0, 0, 0, 0, 0, 0, 0]
autumn_10_zyz_dd = [5, 8, 12, 13, 12, 13, 13, 13, 14]
autumn_10_zyz_ff = [0, 0, 0, 0, 0, 0, 0, 0, 0]
autumn_10_z_dd = [5, 5, 6, 7, 5, 5, 4, 4, 2]
autumn_10_z_ff = [1, 1, 1, 1, 0, 0, 0, 0, 0]
autumn_10_zsz_dd = [4, 4, 5, 4, 2, 2, 1, 0, 2]
autumn_10_zsz_ff = [0, 0, 1, 1, 0, 1, 1, 1, 1]

autumn_11_sz_dd = [1, 0, 0, -1, -1, -3, -4, -6, -7]
autumn_11_sz_ff = [-1, -3, -3, -3, -4, -5, -5, -5, -5]
autumn_11_ssz_dd = [4, 1, 0, -2, -4, -6, -9, -11, -13]
autumn_11_ssz_ff = [0, -2, -3, -3, -4, -5, -5, -8, -8]
autumn_11_s_dd = [7, 3, -1, -4, -7, -9, -15, -17, -20]
autumn_11_s_ff = [0, -1, -3, -4, -4, -5, -5, -11, -11]
autumn_11_ssv_dd = [-3, -8, -10, -12, -15, -16, -20, -25, -30]
autumn_11_ssv_ff = [0, -2, -3, -4, -4, -5, -6, -9, -9]
autumn_11_sv_dd = [-14, -18, -20, -21, -23, -23, -26, -33, -40]
autumn_11_sv_ff = [-1, -3, -3, -4, -5, -6, -7, -7, -7]
autumn_11_vsv_dd = [-15, -18, -21, -23, -27, -34, -36, -44, -57]
autumn_11_vsv_ff = [-2, -4, -5, -6, -7, -7, -9, -9, -9]
autumn_11_v_dd = [-17, -19, -22, -26, -32, -45, -46, -55, -74]
autumn_11_v_ff = [-3, -6, -7, -8, -9, -8, -11, -11, -11]
autumn_11_vyv_dd = [2, 1, 1, 0, -1, -7, -5, -8, -16]
autumn_11_vyv_ff = [-2, -4, -5, -6, -7, -7, -9, -9, -9]
autumn_11_yv_dd = [21, 22, 25, 27, 30, 30, -36, 38, 42]
autumn_11_yv_ff = [-1, -3, -4 ,-5, -6, -6, -7, -8, -8]
autumn_11_yyv_dd = [13, 14, 16, 19, 21, 22, 25, 28, 32]
autumn_11_yyv_ff = [-2, -3, -4, -5, -6, -6, -7, -7, -7]
autumn_11_y_dd = [6, 7, 8, 9, 13, 14, 15, 19, 21]
autumn_11_y_ff = [-3, -4, -5, -6, -6, -7, -7, -7, -7]
autumn_11_yyz_dd = [3, 5, 6, 7, 10, 11, 12, 14, 15]
autumn_11_yyz_ff = [-1, -2, -2, -3, -4, -5, -5, -5, -5]
autumn_11_yz_dd = [1, 3, 5, 6, 8, 8, 9, 9, 8]
autumn_11_yz_ff = [1, 0, 0, -1, -2, -3, -3, -3, -3]
autumn_11_zyz_dd = [-1, 0, 2, 2, 3, 3, 1, 2, 1]
autumn_11_zyz_ff = [1, 0, 0, -1, -2, -2, -3, -3, -4]
autumn_11_z_dd = [-4, -3, -1, -1, -2, -3, -6, -5, -5]
autumn_11_z_ff = [2, 1, -1, -1, -2, -2, -3, -3, -4]
autumn_11_zsz_dd = [-2, -1, 0, -1, -1, -3, -5, -5, -6]
autumn_11_zsz_ff = [0, -1, -2, -2, -3, -3, -4, -4, -5]
