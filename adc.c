#include <project.h>
#include "hal.h"
#include "defines.h"
#include "constants.h"

#define ADC_NUMBER_OF_CHANNELS 4

#define ADC_CYCLE_COUNTER_AUX_CONTROL_REG \
        (*(reg8 *) bSAR_SEQ_ChannelCounter__CONTROL_AUX_CTL_REG)

#define ADC_CONTROL_REG (*(reg8 *) bSAR_SEQ_CtrlReg__CONTROL_REG)

#define ADC_STATUS_REG (*(reg8 *) bSAR_SEQ_EOCSts__STATUS_REG)
#define ADC_STATUS_PTR ( (reg8 *) bSAR_SEQ_EOCSts__STATUS_REG)

#define ADC_CYCLE_COUNTER_ENABLE    (0x20u)
#define ADC_BASE_COMPONENT_ENABLE   (0x01u)
#define ADC_LOAD_COUNTER_PERIOD     (0x02u)
#define ADC_SOFTWARE_SOC_PULSE      (0x04u)

HAL_COMP(adc);

//phase current
HAL_PIN(iu_x);
HAL_PIN(iw_x);
HAL_PIN(iu_y);
HAL_PIN(iw_y);

HAL_PIN(enable);

struct adc_ctx_t {
    accum u_offset_x;
    accum w_offset_x;
    accum u_offset_y;
    accum w_offset_y;
};

static uint16_t adc_buffer[ADC_NUMBER_OF_CHANNELS];

static void ADC_StartConvert(void) {
    ADC_CONTROL_REG |= ((uint8_t)(ADC_SOFTWARE_SOC_PULSE));
}

static int16_t ADC_GetResult16(uint16_t chan) {
    return (adc_buffer[ADC_NUMBER_OF_CHANNELS - 1 - chan] - ADC_SAR_shift);
}

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    static int16_t samples[ADC_NUMBER_OF_CHANNELS];
    uint8_t sample_ch, sample_td;
    uint8_t buffer_ch, buffer_td;
    uint8_t flags;

    /* Init DMA, 2 bytes bursts, each burst requires a request */
    sample_ch = ADC_SAMPLE_DMA_DmaInitialize(2, 1,
            HI16(CYDEV_PERIPH_BASE), HI16(CYDEV_SRAM_BASE));

    /* Init DMA, (ADC_NUMBER_OF_CHANNELS * 2) bytes bursts, each burst
     * requires a request */
    buffer_ch = ADC_BUFFER_DMA_DmaInitialize(ADC_NUMBER_OF_CHANNELS*2, 1,
            HI16(CYDEV_SRAM_BASE), HI16(CYDEV_SRAM_BASE));

    ADC_SAR_Start();
    
    CyDmaClearPendingDrq(sample_ch);
    CyDmaClearPendingDrq(buffer_ch);
    
    /* Provides initialization procedure for the TempBuf DMA
    * Configure this Td as follows:
    *  - The TD is looping on itself
    *  - Increment the destination address, but not the source address
    */
    sample_td = CyDmaTdAllocate();

    CyDmaTdSetConfiguration(sample_td, ADC_NUMBER_OF_CHANNELS*2, sample_td,
            ADC_SAMPLE_DMA__TD_TERMOUT_EN | TD_INC_DST_ADR);

    /* From the SAR to the TempArray */
    CyDmaTdSetAddress(sample_td, LO16(ADC_SAR_ADC_SAR__WRK0),
            LO16((uint32_t) samples));

    /* Associate the TD with the channel */
    CyDmaChSetInitialTd(sample_ch, sample_td);

    /* Provides initialization procedure for the FinalBuf DMA
    * Configure this Td as follows:
    *  - The TD is looping on itself
    *  - Increment the source and destination address
    */
    buffer_td = CyDmaTdAllocate();
    
    CyDmaTdSetConfiguration(buffer_td, ADC_NUMBER_OF_CHANNELS*2, buffer_td,
            ADC_BUFFER_DMA__TD_TERMOUT_EN | TD_INC_SRC_ADR | TD_INC_DST_ADR);

    /* From the the TempArray to Final Array */
    CyDmaTdSetAddress(buffer_td, LO16((uint32_t) samples),
            LO16((uint32_t) adc_buffer));

    /* Associate the TD with the channel */
    CyDmaChSetInitialTd(buffer_ch, buffer_td);
    
    CyDmaChEnable(sample_ch, 1u);
    CyDmaChEnable(buffer_ch, 1u);

    /* Enable Counter and give Enable pulse to set an address of the last 
     * channel */
    flags = CyEnterCriticalSection();
    ADC_CYCLE_COUNTER_AUX_CONTROL_REG |= ADC_CYCLE_COUNTER_ENABLE;
    CyExitCriticalSection(flags);

    /* Enable FSM of the Base Component */
    ADC_CONTROL_REG |= ADC_BASE_COMPONENT_ENABLE;
    ADC_CONTROL_REG |= ADC_LOAD_COUNTER_PERIOD;

    CY_GET_REG8(ADC_STATUS_PTR);

    ADC_StartConvert();
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
    struct adc_ctx_t *ctx      = (struct adc_ctx_t *) ctx_ptr;
    struct adc_pin_ctx_t *pins = (struct adc_pin_ctx_t *) pin_ptr;

    if (PIN(enable) == 0K) {
        ctx->u_offset_x += ((accum) ADC_GetResult16(0) - ctx->u_offset_x) *
                            (accum) ADC_DC_FILT_LP;
        ctx->w_offset_x += ((accum) ADC_GetResult16(1) - ctx->w_offset_x) *
                            (accum) ADC_DC_FILT_LP;
        ctx->u_offset_y += ((accum) ADC_GetResult16(2) - ctx->u_offset_y) *
                            (accum) ADC_DC_FILT_LP;
        ctx->w_offset_y += ((accum) ADC_GetResult16(3) - ctx->w_offset_y) *
                            (accum) ADC_DC_FILT_LP;
    } else {
        PIN(iu_x) = ((accum) ADC_GetResult16(0) - ctx->u_offset_x) * (1K/350K);
        PIN(iw_x) = ((accum) ADC_GetResult16(1) - ctx->w_offset_x) * (1K/350K);
        PIN(iu_y) = ((accum) ADC_GetResult16(2) - ctx->u_offset_y) * (1K/350K);
        PIN(iw_y) = ((accum) ADC_GetResult16(3) - ctx->w_offset_y) * (1K/350K);
    }

    ADC_StartConvert();
}

hal_comp_t adc_comp_struct = {
    .name      = "adc",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct adc_ctx_t),
    .pin_count = sizeof(struct adc_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};
