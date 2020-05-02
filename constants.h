#define PERIOD              0.0002 

#define VEL_ACC_LP          LP_HZ(50.0, PERIOD)
#define VEL_ACC_FF_GAIN     (1000.0*1000.0*PERIOD)
#define VEL_VEL_FF_GAIN     (2*0.9*1000.0*PERIOD)

//#define BUS_DC              141.0K
#define BUS_DC              62.0K
#define BUS_3PH             (BUS_DC / M_SQRT3 * 0.95K)
#define SAT_FACTOR          1.0K
#define SAT_VOLTAGE_DC      (SAT_FACTOR*SAT_FACTOR * BUS_DC*BUS_DC)
#define SAT_VOLTAGE_3PH     (SAT_FACTOR*SAT_FACTOR * BUS_3PH*BUS_3PH)
#define CURPID_MAX_CURRENT  1.0K
#define CURPID_SAT_TIME     0.33K

#define CONF_X_R            1.70        // Resistance (ohms) measured
#define CONF_X_L            0.0026      // Inductance (henry) measured
#define CONF_Y_R            0.95        // Resistance (ohms) measured
#define CONF_Y_L            0.0016      // Inductance (henry) measured
#define CONF_Z_R            17.5        // Resistance (ohms) measured
#define CONF_Z_L            0.0065      // Inductance (henry) measured

#define CURPID_X_KP         (1.000 * CONF_X_L / PERIOD / 2.0)
#define CURPID_X_KI         (1.000 * CONF_X_R * PERIOD / CONF_X_L)
#define CURPID_Y_KP         (1.000 * CONF_Y_L / PERIOD / 2.0)
#define CURPID_Y_KI         (1.000 * CONF_Y_R * PERIOD / CONF_Y_L)
#define CURPID_Z_KP         (1.000 * CONF_Z_L / PERIOD / 2.0)
#define CURPID_Z_KI         (1.000 * CONF_Z_R * PERIOD / CONF_Z_L)

#define FB_POLECOUNT        2K
#define FB_MOT_POLECOUNT    1K
#define FB_COM_POLECOUNT    2K
#define FB_MOT_OFFSET       (M_PI / 6.0K)
#define FB_COM_OFFSET       (M_PI / 6.0K)

#define PID_MAX_VEL         800K
#define PID_MAX_ACC         8000K
#define PID_MAX_TORQUE      8000K

#define PID_POS_P_X         100K
#define PID_VEL_P_X         5K
#define PID_VEL_I_PERIOD_X  (5K * PERIOD)
#define PID_CUR_GAIN_X      2.5e-3K

#define PID_POS_P_Y         100K
#define PID_VEL_P_Y         5K
#define PID_VEL_I_PERIOD_Y  (5K * PERIOD)
#define PID_CUR_GAIN_Y      5.0e-3K

#define PID_POS_P_Z         100K
#define PID_VEL_P_Z         5K
#define PID_VEL_I_PERIOD_Z  (5K * PERIOD)
#define PID_CUR_GAIN_Z      1.0e-3K
