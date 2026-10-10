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

#include "ApplyPerformanceTweaks.h"

#include <QDir>
#include <QFileInfo>

#include "FileSystem.h"
#include "launch/LaunchTask.h"
#include "minecraft/MinecraftInstance.h"
#include "settings/SettingsObject.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ApplyPerformanceTweaks::ApplyPerformanceTweaks(LaunchTask* parent) : LaunchStep(parent) {}

void ApplyPerformanceTweaks::executeTask()
{
    auto settings = m_parent->instance()->settings();

    if (settings->get("OptimizeGameOptions").toBool()) {
        seedGameOptions();
    }

#ifdef Q_OS_WIN
    if (settings->get("UseDiscreteGpu").toBool()) {
        setWindowsGpuPreference();
    }
#endif

    emitSucceeded();
}

void ApplyPerformanceTweaks::seedGameOptions()
{
    const auto optionsPath = FS::PathCombine(m_parent->instance()->gameRoot(), "options.txt");
    if (QFileInfo::exists(optionsPath)) {
        // never touch existing settings: the player (or the modpack) already made their choices
        return;
    }

    // Vanilla's frame rate slider goes from 10 to 260, where 260 means "Unlimited". VSync caps the game at the monitor's
    // refresh rate (and adds input latency), so it's turned off. Old versions simply ignore keys they don't know about.
    const QByteArray defaults = "enableVsync:false\nmaxFps:260\n";
    if (auto res = FS::write(optionsPath, defaults); !res) {
        emit logLine(tr("Couldn't write optimized default game options: %1").arg(res.error()), MessageLevel::Warning);
        return;
    }
    emit logLine(tr("First launch: starting with VSync off and an unlimited frame rate (options.txt)."), MessageLevel::Launcher);
}

#ifdef Q_OS_WIN
void ApplyPerformanceTweaks::setWindowsGpuPreference()
{
    // Same thing as Settings > System > Display > Graphics > "High performance" for the Java runtime.
    // On laptops with hybrid graphics, Java otherwise usually ends up on the integrated GPU.
    // This is a per-user, per-executable preference. It's a no-op on systems with a single GPU.
    const auto javaPath = FS::ResolveExecutable(m_parent->instance()->settings()->get("JavaPath").toString());
    if (javaPath.isEmpty()) {
        return;
    }
    const auto valueName = QDir::toNativeSeparators(QFileInfo(javaPath).absoluteFilePath()).toStdWString();
    const wchar_t* subKey = L"Software\\Microsoft\\DirectX\\UserGpuPreferences";

    // Respect any preference that is already there (e.g. the user explicitly picked "Power saving").
    DWORD existingSize = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, subKey, valueName.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &existingSize) == ERROR_SUCCESS) {
        return;
    }

    static const wchar_t s_highPerformance[] = L"GpuPreference=2;";
    const auto status = RegSetKeyValueW(HKEY_CURRENT_USER, subKey, valueName.c_str(), REG_SZ, s_highPerformance,
                                        static_cast<DWORD>(sizeof(s_highPerformance)));
    if (status != ERROR_SUCCESS) {
        emit logLine(tr("Couldn't set the high performance GPU preference for Java (error %1).").arg(status), MessageLevel::Warning);
        return;
    }
    emit logLine(tr("Set Windows' GPU preference for %1 to \"High performance\".").arg(QString::fromStdWString(valueName)),
                 MessageLevel::Launcher);
}
#endif
