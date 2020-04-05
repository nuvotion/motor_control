/*
* This file is part of the stmbl project.
*
* Copyright (C) 2013-2016 Rene Hopf <renehopf@mac.com>
* Copyright (C) 2013-2016 Nico Stute <crinq@crinq.de>
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
#include "defines.h"
#include "angle.h"
#include "constants.h"

HAL_COMP(pid);

HAL_PIN(pos_ext_cmd);  // cmd in (rad)
HAL_PIN(pos_fb);       // feedback in (rad)

HAL_PIN(vel_ext_cmd);  // cmd in (rad/s)
HAL_PIN(vel_fb);       // feedback in (rad/s)

HAL_PIN(torque_cor_cmd);  // corrected cmd out (Nm)
HAL_PIN(cur_cor_cmd); // Current for PMSM

HAL_PIN(pos_p);
HAL_PIN(vel_p);
HAL_PIN(vel_i);
HAL_PIN(cur_gain);

struct pid_ctx_t {
  sat accum torque_sum;  //integrator
};

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct pid_ctx_t *ctx      = (struct pid_ctx_t *)ctx_ptr;
  struct pid_pin_ctx_t *pins = (struct pid_pin_ctx_t *)pin_ptr;

  sat accum torque_cmd;
  sat accum acc_cmd;

  accum vel_ext_cmd = PIN(vel_ext_cmd);
  accum vel_fb      = PIN(vel_fb);
  accum vel_cmd;
  sat accum vel_error;

  accum pos_ext_cmd = PIN(pos_ext_cmd);
  accum pos_fb      = PIN(pos_fb);
  accum pos_error;

  // pos -> vel
  pos_error = minus(pos_ext_cmd, pos_fb);
  vel_cmd = pos_error * PIN(pos_p);
  vel_cmd += vel_ext_cmd;
  vel_cmd = LIMIT(vel_cmd, PID_MAX_VEL);

  // vel -> acc
  vel_error = vel_cmd - vel_fb;
  acc_cmd = vel_error * PIN(vel_p);
  acc_cmd = LIMIT(acc_cmd, (sat accum) PID_MAX_ACC);

  // acc -> torque
  torque_cmd = acc_cmd;
  ctx->torque_sum += vel_error * PIN(vel_i);
  ctx->torque_sum = // dynamic integral clamping 
    CLAMP(ctx->torque_sum, -PID_MAX_TORQUE - torque_cmd, PID_MAX_TORQUE - torque_cmd);
  torque_cmd += ctx->torque_sum;

  PIN(torque_cor_cmd) = torque_cmd;
  PIN(cur_cor_cmd) = torque_cmd * PIN(cur_gain);
}

hal_comp_t pid_comp_struct = {
    .name      = "pid",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = 0,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct pid_ctx_t),
    .pin_count = sizeof(struct pid_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};
