/*
 * Copyright (c) 2019, Open Source Robotics Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PHIDGETS_API_DC_MOTOR_H
#define PHIDGETS_API_DC_MOTOR_H

#include <functional>
#include <string>
#include <mutex>

#include <libphidget22/phidget22.h>

#include "phidgets_api/phidget22.hpp"

namespace phidgets
{
class DCMotor : public PhidgetChannel
{
public:
  PHIDGET22_NO_COPY_NO_MOVE_NO_ASSIGN(DCMotor)

  explicit DCMotor(const ChannelAddress& channel_address, std::function<void()> velocity_change_handler,
                 std::function<void()> back_emf_change_handler);

  ~DCMotor();

  double getVelocity();
  void setTargetVelocity(double velocity) const;
  double getAcceleration() const;
  void setAcceleration(double acceleration) const;
  bool backEMFSensingSupported() const;
  double getBackEMF();
  void setDataInterval(uint32_t data_interval_ms) const;

  double getBraking() const;
  void setBraking(double braking) const;

  void velocityChangeHandler(double velocity);

  void backEMFChangeHandler(double back_emf);

private:
  PhidgetDCMotorHandle handle_;
  std::mutex mutex_;
  double velocity_ = 0.0;
  std::function<void()> velocity_change_handler_;
  double back_emf_ = 0.0;
  std::function<void()> back_emf_change_handler_;
  bool back_emf_sensing_supported_ = true;

  static void VelocityChangeHandler(PhidgetDCMotorHandle motor_handle, void* ctx, double velocity);
  static void BackEMFChangeHandler(PhidgetDCMotorHandle motor_handle, void* ctx, double back_emf);
};

}  // namespace phidgets

#endif  // PHIDGETS_API_DC_MOTOR_H