#ifndef STRATEGIX_CHECKERSWIDGET_H
#define STRATEGIX_CHECKERSWIDGET_H

#include <QWidget>
#include <vector>
#include <optional>
#include "../../core/checkers/CheckersBoard.h"

namespace Strategix::Checkers {

class CheckersWidget : public QWidget {
    Q_OBJECT

public:
    explicit CheckersWidget(QWidget *parent = nullptr);

    void setBoard(const CheckersBoard& board);
    void setActivePlayer(PieceColor color) { m_activePlayer = color; update(); }
    void setInteractive(bool interactive) { m_interactive = interactive; update(); }
    void setLastMove(const std::optional<Move>& move) { m_lastMove = move; update(); }

    QSize sizeHint() const override { return QSize(520, 520); }
    QSize minimumSizeHint() const override { return QSize(360, 360); }

signals:
    void moveRequested(const Move& move);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QRectF cellRect(int r, int c) const;
    std::pair<int, int> cellAt(const QPointF& pos) const;
    void drawPiece(QPainter& painter, const QRectF& rect, Piece piece, bool isSelected);
    void drawCrown(QPainter& painter, const QRectF& rect);

    CheckersBoard m_board;
    PieceColor m_activePlayer = PieceColor::White;
    bool m_interactive = true;

    std::optional<Position> m_selectedPos;
    std::vector<Move> m_availableMovesFromSelected;
    std::optional<Move> m_lastMove;

    int m_hoveredRow = -1;
    int m_hoveredCol = -1;
};

} // namespace Strategix::Checkers

#endif // STRATEGIX_CHECKERSWIDGET_H
