#include "QuoridorWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <algorithm>
#include <cmath>

namespace Strategix::Quoridor {

QuoridorWidget::QuoridorWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void QuoridorWidget::setBoard(const QuoridorBoard& board) {
    m_board = board;
    update();
}

void QuoridorWidget::setActivePlayer(PlayerId player) {
    m_activePlayer = player;
    update();
}

void QuoridorWidget::setInteractive(bool interactive) {
    m_interactive = interactive;
    update();
}

void QuoridorWidget::setUIMode(UIMode mode) {
    m_uiMode = mode;
    update();
}

void QuoridorWidget::toggleWallOrientation() {
    m_wallOrientation = (m_wallOrientation == WallOrientation::Horizontal)
                            ? WallOrientation::Vertical
                            : WallOrientation::Horizontal;
    update();
}

QRectF QuoridorWidget::boardRect() const {
    qreal side = std::min(width(), height()) - 28;
    qreal x = (width() - side) / 2.0;
    qreal y = (height() - side) / 2.0;
    return QRectF(x, y, side, side);
}

QRectF QuoridorWidget::cellRect(int r, int c) const {
    QRectF bRect = boardRect();
    // 9 cells, 8 grooves. Let groove ratio be 0.2 of cell
    // 9 * s + 8 * (0.2 * s) = 10.6 * s = bRect.width()
    qreal s = bRect.width() / 10.6;
    qreal g = s * 0.2;

    qreal cellX = bRect.left() + c * (s + g);
    qreal cellY = bRect.top() + r * (s + g);
    return QRectF(cellX, cellY, s, s);
}

std::pair<int, int> QuoridorWidget::cellAt(const QPointF& pos) const {
    for (int r = 0; r < QuoridorBoard::SIZE; ++r) {
        for (int c = 0; c < QuoridorBoard::SIZE; ++c) {
            if (cellRect(r, c).contains(pos)) {
                return {r, c};
            }
        }
    }
    return {-1, -1};
}

std::optional<Wall> QuoridorWidget::wallAt(const QPointF& pos) const {
    QRectF bRect = boardRect();
    qreal s = bRect.width() / 10.6;
    qreal g = s * 0.2;

    // Search nearest wall intersection (r: 0..7, c: 0..7)
    qreal bestDist = 1e9;
    std::optional<Wall> bestWall;

    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            qreal interX = bRect.left() + (c + 1) * s + c * g + g / 2.0;
            qreal interY = bRect.top() + (r + 1) * s + r * g + g / 2.0;
            qreal dist = std::hypot(pos.x() - interX, pos.y() - interY);

            if (dist < (s + g) * 0.75 && dist < bestDist) {
                bestDist = dist;
                bestWall = Wall{r, c, m_wallOrientation};
            }
        }
    }

    return bestWall;
}

void QuoridorWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF bRect = boardRect();
    qreal s = bRect.width() / 10.6;
    qreal g = s * 0.2;

    // 1. Board base (deep dark walnut base)
    painter.save();
    painter.setPen(QPen(QColor(42, 36, 30), 4));
    painter.setBrush(QColor(30, 24, 20));
    painter.drawRoundedRect(bRect.adjusted(-6, -6, 6, 6), 14, 14);
    painter.restore();

    // 2. Cells (Smooth wooden square tiles)
    for (int r = 0; r < QuoridorBoard::SIZE; ++r) {
        for (int c = 0; c < QuoridorBoard::SIZE; ++c) {
            QRectF cRect = cellRect(r, c);
            painter.save();
            painter.setPen(Qt::NoPen);

            // Subtle gradient for wooden tile
            QLinearGradient cellGrad(cRect.topLeft(), cRect.bottomRight());
            cellGrad.setColorAt(0.0, QColor(56, 46, 38));
            cellGrad.setColorAt(1.0, QColor(44, 36, 30));
            painter.setBrush(cellGrad);
            painter.drawRoundedRect(cRect, 4, 4);

            // Goal line subtle indicator
            if (r == 0) { // P1 Goal
                painter.setBrush(QColor(224, 90, 71, 20));
                painter.drawRoundedRect(cRect, 4, 4);
            } else if (r == 8) { // P2 Goal
                painter.setBrush(QColor(78, 149, 255, 20));
                painter.drawRoundedRect(cRect, 4, 4);
            }
            painter.restore();
        }
    }

    // 3. Highlight legal pawn moves if in MovePawn mode and interactive
    if (m_interactive && m_uiMode == UIMode::MovePawn) {
        auto legalMoves = m_board.getLegalPawnMoves(m_activePlayer);
        painter.save();
        for (const auto& pos : legalMoves) {
            QRectF cRect = cellRect(pos.row, pos.col);
            if (pos.row == m_hoveredRow && pos.col == m_hoveredCol) {
                painter.setPen(QPen(QColor(229, 169, 60), 2.5));
                painter.setBrush(QColor(229, 169, 60, 90));
            } else {
                painter.setPen(QPen(QColor(229, 169, 60, 150), 1.5, Qt::DashLine));
                painter.setBrush(QColor(229, 169, 60, 30));
            }
            painter.drawRoundedRect(cRect, 4, 4);
        }
        painter.restore();
    }

    // 4. Placed Walls
    auto drawWall = [&](const Wall& w, const QColor& color, bool isPreview) {
        painter.save();
        QRectF wRect;
        if (w.orientation == WallOrientation::Horizontal) {
            qreal wx = bRect.left() + w.col * (s + g);
            qreal wy = bRect.top() + (w.row + 1) * s + w.row * g;
            wRect = QRectF(wx, wy, 2 * s + g, g);
        } else {
            qreal wx = bRect.left() + (w.col + 1) * s + w.col * g;
            qreal wy = bRect.top() + w.row * (s + g);
            wRect = QRectF(wx, wy, g, 2 * s + g);
        }

        if (isPreview) {
            painter.setPen(QPen(color.darker(120), 1.5));
            painter.setBrush(color);
        } else {
            QLinearGradient wallGrad(wRect.topLeft(), wRect.bottomRight());
            wallGrad.setColorAt(0.0, QColor(220, 160, 80));
            wallGrad.setColorAt(1.0, QColor(160, 100, 40));
            painter.setPen(QPen(QColor(110, 65, 20), 1.5));
            painter.setBrush(wallGrad);
        }
        painter.drawRoundedRect(wRect, 3, 3);
        painter.restore();
    };

    for (const auto& w : m_board.walls()) {
        drawWall(w, QColor(200, 140, 60), false);
    }

    // 5. Hover Wall Preview (if in PlaceWall mode and interactive)
    if (m_interactive && m_uiMode == UIMode::PlaceWall && m_hoveredWall) {
        bool canPlace = m_board.canPlaceWall(*m_hoveredWall);
        QColor prevCol = canPlace ? QColor(70, 210, 120, 180) : QColor(240, 70, 70, 180);
        drawWall(*m_hoveredWall, prevCol, true);
    }

    // 6. Pawns
    auto drawPawn = [&](Position pos, PlayerId player) {
        QRectF cRect = cellRect(pos.row, pos.col);
        QPointF center = cRect.center();
        qreal r = s * 0.38;

        painter.save();
        // Drop shadow
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 100));
        painter.drawEllipse(center + QPointF(2, 3), r, r);

        // 3D sphere gradient
        QRadialGradient pawnGrad(center - QPointF(r * 0.3, r * 0.3), r * 1.3);
        if (player == PlayerId::Player1) {
            pawnGrad.setColorAt(0.0, QColor(255, 120, 100));
            pawnGrad.setColorAt(0.7, QColor(210, 45, 35));
            pawnGrad.setColorAt(1.0, QColor(130, 15, 15));
            painter.setPen(QPen(QColor(255, 180, 160), 1.5));
        } else {
            pawnGrad.setColorAt(0.0, QColor(120, 190, 255));
            pawnGrad.setColorAt(0.7, QColor(45, 120, 230));
            pawnGrad.setColorAt(1.0, QColor(15, 60, 150));
            painter.setPen(QPen(QColor(180, 220, 255), 1.5));
        }
        painter.setBrush(pawnGrad);
        painter.drawEllipse(center, r, r);

        // Inner crown highlight
        painter.setPen(QPen(QColor(255, 255, 255, 120), 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, r * 0.45, r * 0.45);

        painter.restore();
    };

    drawPawn(m_board.playerPos(PlayerId::Player1), PlayerId::Player1);
    drawPawn(m_board.playerPos(PlayerId::Player2), PlayerId::Player2);
}

void QuoridorWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_uiMode == UIMode::MovePawn) {
        auto [r, c] = cellAt(event->position());
        if (r != m_hoveredRow || c != m_hoveredCol) {
            m_hoveredRow = r;
            m_hoveredCol = c;
            m_hoveredWall = std::nullopt;
            update();
        }
    } else {
        auto wOpt = wallAt(event->position());
        if (wOpt != m_hoveredWall) {
            m_hoveredWall = wOpt;
            m_hoveredRow = -1;
            m_hoveredCol = -1;
            update();
        }
    }
}

void QuoridorWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_interactive) return;

    if (event->button() == Qt::RightButton) {
        // Right click toggles wall orientation
        toggleWallOrientation();
        m_hoveredWall = wallAt(event->position());
        update();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (m_uiMode == UIMode::MovePawn) {
            auto [r, c] = cellAt(event->position());
            if (r >= 0 && c >= 0 && m_board.isValidPawnMove(m_activePlayer, Position{r, c})) {
                emit pawnMoveRequested(Position{r, c});
            }
        } else {
            auto wOpt = wallAt(event->position());
            if (wOpt && m_board.canPlaceWall(*wOpt)) {
                emit wallPlacementRequested(*wOpt);
            }
        }
    }
}

void QuoridorWidget::leaveEvent(QEvent * /*event*/) {
    m_hoveredRow = -1;
    m_hoveredCol = -1;
    m_hoveredWall = std::nullopt;
    update();
}

void QuoridorWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Space) {
        toggleWallOrientation();
        update();
    } else if (event->key() == Qt::Key_M) {
        setUIMode(UIMode::MovePawn);
        emit modeChanged(UIMode::MovePawn);
    } else if (event->key() == Qt::Key_W) {
        setUIMode(UIMode::PlaceWall);
        emit modeChanged(UIMode::PlaceWall);
    } else {
        QWidget::keyPressEvent(event);
    }
}

} // namespace Strategix::Quoridor
