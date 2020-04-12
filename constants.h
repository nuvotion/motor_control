#define PERIOD              0.0002 

#define VEL_ACC_LP          LP_HZ(50.0, PERIOD)
#define VEL_ACC_FF_GAIN     (1000.0*1000.0*PERIOD)
#define VEL_VEL_FF_GAIN     (2*0.9*1000.0*PERIOD)

#define BUS_DC              62.0K
#define CURPID_BUS_3PH      (BUS_DC / M_SQRT3 * 0.95K)
#define CURPID_MAX_CURRENT  1.0K
#define CURPID_SAT_VOLTAGE  (0.33K*0.33K * CURPID_BUS_3PH*CURPID_BUS_3PH)
#define CURPID_SAT_TIME     (0.33K)

#define CONF_X_R            1.70        // Resistance (ohms) measured
#define CONF_X_L            0.0026      // Inductance (henry) measured
#define CONF_Y_R            0.95        // Resistance (ohms) measured
#define CONF_Y_L            0.0016      // Inductance (henry) measured

#define CURPID_X_KP         (1.000 * CONF_X_L / PERIOD / 2.0)
#define CURPID_X_KI         (1.000 * CONF_X_R * PERIOD / CONF_X_L)
#define CURPID_Y_KP         (1.000 * CONF_Y_L / PERIOD / 2.0)
#define CURPID_Y_KI         (1.000 * CONF_Y_R * PERIOD / CONF_Y_L)

#define FB_POLECOUNT        2K
#define FB_MOT_POLECOUNT    1K
#define FB_COM_POLECOUNT    2K
#define FB_MOT_OFFSET       (M_PI / 6.0K)
#define FB_COM_OFFSET       (M_PI / 6.0K)

#define PID_MAX_VEL         800K
#define PID_MAX_ACC         8000K
#define PID_MAX_TORQUE      8000K

#define PID_POS_P_X         100K
#define PID_VEL_P_X         2K
#define PID_VEL_I_PERIOD_X  (0K * PERIOD)
#define PID_CUR_GAIN_X      4.0e-3K
#define PID_POS_P_Y         100K
#define PID_VEL_P_Y         2K
#define PID_VEL_I_PERIOD_Y  (0K * PERIOD)
#define PID_CUR_GAIN_Y      4.0e-3K