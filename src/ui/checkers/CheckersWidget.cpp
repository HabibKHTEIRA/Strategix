#include "CheckersWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QPainterPath>

namespace Strategix::Checkers {

CheckersWidget::CheckersWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void CheckersWidget::setBoard(const CheckersBoard& board) {
    m_board = board;
    m_selectedPos = std::nullopt;
    m_availableMovesFromSelected.clear();
    update();
}

QRectF CheckersWidget::cellRect(int r, int c) const {
    qreal boardSide = qMin(width(), height()) - 40.0;
    if (boardSide < 240.0) boardSide = 240.0;

    qreal startX = (width() - boardSide) / 2.0;
    qreal startY = (height() - boardSide) / 2.0;

    qreal cellSize = boardSide / 8.0;
    return QRectF(startX + c * cellSize, startY + r * cellSize, cellSize, cellSize);
}

std::pair<int, int> CheckersWidget::cellAt(const QPointF& pos) const {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (cellRect(r, c).contains(pos)) {
                return {r, c};
            }
        }
    }
    return {-1, -1};
}

void CheckersWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF evPos = event->position();
#else
    QPointF evPos = event->pos();
#endif
    auto [r, c] = cellAt(evPos);
    if (r != m_hoveredRow || c != m_hoveredCol) {
        m_hoveredRow = r;
        m_hoveredCol = c;
        if (m_interactive && r >= 0 && c >= 0) {
            Piece p = m_board.pieceAt(r, c);
            if (p.color == m_activePlayer) {
                setCursor(Qt::PointingHandCursor);
            } else {
                bool isDest = false;
                for (const auto& m : m_availableMovesFromSelected) {
                    if (m.to.row == r && m.to.col == c) {
                        isDest = true;
                        break;
                    }
                }
                if (isDest) setCursor(Qt::PointingHandCursor);
                else unsetCursor();
            }
        } else {
            unsetCursor();
        }
        update();
    }
}

void CheckersWidget::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    m_hoveredRow = -1;
    m_hoveredCol = -1;
    unsetCursor();
    update();
}

void CheckersWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (!m_interactive || event->button() != Qt::LeftButton) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF evPos = event->position();
#else
    QPointF evPos = event->pos();
#endif
    auto [r, c] = cellAt(evPos);
    if (r < 0 || c < 0) {
        m_selectedPos = std::nullopt;
        m_availableMovesFromSelected.clear();
        update();
        return;
    }

    Position clickedPos{r, c};

    // If destination of a selected move was clicked:
    for (const auto& move : m_availableMovesFromSelected) {
        if (move.to == clickedPos) {
            emit moveRequested(move);
            m_selectedPos = std::nullopt;
            m_availableMovesFromSelected.clear();
            update();
            return;
        }
    }

    // If a friendly piece was clicked:
    Piece clickedPiece = m_board.pieceAt(clickedPos);
    if (clickedPiece.color == m_activePlayer) {
        m_selectedPos = clickedPos;
        m_availableMovesFromSelected = m_board.getLegalMovesFrom(clickedPos, m_activePlayer);
        update();
    } else {
        m_selectedPos = std::nullopt;
        m_availableMovesFromSelected.clear();
        update();
    }
}

void CheckersWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    qreal boardSide = qMin(width(), height()) - 40.0;
    if (boardSide < 240.0) boardSide = 240.0;

    qreal startX = (width() - boardSide) / 2.0;
    qreal startY = (height() - boardSide) / 2.0;
    QRectF boardRect(startX, startY, boardSide, boardSide);

    // Board Drop Shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(10, 10, 16, 120));
    painter.drawRoundedRect(boardRect.translated(0, 6), 16.0, 16.0);

    // Board Rim
    painter.setPen(QPen(QColor(56, 44, 38), 6.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(boardRect.adjusted(-3, -3, 3, 3), 12.0, 12.0);

    // Draw Squares
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            QRectF cr = cellRect(r, c);
            bool isDark = (r + c) % 2 != 0;

            if (isDark) {
                painter.fillRect(cr, QColor(72, 54, 42)); // Deep wood dark
            } else {
                painter.fillRect(cr, QColor(228, 214, 192)); // Light cream
            }

            // Highlight last move
            if (m_lastMove) {
                if ((m_lastMove->from.row == r && m_lastMove->from.col == c) ||
                    (m_lastMove->to.row == r && m_lastMove->to.col == c)) {
                    painter.fillRect(cr, QColor(229, 169, 60, 80));
                }
            }

            // Draw Piece
            Piece p = m_board.pieceAt(r, c);
            if (!p.isEmpty()) {
                bool isSel = m_selectedPos && (*m_selectedPos == Position{r, c});
                drawPiece(painter, cr, p, isSel);
            }
        }
    }

    // Draw Available Moves indicators
    for (const auto& m : m_availableMovesFromSelected) {
        QRectF cr = cellRect(m.to.row, m.to.col);
        QPointF center = cr.center();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(46, 204, 113, 190)); // Vibrant emerald dot
        painter.drawEllipse(center, 10.0, 10.0);
    }
}

void CheckersWidget::drawPiece(QPainter& painter, const QRectF& rect, Piece piece, bool isSelected) {
    qreal side = qMin(rect.width(), rect.height());
    qreal radius = (side * 0.76) / 2.0;
    QPointF center = rect.center();

    // Drop shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(15, 12, 20, 100));
    painter.drawEllipse(center + QPointF(1.5, 3.5), radius, radius);

    // Piece Body Gradient
    QRadialGradient grad(center - QPointF(radius * 0.3, radius * 0.3), radius * 1.3);
    QColor strokeColor;

    if (piece.isWhite()) {
        grad.setColorAt(0.0, QColor(255, 252, 245));
        grad.setColorAt(0.7, QColor(230, 222, 205));
        grad.setColorAt(1.0, QColor(190, 180, 160));
        strokeColor = QColor(140, 130, 110);
    } else {
        grad.setColorAt(0.0, QColor(68, 62, 70));
        grad.setColorAt(0.7, QColor(42, 38, 44));
        grad.setColorAt(1.0, QColor(22, 18, 24));
        strokeColor = QColor(90, 80, 95);
    }

    QPen pen(strokeColor, 2.0);
    if (isSelected) {
        pen.setColor(QColor(235, 168, 52)); // Gold selected border
        pen.setWidthF(3.5);
    }

    painter.setPen(pen);
    painter.setBrush(grad);
    painter.drawEllipse(center, radius, radius);

    // Inner concentric ridges (checkers piece detail)
    painter.setPen(QPen(strokeColor, 1.2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(center, radius * 0.72, radius * 0.72);
    painter.drawEllipse(center, radius * 0.44, radius * 0.44);

    // King Royal Crown
    if (piece.isKing()) {
        drawCrown(painter, QRectF(center.x() - radius * 0.45, center.y() - radius * 0.45, radius * 0.9, radius * 0.9));
    }
}

void CheckersWidget::drawCrown(QPainter& painter, const QRectF& rect) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    qreal w = rect.width();
    qreal h = rect.height();
    qreal x = rect.x();
    qreal y = rect.y();

    // Crown outline with 3 points
    path.moveTo(x + w * 0.1, y + h * 0.85);
    path.lineTo(x + w * 0.05, y + h * 0.25); // Left peak
    path.lineTo(x + w * 0.32, y + h * 0.52);
    path.lineTo(x + w * 0.50, y + h * 0.12); // Center peak
    path.lineTo(x + w * 0.68, y + h * 0.52);
    path.lineTo(x + w * 0.95, y + h * 0.25); // Right peak
    path.lineTo(x + w * 0.90, y + h * 0.85);
    path.closeSubpath();

    QLinearGradient crownGrad(rect.topLeft(), rect.bottomRight());
    crownGrad.setColorAt(0.0, QColor(255, 225, 75));
    crownGrad.setColorAt(1.0, QColor(220, 150, 20));

    painter.setPen(QPen(QColor(160, 100, 10), 1.5));
    painter.setBrush(crownGrad);
    painter.drawPath(path);

    painter.restore();
}

} // namespace Strategix::Checkers
