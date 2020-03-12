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

HAL_COMP(pid);

HAL_PIN(pos_ext_cmd);  // cmd in (rad)
HAL_PIN(pos_fb);       // feedback in (rad)

HAL_PIN(vel_ext_cmd);  // cmd in (rad/s)
HAL_PIN(vel_fb);       // feedback in (rad/s)

HAL_PIN(torque_cor_cmd);  // corrected cmd out (Nm)

HAL_PIN(cur_gain);    // Gain from torque to current
HAL_PIN(cur_cor_cmd); // Current for PMSM

HAL_PIN(pos_p);  // (1/s)

HAL_PIN(vel_p);  // (1/s)
HAL_PIN(vel_i);

// user limits
HAL_PIN(max_usr_vel);     // (rad/s)
HAL_PIN(max_usr_acc);     // (rad/s^2)
HAL_PIN(max_usr_torque);  // (Nm)

struct pid_ctx_t {
  accum torque_sum;  //integrator
};

static void nrt_init(volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct pid_ctx_t *ctx      = (struct pid_ctx_t *)ctx_ptr;
  struct pid_pin_ctx_t *pins = (struct pid_pin_ctx_t *)pin_ptr;

  ctx->torque_sum = 0.0;
  PIN(pos_p)      = 100.0;   // (1/s)
  PIN(vel_p)      = 2000.0;  // (1/s)
  PIN(vel_i)      = 10.0;
}

static void rt_func(accum period, volatile void *ctx_ptr, volatile hal_pin_inst_t *pin_ptr) {
  struct pid_ctx_t *ctx      = (struct pid_ctx_t *)ctx_ptr;
  struct pid_pin_ctx_t *pins = (struct pid_pin_ctx_t *)pin_ptr;

  accum torque_min     = -PIN(max_usr_torque);
  accum torque_max     =  PIN(max_usr_torque);
  accum torque_cmd;

  accum acc_min     = -PIN(max_usr_acc);
  accum acc_max     = PIN(max_usr_acc);
  accum acc_cmd;

  accum vel_ext_cmd = PIN(vel_ext_cmd);
  accum vel_fb      = PIN(vel_fb);
  accum vel_min     = -PIN(max_usr_vel);
  accum vel_max     =  PIN(max_usr_vel);
  accum vel_cmd;
  accum vel_error;

  accum pos_ext_cmd = PIN(pos_ext_cmd);
  accum pos_fb      = PIN(pos_fb);
  accum pos_error;

  accum pos_p = PIN(pos_p);
  accum vel_p = PIN(vel_p);
  accum vel_i = PIN(vel_i);

  // pos -> vel
  pos_error = minus(pos_ext_cmd, pos_fb);
  vel_cmd = pos_error * pos_p;
  vel_cmd += vel_ext_cmd;
  vel_cmd = CLAMP(vel_cmd, vel_min, vel_max);

  // vel -> acc
  vel_error = vel_cmd - vel_fb;
  acc_cmd = vel_error * vel_p;
  acc_cmd = CLAMP(acc_cmd, acc_min, acc_max);

  // acc -> torque
  torque_cmd = acc_cmd;
  ctx->torque_sum += vel_error * period * vel_i;
  ctx->torque_sum = // dynamic integral clamping 
    CLAMP(ctx->torque_sum, torque_min - torque_cmd, torque_max - torque_cmd);
  torque_cmd += ctx->torque_sum;

  PIN(torque_cor_cmd) = torque_cmd;
  PIN(cur_cor_cmd) = torque_cmd * PIN(cur_gain);
}

hal_comp_t pid_comp_struct = {
    .name      = "pid",
    .nrt       = 0,
    .rt        = rt_func,
    .frt       = 0,
    .nrt_init  = nrt_init,
    .rt_start  = 0,
    .frt_start = 0,
    .rt_stop   = 0,
    .frt_stop  = 0,
    .ctx_size  = sizeof(struct pid_ctx_t),
    .pin_count = sizeof(struct pid_pin_ctx_t) / sizeof(struct hal_pin_inst_t),
};
