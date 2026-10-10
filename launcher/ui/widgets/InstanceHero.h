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

#include <QPixmap>
#include <QString>
#include <QWidget>

class QAction;
class QLabel;
class QToolButton;

/**
 * The large banner at the top of the main window ("Obsidian" layout).
 *
 * It shows a background artwork, the name of the selected instance, a short description line and a big
 * PLAY button. The buttons are driven by the main window's launch/kill actions, so they always mirror
 * the enabled state and the launch menu of those actions.
 */
class InstanceHero : public QWidget {
    Q_OBJECT

   public:
    explicit InstanceHero(QAction* launchAction, QAction* killAction, QWidget* parent = nullptr);
    ~InstanceHero() override = default;

    /// Show the given instance name and description line.
    void setInstanceInfo(const QString& title, const QString& subtitle);
    /// Show the "nothing selected" placeholder.
    void clearInstance();
    /// Re-read the state of the launch/kill actions (enabled state, launch menu, kill visibility).
    void syncActions();

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

   protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

   private:
    void retranslate();
    void updateElidedTexts();

    QAction* m_launchAction = nullptr;
    QAction* m_killAction = nullptr;

    QLabel* m_title = nullptr;
    QLabel* m_subtitle = nullptr;
    QToolButton* m_playButton = nullptr;
    QToolButton* m_launchOptionsButton = nullptr;
    QToolButton* m_killButton = nullptr;

    bool m_hasInstance = false;
    QString m_fullTitle;
    QString m_fullSubtitle;

    QPixmap m_background;
    QPixmap m_scaledBackground;
    QSize m_scaledFor;
};
