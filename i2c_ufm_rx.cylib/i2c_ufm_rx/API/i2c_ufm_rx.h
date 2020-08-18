
/* XCFG I2C Extended Configuration Register */
#define `$INSTANCE_NAME`_XCFG_CLK_EN        (0x80u)

/* CFG I2C Configuration Register */
#define `$INSTANCE_NAME`_CFG_STOP_IE        (0x10u)
#define `$INSTANCE_NAME`_CFG_EN_SLAVE       (0x01u)

/* CSR I2C Control and Status Register */
#define `$INSTANCE_NAME`_CSR_ACK            (0x10u)

#define `$INSTANCE_NAME`_DIVIDER            (BCLK__BUS_CLK__MHZ/16)

#define `$INSTANCE_NAME`_ACT_PWR_EN         ((uint8) `$INSTANCE_NAME`_I2C_FF__PM_ACT_MSK)
#define `$INSTANCE_NAME`_STBY_PWR_EN        ((uint8) `$INSTANCE_NAME`_I2C_FF__PM_STBY_MSK)

#define `$INSTANCE_NAME`_ACT_PWRMGR_REG     (*(reg8 *) `$INSTANCE_NAME`_I2C_FF__PM_ACT_CFG)
#define `$INSTANCE_NAME`_STBY_PWRMGR_REG    (*(reg8 *) `$INSTANCE_NAME`_I2C_FF__PM_STBY_CFG)
#define `$INSTANCE_NAME`_XCFG_REG           (*(reg8 *) `$INSTANCE_NAME`_I2C_FF__XCFG)
#define `$INSTANCE_NAME`_CFG_REG            (*(reg8 *) `$INSTANCE_NAME`_I2C_FF__CFG)
#define `$INSTANCE_NAME`_CSR_REG            (*(reg8 *) `$INSTANCE_NAME`_I2C_FF__CSR)
#define `$INSTANCE_NAME`_DATA_REG           (*(reg8 *) `$INSTANCE_NAME`_I2C_FF__D)
#define `$INSTANCE_NAME`_CLKDIV1_REG        (*(reg8 *) `$INSTANCE_NAME`_I2C_FF__CLK_DIV1)
#define `$INSTANCE_NAME`_CLKDIV2_REG        (*(reg8 *) `$INSTANCE_NAME`_I2C_FF__CLK_DIV2)

void `$INSTANCE_NAME`_SetupDMA(uint8 buf_len, uint8 *buf);
