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

#include "ObsidianTheme.h"

#include <QObject>

QString ObsidianTheme::id()
{
    return "obsidian";
}

QString ObsidianTheme::name()
{
    return QObject::tr("Obsidian");
}

QString ObsidianTheme::tooltip()
{
    return QObject::tr("The default LuciferClient look: near-black, white text and a red accent.");
}

QPalette ObsidianTheme::colorScheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(11, 11, 11));
    palette.setColor(QPalette::WindowText, QColor(240, 240, 240));
    palette.setColor(QPalette::Base, QColor(20, 20, 20));
    palette.setColor(QPalette::AlternateBase, QColor(28, 28, 28));
    palette.setColor(QPalette::ToolTipBase, QColor(28, 28, 28));
    palette.setColor(QPalette::ToolTipText, QColor(240, 240, 240));
    palette.setColor(QPalette::Text, QColor(240, 240, 240));
    palette.setColor(QPalette::Button, QColor(28, 28, 28));
    palette.setColor(QPalette::ButtonText, QColor(240, 240, 240));
    palette.setColor(QPalette::BrightText, QColor(255, 90, 95));
    palette.setColor(QPalette::Link, QColor(255, 107, 107));
    palette.setColor(QPalette::Highlight, QColor(200, 48, 48));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, QColor(122, 122, 122));
    palette.setColor(QPalette::Light, QColor(60, 60, 60));
    palette.setColor(QPalette::Midlight, QColor(44, 44, 44));
    palette.setColor(QPalette::Mid, QColor(34, 34, 34));
    palette.setColor(QPalette::Dark, QColor(8, 8, 8));
    palette.setColor(QPalette::Shadow, Qt::black);
    return fadeInactive(palette, fadeAmount(), fadeColor());
}

double ObsidianTheme::fadeAmount()
{
    return 0.5;
}

QColor ObsidianTheme::fadeColor()
{
    return QColor(11, 11, 11);
}

bool ObsidianTheme::hasStyleSheet()
{
    return true;
}

QString ObsidianTheme::appStyleSheet()
{
    // Only named main window widgets are styled here, so dialogs keep the regular (dark) Fusion look.
    return QStringLiteral(R"(
QToolTip {
    color: #f0f0f0;
    background-color: #1c1c1c;
    border: 1px solid #3a3a3a;
    padding: 4px;
}

/* header */
QToolBar#mainToolBar {
    background-color: #0e0e0e;
    border: none;
    border-bottom: 1px solid #a3262a;
    padding: 0px 14px;
    spacing: 2px;
}
QToolBar#mainToolBar QToolButton {
    color: #bdbdbd;
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 6px;
}
QToolBar#mainToolBar QToolButton:hover {
    color: #ffffff;
    background-color: #1d1d1d;
}
QToolBar#mainToolBar QToolButton:pressed {
    background-color: #262626;
}
QToolBar#mainToolBar QToolButton::menu-indicator {
    image: none;
    width: 0px;
}
QToolBar#mainToolBar QToolButton[headerTab="true"] {
    color: #cfcfcf;
    font-size: 14px;
    border-radius: 0px;
    padding: 0px 15px;
    min-height: 62px;
}
QToolBar#mainToolBar QToolButton[headerTab="true"]:hover {
    color: #ffffff;
    background-color: #181818;
}
QToolBar#mainToolBar QToolButton[headerTab="true"]:checked {
    color: #ffffff;
    background-color: #232323;
}

/* instance actions below the instance tiles */
QWidget#instanceActionRow QToolButton {
    color: #d6d6d6;
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 6px 12px;
}
QWidget#instanceActionRow QToolButton:hover {
    color: #ffffff;
    background-color: #1a1a1a;
}
QWidget#instanceActionRow QToolButton:pressed {
    background-color: #242424;
}
QWidget#instanceActionRow QToolButton:disabled {
    color: #555555;
}
QWidget#instanceActionRow QToolButton::menu-indicator {
    image: none;
    width: 0px;
}

/* status bar */
QStatusBar {
    color: #9a9a9a;
    background-color: #0b0b0b;
    border-top: 1px solid #1c1c1c;
}
QStatusBar::item {
    border: none;
}
QStatusBar QLabel {
    padding: 2px 8px;
}

/* slim scroll bars */
QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 0px;
}
QScrollBar:horizontal {
    background: transparent;
    height: 10px;
    margin: 0px;
}
QScrollBar::handle:vertical {
    background-color: #2c2c2c;
    border-radius: 5px;
    min-height: 32px;
}
QScrollBar::handle:horizontal {
    background-color: #2c2c2c;
    border-radius: 5px;
    min-width: 32px;
}
QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
    background-color: #3c3c3c;
}
QScrollBar::add-line, QScrollBar::sub-line {
    width: 0px;
    height: 0px;
}
QScrollBar::add-page, QScrollBar::sub-page {
    background: none;
}
)");
}
