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

#include <QList>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include "modplatform/ModIndex.h"
#include "tasks/Task.h"

class MinecraftInstance;

// "Boost FPS": installs a curated set of well known, client side performance mods (Sodium, Lithium, FerriteCore, ...) from
// Modrinth that match the instance's Minecraft version and mod loader, along with their required dependencies.
// Mods that are already installed - or that would conflict with something already installed (e.g. OptiFine vs. Sodium) - are
// skipped, and mods without a compatible release are simply left out.
class InstallPerformanceModsTask : public Task {
    Q_OBJECT
   public:
    explicit InstallPerformanceModsTask(MinecraftInstance* instance);
    ~InstallPerformanceModsTask() override = default;

    QStringList installedMods() const { return m_installed; }
    QStringList skippedMods() const { return m_skipped; }
    QStringList unavailableMods() const { return m_unavailable; }

    bool canAbort() const override { return true; }
    bool abort() override;

   protected:
    void executeTask() override;

   private:
    struct Candidate {
        QString name;
        // Modrinth project slugs/IDs, tried in order until one of them has a compatible version
        QStringList projects;
        // lower case file name fragments meaning "this mod, or an incompatible alternative, is already installed"
        QStringList conflicts;
        QString requiredBy;  // set for dependencies
    };
    struct PendingDownload {
        ModPlatform::IndexedPack::Ptr pack;
        ModPlatform::IndexedVersion version;
        QString name;
    };

    QList<Candidate> candidatesForLoader() const;
    void scanInstalledMods();
    QString findInstalled(const QStringList& fragments) const;
    QString findInstalledByStem(const QString& stem) const;

    void resolveNext();
    void onVersionsReceived(const Candidate& candidate, int projectIndex, const QVector<ModPlatform::IndexedVersion>& versions);
    void downloadNext();
    void updateProgress();

    MinecraftInstance* m_instance;
    QString m_mcVersion;
    ModPlatform::ModLoaderTypes m_loaders;

    QStringList m_installedFiles;  // lower case file names in the mods folder (including disabled ones)
    QString m_metadata;            // contents of the mod metadata index, used to detect projects by their ID

    QList<QPair<Candidate, int>> m_queue;  // candidate and index of the project to try next
    QSet<QString> m_seenProjects;
    QList<PendingDownload> m_downloads;
    Task::Ptr m_current;
    int m_stepsDone = 0;

    QStringList m_installed;
    QStringList m_skipped;
    QStringList m_unavailable;
};
