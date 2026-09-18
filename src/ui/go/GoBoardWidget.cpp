#include "GoBoardWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QRadialGradient>
#include <QLinearGradient>

namespace Strategix::Go {

GoBoardWidget::GoBoardWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMouseTracking(true);
}

void GoBoardWidget::setBoard(const GoBoard& board) {
    m_board = board;
    update();
}

QPointF GoBoardWidget::intersectionPos(int r, int c) const {
    qreal boardSide = qMin(width(), height()) - 40.0;
    if (boardSide < 240.0) boardSide = 240.0;

    qreal startX = (width() - boardSide) / 2.0;
    qreal startY = (height() - boardSide) / 2.0;

    qreal margin = 28.0;
    qreal gridWidth = boardSide - 2.0 * margin;
    qreal step = gridWidth / (m_board.size() - 1);

    return QPointF(startX + margin + c * step, startY + margin + r * step);
}

std::optional<Point> GoBoardWidget::nearestIntersection(const QPointF& pos) const {
    qreal boardSide = qMin(width(), height()) - 40.0;
    if (boardSide < 240.0) boardSide = 240.0;

    qreal margin = 28.0;
    qreal gridWidth = boardSide - 2.0 * margin;
    qreal step = gridWidth / (m_board.size() - 1);

    qreal maxDist = step * 0.48;

    for (int r = 0; r < m_board.size(); ++r) {
        for (int c = 0; c < m_board.size(); ++c) {
            QPointF p = intersectionPos(r, c);
            qreal dx = p.x() - pos.x();
            qreal dy = p.y() - pos.y();
            if (dx * dx + dy * dy <= maxDist * maxDist) {
                return Point{r, c};
            }
        }
    }
    return std::nullopt;
}

void GoBoardWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF evPos = event->position();
#else
    QPointF evPos = event->pos();
#endif
    auto pt = nearestIntersection(evPos);
    if (pt != m_hoveredPoint) {
        m_hoveredPoint = pt;
        if (m_interactive && m_hoveredPoint && m_board.stoneAt(*m_hoveredPoint) == StoneColor::None) {
            setCursor(Qt::PointingHandCursor);
        } else {
            unsetCursor();
        }
        update();
    }
}

void GoBoardWidget::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);
    m_hoveredPoint = std::nullopt;
    unsetCursor();
    update();
}

void GoBoardWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
    if (!m_interactive || event->button() != Qt::LeftButton) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF evPos = event->position();
#else
    QPointF evPos = event->pos();
#endif
    auto pt = nearestIntersection(evPos);
    if (pt && m_board.stoneAt(*pt) == StoneColor::None) {
        emit pointClicked(*pt);
    }
}

void GoBoardWidget::paintEvent(QPaintEvent * /*event*/) {
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

    // Board Wood Texture Gradient (Kaya)
    QLinearGradient woodGrad(boardRect.topLeft(), boardRect.bottomRight());
    woodGrad.setColorAt(0.0, QColor(226, 185, 120));
    woodGrad.setColorAt(0.5, QColor(212, 168, 100));
    woodGrad.setColorAt(1.0, QColor(194, 148, 80));

    painter.setPen(QPen(QColor(120, 85, 45), 3.0));
    painter.setBrush(woodGrad);
    painter.drawRoundedRect(boardRect, 10.0, 10.0);

    // Step calculation
    int size = m_board.size();
    qreal margin = 28.0;
    qreal gridWidth = boardSide - 2.0 * margin;
    qreal step = gridWidth / (size - 1);
    qreal stoneRadius = step * 0.44;

    // Draw Grid Lines
    painter.setPen(QPen(QColor(50, 35, 20), 1.5));
    for (int i = 0; i < size; ++i) {
        // Horizontal lines
        QPointF left = intersectionPos(i, 0);
        QPointF right = intersectionPos(i, size - 1);
        painter.drawLine(left, right);

        // Vertical lines
        QPointF top = intersectionPos(0, i);
        QPointF bottom = intersectionPos(size - 1, i);
        painter.drawLine(top, bottom);
    }

    // Draw Star Points (Hoshi) for 9x9 board
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(50, 35, 20));
    std::vector<Point> hoshis;
    if (size == 9) {
        hoshis = {{2, 2}, {2, 6}, {6, 2}, {6, 6}, {4, 4}};
    }
    for (const auto& h : hoshis) {
        painter.drawEllipse(intersectionPos(h.row, h.col), 3.5, 3.5);
    }

    // Draw Coordinate Labels (A..J without I, and 1..9)
    painter.setPen(QColor(80, 55, 30));
    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);

    static const char colLetters[] = "ABCDEFGHJKLMNOPQRST";
    for (int i = 0; i < size; ++i) {
        // Column Letters
        QPointF topPos = intersectionPos(0, i);
        QRectF colRect(topPos.x() - 10, startY + 6, 20, 16);
        painter.drawText(colRect, Qt::AlignCenter, QString(colLetters[i]));

        // Row Numbers
        QPointF leftPos = intersectionPos(i, 0);
        QRectF rowRect(startX + 6, leftPos.y() - 8, 16, 16);
        painter.drawText(rowRect, Qt::AlignCenter, QString::number(size - i));
    }

    // Draw Placed Stones
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            StoneColor sc = m_board.stoneAt(r, c);
            if (sc != StoneColor::None) {
                QPointF center = intersectionPos(r, c);
                drawStone(painter, center, stoneRadius, sc, false);

                // Last move marker
                if (m_lastMove && m_lastMove->row == r && m_lastMove->col == c) {
                    painter.setPen(QPen((sc == StoneColor::Black) ? QColor(229, 169, 60) : QColor(220, 60, 60), 2.5));
                    painter.setBrush(Qt::NoBrush);
                    painter.drawEllipse(center, stoneRadius * 0.45, stoneRadius * 0.45);
                }
            }
        }
    }

    // Draw Hovered Ghost Stone
    if (m_interactive && m_hoveredPoint && m_board.stoneAt(*m_hoveredPoint) == StoneColor::None) {
        QPointF center = intersectionPos(m_hoveredPoint->row, m_hoveredPoint->col);
        drawStone(painter, center, stoneRadius, m_activePlayer, true);
    }
}

void GoBoardWidget::drawStone(QPainter& painter, const QPointF& center, qreal radius, StoneColor color, bool ghost) {
    painter.save();

    if (ghost) {
        painter.setOpacity(0.45);
    }

    // Drop shadow
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(10, 8, 15, 90));
    painter.drawEllipse(center + QPointF(1.5, 3.0), radius, radius);

    // Stone body radial gradient (3D clamshell/slate look)
    QPointF lightOffset = center - QPointF(radius * 0.32, radius * 0.32);
    QRadialGradient grad(lightOffset, radius * 1.35);

    if (color == StoneColor::Black) {
        grad.setColorAt(0.0, QColor(75, 70, 78));
        grad.setColorAt(0.65, QColor(32, 28, 34));
        grad.setColorAt(1.0, QColor(10, 8, 12));
        painter.setPen(QPen(QColor(15, 12, 18), 1.0));
    } else {
        grad.setColorAt(0.0, QColor(255, 255, 255));
        grad.setColorAt(0.7, QColor(235, 232, 226));
        grad.setColorAt(1.0, QColor(195, 190, 180));
        painter.setPen(QPen(QColor(160, 155, 145), 1.0));
    }

    painter.setBrush(grad);
    painter.drawEllipse(center, radius, radius);

    painter.restore();
}

} // namespace Strategix::Go
