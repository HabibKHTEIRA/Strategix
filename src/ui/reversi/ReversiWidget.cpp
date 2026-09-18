#include "ReversiWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <algorithm>

namespace Strategix::Reversi {

ReversiWidget::ReversiWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}

void ReversiWidget::setBoard(const ReversiBoard& board) {
    m_board = board;
    update();
}

void ReversiWidget::setActivePlayer(Piece player) {
    m_activePlayer = player;
    update();
}

void ReversiWidget::setInteractive(bool interactive) {
    m_interactive = interactive;
    update();
}

void ReversiWidget::setLastMove(const std::optional<Move>& move) {
    m_lastMove = move;
    update();
}

QRectF ReversiWidget::boardRect() const {
    qreal side = std::min(width(), height()) - 24;
    qreal x = (width() - side) / 2.0;
    qreal y = (height() - side) / 2.0;
    return QRectF(x, y, side, side);
}

QRectF ReversiWidget::cellRect(int r, int c) const {
    QRectF bRect = boardRect();
    qreal cellSide = bRect.width() / ReversiBoard::SIZE;
    return QRectF(bRect.left() + c * cellSide, bRect.top() + r * cellSide, cellSide, cellSide);
}

std::pair<int, int> ReversiWidget::cellAt(const QPointF& pos) const {
    QRectF bRect = boardRect();
    if (!bRect.contains(pos)) return {-1, -1};
    qreal cellSide = bRect.width() / ReversiBoard::SIZE;
    int col = static_cast<int>((pos.x() - bRect.left()) / cellSide);
    int row = static_cast<int>((pos.y() - bRect.top()) / cellSide);
    if (row >= 0 && row < ReversiBoard::SIZE && col >= 0 && col < ReversiBoard::SIZE) {
        return {row, col};
    }
    return {-1, -1};
}

void ReversiWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF bRect = boardRect();
    qreal cellSide = bRect.width() / ReversiBoard::SIZE;
    qreal discRadius = cellSide * 0.40;

    // 1. Board Outer Border and Felt Background
    painter.save();
    painter.setPen(QPen(QColor(18, 55, 32), 6));
    painter.setBrush(QColor(24, 75, 45)); // Classic dark green felt
    painter.drawRoundedRect(bRect.adjusted(-3, -3, 3, 3), 8, 8);

    // Grid lines
    painter.setPen(QPen(QColor(14, 45, 26), 1.5));
    for (int i = 0; i <= ReversiBoard::SIZE; ++i) {
        qreal pos = bRect.left() + i * cellSide;
        painter.drawLine(QPointF(pos, bRect.top()), QPointF(pos, bRect.bottom()));
        qreal posY = bRect.top() + i * cellSide;
        painter.drawLine(QPointF(bRect.left(), posY), QPointF(bRect.right(), posY));
    }

    // Standard 4 board guide dots at (2,2), (2,6), (6,2), (6,6)
    static const int dotCoords[4][2] = {{2, 2}, {2, 6}, {6, 2}, {6, 6}};
    painter.setBrush(QColor(14, 45, 26));
    painter.setPen(Qt::NoPen);
    for (const auto& d : dotCoords) {
        qreal x = bRect.left() + d[1] * cellSide;
        qreal y = bRect.top() + d[0] * cellSide;
        painter.drawEllipse(QPointF(x, y), 3.5, 3.5);
    }
    painter.restore();

    // 2. Legal Moves Highlights
    auto legalMoves = m_board.getLegalMoves(m_activePlayer);

    if (m_interactive) {
        painter.save();
        for (const auto& mv : legalMoves) {
            QRectF cRect = cellRect(mv.row, mv.col);
            QPointF center = cRect.center();

            if (mv.row == m_hoveredRow && mv.col == m_hoveredCol) {
                // Ghost piece preview
                painter.setPen(Qt::NoPen);
                QColor ghostColor = (m_activePlayer == Piece::Black) ? QColor(20, 20, 20, 150) : QColor(240, 240, 240, 150);
                painter.setBrush(ghostColor);
                painter.drawEllipse(center, discRadius * 0.9, discRadius * 0.9);
            } else {
                // Small dot indicating valid move
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(229, 169, 60, 160)); // Gold dot
                painter.drawEllipse(center, 5.0, 5.0);
            }
        }
        painter.restore();
    }

    // 3. Draw Discs
    for (int r = 0; r < ReversiBoard::SIZE; ++r) {
        for (int c = 0; c < ReversiBoard::SIZE; ++c) {
            Piece p = m_board.pieceAt(r, c);
            if (p == Piece::Empty) continue;

            QRectF cRect = cellRect(r, c);
            QPointF center = cRect.center();

            painter.save();

            // Shadow
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 90));
            painter.drawEllipse(center + QPointF(2, 3), discRadius, discRadius);

            // 3D Disc Gradient
            QRadialGradient discGrad(center - QPointF(discRadius * 0.35, discRadius * 0.35), discRadius * 1.3);
            if (p == Piece::Black) {
                discGrad.setColorAt(0.0, QColor(60, 60, 68));
                discGrad.setColorAt(0.6, QColor(25, 25, 30));
                discGrad.setColorAt(1.0, QColor(10, 10, 14));
                painter.setPen(QPen(QColor(15, 15, 20), 1.5));
            } else {
                discGrad.setColorAt(0.0, QColor(255, 255, 255));
                discGrad.setColorAt(0.7, QColor(235, 235, 240));
                discGrad.setColorAt(1.0, QColor(190, 190, 200));
                painter.setPen(QPen(QColor(170, 170, 180), 1.5));
            }

            painter.setBrush(discGrad);
            painter.drawEllipse(center, discRadius, discRadius);

            // Subtle inner ring
            painter.setPen(QPen(p == Piece::Black ? QColor(255, 255, 255, 30) : QColor(0, 0, 0, 25), 1.0));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(center, discRadius * 0.75, discRadius * 0.75);

            // Last move marker (ring or golden dot)
            if (m_lastMove && m_lastMove->row == r && m_lastMove->col == c) {
                painter.setPen(QPen(QColor(229, 169, 60), 2.5));
                painter.setBrush(Qt::NoBrush);
                painter.drawEllipse(center, discRadius * 0.3, discRadius * 0.3);
            }

            painter.restore();
        }
    }
}

void ReversiWidget::mouseMoveEvent(QMouseEvent *event) {
    auto [r, c] = cellAt(event->position());
    if (r != m_hoveredRow || c != m_hoveredCol) {
        m_hoveredRow = r;
        m_hoveredCol = c;
        update();
    }
}

void ReversiWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_interactive) return;
    if (event->button() == Qt::LeftButton) {
        auto [r, c] = cellAt(event->position());
        if (r >= 0 && c >= 0 && m_board.isValidMove(r, c, m_activePlayer)) {
            emit cellClicked(r, c);
        }
    }
}

void ReversiWidget::leaveEvent(QEvent * /*event*/) {
    m_hoveredRow = -1;
    m_hoveredCol = -1;
    update();
}

} // namespace Strategix::Reversi
