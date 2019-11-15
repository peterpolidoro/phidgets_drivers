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

#include "phidgets_api/dc_motor.hpp"

namespace phidgets
{
DCMotor::DCMotor(const ChannelAddress& channel_address, std::function<void()> velocity_change_handler,
             std::function<void()> back_emf_change_handler)
  : PhidgetChannel(channel_address)
  , velocity_change_handler_(velocity_change_handler)
  , back_emf_change_handler_(back_emf_change_handler)
{
  PhidgetReturnCode ret = PhidgetDCMotor_create(&handle_);
  if (ret != EPHIDGET_OK)
  {
    throw Phidget22Error("Failed to create DCMotor handle for channel " + std::to_string(channel_address.channel), ret);
  }

  openWaitForAttachment(reinterpret_cast<PhidgetHandle>(handle_), getChannelAddress());

  ret = PhidgetDCMotor_setOnVelocityUpdateHandler(handle_, VelocityChangeHandler, this);
  if (ret != EPHIDGET_OK)
  {
    throw Phidget22Error("Failed to set duty cycle update handler for DCMotor channel " +
                             std::to_string(getChannelAddress().channel),
                         ret);
  }

  ret = PhidgetDCMotor_setBackEMFSensingState(handle_, 1);
  if (ret == EPHIDGET_UNSUPPORTED)
  {
    back_emf_sensing_supported_ = false;
  }
  else if (ret == EPHIDGET_OK)
  {
    ret = PhidgetDCMotor_setOnBackEMFChangeHandler(handle_, BackEMFChangeHandler, this);
    if (ret != EPHIDGET_OK)
    {
      throw Phidget22Error("Failed to set back EMF update handler for DCMotor channel " +
                               std::to_string(getChannelAddress().channel),
                           ret);
    }
  }
  else
  {
    throw Phidget22Error(
        "Failed to set back EMF sensing state DCMotor channel " + std::to_string(getChannelAddress().channel), ret);
  }
}

DCMotor::~DCMotor()
{
  close(reinterpret_cast<PhidgetHandle>(handle_));
  PhidgetDCMotor_delete(&handle_);
}

double DCMotor::getVelocity()
{
  std::lock_guard<std::mutex> lock(mutex_);
  return velocity_;
}

void DCMotor::setTargetVelocity(double velocity) const
{
  PhidgetReturnCode ret = PhidgetDCMotor_setTargetVelocity(handle_, velocity);
  if (ret != EPHIDGET_OK)
  {
    throw Phidget22Error("Failed to set target velocity for DCMotor channel " + std::to_string(getChannelAddress().channel),
                         ret);
  }
}

double DCMotor::getAcceleration() const
{
  double accel;
  PhidgetReturnCode ret = PhidgetDCMotor_getAcceleration(handle_, &accel);
  if (ret != EPHIDGET_OK)
  {
    throw Phidget22Error("Failed to get acceleration for DCMotor channel " + std::to_string(getChannelAddress().channel),
                         ret);
  }
  return accel;
}

void DCMotor::setAcceleration(double acceleration) const
{
  PhidgetReturnCode ret = PhidgetDCMotor_setAcceleration(handle_, acceleration);
  if (ret != EPHIDGET_OK)
  {
    throw Phidget22Error("Failed to set acceleration for DCMotor channel " + std::to_string(getChannelAddress().channel),
                         ret);
  }
}

bool DCMotor::backEMFSensingSupported() const
{
  return back_emf_sensing_supported_;
}

double DCMotor::getBackEMF()
{
  if (!back_emf_sensing_supported_)
  {
    throw Phidget22Error("Back EMF sensing not supported", EPHIDGET_UNSUPPORTED);
  }
  std::lock_guard<std::mutex> lock(mutex_);
  return back_emf_;
}

void DCMotor::setDataInterval(uint32_t data_interval_ms) const
{
  PhidgetReturnCode ret = PhidgetDCMotor_setDataInterval(handle_, data_interval_ms);
  if (ret != EPHIDGET_OK)
  {
    throw Phidget22Error("Failed to set data interval for DCMotor channel " + std::to_string(getChannelAddress().channel),
                         ret);
  }
}

double DCMotor::getBraking() const
{
  double braking;
  PhidgetReturnCode ret = PhidgetDCMotor_getBrakingStrength(handle_, &braking);
  if (ret != EPHIDGET_OK)
  {
    throw Phidget22Error(
        "Failed to get braking strength for DCMotor channel " + std::to_string(getChannelAddress().channel), ret);
  }
  return braking;
}

void DCMotor::setBraking(double braking) const
{
  PhidgetReturnCode ret = PhidgetDCMotor_setTargetBrakingStrength(handle_, braking);
  if (ret != EPHIDGET_OK)
  {
    throw Phidget22Error(
        "Failed to set braking strength for DCMotor channel " + std::to_string(getChannelAddress().channel), ret);
  }
}

void DCMotor::velocityChangeHandler(double velocity)
{
  {
    std::lock_guard<std::mutex> lock(mutex_);
    velocity_ = velocity;
  }
  velocity_change_handler_();
}

void DCMotor::backEMFChangeHandler(double back_emf)
{
  {
    std::lock_guard<std::mutex> lock(mutex_);
    back_emf_ = back_emf;
  }
  back_emf_change_handler_();
}

void DCMotor::VelocityChangeHandler(PhidgetDCMotorHandle /* motor_handle */, void* ctx, double velocity)
{
  ((DCMotor*)ctx)->velocityChangeHandler(velocity);
}

void DCMotor::BackEMFChangeHandler(PhidgetDCMotorHandle /* motor_handle */, void* ctx, double back_emf)
{
  ((DCMotor*)ctx)->backEMFChangeHandler(back_emf);
}

}  // namespace phidgets