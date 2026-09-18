#ifndef STRATEGIX_REVERSIWIDGET_H
#define STRATEGIX_REVERSIWIDGET_H

#include <QWidget>
#include <vector>
#include <optional>
#include "../../core/reversi/ReversiBoard.h"

namespace Strategix::Reversi {

class ReversiWidget : public QWidget {
    Q_OBJECT

public:
    explicit ReversiWidget(QWidget *parent = nullptr);

    void setBoard(const ReversiBoard& board);
    void setActivePlayer(Piece player);
    void setInteractive(bool interactive);
    void setLastMove(const std::optional<Move>& move);

    QSize sizeHint() const override { return QSize(520, 520); }
    QSize minimumSizeHint() const override { return QSize(340, 340); }

signals:
    void cellClicked(int r, int c);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QRectF boardRect() const;
    QRectF cellRect(int r, int c) const;
    std::pair<int, int> cellAt(const QPointF& pos) const;

    ReversiBoard m_board;
    Piece m_activePlayer = Piece::Black;
    bool m_interactive = true;
    std::optional<Move> m_lastMove;
    int m_hoveredRow = -1;
    int m_hoveredCol = -1;
};

} // namespace Strategix::Reversi

#endif // STRATEGIX_REVERSIWIDGET_H
