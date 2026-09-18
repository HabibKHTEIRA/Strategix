#ifndef STRATEGIX_MORRISWIDGET_H
#define STRATEGIX_MORRISWIDGET_H

#include <QWidget>
#include <vector>
#include <optional>
#include "../../core/morris/MorrisBoard.h"
#include "../../core/morris/MorrisGame.h"

namespace Strategix::Morris {

class MorrisWidget : public QWidget {
    Q_OBJECT

public:
    explicit MorrisWidget(QWidget *parent = nullptr);

    void setBoard(const MorrisBoard& board);
    void setActivePlayer(Piece player);
    void setGamePhase(GamePhase phase);
    void setAwaitingCapture(bool awaitingCapture);
    void setInteractive(bool interactive);
    void setLastMove(const std::optional<Move>& move);

    QSize sizeHint() const override { return QSize(540, 540); }
    QSize minimumSizeHint() const override { return QSize(350, 350); }

signals:
    void pointClicked(int pt);
    void moveRequested(int from, int to);
    void captureRequested(int pt);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QPointF pointCoords(int pt) const;
    int pointAt(const QPointF& pos) const;

    MorrisBoard m_board;
    Piece m_activePlayer = Piece::White;
    GamePhase m_phase = GamePhase::Placing;
    bool m_awaitingCapture = false;
    bool m_interactive = true;

    int m_selectedFrom = -1;
    int m_hoveredPt = -1;
    std::optional<Move> m_lastMove;
};

} // namespace Strategix::Morris

#endif // STRATEGIX_MORRISWIDGET_H
