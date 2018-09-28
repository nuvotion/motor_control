#include <project.h>

#include <hal.h>

uint32_t hal_get_systick_value() {
  return 0;
}

uint32_t hal_get_systick_reload() {
  return 0;
}

uint32_t hal_get_systick_freq() {
  return 0;
}

volatile uint64_t systime = 0;

int main(void) {
    CyGlobalIntEnable;

    hal_init(0.0002, 0.00005);

    hal_parse("load dq");
    hal_parse("load idq");
    hal_parse("load svm");
    hal_parse("load curpid");

    for(;;) {
        hal_run_frt();
        hal_run_rt();
        hal_run_nrt();
    }
}
