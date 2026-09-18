#ifndef STRATEGIX_CONNECTFOURWIDGET_H
#define STRATEGIX_CONNECTFOURWIDGET_H

#include <QWidget>
#include <vector>
#include <optional>
#include "../../core/connectfour/ConnectFourBoard.h"

namespace Strategix::ConnectFour {

class ConnectFourWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConnectFourWidget(QWidget *parent = nullptr);

    void setBoard(const ConnectFourBoard& board);
    void setActivePlayer(Player player);
    void setInteractive(bool interactive);
    void setWinningLine(const std::vector<std::pair<int, int>>& line);
    void setLastMove(int r, int c);

    QSize sizeHint() const override { return QSize(560, 480); }
    QSize minimumSizeHint() const override { return QSize(350, 300); }

signals:
    void columnSelected(int col);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QRectF boardRect() const;
    QRectF cellRect(int r, int c) const;
    int columnAt(const QPointF& pos) const;

    ConnectFourBoard m_board;
    Player m_activePlayer = Player::Red;
    bool m_interactive = true;
    int m_hoveredCol = -1;
    std::pair<int, int> m_lastMove = {-1, -1};
    std::vector<std::pair<int, int>> m_winningLine;
};

} // namespace Strategix::ConnectFour

#endif // STRATEGIX_CONNECTFOURWIDGET_H
