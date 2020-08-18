#include <project.h>

void `$INSTANCE_NAME`_SetupDMA(uint8 buf_len, uint8 *buf) {
    uint8 intState;
    uint8 i, curr_td, next_td, ack_td, first_td;
    uint8 rx_dma_ch, ack_dma_ch;
    static uint8 dma_ack = `$INSTANCE_NAME`_CSR_ACK;

    `$INSTANCE_NAME`_CFG_REG  = `$INSTANCE_NAME`_CFG_EN_SLAVE |
                                `$INSTANCE_NAME`_CFG_STOP_IE;
    `$INSTANCE_NAME`_XCFG_REG = `$INSTANCE_NAME`_XCFG_CLK_EN;
    `$INSTANCE_NAME`_CLKDIV1_REG = LO8(`$INSTANCE_NAME`_DIVIDER);
    `$INSTANCE_NAME`_CLKDIV2_REG = HI8(`$INSTANCE_NAME`_DIVIDER);

    /* Enable power to block */
    intState = CyEnterCriticalSection();
    `$INSTANCE_NAME`_ACT_PWRMGR_REG  |= `$INSTANCE_NAME`_ACT_PWR_EN;
    `$INSTANCE_NAME`_STBY_PWRMGR_REG |= `$INSTANCE_NAME`_STBY_PWR_EN;
    CyExitCriticalSection(intState);

    curr_td = CyDmaTdAllocate();
    first_td = curr_td;

    for (i = 0; i < buf_len-1; i++) {
        next_td = CyDmaTdAllocate();
        if ((i % 2) == 0) {
            CyDmaTdSetConfiguration(curr_td, 1, next_td,
                    CY_DMA_TD_AUTO_EXEC_NEXT);
            CyDmaTdSetAddress(curr_td,
                    LO16((uint32) &`$INSTANCE_NAME`_CSR_REG),
                    LO16((uint32) &buf[i/2]));
        } else {
            CyDmaTdSetConfiguration(curr_td, 1, next_td,
                    `$INSTANCE_NAME`_RX_DMA__TD_TERMOUT_EN);
            CyDmaTdSetAddress(curr_td,
                    LO16((uint32) &`$INSTANCE_NAME`_DATA_REG),
                    LO16((uint32) &buf[i/2 + buf_len/2]));
        }
        curr_td = next_td;
    }
    CyDmaTdSetConfiguration(curr_td, 1, first_td,
            `$INSTANCE_NAME`_RX_DMA__TD_TERMOUT_EN);
    CyDmaTdSetAddress(curr_td,
            LO16((uint32) &`$INSTANCE_NAME`_DATA_REG),
            LO16((uint32) &buf[i/2 + buf_len/2]));

    ack_td = CyDmaTdAllocate();
    CyDmaTdSetConfiguration(ack_td, 1, ack_td, 0);
    CyDmaTdSetAddress(ack_td,
            LO16((uint32) &dma_ack),
            LO16((uint32) &`$INSTANCE_NAME`_CSR_REG));

    ack_dma_ch = `$INSTANCE_NAME`_ACK_DMA_DmaInitialize(0, 0,
            HI16(CYDEV_SRAM_BASE),
            HI16(CYDEV_PERIPH_BASE));
    CyDmaClearPendingDrq(ack_dma_ch);
    CyDmaChSetInitialTd(ack_dma_ch, ack_td);
    CyDmaChEnable(ack_dma_ch, 1);

    rx_dma_ch = `$INSTANCE_NAME`_RX_DMA_DmaInitialize(0, 0,
            HI16(CYDEV_PERIPH_BASE),
            HI16(CYDEV_SRAM_BASE));
    CyDmaClearPendingDrq(rx_dma_ch);
    CyDmaChSetInitialTd(rx_dma_ch, first_td);
    CyDmaChEnable(rx_dma_ch, 1);
}
