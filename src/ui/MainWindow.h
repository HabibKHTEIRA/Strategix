#ifndef STRATEGIX_MAINWINDOW_H
#define STRATEGIX_MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QActionGroup>

#include "../localization/LanguageManager.h"
#include "../core/checkers/CheckersGame.h"
#include "../core/checkers/CheckersAI.h"
#include "../core/go/GoGame.h"
#include "../core/go/GoAI.h"
#include "../core/connectfour/ConnectFourGame.h"
#include "../core/connectfour/ConnectFourAI.h"
#include "../core/reversi/ReversiGame.h"
#include "../core/reversi/ReversiAI.h"
#include "../core/quoridor/QuoridorGame.h"
#include "../core/quoridor/QuoridorAI.h"
#include "../core/morris/MorrisGame.h"
#include "../core/morris/MorrisAI.h"
#include "../core/dotsandboxes/DotsAndBoxesGame.h"
#include "../core/dotsandboxes/DotsAndBoxesAI.h"

#include "GameSelectionWidget.h"
#include "checkers/CheckersWidget.h"
#include "go/GoBoardWidget.h"
#include "connectfour/ConnectFourWidget.h"
#include "reversi/ReversiWidget.h"
#include "quoridor/QuoridorWidget.h"
#include "morris/MorrisWidget.h"
#include "dotsandboxes/DotsAndBoxesWidget.h"

namespace Strategix {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void showHome();
    void startCheckers();
    void startGo();
    void startConnectFour();
    void startReversi();
    void startQuoridor();
    void startMorris();
    void startDotsAndBoxes();
    void onLanguageChanged(Language lang);

private slots:
    // Checkers
    void onCheckersMoveRequested(const Checkers::Move& move);
    void onCheckersAIMove();

    // Go
    void onGoPointClicked(Go::Point p);
    void onGoAIMove();
    void onGoPassTurn();

    // Connect Four
    void onConnectFourColumnSelected(int col);
    void onConnectFourAIMove();

    // Reversi
    void onReversiCellClicked(int r, int c);
    void onReversiAIMove();

    // Quoridor
    void onQuoridorPawnMove(Quoridor::Position dest);
    void onQuoridorWall(Quoridor::Wall wall);
    void onQuoridorAIMove();
    void onQuoridorToggleMode();

    // Nine Men's Morris
    void onMorrisPointClicked(int pt);
    void onMorrisMove(int from, int to);
    void onMorrisCapture(int pt);
    void onMorrisAIMove();

    // Dots and Boxes
    void onDotsAndBoxesEdge(const DotsAndBoxes::Edge& edge);
    void onDotsAndBoxesAIMove();

    // General controls
    void onNewGame();
    void onUndo();
    void onShowRules();
    void onShowAbout();
    void onModeChanged();
    void onDifficultyChanged();
    void onToggleTheme();

private:
    void createToolbar();
    void setupUI();
    void updateUI();
    void updateTexts();
    void applyTheme();

    enum class ActiveGame {
        None,
        Checkers,
        Go,
        ConnectFour,
        Reversi,
        Quoridor,
        Morris,
        DotsAndBoxes
    };
    ActiveGame m_activeGame = ActiveGame::None;

    // Game engines & AIs
    Checkers::CheckersGame m_checkersGame;
    Checkers::CheckersAI m_checkersAI;

    Go::GoGame m_goGame;
    Go::GoAI m_goAI;

    ConnectFour::ConnectFourGame m_c4Game;
    ConnectFour::ConnectFourAI m_c4AI;

    Reversi::ReversiGame m_reversiGame;
    Reversi::ReversiAI m_reversiAI;

    Quoridor::QuoridorGame m_quoridorGame;
    Quoridor::QuoridorAI m_quoridorAI;

    Morris::MorrisGame m_morrisGame;
    Morris::MorrisAI m_morrisAI;

    DotsAndBoxes::DotsAndBoxesGame m_dbGame;
    DotsAndBoxes::DotsAndBoxesAI m_dbAI;

    // UI Widgets
    QStackedWidget *m_stackedWidget;
    GameSelectionWidget *m_homeWidget;

    // Checkers UI
    QWidget *m_checkersContainer;
    Checkers::CheckersWidget *m_checkersWidget;
    QLabel *m_checkersStatusLabel;
    QLabel *m_checkersScoreLabel;

    // Go UI
    QWidget *m_goContainer;
    Go::GoBoardWidget *m_goBoardWidget;
    QLabel *m_goStatusLabel;
    QLabel *m_goScoreLabel;
    QPushButton *m_goPassBtn;

    // Connect Four UI
    QWidget *m_c4Container;
    ConnectFour::ConnectFourWidget *m_c4Widget;
    QLabel *m_c4StatusLabel;

    // Reversi UI
    QWidget *m_reversiContainer;
    Reversi::ReversiWidget *m_reversiWidget;
    QLabel *m_reversiStatusLabel;
    QLabel *m_reversiScoreLabel;

    // Quoridor UI
    QWidget *m_quoridorContainer;
    Quoridor::QuoridorWidget *m_quoridorWidget;
    QLabel *m_quoridorStatusLabel;
    QLabel *m_quoridorWallsLabel;
    QPushButton *m_quoridorModeBtn;

    // Morris UI
    QWidget *m_morrisContainer;
    Morris::MorrisWidget *m_morrisWidget;
    QLabel *m_morrisStatusLabel;
    QLabel *m_morrisPhaseLabel;

    // Dots and Boxes UI
    QWidget *m_dbContainer;
    DotsAndBoxes::DotsAndBoxesWidget *m_dbWidget;
    QLabel *m_dbStatusLabel;
    QLabel *m_dbScoreLabel;

    // Toolbar controls
    QPushButton *m_homeBtn;
    QPushButton *m_newGameBtn;
    QPushButton *m_undoBtn;
    QComboBox *m_modeCombo;
    QComboBox *m_diffCombo;
    QComboBox *m_langCombo;
    QPushButton *m_themeBtn;
    QPushButton *m_rulesBtn;
    QPushButton *m_aboutBtn;

    bool m_isDarkMode = true;
};

} // namespace Strategix

#endif // STRATEGIX_MAINWINDOW_H
