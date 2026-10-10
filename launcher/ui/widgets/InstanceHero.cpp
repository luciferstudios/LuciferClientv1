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

#include "InstanceHero.h"

#include <QAction>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
constexpr qreal kHeroRadius = 8.0;

// The banner always sits on top of a dark artwork, so it carries its own style sheet and looks the same with every theme.
const char* const kHeroStyleSheet = R"(
QLabel#heroTitle {
    color: #ffffff;
    background: transparent;
    font-size: 30px;
    font-weight: bold;
}
QLabel#heroSubtitle {
    color: #d4d4d4;
    background: transparent;
    font-size: 14px;
}
QToolButton#heroPlayButton, QToolButton#heroMenuButton, QToolButton#heroKillButton {
    background-color: rgba(0, 0, 0, 70);
    border-radius: 6px;
    padding: 8px 48px;
    min-height: 24px;
    font-size: 15px;
    font-weight: bold;
}
QToolButton#heroPlayButton {
    color: #ffffff;
    border: 2px solid #ffffff;
}
QToolButton#heroMenuButton {
    color: #ffffff;
    border: 2px solid rgba(255, 255, 255, 150);
    padding: 8px 10px;
}
QToolButton#heroMenuButton::menu-indicator {
    image: none;
    width: 0px;
}
QToolButton#heroKillButton {
    color: #ff7a7a;
    border: 2px solid #e5484d;
    padding: 8px 24px;
}
QToolButton#heroPlayButton:hover, QToolButton#heroMenuButton:hover {
    background-color: rgba(255, 255, 255, 30);
}
QToolButton#heroPlayButton:pressed, QToolButton#heroMenuButton:pressed {
    background-color: rgba(255, 255, 255, 60);
}
QToolButton#heroKillButton:hover {
    background-color: rgba(229, 72, 77, 50);
}
QToolButton#heroPlayButton:disabled, QToolButton#heroMenuButton:disabled {
    color: rgba(255, 255, 255, 90);
    border-color: rgba(255, 255, 255, 70);
}
)";
}  // namespace

InstanceHero::InstanceHero(QAction* launchAction, QAction* killAction, QWidget* parent)
    : QWidget(parent), m_launchAction(launchAction), m_killAction(killAction), m_background(QStringLiteral(":/backgrounds/lucifer-hero"))
{
    setObjectName(QStringLiteral("instanceHero"));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 18, 24, 18);
    layout->setSpacing(6);
    layout->addStretch(1);

    m_title = new QLabel(this);
    m_title->setObjectName(QStringLiteral("heroTitle"));
    m_title->setAlignment(Qt::AlignCenter);
    m_title->setTextFormat(Qt::PlainText);
    // the text is elided manually, so the label must never force the window to grow
    m_title->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    layout->addWidget(m_title);

    m_subtitle = new QLabel(this);
    m_subtitle->setObjectName(QStringLiteral("heroSubtitle"));
    m_subtitle->setAlignment(Qt::AlignCenter);
    m_subtitle->setTextFormat(Qt::PlainText);
    m_subtitle->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    layout->addWidget(m_subtitle);

    layout->addSpacing(14);

    auto* buttonRow = new QHBoxLayout();
    buttonRow->setSpacing(8);
    buttonRow->addStretch(1);

    m_playButton = new QToolButton(this);
    m_playButton->setObjectName(QStringLiteral("heroPlayButton"));
    m_playButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_playButton->setCursor(Qt::PointingHandCursor);
    connect(m_playButton, &QToolButton::clicked, m_launchAction, &QAction::trigger);
    buttonRow->addWidget(m_playButton);

    m_launchOptionsButton = new QToolButton(this);
    m_launchOptionsButton->setObjectName(QStringLiteral("heroMenuButton"));
    m_launchOptionsButton->setArrowType(Qt::DownArrow);
    m_launchOptionsButton->setPopupMode(QToolButton::InstantPopup);
    m_launchOptionsButton->setCursor(Qt::PointingHandCursor);
    buttonRow->addWidget(m_launchOptionsButton);

    m_killButton = new QToolButton(this);
    m_killButton->setObjectName(QStringLiteral("heroKillButton"));
    m_killButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
    m_killButton->setCursor(Qt::PointingHandCursor);
    connect(m_killButton, &QToolButton::clicked, m_killAction, &QAction::trigger);
    buttonRow->addWidget(m_killButton);

    buttonRow->addStretch(1);
    layout->addLayout(buttonRow);
    layout->addStretch(1);

    setStyleSheet(QString::fromUtf8(kHeroStyleSheet));

    // keep the buttons in sync with the actions they represent
    connect(m_launchAction, &QAction::changed, this, &InstanceHero::syncActions);
    connect(m_killAction, &QAction::changed, this, &InstanceHero::syncActions);

    retranslate();
    clearInstance();
}

void InstanceHero::setInstanceInfo(const QString& title, const QString& subtitle)
{
    m_hasInstance = true;
    m_fullTitle = title;
    m_fullSubtitle = subtitle;
    updateElidedTexts();
    syncActions();
}

void InstanceHero::clearInstance()
{
    m_hasInstance = false;
    m_fullTitle = tr("No instance selected");
    m_fullSubtitle = tr("Pick an instance below, or create a new one with the + button.");
    updateElidedTexts();
    syncActions();
}

void InstanceHero::syncActions()
{
    const bool canLaunch = m_hasInstance && m_launchAction->isEnabled();
    m_playButton->setEnabled(canLaunch);
    m_playButton->setToolTip(m_launchAction->toolTip());

    QMenu* launchMenu = m_launchAction->menu();
    const bool hasLaunchMenu = launchMenu != nullptr && !launchMenu->isEmpty();
    m_launchOptionsButton->setMenu(hasLaunchMenu ? launchMenu : nullptr);
    m_launchOptionsButton->setEnabled(canLaunch);
    m_launchOptionsButton->setVisible(m_hasInstance && hasLaunchMenu);

    const bool canKill = m_hasInstance && m_killAction->isEnabled();
    m_killButton->setToolTip(m_killAction->toolTip());
    m_killButton->setVisible(canKill);
}

void InstanceHero::retranslate()
{
    m_playButton->setText(tr("PLAY"));
    m_launchOptionsButton->setToolTip(tr("More ways to launch this instance"));
    m_killButton->setText(tr("KILL"));
    if (!m_hasInstance) {
        clearInstance();
    } else {
        syncActions();
    }
}

void InstanceHero::updateElidedTexts()
{
    const int available = qMax(0, width() - 64);
    m_title->ensurePolished();
    m_subtitle->ensurePolished();
    m_title->setText(m_title->fontMetrics().elidedText(m_fullTitle, Qt::ElideRight, available));
    m_subtitle->setText(m_subtitle->fontMetrics().elidedText(m_fullSubtitle, Qt::ElideRight, available));
    m_title->setToolTip(m_title->text() != m_fullTitle ? m_fullTitle : QString());
    m_subtitle->setToolTip(m_subtitle->text() != m_fullSubtitle ? m_fullSubtitle : QString());
}

QSize InstanceHero::sizeHint() const
{
    return QSize(640, 230);
}

QSize InstanceHero::minimumSizeHint() const
{
    return QSize(320, 170);
}

void InstanceHero::paintEvent([[maybe_unused]] QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRectF frame = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath shape;
    shape.addRoundedRect(frame, kHeroRadius, kHeroRadius);

    painter.save();
    painter.setClipPath(shape);

    if (!m_background.isNull()) {
        // scale the artwork so it covers the whole banner ("object-fit: cover"), then center it
        const qreal ratio = devicePixelRatioF();
        if (m_scaledBackground.isNull() || m_scaledFor != size()) {
            const QSize target = m_background.size().scaled(size(), Qt::KeepAspectRatioByExpanding);
            m_scaledBackground = m_background.scaled(target * ratio, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            m_scaledBackground.setDevicePixelRatio(ratio);
            m_scaledFor = size();
        }
        const QSizeF shown = m_scaledBackground.deviceIndependentSize();
        const QPointF topLeft((width() - shown.width()) / 2.0, (height() - shown.height()) / 2.0);
        painter.drawPixmap(topLeft, m_scaledBackground);
    } else {
        QLinearGradient fallback(0, 0, 0, height());
        fallback.setColorAt(0.0, QColor(38, 40, 46));
        fallback.setColorAt(1.0, QColor(16, 16, 18));
        painter.fillRect(rect(), fallback);
    }

    // darken the artwork so the white text stays readable, a bit more towards the bottom
    QLinearGradient shade(0, 0, 0, height());
    shade.setColorAt(0.0, QColor(0, 0, 0, 90));
    shade.setColorAt(1.0, QColor(0, 0, 0, 150));
    painter.fillRect(rect(), shade);

    painter.restore();

    // thin outline
    painter.setPen(QPen(QColor(255, 255, 255, 40), 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(shape);
}

void InstanceHero::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateElidedTexts();
}

void InstanceHero::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    } else if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange) {
        updateElidedTexts();
    }
    QWidget::changeEvent(event);
}
