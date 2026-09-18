#include "ConnectFourWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <algorithm>

namespace Strategix::ConnectFour {

ConnectFourWidget::ConnectFourWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}

void ConnectFourWidget::setBoard(const ConnectFourBoard& board) {
    m_board = board;
    update();
}

void ConnectFourWidget::setActivePlayer(Player player) {
    m_activePlayer = player;
    update();
}

void ConnectFourWidget::setInteractive(bool interactive) {
    m_interactive = interactive;
    update();
}

void ConnectFourWidget::setWinningLine(const std::vector<std::pair<int, int>>& line) {
    m_winningLine = line;
    update();
}

void ConnectFourWidget::setLastMove(int r, int c) {
    m_lastMove = {r, c};
    update();
}

QRectF ConnectFourWidget::boardRect() const {
    qreal availableW = width() - 32;
    qreal availableH = height() - 48; // room for top ghost disc

    qreal aspect = static_cast<qreal>(ConnectFourBoard::COLS) / ConnectFourBoard::ROWS;
    qreal w = availableW;
    qreal h = w / aspect;

    if (h > availableH) {
        h = availableH;
        w = h * aspect;
    }

    qreal x = (width() - w) / 2.0;
    qreal y = ((height() + 32) - h) / 2.0;
    return QRectF(x, y, w, h);
}

QRectF ConnectFourWidget::cellRect(int r, int c) const {
    QRectF bRect = boardRect();
    qreal cellW = bRect.width() / ConnectFourBoard::COLS;
    qreal cellH = bRect.height() / ConnectFourBoard::ROWS;

    // Connect Four row 0 is bottom row
    qreal cellY = bRect.bottom() - (r + 1) * cellH;
    qreal cellX = bRect.left() + c * cellW;

    return QRectF(cellX, cellY, cellW, cellH);
}

int ConnectFourWidget::columnAt(const QPointF& pos) const {
    QRectF bRect = boardRect();
    if (pos.x() < bRect.left() || pos.x() > bRect.right()) return -1;
    qreal cellW = bRect.width() / ConnectFourBoard::COLS;
    int col = static_cast<int>((pos.x() - bRect.left()) / cellW);
    if (col >= 0 && col < ConnectFourBoard::COLS) return col;
    return -1;
}

void ConnectFourWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF bRect = boardRect();
    qreal cellW = bRect.width() / ConnectFourBoard::COLS;
    qreal cellH = bRect.height() / ConnectFourBoard::ROWS;
    qreal discRadius = std::min(cellW, cellH) * 0.40;

    // 1. Draw ghost disc on top of hovered column if interactive
    if (m_interactive && m_hoveredCol >= 0 && !m_board.isColumnFull(m_hoveredCol)) {
        qreal topY = bRect.top() - discRadius - 8;
        qreal topX = bRect.left() + (m_hoveredCol + 0.5) * cellW;

        painter.save();
        painter.setPen(Qt::NoPen);
        QColor ghostCol = (m_activePlayer == Player::Red) ? QColor(224, 90, 71, 180) : QColor(245, 185, 45, 180);
        painter.setBrush(ghostCol);
        painter.drawEllipse(QPointF(topX, topY), discRadius * 0.85, discRadius * 0.85);

        // Subtle column highlight line
        QColor colGlow(255, 255, 255, 20);
        painter.setBrush(colGlow);
        painter.drawRoundedRect(QRectF(bRect.left() + m_hoveredCol * cellW + 2, bRect.top(), cellW - 4, bRect.height()), 8, 8);
        painter.restore();
    }

    // 2. Outer Board Frame (Classic dark blue / indigo sleek cabinet)
    painter.save();
    QPainterPath framePath;
    framePath.addRoundedRect(bRect.adjusted(-8, -8, 8, 8), 16, 16);

    QLinearGradient boardGrad(bRect.topLeft(), bRect.bottomRight());
    boardGrad.setColorAt(0.0, QColor(28, 42, 85));
    boardGrad.setColorAt(1.0, QColor(16, 25, 52));

    painter.setPen(QPen(QColor(60, 95, 185), 3));
    painter.setBrush(boardGrad);
    painter.drawPath(framePath);
    painter.restore();

    // 3. Draw Discs and Slots
    for (int r = 0; r < ConnectFourBoard::ROWS; ++r) {
        for (int c = 0; c < ConnectFourBoard::COLS; ++c) {
            QRectF cRect = cellRect(r, c);
            QPointF center = cRect.center();
            Player p = m_board.cell(r, c);

            // Empty slot (dark circular cutout with inner shadow)
            painter.save();
            painter.setPen(QPen(QColor(12, 18, 38), 2));
            QRadialGradient slotHole(center, discRadius);
            slotHole.setColorAt(0.0, QColor(14, 12, 20));
            slotHole.setColorAt(0.85, QColor(18, 16, 26));
            slotHole.setColorAt(1.0, QColor(32, 28, 44));
            painter.setBrush(slotHole);
            painter.drawEllipse(center, discRadius, discRadius);
            painter.restore();

            if (p != Player::None) {
                painter.save();
                bool isWinning = false;
                for (const auto& w : m_winningLine) {
                    if (w.first == r && w.second == c) {
                        isWinning = true;
                        break;
                    }
                }

                // 3D Disc Gradient
                QRadialGradient discGrad(center - QPointF(discRadius * 0.3, discRadius * 0.3), discRadius * 1.2);
                if (p == Player::Red) {
                    discGrad.setColorAt(0.0, QColor(255, 120, 100));
                    discGrad.setColorAt(0.7, QColor(220, 50, 40));
                    discGrad.setColorAt(1.0, QColor(140, 20, 20));
                } else {
                    discGrad.setColorAt(0.0, QColor(255, 235, 120));
                    discGrad.setColorAt(0.7, QColor(240, 180, 20));
                    discGrad.setColorAt(1.0, QColor(170, 110, 10));
                }

                if (isWinning) {
                    painter.setPen(QPen(QColor(255, 255, 255), 4));
                } else if (m_lastMove.first == r && m_lastMove.second == c) {
                    painter.setPen(QPen(QColor(255, 255, 255, 180), 2));
                } else {
                    painter.setPen(QPen(QColor(0, 0, 0, 80), 1.5));
                }

                painter.setBrush(discGrad);
                painter.drawEllipse(center, discRadius * 0.95, discRadius * 0.95);

                // Glossy inner rim
                painter.setPen(QPen(QColor(255, 255, 255, 60), 1.5));
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(center, discRadius * 0.6, discRadius * 0.6);

                if (isWinning) {
                    painter.setPen(QPen(QColor(255, 255, 255), 2));
                    painter.setBrush(QColor(255, 255, 255, 220));
                    painter.drawEllipse(center, discRadius * 0.25, discRadius * 0.25);
                }

                painter.restore();
            }
        }
    }
}

void ConnectFourWidget::mouseMoveEvent(QMouseEvent *event) {
    int col = columnAt(event->position());
    if (col != m_hoveredCol) {
        m_hoveredCol = col;
        update();
    }
}

void ConnectFourWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_interactive) return;
    if (event->button() == Qt::LeftButton) {
        int col = columnAt(event->position());
        if (col >= 0 && !m_board.isColumnFull(col)) {
            emit columnSelected(col);
        }
    }
}

void ConnectFourWidget::leaveEvent(QEvent * /*event*/) {
    m_hoveredCol = -1;
    update();
}

} // namespace Strategix::ConnectFour
