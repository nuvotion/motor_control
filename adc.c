#include <project.h>
#include "adc.h"

static uint16_t adc_buffer[ADC_NUMBER_OF_CHANNELS];

void ADC_Start(void) {
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
}

void ADC_StartConvert(void) {
    ADC_CONTROL_REG |= ((uint8_t)(ADC_SOFTWARE_SOC_PULSE));
}

int16_t ADC_GetResult16(uint16_t chan) {
    return (adc_buffer[ADC_NUMBER_OF_CHANNELS - 1 - chan] - ADC_SAR_shift);
}
