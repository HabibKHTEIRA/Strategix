#include "MorrisWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <algorithm>
#include <cmath>

namespace Strategix::Morris {

MorrisWidget::MorrisWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}

void MorrisWidget::setBoard(const MorrisBoard& board) {
    m_board = board;
    m_selectedFrom = -1;
    update();
}

void MorrisWidget::setActivePlayer(Piece player) {
    m_activePlayer = player;
    m_selectedFrom = -1;
    update();
}

void MorrisWidget::setGamePhase(GamePhase phase) {
    m_phase = phase;
    m_selectedFrom = -1;
    update();
}

void MorrisWidget::setAwaitingCapture(bool awaitingCapture) {
    m_awaitingCapture = awaitingCapture;
    m_selectedFrom = -1;
    update();
}

void MorrisWidget::setInteractive(bool interactive) {
    m_interactive = interactive;
    update();
}

void MorrisWidget::setLastMove(const std::optional<Move>& move) {
    m_lastMove = move;
    update();
}

QPointF MorrisWidget::pointCoords(int pt) const {
    qreal side = std::min(width(), height()) - 60.0;
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;

    int ring = pt / 8;
    int pos = pt % 8;

    // Radii of the 3 squares
    static const qreal scales[3] = {1.0, 0.66, 0.33};
    qreal h = (side / 2.0) * scales[ring];

    switch (pos) {
        case 0: return QPointF(cx - h, cy - h); // Top-Left
        case 1: return QPointF(cx,     cy - h); // Top-Mid
        case 2: return QPointF(cx + h, cy - h); // Top-Right
        case 3: return QPointF(cx + h, cy);     // Mid-Right
        case 4: return QPointF(cx + h, cy + h); // Bot-Right
        case 5: return QPointF(cx,     cy + h); // Bot-Mid
        case 6: return QPointF(cx - h, cy + h); // Bot-Left
        case 7: return QPointF(cx - h, cy);     // Mid-Left
        default: return QPointF(cx, cy);
    }
}

int MorrisWidget::pointAt(const QPointF& pos) const {
    for (int i = 0; i < MorrisBoard::NUM_POINTS; ++i) {
        QPointF pt = pointCoords(i);
        if (std::hypot(pos.x() - pt.x(), pos.y() - pt.y()) <= 24.0) {
            return i;
        }
    }
    return -1;
}

void MorrisWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    qreal side = std::min(width(), height()) - 60.0;
    qreal cx = width() / 2.0;
    qreal cy = height() / 2.0;
    QRectF bRect(cx - side / 2.0, cy - side / 2.0, side, side);

    // 1. Board base (elegant dark slate background)
    painter.save();
    painter.setPen(QPen(QColor(54, 50, 70), 4));
    painter.setBrush(QColor(26, 24, 34));
    painter.drawRoundedRect(bRect.adjusted(-20, -20, 20, 20), 16, 16);
    painter.restore();

    // 2. Lines of the board
    painter.save();
    QPen linePen(QColor(165, 160, 180), 3.5);
    linePen.setCapStyle(Qt::RoundCap);
    linePen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(linePen);
    painter.setBrush(Qt::NoBrush);

    // 3 Concentric squares
    for (int ring = 0; ring < 3; ++ring) {
        QPointF p0 = pointCoords(ring * 8 + 0);
        QPointF p2 = pointCoords(ring * 8 + 2);
        QPointF p4 = pointCoords(ring * 8 + 4);
        QPointF p6 = pointCoords(ring * 8 + 6);

        QPolygonF poly;
        poly << p0 << p2 << p4 << p6;
        painter.drawPolygon(poly);
    }

    // 4 Cross lines connecting midpoints
    painter.drawLine(pointCoords(1), pointCoords(17)); // Top
    painter.drawLine(pointCoords(3), pointCoords(19)); // Right
    painter.drawLine(pointCoords(5), pointCoords(21)); // Bottom
    painter.drawLine(pointCoords(7), pointCoords(23)); // Left
    painter.restore();

    // 3. Highlight points: legal moves, selected piece, or capturable pieces
    std::vector<int> highlightPoints;
    QColor highlightColor(229, 169, 60);

    if (m_interactive) {
        if (m_awaitingCapture) {
            highlightPoints = m_board.getCapturablePieces(opponent(m_activePlayer));
            highlightColor = QColor(230, 60, 60);
        } else if (m_phase == GamePhase::Placing) {
            highlightPoints = m_board.getLegalPlacements();
            highlightColor = QColor(229, 169, 60);
        } else {
            // Moving / Flying
            if (m_selectedFrom != -1) {
                bool isFlying = (m_phase == GamePhase::Flying);
                highlightPoints = m_board.getLegalMovesFrom(m_selectedFrom, isFlying);
                highlightColor = QColor(80, 210, 120);
            }
        }
    }

    // 4. Draw Intersections (empty sockets)
    for (int i = 0; i < MorrisBoard::NUM_POINTS; ++i) {
        QPointF pt = pointCoords(i);
        Piece p = m_board.pieceAt(i);

        painter.save();
        bool isHighlighted = std::find(highlightPoints.begin(), highlightPoints.end(), i) != highlightPoints.end();
        bool isHovered = (i == m_hoveredPt);

        if (p == Piece::Empty) {
            painter.setPen(QPen(isHighlighted ? highlightColor : QColor(100, 95, 120), isHighlighted ? 2.5 : 1.5));
            painter.setBrush(isHovered && isHighlighted ? highlightColor : QColor(36, 32, 46));
            painter.drawEllipse(pt, isHighlighted ? 9.0 : 6.0, isHighlighted ? 9.0 : 6.0);
        } else {
            // Draw piece
            qreal pieceRadius = 18.0;

            // Shadow
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 100));
            painter.drawEllipse(pt + QPointF(2, 3), pieceRadius, pieceRadius);

            // Piece gradient
            QRadialGradient pieceGrad(pt - QPointF(pieceRadius * 0.35, pieceRadius * 0.35), pieceRadius * 1.3);
            if (p == Piece::White) {
                pieceGrad.setColorAt(0.0, QColor(255, 255, 255));
                pieceGrad.setColorAt(0.7, QColor(230, 230, 238));
                pieceGrad.setColorAt(1.0, QColor(180, 180, 195));
                painter.setPen(QPen(QColor(150, 150, 165), 1.5));
            } else {
                pieceGrad.setColorAt(0.0, QColor(65, 65, 75));
                pieceGrad.setColorAt(0.7, QColor(30, 30, 36));
                pieceGrad.setColorAt(1.0, QColor(15, 15, 20));
                painter.setPen(QPen(QColor(20, 20, 26), 1.5));
            }

            if (i == m_selectedFrom) {
                painter.setPen(QPen(QColor(80, 210, 120), 3.5));
            } else if (m_awaitingCapture && isHighlighted) {
                painter.setPen(QPen(QColor(240, 60, 60), 3.0));
            } else if (m_lastMove && (m_lastMove->to == i || m_lastMove->from == i)) {
                painter.setPen(QPen(QColor(229, 169, 60), 2.0));
            }

            painter.setBrush(pieceGrad);
            painter.drawEllipse(pt, pieceRadius, pieceRadius);

            // Ring ornament
            painter.setPen(QPen(p == Piece::White ? QColor(255, 255, 255, 80) : QColor(255, 255, 255, 30), 1.0));
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(pt, pieceRadius * 0.55, pieceRadius * 0.55);
        }
        painter.restore();
    }
}

void MorrisWidget::mouseMoveEvent(QMouseEvent *event) {
    int pt = pointAt(event->position());
    if (pt != m_hoveredPt) {
        m_hoveredPt = pt;
        update();
    }
}

void MorrisWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_interactive) return;
    if (event->button() != Qt::LeftButton) return;

    int pt = pointAt(event->position());
    if (pt == -1) return;

    if (m_awaitingCapture) {
        auto capturable = m_board.getCapturablePieces(opponent(m_activePlayer));
        if (std::find(capturable.begin(), capturable.end(), pt) != capturable.end()) {
            emit captureRequested(pt);
        }
        return;
    }

    if (m_phase == GamePhase::Placing) {
        if (m_board.pieceAt(pt) == Piece::Empty) {
            emit pointClicked(pt);
        }
    } else {
        // Moving or Flying
        Piece atPt = m_board.pieceAt(pt);
        if (atPt == m_activePlayer) {
            // Select this piece
            m_selectedFrom = pt;
            update();
        } else if (m_selectedFrom != -1 && atPt == Piece::Empty) {
            // Attempt to move selected piece here
            bool isFlying = (m_phase == GamePhase::Flying);
            auto legal = m_board.getLegalMovesFrom(m_selectedFrom, isFlying);
            if (std::find(legal.begin(), legal.end(), pt) != legal.end()) {
                int from = m_selectedFrom;
                m_selectedFrom = -1;
                emit moveRequested(from, pt);
            }
        }
    }
}

void MorrisWidget::leaveEvent(QEvent * /*event*/) {
    m_hoveredPt = -1;
    update();
}

} // namespace Strategix::Morris
