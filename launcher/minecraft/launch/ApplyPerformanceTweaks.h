// SPDX-License-Identifier: GPL-3.0-only
/*
 *  LuciferClient - Minecraft Launcher
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <launch/LaunchStep.h>

// Launch-time FPS tweaks that live outside of the JVM command line:
//  - seeds a fresh instance's options.txt with VSync off and an unlimited frame rate ("OptimizeGameOptions")
//  - on Windows, asks the OS to run the Java runtime on the high performance (discrete) GPU ("UseDiscreteGpu")
// None of these tweaks are critical: failures are logged and the launch continues.
class ApplyPerformanceTweaks : public LaunchStep {
    Q_OBJECT
   public:
    explicit ApplyPerformanceTweaks(LaunchTask* parent);
    ~ApplyPerformanceTweaks() override = default;

    void executeTask() override;
    bool canAbort() const override { return false; }

   private:
    void seedGameOptions();
#ifdef Q_OS_WIN
    void setWindowsGpuPreference();
#endif
};
