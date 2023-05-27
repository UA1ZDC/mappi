#ifndef PHYSICAL_CONSTANTS_H
#define PHYSICAL_CONSTANTS_H

const double LIGHT_SPEED = 2.999979250101010101E+8; //m/s
const double EPSILON_0 = 8.85418781762E-12; //1E+7/(4*PI*LIGHT_SPEED*LIGHT_SPEED); //F/m [A*s/(V*m)] (vacuum electr. permittivity)
const double MU_0 = 1.25663706144E-6;//4*PI*1E-7;//Hn/m [V*s/(A*m)] (vacuum magn. permittivity)
const double ELECTRON_CHARGE = 1.602191770707070E-19; //Kl
const double ELECTRON_MASS = 9.109558545454E-31; //kg
const double UNI_ATOM_MASS_UNIT = 1.66053111E-27; //kg
const double ATOM_MASS_UNIT = 1.6597E-27; //kg
const double ELECTRON_VOLT = 1.60210E-19; //J
const double VACUUM_WAVE_RESIST = 376.730313462; //sqrt(MU_0/EPSILON_0);
const double BOLTSMAN_CONSTANT = 1.38E-16; //erg/grad of KELVIN

const double ATOM_MASS_H = 1.00784*ATOM_MASS_UNIT;   //kg
const double ATOM_MASS_HE = 4.002602*ATOM_MASS_UNIT; //kg
const double ATOM_MASS_O = 15.99903*ATOM_MASS_UNIT;  //kg
const double ATOM_MASS_N = 14.00643*ATOM_MASS_UNIT;  //kg

const double RADIUS_H = 53.E-10;    //cm
const double RADIUS_H2 = 115.E-10;  //cm
const double RADIUS_HE = 122.E-10;   //cm
const double RADIUS_N = 92.E-10;    //cm
const double RADIUS_N2 = 160.E-10;  //cm
const double RADIUS_O = 60.E-10;    //cm
const double RADIUS_O2 = 150.E-10;  //cm
const double RADIUS_NO = 115.E-10;  //cm

const double RADIUS_H_MINUS1_ION = 54.E-10;    //cm
const double RADIUS_HE_MINUS1_ION = 93.E-10;   //cm
const double RADIUS_N_MINUS3_ION = 132.E-10;    //cm
const double RADIUS_N_PLUS3_ION = 30.E-10;  //cm
const double RADIUS_N_PLUS5_ION = 27.E-10;  //cm
const double RADIUS_O_MINUS2_ION = 128.E-10;    //cm



const double CGS_ELECTRON_CHARGE = 4.80325021E-10; //CGSE
const double CGS_ELECTRON_MASS = 9.109558545454E-28; //g
const double CGS_LIGHT_SPEED = 2.999979250101010101E+10; //cm/s

const double CGS_CHARGE = 3.33564E-10; //CGSE
const double CGS_ELECTR_INTENSITY = 2.997825E+4; // V/m
const double CGS_MAGNIT_INTENSITY = 79.5775; // A/m
const double CGS_ELECTR_CONDUCTIVITY = 1.11265E-10 ; // Sm/m

const double UNIVERSAL_GAS_CONSTANT =8.31441E+3;//J/(kmol K)
const double SPECIFIC_DRY_AIR_CONSTANT =287.; //J/(kg K)
const double SPECIFIC_WATER_VAPOUR_CONSTANT =461.5;//J/(kg K)

#endif
