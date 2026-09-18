#ifndef STRATEGIX_DOTSANDBOXESWIDGET_H
#define STRATEGIX_DOTSANDBOXESWIDGET_H

#include <QWidget>
#include <vector>
#include <optional>
#include "../../core/dotsandboxes/DotsAndBoxesBoard.h"

namespace Strategix::DotsAndBoxes {

class DotsAndBoxesWidget : public QWidget {
    Q_OBJECT

public:
    explicit DotsAndBoxesWidget(QWidget *parent = nullptr);

    void setBoard(const DotsAndBoxesBoard& board);
    void setActivePlayer(Player player);
    void setInteractive(bool interactive);
    void setLastEdge(const std::optional<Edge>& edge);

    QSize sizeHint() const override { return QSize(520, 520); }
    QSize minimumSizeHint() const override { return QSize(340, 340); }

signals:
    void edgeClicked(const Edge& edge);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QRectF boardRect() const;
    QPointF dotCoords(int r, int c) const;
    std::optional<Edge> edgeAt(const QPointF& pos) const;

    DotsAndBoxesBoard m_board;
    Player m_activePlayer = Player::Player1;
    bool m_interactive = true;
    std::optional<Edge> m_lastEdge;
    std::optional<Edge> m_hoveredEdge;
};

} // namespace Strategix::DotsAndBoxes

#endif // STRATEGIX_DOTSANDBOXESWIDGET_H
