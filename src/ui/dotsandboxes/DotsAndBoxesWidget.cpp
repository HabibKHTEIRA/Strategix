#include "DotsAndBoxesWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <algorithm>
#include <cmath>

namespace Strategix::DotsAndBoxes {

DotsAndBoxesWidget::DotsAndBoxesWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}

void DotsAndBoxesWidget::setBoard(const DotsAndBoxesBoard& board) {
    m_board = board;
    update();
}

void DotsAndBoxesWidget::setActivePlayer(Player player) {
    m_activePlayer = player;
    update();
}

void DotsAndBoxesWidget::setInteractive(bool interactive) {
    m_interactive = interactive;
    update();
}

void DotsAndBoxesWidget::setLastEdge(const std::optional<Edge>& edge) {
    m_lastEdge = edge;
    update();
}

QRectF DotsAndBoxesWidget::boardRect() const {
    qreal side = std::min(width(), height()) - 48.0;
    qreal x = (width() - side) / 2.0;
    qreal y = (height() - side) / 2.0;
    return QRectF(x, y, side, side);
}

QPointF DotsAndBoxesWidget::dotCoords(int r, int c) const {
    QRectF bRect = boardRect();
    qreal stepX = bRect.width() / (DotsAndBoxesBoard::DOT_COLS - 1);
    qreal stepY = bRect.height() / (DotsAndBoxesBoard::DOT_ROWS - 1);
    return QPointF(bRect.left() + c * stepX, bRect.top() + r * stepY);
}

static qreal distToSegment(const QPointF& p, const QPointF& v, const QPointF& w) {
    qreal l2 = (v.x() - w.x()) * (v.x() - w.x()) + (v.y() - w.y()) * (v.y() - w.y());
    if (l2 == 0) return std::hypot(p.x() - v.x(), p.y() - v.y());
    qreal t = ((p.x() - v.x()) * (w.x() - v.x()) + (p.y() - v.y()) * (w.y() - v.y())) / l2;
    t = std::max(0.0, std::min(1.0, t));
    QPointF proj(v.x() + t * (w.x() - v.x()), v.y() + t * (w.y() - v.y()));
    return std::hypot(p.x() - proj.x(), p.y() - proj.y());
}

std::optional<Edge> DotsAndBoxesWidget::edgeAt(const QPointF& pos) const {
    qreal bestDist = 18.0; // max click threshold
    std::optional<Edge> bestEdge;

    // Horizontal edges
    for (int r = 0; r < DotsAndBoxesBoard::DOT_ROWS; ++r) {
        for (int c = 0; c < DotsAndBoxesBoard::DOT_COLS - 1; ++c) {
            if (!m_board.isEdgeDrawn(EdgeType::Horizontal, r, c)) {
                QPointF p1 = dotCoords(r, c);
                QPointF p2 = dotCoords(r, c + 1);
                qreal d = distToSegment(pos, p1, p2);
                if (d < bestDist) {
                    bestDist = d;
                    bestEdge = Edge{EdgeType::Horizontal, r, c};
                }
            }
        }
    }

    // Vertical edges
    for (int r = 0; r < DotsAndBoxesBoard::DOT_ROWS - 1; ++r) {
        for (int c = 0; c < DotsAndBoxesBoard::DOT_COLS; ++c) {
            if (!m_board.isEdgeDrawn(EdgeType::Vertical, r, c)) {
                QPointF p1 = dotCoords(r, c);
                QPointF p2 = dotCoords(r + 1, c);
                qreal d = distToSegment(pos, p1, p2);
                if (d < bestDist) {
                    bestDist = d;
                    bestEdge = Edge{EdgeType::Vertical, r, c};
                }
            }
        }
    }

    return bestEdge;
}

void DotsAndBoxesWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRectF bRect = boardRect();

    // 1. Board background card
    painter.save();
    painter.setPen(QPen(QColor(52, 48, 66), 3));
    painter.setBrush(QColor(24, 22, 32));
    painter.drawRoundedRect(bRect.adjusted(-16, -16, 16, 16), 14, 14);
    painter.restore();

    // 2. Boxes (captured boxes fill)
    for (int r = 0; r < DotsAndBoxesBoard::BOX_ROWS; ++r) {
        for (int c = 0; c < DotsAndBoxesBoard::BOX_COLS; ++c) {
            Player owner = m_board.boxOwner(r, c);
            if (owner != Player::None) {
                QPointF tl = dotCoords(r, c);
                QPointF br = dotCoords(r + 1, c + 1);
                QRectF boxRect(tl, br);

                painter.save();
                painter.setPen(Qt::NoPen);
                if (owner == Player::Player1) {
                    painter.setBrush(QColor(78, 149, 255, 60)); // Blue tint
                } else {
                    painter.setBrush(QColor(224, 90, 71, 60));  // Red tint
                }
                painter.drawRoundedRect(boxRect.adjusted(3, 3, -3, -3), 6, 6);

                // Monogram badge in center
                painter.setPen(owner == Player::Player1 ? QColor(78, 149, 255, 180) : QColor(224, 90, 71, 180));
                QFont f = painter.font();
                f.setPointSize(16);
                f.setBold(true);
                painter.setFont(f);
                painter.drawText(boxRect, Qt::AlignCenter, (owner == Player::Player1) ? "1" : "2");
                painter.restore();
            }
        }
    }

    // 3. Faint guide lines for undrawn edges
    painter.save();
    painter.setPen(QPen(QColor(42, 38, 54), 1.5, Qt::DashLine));
    for (int r = 0; r < DotsAndBoxesBoard::DOT_ROWS; ++r) {
        for (int c = 0; c < DotsAndBoxesBoard::DOT_COLS - 1; ++c) {
            if (!m_board.isEdgeDrawn(EdgeType::Horizontal, r, c)) {
                painter.drawLine(dotCoords(r, c), dotCoords(r, c + 1));
            }
        }
    }
    for (int r = 0; r < DotsAndBoxesBoard::DOT_ROWS - 1; ++r) {
        for (int c = 0; c < DotsAndBoxesBoard::DOT_COLS; ++c) {
            if (!m_board.isEdgeDrawn(EdgeType::Vertical, r, c)) {
                painter.drawLine(dotCoords(r, c), dotCoords(r + 1, c));
            }
        }
    }
    painter.restore();

    // 4. Hover edge preview
    if (m_interactive && m_hoveredEdge) {
        painter.save();
        QColor hoverCol = (m_activePlayer == Player::Player1) ? QColor(78, 149, 255, 180) : QColor(224, 90, 71, 180);
        QPen hoverPen(hoverCol, 4.0, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(hoverPen);

        if (m_hoveredEdge->type == EdgeType::Horizontal) {
            painter.drawLine(dotCoords(m_hoveredEdge->row, m_hoveredEdge->col),
                             dotCoords(m_hoveredEdge->row, m_hoveredEdge->col + 1));
        } else {
            painter.drawLine(dotCoords(m_hoveredEdge->row, m_hoveredEdge->col),
                             dotCoords(m_hoveredEdge->row + 1, m_hoveredEdge->col));
        }
        painter.restore();
    }

    // 5. Drawn Edges
    auto drawLineEdge = [&](QPointF p1, QPointF p2, bool isLast) {
        painter.save();
        if (isLast) {
            painter.setPen(QPen(QColor(229, 169, 60), 4.5, Qt::SolidLine, Qt::RoundCap)); // Gold highlight
        } else {
            painter.setPen(QPen(QColor(180, 175, 200), 4.0, Qt::SolidLine, Qt::RoundCap));
        }
        painter.drawLine(p1, p2);
        painter.restore();
    };

    // Horizontal drawn edges
    for (int r = 0; r < DotsAndBoxesBoard::DOT_ROWS; ++r) {
        for (int c = 0; c < DotsAndBoxesBoard::DOT_COLS - 1; ++c) {
            if (m_board.isEdgeDrawn(EdgeType::Horizontal, r, c)) {
                bool isLast = m_lastEdge && (m_lastEdge->type == EdgeType::Horizontal && m_lastEdge->row == r && m_lastEdge->col == c);
                drawLineEdge(dotCoords(r, c), dotCoords(r, c + 1), isLast);
            }
        }
    }

    // Vertical drawn edges
    for (int r = 0; r < DotsAndBoxesBoard::DOT_ROWS - 1; ++r) {
        for (int c = 0; c < DotsAndBoxesBoard::DOT_COLS; ++c) {
            if (m_board.isEdgeDrawn(EdgeType::Vertical, r, c)) {
                bool isLast = m_lastEdge && (m_lastEdge->type == EdgeType::Vertical && m_lastEdge->row == r && m_lastEdge->col == c);
                drawLineEdge(dotCoords(r, c), dotCoords(r + 1, c), isLast);
            }
        }
    }

    // 6. Dots (vertices)
    for (int r = 0; r < DotsAndBoxesBoard::DOT_ROWS; ++r) {
        for (int c = 0; c < DotsAndBoxesBoard::DOT_COLS; ++c) {
            QPointF pt = dotCoords(r, c);

            painter.save();
            painter.setPen(QPen(QColor(15, 14, 20), 1.5));
            QRadialGradient dotGrad(pt - QPointF(1.5, 1.5), 6.0);
            dotGrad.setColorAt(0.0, QColor(255, 255, 255));
            dotGrad.setColorAt(0.7, QColor(200, 195, 215));
            dotGrad.setColorAt(1.0, QColor(140, 135, 155));
            painter.setBrush(dotGrad);
            painter.drawEllipse(pt, 5.0, 5.0);
            painter.restore();
        }
    }
}

void DotsAndBoxesWidget::mouseMoveEvent(QMouseEvent *event) {
    auto edgeOpt = edgeAt(event->position());
    if (edgeOpt != m_hoveredEdge) {
        m_hoveredEdge = edgeOpt;
        update();
    }
}

void DotsAndBoxesWidget::mousePressEvent(QMouseEvent *event) {
    if (!m_interactive) return;
    if (event->button() == Qt::LeftButton) {
        auto edgeOpt = edgeAt(event->position());
        if (edgeOpt) {
            emit edgeClicked(*edgeOpt);
        }
    }
}

void DotsAndBoxesWidget::leaveEvent(QEvent * /*event*/) {
    m_hoveredEdge = std::nullopt;
    update();
}

} // namespace Strategix::DotsAndBoxes
