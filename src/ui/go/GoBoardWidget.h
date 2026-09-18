#ifndef STRATEGIX_GOBOARDWIDGET_H
#define STRATEGIX_GOBOARDWIDGET_H

#include <QWidget>
#include <optional>
#include "../../core/go/GoBoard.h"

namespace Strategix::Go {

class GoBoardWidget : public QWidget {
    Q_OBJECT

public:
    explicit GoBoardWidget(QWidget *parent = nullptr);

    void setBoard(const GoBoard& board);
    void setActivePlayer(StoneColor color) { m_activePlayer = color; update(); }
    void setInteractive(bool interactive) { m_interactive = interactive; update(); }
    void setLastMove(const std::optional<Point>& p) { m_lastMove = p; update(); }

    QSize sizeHint() const override { return QSize(520, 520); }
    QSize minimumSizeHint() const override { return QSize(360, 360); }

signals:
    void pointClicked(Point p);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPointF intersectionPos(int r, int c) const;
    std::optional<Point> nearestIntersection(const QPointF& pos) const;
    void drawStone(QPainter& painter, const QPointF& center, qreal radius, StoneColor color, bool ghost);

    GoBoard m_board;
    StoneColor m_activePlayer = StoneColor::Black;
    bool m_interactive = true;

    std::optional<Point> m_hoveredPoint;
    std::optional<Point> m_lastMove;
};

} // namespace Strategix::Go

#endif // STRATEGIX_GOBOARDWIDGET_H
