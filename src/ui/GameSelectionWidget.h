#ifndef STRATEGIX_GAMESELECTIONWIDGET_H
#define STRATEGIX_GAMESELECTIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

namespace Strategix {

class GameSelectionWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameSelectionWidget(QWidget *parent = nullptr);

    void updateTexts();
    void setDarkMode(bool dark);

signals:
    void checkersSelected();
    void goSelected();
    void connectFourSelected();
    void reversiSelected();
    void quoridorSelected();
    void morrisSelected();
    void dotsAndBoxesSelected();

private:
    QWidget* createGameCard(const QString& titleKey, const QString& descKey,
                            const QString& imageResourcePath, const QString& accentColor,
                            void (GameSelectionWidget::*signal)());

    QLabel *m_titleLabel;
    QLabel *m_subtitleLabel;
};

} // namespace Strategix

#endif // STRATEGIX_GAMESELECTIONWIDGET_H
