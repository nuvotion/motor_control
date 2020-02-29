/*
* This file is part of the stmbl project.
*
* Copyright (C) 2013-2017 Rene Hopf <renehopf@mac.com>
* Copyright (C) 2013-2017 Nico Stute <crinq@crinq.de>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hal.h"
#include <string.h>
#include <stdio.h>
#include <stdfix.h>
#include "defines.h"

hal_t hal;

hal_comp_t *comp_by_name(NAME name) {
  for(int i = 0; i < comp_count; i++) {
    if(!strncmp(name, comps[i]->name, sizeof(NAME))) {
      return (comps[i]);
    }
  }
  return (0);
}

volatile hal_comp_inst_t *comp_inst_by_name(NAME name, uint32_t instance) {
  hal_comp_t *comp = comp_by_name(name);
  if(comp) {
    for(int i = 0; i < hal.comp_inst_count; i++) {
      if(hal.comp_insts[i].comp == comp && instance == hal.comp_insts[i].instance) {
        return (&hal.comp_insts[i]);
      }
    }
  }
  return (0);
}

pin_t *pin_by_name(NAME comp_name, NAME pin_name) {
  uint32_t offset = 0;
  for(int i = 0; i < comp_count; i++) {
    if(!strncmp(comp_name, comps[i]->name, sizeof(NAME))) {
      for(int j = 0; j < comps[i]->pin_count; j++) {
        if(!strncmp(pin_name, pins[j + offset], sizeof(NAME))) {
          return (&pins[j + offset]);
        }
      }
    }
  }
  return (0);
}

volatile hal_pin_inst_t *pin_inst_by_name(NAME comp_name, uint32_t instance, NAME pin_name) {
  volatile hal_comp_inst_t *comp = comp_inst_by_name(comp_name, instance);
  if(comp) {
    for(int i = 0; i < comp->comp->pin_count; i++) {
      if(!strncmp(pin_name, comp->pins[i], sizeof(NAME))) {
        return (&comp->pin_insts[i]);
      }
    }
  }
  return (0);
}

pin_t *pin_by_pin_inst(volatile hal_pin_inst_t *p) {
  for(int i = 0; i < hal.comp_inst_count; i++) {
    for(int j = 0; j < hal.comp_insts[i].comp->pin_count; j++) {
      if(&(hal.comp_insts[i].pin_insts[j]) == p) {
        return (&hal.comp_insts[i].pins[j]);
      }
    }
  }
  return (0);
}

volatile hal_comp_inst_t *comp_inst_by_pin_inst(volatile hal_pin_inst_t *p) {
  for(int i = 0; i < hal.comp_inst_count; i++) {
    for(int j = 0; j < hal.comp_insts[i].comp->pin_count; j++) {
      if(&(hal.comp_insts[i].pin_insts[j]) == p) {
        return (&hal.comp_insts[i]);
      }
    }
  }
  return (0);
}

uint32_t load_comp(hal_comp_t *comp) {
  if(!comp) {
    return (0);
  }
  if(hal.comp_inst_count >= HAL_MAX_COMPS - 1) {
    return (0);
  }
  if(hal.pin_inst_count + comp->pin_count >= HAL_MAX_PINS - 1) {
    return (0);
  }
  uint32_t ctx_size = (((comp->ctx_size + 3) / 4) * 4);
  if(hal.ctx_count + ctx_size >= HAL_MAX_CTX - 1) {
    return (0);
  }

  // load comp
  hal.comp_insts[hal.comp_inst_count].comp      = comp;
  hal.comp_insts[hal.comp_inst_count].ctx       = &hal.ctxs[hal.ctx_count];
  hal.comp_insts[hal.comp_inst_count].pin_insts = &hal.pin_insts[hal.pin_inst_count];
  hal.comp_insts[hal.comp_inst_count].ctx_size  = ctx_size;
  hal.comp_insts[hal.comp_inst_count].state     = PRE_INIT;

  uint32_t offset = 0;
  for(int i = 0; i < comp_count; i++) {
    if(comps[i] == comp) {
      hal.comp_insts[hal.comp_inst_count].pins = &pins[offset];
    }
    offset += comps[i]->pin_count;
  }
  hal.comp_insts[hal.comp_inst_count].instance = 0;
  for(int i = 0; i < hal.comp_inst_count; i++) {
    if(hal.comp_insts[i].comp == comp) {
      hal.comp_insts[hal.comp_inst_count].instance++;
    }
  }


  // load pins
  for(int i = hal.pin_inst_count; i < hal.pin_inst_count + comp->pin_count; i++) {
    hal.pin_insts[i].value  = 0;
    hal.pin_insts[i].source = &hal.pin_insts[i];
  }
  hal.pin_inst_count += comp->pin_count;

  // load ctx
  for(int i = hal.ctx_count; i < hal.ctx_count + ctx_size; i++) {
    hal.ctxs[i] = 0;
  }

  hal.ctx_count += ctx_size;

  if(hal.comp_insts[hal.comp_inst_count].comp->nrt_init != 0) {
    hal.comp_insts[hal.comp_inst_count].comp->nrt_init(hal.comp_insts[hal.comp_inst_count].ctx, hal.comp_insts[hal.comp_inst_count].pin_insts);
  }
  hal.comp_insts[hal.comp_inst_count].nrt_ticks     = 0;
  hal.comp_insts[hal.comp_inst_count].nrt_max_ticks = 0;
  hal.comp_insts[hal.comp_inst_count].state         = PRE_HW_INIT;

  hal.comp_inst_count++;

  return (1);
}

void hal_run_rt() {
#ifdef HAL_CALC_TIME
  uint32_t hal_start = hal_get_systick_value();
#endif

  switch(hal.rt_state) {
    case RT_STOP:
      return;
    case RT_CALC:  // call stop
      hal.rt_state  = RT_STOP;
      hal.hal_state = RT_TOO_LONG;
      hal.frt_state = RT_STOP;
      return;
    case RT_SLEEP:
      if(hal.active_rt_func > -1) {  // call stop
        hal.rt_state  = RT_STOP;
        hal.hal_state = MISC_ERROR;
        hal.frt_state = RT_STOP;
        return;
      }
      hal.rt_state = RT_CALC;
  }

#ifdef HAL_COMP_CALC_TIME
  uint32_t start = hal_get_systick_value();
#endif

  for(hal.active_rt_func = 0; hal.active_rt_func < hal.rt_comp_count; hal.active_rt_func++) {
    hal.rt_comps[hal.active_rt_func]->comp->rt(hal.rt_period, hal.rt_comps[hal.active_rt_func]->ctx, hal.rt_comps[hal.active_rt_func]->pin_insts);
#ifdef HAL_COMP_CALC_TIME
    uint32_t end_ticks = hal_get_systick_value();
    if(start < end_ticks) {
      start += hal_get_systick_reload();
    }
    hal.rt_comps[hal.active_rt_func]->rt_ticks     = start - end_ticks;
    hal.rt_comps[hal.active_rt_func]->rt_max_ticks = MAX(hal.rt_comps[hal.active_rt_func]->rt_max_ticks, hal.rt_comps[hal.active_rt_func]->rt_ticks);
    start                                          = end_ticks;
#endif
  }
  hal.active_rt_func = -1;

  if(hal.rt_state == RT_CALC) {
    hal.rt_state = RT_SLEEP;
  }

#ifdef HAL_CALC_TIME
  uint32_t hal_end = hal_get_systick_value();
  if(hal_start < hal_end) {
    hal_start += hal_get_systick_reload();
  }
  hal.rt_ticks     = hal_start - hal_end;
  hal.rt_max_ticks = MAX(hal.rt_max_ticks, hal.rt_ticks);
#endif

#ifdef HAL_WATCHDOG
  hal_reset_watchdog();
#endif
}

void hal_run_frt() {
#ifdef HAL_CALC_TIME
  uint32_t hal_start = hal_get_systick_value();
#endif

  switch(hal.frt_state) {
    case RT_STOP:
      return;
    case RT_CALC:
      hal.rt_state  = RT_STOP;
      hal.hal_state = FRT_TOO_LONG;
      hal.frt_state = RT_STOP;
      return;
    case RT_SLEEP:
      if(hal.active_frt_func > -1) {
        hal.rt_state  = RT_STOP;
        hal.hal_state = MISC_ERROR;
        hal.frt_state = RT_STOP;
        return;
      }
      hal.frt_state = RT_CALC;
  }

#ifdef HAL_COMP_CALC_TIME
  uint32_t start = hal_get_systick_value();
#endif

  for(hal.active_frt_func = 0; hal.active_frt_func < hal.frt_comp_count; hal.active_frt_func++) {
    hal.frt_comps[hal.active_frt_func]->comp->frt(hal.frt_period, hal.frt_comps[hal.active_frt_func]->ctx, hal.frt_comps[hal.active_frt_func]->pin_insts);
#ifdef HAL_COMP_CALC_TIME
    uint32_t end_ticks = hal_get_systick_value();
    if(start < end_ticks) {
      start += hal_get_systick_reload();
    }
    hal.frt_comps[hal.active_frt_func]->frt_ticks     = start - end_ticks;
    hal.frt_comps[hal.active_frt_func]->frt_max_ticks = MAX(hal.frt_comps[hal.active_frt_func]->frt_max_ticks, hal.frt_comps[hal.active_frt_func]->frt_ticks);
    start                                             = end_ticks;
#endif
  }
  hal.active_frt_func = -1;

  if(hal.frt_state == RT_CALC) {
    hal.frt_state = RT_SLEEP;
  }

#ifdef HAL_CALC_TIME
  uint32_t hal_end = hal_get_systick_value();
  if(hal_start < hal_end) {
    hal_start += hal_get_systick_reload();
  }
  hal.frt_ticks     = hal_start - hal_end;
  hal.frt_max_ticks = MAX(hal.frt_max_ticks, hal.frt_ticks);
#endif

#ifdef HAL_WATCHDOG
  hal_reset_watchdog();
#endif
}

void hal_run_nrt() {
#ifdef HAL_CALC_TIME
  uint32_t hal_start = hal_get_systick_value();
#endif

#ifdef HAL_COMP_CALC_TIME
  uint32_t start = hal_get_systick_value();
#endif

  for(hal.active_nrt_func = 0; hal.active_nrt_func < hal.comp_inst_count; hal.active_nrt_func++) {
    if(hal.comp_insts[hal.active_nrt_func].comp->nrt != 0) {
      hal.comp_insts[hal.active_nrt_func].comp->nrt(hal.comp_insts[hal.active_nrt_func].ctx, hal.comp_insts[hal.active_nrt_func].pin_insts);
#ifdef HAL_COMP_CALC_TIME
      uint32_t end_ticks = hal_get_systick_value();
      if(start < end_ticks) {
        start += hal_get_systick_reload();
      }
      hal.comp_insts[hal.active_nrt_func].nrt_ticks     = start - end_ticks;
      hal.comp_insts[hal.active_nrt_func].nrt_max_ticks = MAX(hal.comp_insts[hal.active_nrt_func].nrt_max_ticks, hal.comp_insts[hal.active_nrt_func].nrt_ticks);
      start                                             = end_ticks;
#endif
    }
  }
  hal.active_nrt_func = -1;

#ifdef HAL_CALC_TIME
  uint32_t hal_end = hal_get_systick_value();
  if(hal_start < hal_end) {
    hal_start += hal_get_systick_reload();
  }
  hal.nrt_ticks     = hal_start - hal_end;
  hal.nrt_max_ticks = MAX(hal.nrt_max_ticks, hal.nrt_ticks);
#endif

#ifdef HAL_WATCHDOG
  hal_reset_watchdog();
#endif
}

void hal_init_hw() {
  for(int i = 0; i < hal.comp_inst_count; i++) {
    if(hal.comp_insts[i].state == PRE_HW_INIT) {
      if(hal.comp_insts[i].comp->hw_init != 0) {
        hal.comp_insts[i].comp->hw_init(hal.comp_insts[i].ctx, hal.comp_insts[i].pin_insts);
      }
      hal.comp_insts[i].state = STARTED;
    }
  }
}


void load(char *ptr) {
  load_comp(comp_by_name(ptr));
}

void sort_rt() {
  accum min = SACCUM_MAX;
  int min_index = -1;
  accum rt_prio = 0K;
  char added[HAL_MAX_COMPS];
  struct pin_ctx_t *pins;

  for(int i = 0; i < hal.comp_inst_count; i++) {
    added[i] = 0;
  }

  hal.rt_comp_count = 0;
  for(int i = 0; i < hal.comp_inst_count; i++) {
    min       = SACCUM_MAX;
    min_index = -1;
    for(int j = hal.comp_inst_count - 1; j >= 0; j--) {
      pins    = (struct pin_ctx_t *)(hal.comp_insts[j].pin_insts);
      rt_prio = PIN(rt_prio);
      if(rt_prio <= min && added[j] == 0 && rt_prio > 0K && hal.comp_insts[j].comp->rt != 0) {
        min       = rt_prio;
        min_index = j;
      }
    }
    if(min_index >= 0) {
      added[min_index]                  = 1;
      hal.rt_comps[hal.rt_comp_count++] = &hal.comp_insts[min_index];
    }
  }
}

void sort_frt() {
  accum min = SACCUM_MAX;
  int min_index = -1;
  accum frt_prio = 0K;
  char added[HAL_MAX_COMPS];
  struct pin_ctx_t *pins;

  for(int i = 0; i < hal.comp_inst_count; i++) {
    added[i] = 0;
  }

  hal.frt_comp_count = 0;
  for(int i = 0; i < hal.comp_inst_count; i++) {
    min       = SACCUM_MAX;
    min_index = -1;
    for(int j = hal.comp_inst_count - 1; j >= 0; j--) {
      pins     = (struct pin_ctx_t *)(hal.comp_insts[j].pin_insts);
      frt_prio = PIN(frt_prio);
      if(frt_prio <= min && added[j] == 0 && frt_prio > 0K && hal.comp_insts[j].comp->frt != 0) {
        min       = frt_prio;
        min_index = j;
      }
    }
    if(min_index >= 0) {
      added[min_index]                    = 1;
      hal.frt_comps[hal.frt_comp_count++] = &hal.comp_insts[min_index];
    }
  }
}

void start_rt() {
  for(int i = 0; i < hal.rt_comp_count; i++) {
    if(hal.rt_comps[i]->comp->rt_start != 0) {
      hal.rt_comps[i]->comp->rt_start(hal.rt_comps[i]->ctx, hal.rt_comps[i]->pin_insts);
    }
    hal.rt_comps[i]->rt_ticks     = 0;
    hal.rt_comps[i]->rt_max_ticks = 0;
  }

  hal.rt_ticks     = 0;
  hal.rt_max_ticks = 0;

  hal.rt_state = RT_SLEEP;
}

void start_frt() {
  for(int i = 0; i < hal.frt_comp_count; i++) {
    if(hal.frt_comps[i]->comp->frt_start != 0) {
      hal.frt_comps[i]->comp->frt_start(hal.frt_comps[i]->ctx, hal.frt_comps[i]->pin_insts);
    }
    hal.frt_comps[i]->frt_ticks     = 0;
    hal.frt_comps[i]->frt_max_ticks = 0;
  }

  hal.frt_ticks     = 0;
  hal.frt_max_ticks = 0;

  hal.frt_state = RT_SLEEP;
}

void hal_start() {
  hal.hal_state = HAL_OK2;

  sort_rt();
  sort_frt();
  hal_init_hw();
  start_rt();
  start_frt();
}

void stop_rt() {
  hal.rt_state = RT_STOP;

  for(int i = 0; i < hal.rt_comp_count; i++) {
    if(hal.rt_comps[i]->comp->rt_stop != 0) {
      hal.rt_comps[i]->comp->rt_stop(hal.rt_comps[i]->ctx, hal.rt_comps[i]->pin_insts);
    }
  }
}

void stop_frt() {
  hal.frt_state = RT_STOP;

  for(int i = 0; i < hal.frt_comp_count; i++) {
    if(hal.frt_comps[i]->comp->frt_stop != 0) {
      hal.frt_comps[i]->comp->frt_stop(hal.frt_comps[i]->ctx, hal.frt_comps[i]->pin_insts);
    }
  }
}

void hal_stop() {
  stop_rt();
  stop_frt();
}

void hal_init(accum rt_period, accum frt_period) {
  hal.rt_state  = RT_STOP;
  hal.frt_state = RT_STOP;

  for(int i = 0; i < HAL_MAX_COMPS; i++) {
    hal.rt_comps[i]  = 0;
    hal.frt_comps[i] = 0;
  }

  hal.comp_inst_count = 0;
  hal.rt_comp_count   = 0;
  hal.frt_comp_count  = 0;
  hal.pin_inst_count  = 0;

  for(int i = 0; i < HAL_MAX_CTX; i++) {
    hal.ctxs[i] = 0;
  }
  hal.ctx_count = 0;

  hal.active_rt_func  = -1;
  hal.active_frt_func = -1;
  hal.active_nrt_func = -1;

  hal.rt_ticks      = 0;
  hal.rt_max_ticks  = 0;
  hal.frt_ticks     = 0;
  hal.frt_max_ticks = 0;
  hal.nrt_ticks     = 0;
  hal.nrt_max_ticks = 0;
  hal.rt_period     = rt_period;
  hal.frt_period    = frt_period;
  hal.nrt_ticks     = 0;
  hal.nrt_max_ticks = 0;
}

void hal_set_debug_level(uint32_t debug_level) {
  hal.debug_level = debug_level;
}
