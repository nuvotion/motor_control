#define ADC_NUMBER_OF_CHANNELS 4

#define ADC_CYCLE_COUNTER_AUX_CONTROL_REG \
        (*(reg8 *) bSAR_SEQ_ChannelCounter__CONTROL_AUX_CTL_REG)

#define ADC_CONTROL_REG \
        (*(reg8 *) bSAR_SEQ_CtrlReg__CONTROL_REG)

#define ADC_STATUS_PTR ((reg8 *) bSAR_SEQ_EOCSts__STATUS_REG)

#define ADC_CYCLE_COUNTER_ENABLE    (0x20u)
#define ADC_BASE_COMPONENT_ENABLE   (0x01u)
#define ADC_LOAD_COUNTER_PERIOD     (0x02u)
#define ADC_SOFTWARE_SOC_PULSE      (0x04u)

extern void ADC_Start(void);
extern void ADC_StartConvert(void);
extern int16_t ADC_GetResult16(uint16_t chan);
