#include <project.h>

void `$INSTANCE_NAME`_Setup(uint16_t period) {
    `$INSTANCE_NAME`_PERIOD_REG = period - 1;
    `$INSTANCE_NAME`_CONTROL_REG = (1 << 7);
}

uint16_t `$INSTANCE_NAME`_Read(void) {
    return `$INSTANCE_NAME`_COUNT_REG;
}
