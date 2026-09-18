#ifndef STRATEGIX_QUORIDORWIDGET_H
#define STRATEGIX_QUORIDORWIDGET_H

#include <QWidget>
#include <vector>
#include <optional>
#include "../../core/quoridor/QuoridorBoard.h"

namespace Strategix::Quoridor {

enum class UIMode {
    MovePawn,
    PlaceWall
};

class QuoridorWidget : public QWidget {
    Q_OBJECT

public:
    explicit QuoridorWidget(QWidget *parent = nullptr);

    void setBoard(const QuoridorBoard& board);
    void setActivePlayer(PlayerId player);
    void setInteractive(bool interactive);
    void setUIMode(UIMode mode);
    UIMode uiMode() const { return m_uiMode; }
    void toggleWallOrientation();
    WallOrientation wallOrientation() const { return m_wallOrientation; }

    QSize sizeHint() const override { return QSize(540, 540); }
    QSize minimumSizeHint() const override { return QSize(360, 360); }

signals:
    void pawnMoveRequested(Position dest);
    void wallPlacementRequested(Wall wall);
    void modeChanged(UIMode mode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QRectF boardRect() const;
    QRectF cellRect(int r, int c) const;
    std::pair<int, int> cellAt(const QPointF& pos) const;
    std::optional<Wall> wallAt(const QPointF& pos) const;

    QuoridorBoard m_board;
    PlayerId m_activePlayer = PlayerId::Player1;
    bool m_interactive = true;
    UIMode m_uiMode = UIMode::MovePawn;
    WallOrientation m_wallOrientation = WallOrientation::Horizontal;

    int m_hoveredRow = -1;
    int m_hoveredCol = -1;
    std::optional<Wall> m_hoveredWall;
};

} // namespace Strategix::Quoridor

#endif // STRATEGIX_QUORIDORWIDGET_H
