
#define `$INSTANCE_NAME`_PERIOD_REG \
	    (*(reg16 *) `$INSTANCE_NAME`_counterdp_u0__16BIT_D0_REG)

#define `$INSTANCE_NAME`_CONTROL_REG \
	    (*(reg8 *) `$INSTANCE_NAME`_ctrlreg__CONTROL_REG)

#define `$INSTANCE_NAME`_COUNT_REG \
	    (*(reg16 *) `$INSTANCE_NAME`_counterdp_u0__16BIT_A0_REG)

extern void `$INSTANCE_NAME`_Setup(uint16_t period);

extern uint16_t `$INSTANCE_NAME`_Read(void);
