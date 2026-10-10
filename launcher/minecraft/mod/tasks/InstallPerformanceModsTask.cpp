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

#include "InstallPerformanceModsTask.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>

#include "Application.h"
#include "ResourceDownloadTask.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "minecraft/mod/ModFolderModel.h"
#include "modplatform/modrinth/ModrinthAPI.h"

namespace {
// Display names for common dependencies (dependencies are only known by their project ID)
QString knownProjectName(const QString& projectId)
{
    if (projectId == "P7dR8mSH") {
        return QStringLiteral("Fabric API");
    }
    if (projectId == "qvIfYCYJ") {
        return QStringLiteral("Quilted Fabric API");
    }
    return {};
}

// "sodium-fabric-0.6.0+mc1.21.1.jar" -> "sodium-fabric", "lithium-fabric-mc1.21.1-0.13.1.jar" -> "lithium-fabric"
QString fileNameStem(const QString& fileName)
{
    static const QRegularExpression s_versionStart(QStringLiteral("[-_+ ](v|mc)?\\d"));
    auto name = fileName.toLower();
    if (name.endsWith(".jar")) {
        name.chop(4);
    }
    if (auto match = s_versionStart.match(name); match.hasMatch() && match.capturedStart() > 0) {
        name.truncate(match.capturedStart());
    }
    return name;
}

const ModPlatform::IndexedVersion* pickVersion(const QVector<ModPlatform::IndexedVersion>& versions)
{
    // versions are sorted newest first: prefer the newest release, then the newest beta, then anything
    for (auto type : { ModPlatform::IndexedVersionType::Release, ModPlatform::IndexedVersionType::Beta }) {
        for (const auto& version : versions) {
            if (version.versionType == type) {
                return &version;
            }
        }
    }
    return versions.isEmpty() ? nullptr : &versions.first();
}
}  // namespace

InstallPerformanceModsTask::InstallPerformanceModsTask(MinecraftInstance* instance) : m_instance(instance) {}

QList<InstallPerformanceModsTask::Candidate> InstallPerformanceModsTask::candidatesForLoader() const
{
    // Renderer replacements: only ever one of these (or OptiFine) can be installed.
    const QStringList rendererConflicts = { "sodium", "embeddium", "rubidium", "celeritas", "optifine", "optifabric", "vulkanmod" };
    const Candidate ferriteCore{ "FerriteCore", { "ferrite-core" }, { "ferritecore", "ferrite-core" }, {} };
    const Candidate modernFix{ "ModernFix", { "modernfix" }, { "modernfix" }, {} };
    const Candidate immediatelyFast{ "ImmediatelyFast", { "immediatelyfast" }, { "immediatelyfast" }, {} };
    const Candidate entityCulling{ "Entity Culling", { "entityculling" }, { "entityculling", "entity-culling", "entity_culling" }, {} };
    const QStringList lithiumConflicts = { "lithium", "canary", "radium" };

    if (m_loaders & ModPlatform::NeoForge) {
        return { { "Sodium / Embeddium", { "sodium", "embeddium" }, rendererConflicts, {} },
                 { "Lithium", { "lithium", "canary" }, lithiumConflicts, {} },
                 ferriteCore,
                 modernFix,
                 immediatelyFast,
                 entityCulling };
    }
    if (m_loaders & ModPlatform::Forge) {
        return { { "Embeddium", { "embeddium" }, rendererConflicts, {} },
                 { "Canary", { "canary" }, lithiumConflicts, {} },
                 ferriteCore,
                 modernFix,
                 immediatelyFast,
                 entityCulling };
    }
    if (m_loaders & (ModPlatform::Fabric | ModPlatform::Quilt)) {
        return { { "Sodium", { "sodium" }, rendererConflicts, {} },
                 { "Lithium", { "lithium" }, lithiumConflicts, {} },
                 ferriteCore,
                 modernFix,
                 immediatelyFast,
                 entityCulling };
    }
    return {};
}

void InstallPerformanceModsTask::executeTask()
{
    auto* profile = m_instance->getPackProfile();
    if (profile == nullptr) {
        emitFailed(tr("Couldn't read the instance's components."));
        return;
    }
    m_mcVersion = profile->getComponentVersion("net.minecraft");
    if (m_mcVersion.isEmpty()) {
        if (auto res = profile->reload(Net::Mode::Offline); !res) {
            qWarning() << "Failed to reload components:" << res.error();
        }
        m_mcVersion = profile->getComponentVersion("net.minecraft");
    }
    auto loaders = profile->getSupportedModLoaders();
    if (m_mcVersion.isEmpty() || !loaders.has_value()) {
        emitFailed(tr("Performance mods need a mod loader. Install Fabric (recommended), Quilt, NeoForge or Forge first."));
        return;
    }
    m_loaders = loaders.value();

    const auto candidates = candidatesForLoader();
    if (candidates.isEmpty()) {
        emitFailed(tr("There are no performance mods available for this instance's mod loader."));
        return;
    }

    scanInstalledMods();
    for (const auto& candidate : candidates) {
        if (auto found = findInstalled(candidate.conflicts); !found.isEmpty()) {
            m_skipped << tr("%1 (found %2)").arg(candidate.name, found);
            continue;
        }
        m_queue.append({ candidate, 0 });
    }

    setStatus(tr("Looking for performance mods for Minecraft %1...").arg(m_mcVersion));
    resolveNext();
}

void InstallPerformanceModsTask::scanInstalledMods()
{
    const QDir modsDir(m_instance->modsRoot());
    for (const auto& entry : modsDir.entryInfoList(QDir::Files)) {
        m_installedFiles << entry.fileName().toLower();
    }
    // project IDs of mods that were installed through the launcher
    QDirIterator it(modsDir.filePath(".index"), { "*.toml" }, QDir::Files);
    while (it.hasNext()) {
        QFile file(it.next());
        if (file.open(QIODevice::ReadOnly)) {
            m_metadata += QString::fromUtf8(file.readAll());
        }
    }
}

QString InstallPerformanceModsTask::findInstalled(const QStringList& fragments) const
{
    for (const auto& file : m_installedFiles) {
        for (const auto& fragment : fragments) {
            if (file.contains(fragment)) {
                return file;
            }
        }
    }
    return {};
}

QString InstallPerformanceModsTask::findInstalledByStem(const QString& stem) const
{
    if (stem.isEmpty()) {
        return {};
    }
    // "fabric-api" matches "fabric-api-0.92.2+1.20.1.jar" but "mod" doesn't match "modernfix-....jar"
    for (const auto& file : m_installedFiles) {
        if (file.startsWith(stem) && (file.size() == stem.size() || !file.at(stem.size()).isLetter())) {
            return file;
        }
    }
    return {};
}

void InstallPerformanceModsTask::updateProgress()
{
    setProgress(m_stepsDone, m_stepsDone + m_queue.size() + m_downloads.size());
}

void InstallPerformanceModsTask::resolveNext()
{
    if (!isRunning()) {
        return;
    }
    updateProgress();
    if (m_queue.isEmpty()) {
        downloadNext();
        return;
    }

    const auto next = m_queue.takeFirst();
    const Candidate candidate = next.first;
    const int projectIndex = next.second;
    const auto& project = candidate.projects.at(projectIndex);
    m_seenProjects.insert(project);

    auto pack = std::make_shared<ModPlatform::IndexedPack>();
    pack->addonId = project;
    pack->provider = ModPlatform::ResourceProvider::MODRINTH;
    pack->name = candidate.name;
    pack->slug = project;
    pack->resourceType = ModPlatform::ResourceType::Mod;

    ResourceAPI::VersionSearchArgs args{ .pack = pack,
                                         .mcVersions = std::vector<Version>{ Version(m_mcVersion) },
                                         .loaders = m_loaders,
                                         .resourceType = ModPlatform::ResourceType::Mod,
                                         .includeChangelog = false };

    ResourceAPI::Callback<QVector<ModPlatform::IndexedVersion>> callbacks;
    callbacks.onSucceed = [this, candidate, projectIndex](QVector<ModPlatform::IndexedVersion>& versions) {
        if (isRunning()) {
            onVersionsReceived(candidate, projectIndex, versions);
        }
    };
    callbacks.onFail = [this, candidate](const QString& reason, int) {
        if (isRunning()) {
            m_unavailable << tr("%1 (%2)").arg(candidate.name, reason);
        }
    };

    setStatus(tr("Looking for %1...").arg(candidate.name));
    m_current = ModrinthAPI::get().getProjectVersions(args, callbacks);
    if (!m_current) {
        m_unavailable << candidate.name;
        resolveNext();
        return;
    }
    m_stepsDone++;
    connect(m_current.get(), &Task::finished, this, &InstallPerformanceModsTask::resolveNext);
    m_current->start();
}

void InstallPerformanceModsTask::onVersionsReceived(const Candidate& candidate,
                                                    int projectIndex,
                                                    const QVector<ModPlatform::IndexedVersion>& versions)
{
    const auto* version = pickVersion(versions);
    if (version == nullptr) {
        if (projectIndex + 1 < candidate.projects.size()) {
            // try the alternative (e.g. Embeddium when Sodium doesn't support this version)
            m_queue.prepend({ candidate, projectIndex + 1 });
        } else if (candidate.requiredBy.isEmpty()) {
            m_unavailable << tr("%1 (no version for Minecraft %2)").arg(candidate.name, m_mcVersion);
        } else {
            m_unavailable << tr("%1, required by %2 (no compatible version)").arg(candidate.name, candidate.requiredBy);
        }
        return;
    }

    const auto projectId = version->addonId.toString();
    // "Sodium / Embeddium" -> the one that was actually picked
    auto name = candidate.name;
    if (const auto names = candidate.name.split(" / "); names.size() == candidate.projects.size()) {
        name = names.at(projectIndex);
    }
    if (!candidate.requiredBy.isEmpty()) {
        name = knownProjectName(projectId);
        if (name.isEmpty()) {
            name = fileNameStem(version->fileName);
        }
    }
    m_seenProjects.insert(projectId);

    // already installed through the launcher, or by file name (e.g. a dependency that was added by hand)
    if (!projectId.isEmpty() && m_metadata.contains('"' + projectId + '"')) {
        m_skipped << tr("%1 (already installed)").arg(name);
        return;
    }
    if (auto found = findInstalledByStem(fileNameStem(version->fileName)); !found.isEmpty()) {
        m_skipped << tr("%1 (found %2)").arg(name, found);
        return;
    }

    auto pack = std::make_shared<ModPlatform::IndexedPack>();
    pack->addonId = projectId;
    pack->provider = ModPlatform::ResourceProvider::MODRINTH;
    pack->name = name;
    pack->slug = candidate.projects.at(projectIndex);
    pack->resourceType = ModPlatform::ResourceType::Mod;
    m_downloads.append({ pack, *version, name });

    for (const auto& dependency : version->dependencies) {
        const auto dependencyId = dependency.addonId.toString();
        if (dependency.type != ModPlatform::DependencyType::REQUIRED || dependencyId.isEmpty() || m_seenProjects.contains(dependencyId)) {
            continue;
        }
        m_seenProjects.insert(dependencyId);
        auto dependencyName = knownProjectName(dependencyId);
        m_queue.append({ Candidate{ dependencyName.isEmpty() ? dependencyId : dependencyName, { dependencyId }, {}, name }, 0 });
    }
}

void InstallPerformanceModsTask::downloadNext()
{
    if (!isRunning()) {
        return;
    }
    updateProgress();
    if (m_downloads.isEmpty()) {
        m_current.reset();
        emitSucceeded();
        return;
    }

    const auto download = m_downloads.takeFirst();
    const bool isIndexed = !APPLICATION->settings()->get("ModMetadataDisabled").toBool();
    auto task = makeShared<ResourceDownloadTask>(download.pack, download.version, m_instance->loaderModList(), isIndexed,
                                                 QStringLiteral("standalone"));

    setStatus(tr("Installing %1...").arg(download.name));
    connect(task.get(), &Task::succeeded, this, [this, download] { m_installed << download.name; });
    connect(task.get(), &Task::failed, this,
            [this, download](const QString& reason) { m_unavailable << tr("%1 (download failed: %2)").arg(download.name, reason); });
    connect(task.get(), &Task::finished, this, &InstallPerformanceModsTask::downloadNext);
    m_current = task;
    m_stepsDone++;
    task->start();
}

bool InstallPerformanceModsTask::abort()
{
    // change our state first, so that the current step finishing doesn't start the next one
    emitAborted();
    if (m_current && m_current->isRunning()) {
        m_current->abort();
    }
    return true;
}
