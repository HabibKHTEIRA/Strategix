#include "GameSelectionWidget.h"
#include "../localization/LanguageManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QFile>

namespace Strategix {

static QPixmap loadRoundedThumbnail(const QString& path, int w, int h, int radius = 8) {
    QPixmap src(path);
    if (src.isNull()) {
        QString localPath = path;
        if (localPath.startsWith(":/")) {
            localPath = localPath.mid(2);
        }
        src.load(localPath);
    }

    if (src.isNull()) {
        QPixmap empty(w, h);
        empty.fill(QColor(40, 40, 44));
        return empty;
    }

    QPixmap scaled = src.scaled(w, h, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    int cx = (scaled.width() - w) / 2;
    int cy = (scaled.height() - h) / 2;
    QPixmap cropped = scaled.copy(cx, cy, w, h);

    QPixmap rounded(w, h);
    rounded.fill(Qt::transparent);

    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath clipPath;
    clipPath.addRoundedRect(0, 0, w, h, radius, radius);
    painter.setClipPath(clipPath);
    painter.drawPixmap(0, 0, cropped);

    return rounded;
}

GameSelectionWidget::GameSelectionWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(12, 10, 12, 10);
    rootLayout->setSpacing(8);
    rootLayout->setAlignment(Qt::AlignCenter);

    // Header Title
    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName("homeTitle");
    m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; letter-spacing: 0.5px;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    rootLayout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(this);
    m_subtitleLabel->setObjectName("homeSubtitle");
    m_subtitleLabel->setStyleSheet("font-size: 13px; margin-bottom: 4px;");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    rootLayout->addWidget(m_subtitleLabel);

    // Row 0: 4 games (Checkers, Go, Connect Four, Reversi)
    QHBoxLayout *row0Layout = new QHBoxLayout;
    row0Layout->setAlignment(Qt::AlignCenter);
    row0Layout->setSpacing(16);

    row0Layout->addWidget(createGameCard("game_checkers", "game_checkers_desc",
        ":/img/40-pions-en-bois-pour-jeu-de-dames-35-cm-moulures-dans-coffret-bois.webp",
        "#71717A", &GameSelectionWidget::checkersSelected));

    row0Layout->addWidget(createGameCard("game_go", "game_go_desc",
        ":/img/go.webp",
        "#71717A", &GameSelectionWidget::goSelected));

    row0Layout->addWidget(createGameCard("game_connectfour", "game_connectfour_desc",
        ":/img/connect_four.webp",
        "#71717A", &GameSelectionWidget::connectFourSelected));

    row0Layout->addWidget(createGameCard("game_reversi", "game_reversi_desc",
        ":/img/reversi.webp",
        "#71717A", &GameSelectionWidget::reversiSelected));

    rootLayout->addLayout(row0Layout);

    // Row 1: 3 games (Quoridor, Nine Men's Morris, Dots and Boxes)
    QHBoxLayout *row1Layout = new QHBoxLayout;
    row1Layout->setAlignment(Qt::AlignCenter);
    row1Layout->setSpacing(16);

    row1Layout->addWidget(createGameCard("game_quoridor", "game_quoridor_desc",
        ":/img/Quoridor.webp",
        "#71717A", &GameSelectionWidget::quoridorSelected));

    row1Layout->addWidget(createGameCard("game_morris", "game_morris_desc",
        ":/img/nine_mens_morris.webp",
        "#71717A", &GameSelectionWidget::morrisSelected));

    row1Layout->addWidget(createGameCard("game_dotsandboxes", "game_dotsandboxes_desc",
        ":/img/dot_and_boxes.webp",
        "#71717A", &GameSelectionWidget::dotsAndBoxesSelected));

    rootLayout->addLayout(row1Layout);

    setDarkMode(true);
    updateTexts();

    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &GameSelectionWidget::updateTexts);
}

QWidget* GameSelectionWidget::createGameCard(const QString& titleKey, const QString& descKey,
                                           const QString& imageResourcePath, const QString& /*accentColor*/,
                                           void (GameSelectionWidget::*signal)()) {
    QFrame *card = new QFrame(this);
    card->setObjectName("gameCard");
    card->setFixedSize(225, 260);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(12, 12, 12, 12);
    cardLayout->setSpacing(8);
    cardLayout->setAlignment(Qt::AlignCenter);

    // Image thumbnail (200 x 105)
    QLabel *imageLabel = new QLabel(card);
    imageLabel->setObjectName("cardImage");
    imageLabel->setFixedSize(201, 105);
    imageLabel->setAlignment(Qt::AlignCenter);
    QPixmap thumb = loadRoundedThumbnail(imageResourcePath, 201, 105, 6);
    imageLabel->setPixmap(thumb);
    cardLayout->addWidget(imageLabel, 0, Qt::AlignCenter);

    // Title
    QLabel *title = new QLabel(card);
    title->setObjectName("cardTitle");
    title->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(title);

    // Description
    QLabel *desc = new QLabel(card);
    desc->setObjectName("cardDesc");
    desc->setAlignment(Qt::AlignCenter);
    desc->setWordWrap(true);
    cardLayout->addWidget(desc, 1);

    // Play Button
    QPushButton *playBtn = new QPushButton(card);
    playBtn->setObjectName("cardPlayBtn");
    playBtn->setCursor(Qt::PointingHandCursor);
    connect(playBtn, &QPushButton::clicked, this, signal);
    cardLayout->addWidget(playBtn, 0, Qt::AlignCenter);

    card->setProperty("titleKey", titleKey);
    card->setProperty("descKey", descKey);

    return card;
}

void GameSelectionWidget::setDarkMode(bool dark) {
    if (dark) {
        m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #F4F4F5; letter-spacing: 0.5px;");
        m_subtitleLabel->setStyleSheet("font-size: 13px; color: #A1A1AA; margin-bottom: 4px;");

        for (QFrame *card : findChildren<QFrame*>("gameCard")) {
            card->setStyleSheet(
                "QFrame#gameCard {"
                "  background-color: #202024;"
                "  border: 1px solid #323238;"
                "  border-radius: 12px;"
                "}"
                "QFrame#gameCard:hover {"
                "  border: 1px solid #71717A;"
                "  background-color: #27272D;"
                "}"
            );

            QLabel *title = card->findChild<QLabel*>("cardTitle");
            if (title) title->setStyleSheet("font-size: 15px; font-weight: bold; color: #F4F4F5;");

            QLabel *desc = card->findChild<QLabel*>("cardDesc");
            if (desc) desc->setStyleSheet("font-size: 11px; color: #A1A1AA; line-height: 1.3;");

            QPushButton *btn = card->findChild<QPushButton*>("cardPlayBtn");
            if (btn) {
                btn->setStyleSheet(
                    "QPushButton#cardPlayBtn {"
                    "  background-color: #2E2E34;"
                    "  color: #F4F4F5;"
                    "  font-weight: 600;"
                    "  font-size: 12px;"
                    "  padding: 6px 18px;"
                    "  border-radius: 6px;"
                    "  border: 1px solid #3F3F46;"
                    "}"
                    "QPushButton#cardPlayBtn:hover {"
                    "  background-color: #3F3F46;"
                    "  border-color: #A1A1AA;"
                    "  color: #FFFFFF;"
                    "}"
                );
            }
        }
    } else {
        // Light Mode styling
        m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #18181B; letter-spacing: 0.5px;");
        m_subtitleLabel->setStyleSheet("font-size: 13px; color: #71717A; margin-bottom: 4px;");

        for (QFrame *card : findChildren<QFrame*>("gameCard")) {
            card->setStyleSheet(
                "QFrame#gameCard {"
                "  background-color: #FFFFFF;"
                "  border: 1px solid #E4E4E7;"
                "  border-radius: 12px;"
                "}"
                "QFrame#gameCard:hover {"
                "  border: 1px solid #71717A;"
                "  background-color: #FAFAFA;"
                "}"
            );

            QLabel *title = card->findChild<QLabel*>("cardTitle");
            if (title) title->setStyleSheet("font-size: 15px; font-weight: bold; color: #18181B;");

            QLabel *desc = card->findChild<QLabel*>("cardDesc");
            if (desc) desc->setStyleSheet("font-size: 11px; color: #71717A; line-height: 1.3;");

            QPushButton *btn = card->findChild<QPushButton*>("cardPlayBtn");
            if (btn) {
                btn->setStyleSheet(
                    "QPushButton#cardPlayBtn {"
                    "  background-color: #F4F4F5;"
                    "  color: #18181B;"
                    "  font-weight: 600;"
                    "  font-size: 12px;"
                    "  padding: 6px 18px;"
                    "  border-radius: 6px;"
                    "  border: 1px solid #D4D4D8;"
                    "}"
                    "QPushButton#cardPlayBtn:hover {"
                    "  background-color: #E4E4E7;"
                    "  border-color: #71717A;"
                    "  color: #000000;"
                    "}"
                );
            }
        }
    }
}

void GameSelectionWidget::updateTexts() {
    m_titleLabel->setText(trText("choose_game"));
    m_subtitleLabel->setText(trText("choose_game_subtitle"));

    for (QFrame *card : findChildren<QFrame*>("gameCard")) {
        QString tKey = card->property("titleKey").toString();
        QString dKey = card->property("descKey").toString();

        QLabel *title = card->findChild<QLabel*>("cardTitle");
        if (title) title->setText(trText(tKey));

        QLabel *desc = card->findChild<QLabel*>("cardDesc");
        if (desc) desc->setText(trText(dKey));

        QPushButton *btn = card->findChild<QPushButton*>("cardPlayBtn");
        if (btn) btn->setText(trText("play"));
    }
}

} // namespace Strategix
