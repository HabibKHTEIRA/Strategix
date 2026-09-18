#include "MainWindow.h"
#include "dialogs/RulesDialog.h"
#include "dialogs/AboutDialog.h"

#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QApplication>
#include <QFile>

namespace Strategix {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(trText("app_title"));
    resize(1050, 750);
    setMinimumSize(850, 640);

    createToolbar();
    setupUI();
    updateTexts();

    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &MainWindow::onLanguageChanged);

    showHome();
}

void MainWindow::createToolbar() {
    QToolBar *toolBar = addToolBar("Navigation");
    toolBar->setMovable(false);
    toolBar->setObjectName("mainToolBar");

    m_homeBtn = new QPushButton(this);
    m_homeBtn->setObjectName("toolbarBtn");
    connect(m_homeBtn, &QPushButton::clicked, this, &MainWindow::showHome);
    toolBar->addWidget(m_homeBtn);

    toolBar->addSeparator();

    m_newGameBtn = new QPushButton(this);
    m_newGameBtn->setObjectName("toolbarBtn");
    connect(m_newGameBtn, &QPushButton::clicked, this, &MainWindow::onNewGame);
    toolBar->addWidget(m_newGameBtn);

    m_undoBtn = new QPushButton(this);
    m_undoBtn->setObjectName("toolbarBtn");
    connect(m_undoBtn, &QPushButton::clicked, this, &MainWindow::onUndo);
    toolBar->addWidget(m_undoBtn);

    toolBar->addSeparator();

    // Mode Selector
    m_modeCombo = new QComboBox(this);
    m_modeCombo->setObjectName("toolbarCombo");
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModeChanged);
    toolBar->addWidget(m_modeCombo);

    // Difficulty Selector
    m_diffCombo = new QComboBox(this);
    m_diffCombo->setObjectName("toolbarCombo");
    connect(m_diffCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onDifficultyChanged);
    toolBar->addWidget(m_diffCombo);

    // Spacer
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolBar->addWidget(spacer);

    // Language Selector
    m_langCombo = new QComboBox(this);
    m_langCombo->setObjectName("toolbarCombo");
    m_langCombo->addItem(QString::fromUtf8("🇫🇷 Français"), static_cast<int>(Language::French));
    m_langCombo->addItem(QString::fromUtf8("🇬🇧 English"), static_cast<int>(Language::English));
    m_langCombo->addItem(QString::fromUtf8("🇸🇦 العربية"), static_cast<int>(Language::Arabic));
    connect(m_langCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        Language lang = static_cast<Language>(m_langCombo->itemData(index).toInt());
        LanguageManager::instance().setLanguage(lang);
    });
    toolBar->addWidget(m_langCombo);

    m_themeBtn = new QPushButton(this);
    m_themeBtn->setObjectName("toolbarBtn");
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    toolBar->addWidget(m_themeBtn);

    toolBar->addSeparator();

    m_rulesBtn = new QPushButton(this);
    m_rulesBtn->setObjectName("toolbarBtn");
    connect(m_rulesBtn, &QPushButton::clicked, this, &MainWindow::onShowRules);
    toolBar->addWidget(m_rulesBtn);

    m_aboutBtn = new QPushButton(this);
    m_aboutBtn->setObjectName("toolbarBtn");
    connect(m_aboutBtn, &QPushButton::clicked, this, &MainWindow::onShowAbout);
    toolBar->addWidget(m_aboutBtn);
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(8, 8, 8, 8);
    rootLayout->setSpacing(8);

    m_stackedWidget = new QStackedWidget(this);

    // 0. Home Screen
    m_homeWidget = new GameSelectionWidget(this);
    connect(m_homeWidget, &GameSelectionWidget::checkersSelected, this, &MainWindow::startCheckers);
    connect(m_homeWidget, &GameSelectionWidget::goSelected, this, &MainWindow::startGo);
    connect(m_homeWidget, &GameSelectionWidget::connectFourSelected, this, &MainWindow::startConnectFour);
    connect(m_homeWidget, &GameSelectionWidget::reversiSelected, this, &MainWindow::startReversi);
    connect(m_homeWidget, &GameSelectionWidget::quoridorSelected, this, &MainWindow::startQuoridor);
    connect(m_homeWidget, &GameSelectionWidget::morrisSelected, this, &MainWindow::startMorris);
    connect(m_homeWidget, &GameSelectionWidget::dotsAndBoxesSelected, this, &MainWindow::startDotsAndBoxes);
    m_stackedWidget->addWidget(m_homeWidget);

    // 1. Checkers Screen
    m_checkersContainer = new QWidget(this);
    QVBoxLayout *cLayout = new QVBoxLayout(m_checkersContainer);
    cLayout->setContentsMargins(6, 6, 6, 6);
    QWidget *cHeader = new QWidget(m_checkersContainer);
    cHeader->setObjectName("statusBanner");
    QHBoxLayout *cHeaderLayout = new QHBoxLayout(cHeader);
    cHeaderLayout->setContentsMargins(16, 8, 16, 8);
    m_checkersStatusLabel = new QLabel(cHeader);
    m_checkersStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    cHeaderLayout->addWidget(m_checkersStatusLabel);
    cHeaderLayout->addStretch();
    m_checkersScoreLabel = new QLabel(cHeader);
    m_checkersScoreLabel->setStyleSheet("font-size: 14px; color: #E5A93C; font-weight: 500;");
    cHeaderLayout->addWidget(m_checkersScoreLabel);
    cLayout->addWidget(cHeader);
    m_checkersWidget = new Checkers::CheckersWidget(m_checkersContainer);
    connect(m_checkersWidget, &Checkers::CheckersWidget::moveRequested, this, &MainWindow::onCheckersMoveRequested);
    cLayout->addWidget(m_checkersWidget, 1, Qt::AlignCenter);
    m_stackedWidget->addWidget(m_checkersContainer);

    // 2. Go Screen
    m_goContainer = new QWidget(this);
    QVBoxLayout *gLayout = new QVBoxLayout(m_goContainer);
    gLayout->setContentsMargins(6, 6, 6, 6);
    QWidget *gHeader = new QWidget(m_goContainer);
    gHeader->setObjectName("statusBanner");
    QHBoxLayout *gHeaderLayout = new QHBoxLayout(gHeader);
    gHeaderLayout->setContentsMargins(16, 8, 16, 8);
    m_goStatusLabel = new QLabel(gHeader);
    m_goStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    gHeaderLayout->addWidget(m_goStatusLabel);
    gHeaderLayout->addStretch();
    m_goPassBtn = new QPushButton(gHeader);
    m_goPassBtn->setStyleSheet("background-color: #383248; border: 1px solid #E5A93C; color: #F0EDF6; padding: 6px 14px; font-weight: bold;");
    connect(m_goPassBtn, &QPushButton::clicked, this, &MainWindow::onGoPassTurn);
    gHeaderLayout->addWidget(m_goPassBtn);
    m_goScoreLabel = new QLabel(gHeader);
    m_goScoreLabel->setStyleSheet("font-size: 14px; color: #4E95FF; font-weight: 500; margin-left: 12px;");
    gHeaderLayout->addWidget(m_goScoreLabel);
    gLayout->addWidget(gHeader);
    m_goBoardWidget = new Go::GoBoardWidget(m_goContainer);
    connect(m_goBoardWidget, &Go::GoBoardWidget::pointClicked, this, &MainWindow::onGoPointClicked);
    gLayout->addWidget(m_goBoardWidget, 1, Qt::AlignCenter);
    m_stackedWidget->addWidget(m_goContainer);

    // 3. Connect Four Screen
    m_c4Container = new QWidget(this);
    QVBoxLayout *c4Layout = new QVBoxLayout(m_c4Container);
    c4Layout->setContentsMargins(6, 6, 6, 6);
    QWidget *c4Header = new QWidget(m_c4Container);
    c4Header->setObjectName("statusBanner");
    QHBoxLayout *c4HeaderLayout = new QHBoxLayout(c4Header);
    c4HeaderLayout->setContentsMargins(16, 8, 16, 8);
    m_c4StatusLabel = new QLabel(c4Header);
    m_c4StatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    c4HeaderLayout->addWidget(m_c4StatusLabel);
    c4HeaderLayout->addStretch();
    c4Layout->addWidget(c4Header);
    m_c4Widget = new ConnectFour::ConnectFourWidget(m_c4Container);
    connect(m_c4Widget, &ConnectFour::ConnectFourWidget::columnSelected, this, &MainWindow::onConnectFourColumnSelected);
    c4Layout->addWidget(m_c4Widget, 1, Qt::AlignCenter);
    m_stackedWidget->addWidget(m_c4Container);

    // 4. Reversi Screen
    m_reversiContainer = new QWidget(this);
    QVBoxLayout *revLayout = new QVBoxLayout(m_reversiContainer);
    revLayout->setContentsMargins(6, 6, 6, 6);
    QWidget *revHeader = new QWidget(m_reversiContainer);
    revHeader->setObjectName("statusBanner");
    QHBoxLayout *revHeaderLayout = new QHBoxLayout(revHeader);
    revHeaderLayout->setContentsMargins(16, 8, 16, 8);
    m_reversiStatusLabel = new QLabel(revHeader);
    m_reversiStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    revHeaderLayout->addWidget(m_reversiStatusLabel);
    revHeaderLayout->addStretch();
    m_reversiScoreLabel = new QLabel(revHeader);
    m_reversiScoreLabel->setStyleSheet("font-size: 14px; color: #2ECC71; font-weight: 500;");
    revHeaderLayout->addWidget(m_reversiScoreLabel);
    revLayout->addWidget(revHeader);
    m_reversiWidget = new Reversi::ReversiWidget(m_reversiContainer);
    connect(m_reversiWidget, &Reversi::ReversiWidget::cellClicked, this, &MainWindow::onReversiCellClicked);
    revLayout->addWidget(m_reversiWidget, 1, Qt::AlignCenter);
    m_stackedWidget->addWidget(m_reversiContainer);

    // 5. Quoridor Screen
    m_quoridorContainer = new QWidget(this);
    QVBoxLayout *qLayout = new QVBoxLayout(m_quoridorContainer);
    qLayout->setContentsMargins(6, 6, 6, 6);
    QWidget *qHeader = new QWidget(m_quoridorContainer);
    qHeader->setObjectName("statusBanner");
    QHBoxLayout *qHeaderLayout = new QHBoxLayout(qHeader);
    qHeaderLayout->setContentsMargins(16, 8, 16, 8);
    m_quoridorStatusLabel = new QLabel(qHeader);
    m_quoridorStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    qHeaderLayout->addWidget(m_quoridorStatusLabel);
    qHeaderLayout->addStretch();
    m_quoridorModeBtn = new QPushButton(qHeader);
    m_quoridorModeBtn->setStyleSheet("background-color: #383248; border: 1px solid #E67E22; color: #F0EDF6; padding: 6px 14px; font-weight: bold;");
    connect(m_quoridorModeBtn, &QPushButton::clicked, this, &MainWindow::onQuoridorToggleMode);
    qHeaderLayout->addWidget(m_quoridorModeBtn);
    m_quoridorWallsLabel = new QLabel(qHeader);
    m_quoridorWallsLabel->setStyleSheet("font-size: 14px; color: #E67E22; font-weight: 500; margin-left: 12px;");
    qHeaderLayout->addWidget(m_quoridorWallsLabel);
    qLayout->addWidget(qHeader);
    m_quoridorWidget = new Quoridor::QuoridorWidget(m_quoridorContainer);
    connect(m_quoridorWidget, &Quoridor::QuoridorWidget::pawnMoveRequested, this, &MainWindow::onQuoridorPawnMove);
    connect(m_quoridorWidget, &Quoridor::QuoridorWidget::wallPlacementRequested, this, &MainWindow::onQuoridorWall);
    connect(m_quoridorWidget, &Quoridor::QuoridorWidget::modeChanged, this, [this](Quoridor::UIMode /*mode*/) { updateTexts(); });
    qLayout->addWidget(m_quoridorWidget, 1, Qt::AlignCenter);
    m_stackedWidget->addWidget(m_quoridorContainer);

    // 6. Nine Men's Morris Screen
    m_morrisContainer = new QWidget(this);
    QVBoxLayout *mLayout = new QVBoxLayout(m_morrisContainer);
    mLayout->setContentsMargins(6, 6, 6, 6);
    QWidget *mHeader = new QWidget(m_morrisContainer);
    mHeader->setObjectName("statusBanner");
    QHBoxLayout *mHeaderLayout = new QHBoxLayout(mHeader);
    mHeaderLayout->setContentsMargins(16, 8, 16, 8);
    m_morrisStatusLabel = new QLabel(mHeader);
    m_morrisStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    mHeaderLayout->addWidget(m_morrisStatusLabel);
    mHeaderLayout->addStretch();
    m_morrisPhaseLabel = new QLabel(mHeader);
    m_morrisPhaseLabel->setStyleSheet("font-size: 14px; color: #9B59B6; font-weight: 500;");
    mHeaderLayout->addWidget(m_morrisPhaseLabel);
    mLayout->addWidget(mHeader);
    m_morrisWidget = new Morris::MorrisWidget(m_morrisContainer);
    connect(m_morrisWidget, &Morris::MorrisWidget::pointClicked, this, &MainWindow::onMorrisPointClicked);
    connect(m_morrisWidget, &Morris::MorrisWidget::moveRequested, this, &MainWindow::onMorrisMove);
    connect(m_morrisWidget, &Morris::MorrisWidget::captureRequested, this, &MainWindow::onMorrisCapture);
    mLayout->addWidget(m_morrisWidget, 1, Qt::AlignCenter);
    m_stackedWidget->addWidget(m_morrisContainer);

    // 7. Dots and Boxes Screen
    m_dbContainer = new QWidget(this);
    QVBoxLayout *dbLayout = new QVBoxLayout(m_dbContainer);
    dbLayout->setContentsMargins(6, 6, 6, 6);
    QWidget *dbHeader = new QWidget(m_dbContainer);
    dbHeader->setObjectName("statusBanner");
    QHBoxLayout *dbHeaderLayout = new QHBoxLayout(dbHeader);
    dbHeaderLayout->setContentsMargins(16, 8, 16, 8);
    m_dbStatusLabel = new QLabel(dbHeader);
    m_dbStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #FFFFFF;");
    dbHeaderLayout->addWidget(m_dbStatusLabel);
    dbHeaderLayout->addStretch();
    m_dbScoreLabel = new QLabel(dbHeader);
    m_dbScoreLabel->setStyleSheet("font-size: 14px; color: #1ABC9C; font-weight: 500;");
    dbHeaderLayout->addWidget(m_dbScoreLabel);
    dbLayout->addWidget(dbHeader);
    m_dbWidget = new DotsAndBoxes::DotsAndBoxesWidget(m_dbContainer);
    connect(m_dbWidget, &DotsAndBoxes::DotsAndBoxesWidget::edgeClicked, this, &MainWindow::onDotsAndBoxesEdge);
    dbLayout->addWidget(m_dbWidget, 1, Qt::AlignCenter);
    m_stackedWidget->addWidget(m_dbContainer);

    rootLayout->addWidget(m_stackedWidget);
}

void MainWindow::showHome() {
    m_activeGame = ActiveGame::None;
    m_stackedWidget->setCurrentIndex(0);

    m_newGameBtn->setVisible(false);
    m_undoBtn->setVisible(false);
    m_modeCombo->setVisible(false);
    m_diffCombo->setVisible(false);
}

void MainWindow::startCheckers() {
    m_activeGame = ActiveGame::Checkers;
    m_stackedWidget->setCurrentIndex(1);
    m_newGameBtn->setVisible(true);
    m_undoBtn->setVisible(true);
    m_modeCombo->setVisible(true);
    m_diffCombo->setVisible(true);
    onNewGame();
}

void MainWindow::startGo() {
    m_activeGame = ActiveGame::Go;
    m_stackedWidget->setCurrentIndex(2);
    m_newGameBtn->setVisible(true);
    m_undoBtn->setVisible(true);
    m_modeCombo->setVisible(true);
    m_diffCombo->setVisible(true);
    onNewGame();
}

void MainWindow::startConnectFour() {
    m_activeGame = ActiveGame::ConnectFour;
    m_stackedWidget->setCurrentIndex(3);
    m_newGameBtn->setVisible(true);
    m_undoBtn->setVisible(true);
    m_modeCombo->setVisible(true);
    m_diffCombo->setVisible(true);
    onNewGame();
}

void MainWindow::startReversi() {
    m_activeGame = ActiveGame::Reversi;
    m_stackedWidget->setCurrentIndex(4);
    m_newGameBtn->setVisible(true);
    m_undoBtn->setVisible(true);
    m_modeCombo->setVisible(true);
    m_diffCombo->setVisible(true);
    onNewGame();
}

void MainWindow::startQuoridor() {
    m_activeGame = ActiveGame::Quoridor;
    m_stackedWidget->setCurrentIndex(5);
    m_newGameBtn->setVisible(true);
    m_undoBtn->setVisible(true);
    m_modeCombo->setVisible(true);
    m_diffCombo->setVisible(true);
    onNewGame();
}

void MainWindow::startMorris() {
    m_activeGame = ActiveGame::Morris;
    m_stackedWidget->setCurrentIndex(6);
    m_newGameBtn->setVisible(true);
    m_undoBtn->setVisible(true);
    m_modeCombo->setVisible(true);
    m_diffCombo->setVisible(true);
    onNewGame();
}

void MainWindow::startDotsAndBoxes() {
    m_activeGame = ActiveGame::DotsAndBoxes;
    m_stackedWidget->setCurrentIndex(7);
    m_newGameBtn->setVisible(true);
    m_undoBtn->setVisible(true);
    m_modeCombo->setVisible(true);
    m_diffCombo->setVisible(true);
    onNewGame();
}

void MainWindow::onNewGame() {
    bool isAI = (m_modeCombo->currentIndex() == 1);
    int diffIdx = m_diffCombo->currentIndex();

    if (m_activeGame == ActiveGame::Checkers) {
        auto mode = isAI ? Checkers::GameMode::PlayerVsAI : Checkers::GameMode::TwoPlayers;
        auto diff = static_cast<Checkers::AIDifficulty>(diffIdx);
        m_checkersAI.setDifficulty(diff);
        m_checkersGame.startNewGame(mode, diff);
        m_checkersWidget->setLastMove(std::nullopt);
    } else if (m_activeGame == ActiveGame::Go) {
        auto mode = isAI ? Go::GameMode::PlayerVsAI : Go::GameMode::TwoPlayers;
        auto diff = static_cast<Go::AIDifficulty>(diffIdx);
        m_goAI.setDifficulty(diff);
        m_goGame.startNewGame(mode, diff);
        m_goBoardWidget->setLastMove(std::nullopt);
    } else if (m_activeGame == ActiveGame::ConnectFour) {
        auto mode = isAI ? ConnectFour::GameMode::PlayerVsAI : ConnectFour::GameMode::TwoPlayers;
        auto diff = static_cast<ConnectFour::AIDifficulty>(diffIdx);
        m_c4AI.setDifficulty(diff);
        m_c4Game.startNewGame(mode, diff);
        m_c4Widget->setWinningLine({});
        m_c4Widget->setLastMove(-1, -1);
    } else if (m_activeGame == ActiveGame::Reversi) {
        auto mode = isAI ? Reversi::GameMode::PlayerVsAI : Reversi::GameMode::TwoPlayers;
        auto diff = static_cast<Reversi::AIDifficulty>(diffIdx);
        m_reversiAI.setDifficulty(diff);
        m_reversiGame.startNewGame(mode, diff);
        m_reversiWidget->setLastMove(std::nullopt);
    } else if (m_activeGame == ActiveGame::Quoridor) {
        auto mode = isAI ? Quoridor::GameMode::PlayerVsAI : Quoridor::GameMode::TwoPlayers;
        auto diff = static_cast<Quoridor::AIDifficulty>(diffIdx);
        m_quoridorAI.setDifficulty(diff);
        m_quoridorGame.startNewGame(mode, diff);
        m_quoridorWidget->setUIMode(Quoridor::UIMode::MovePawn);
    } else if (m_activeGame == ActiveGame::Morris) {
        auto mode = isAI ? Morris::GameMode::PlayerVsAI : Morris::GameMode::TwoPlayers;
        auto diff = static_cast<Morris::AIDifficulty>(diffIdx);
        m_morrisAI.setDifficulty(diff);
        m_morrisGame.startNewGame(mode, diff);
        m_morrisWidget->setLastMove(std::nullopt);
    } else if (m_activeGame == ActiveGame::DotsAndBoxes) {
        auto mode = isAI ? DotsAndBoxes::GameMode::PlayerVsAI : DotsAndBoxes::GameMode::TwoPlayers;
        auto diff = static_cast<DotsAndBoxes::AIDifficulty>(diffIdx);
        m_dbAI.setDifficulty(diff);
        m_dbGame.startNewGame(mode, diff);
        m_dbWidget->setLastEdge(std::nullopt);
    }

    updateUI();
}

void MainWindow::onUndo() {
    switch (m_activeGame) {
        case ActiveGame::Checkers: m_checkersGame.undo(); break;
        case ActiveGame::Go: m_goGame.undo(); break;
        case ActiveGame::ConnectFour: m_c4Game.undo(); break;
        case ActiveGame::Reversi: m_reversiGame.undo(); break;
        case ActiveGame::Quoridor: m_quoridorGame.undo(); break;
        case ActiveGame::Morris: m_morrisGame.undo(); break;
        case ActiveGame::DotsAndBoxes: m_dbGame.undo(); break;
        default: break;
    }
    updateUI();
}

void MainWindow::onModeChanged() {
    onNewGame();
}

void MainWindow::onDifficultyChanged() {
    int diffIdx = m_diffCombo->currentIndex();
    if (m_activeGame == ActiveGame::Checkers) {
        m_checkersAI.setDifficulty(static_cast<Checkers::AIDifficulty>(diffIdx));
    } else if (m_activeGame == ActiveGame::Go) {
        m_goAI.setDifficulty(static_cast<Go::AIDifficulty>(diffIdx));
    } else if (m_activeGame == ActiveGame::ConnectFour) {
        m_c4AI.setDifficulty(static_cast<ConnectFour::AIDifficulty>(diffIdx));
    } else if (m_activeGame == ActiveGame::Reversi) {
        m_reversiAI.setDifficulty(static_cast<Reversi::AIDifficulty>(diffIdx));
    } else if (m_activeGame == ActiveGame::Quoridor) {
        m_quoridorAI.setDifficulty(static_cast<Quoridor::AIDifficulty>(diffIdx));
    } else if (m_activeGame == ActiveGame::Morris) {
        m_morrisAI.setDifficulty(static_cast<Morris::AIDifficulty>(diffIdx));
    } else if (m_activeGame == ActiveGame::DotsAndBoxes) {
        m_dbAI.setDifficulty(static_cast<DotsAndBoxes::AIDifficulty>(diffIdx));
    }
}

// -------------------------------------------------------------
// Checkers Handlers
// -------------------------------------------------------------
void MainWindow::onCheckersMoveRequested(const Checkers::Move& move) {
    if (m_checkersGame.isAITurn() || m_checkersGame.status() != Checkers::GameStatus::InProgress) return;

    m_checkersGame.makeMove(move);
    m_checkersWidget->setLastMove(move);
    updateUI();

    if (m_checkersGame.status() != Checkers::GameStatus::InProgress) {
        QString winMsg = (m_checkersGame.status() == Checkers::GameStatus::WhiteWon) ? trText("white_won") : trText("black_won");
        QMessageBox::information(this, trText("game_checkers"), QString("<h3>%1</h3>").arg(winMsg));
        return;
    }

    if (m_checkersGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onCheckersAIMove);
    }
}

void MainWindow::onCheckersAIMove() {
    if (!m_checkersGame.isAITurn() || m_checkersGame.status() != Checkers::GameStatus::InProgress) return;

    Checkers::Move aiMove = m_checkersAI.chooseMove(m_checkersGame.board(), Checkers::PieceColor::Black);
    m_checkersGame.makeMove(aiMove);
    m_checkersWidget->setLastMove(aiMove);
    updateUI();

    if (m_checkersGame.status() != Checkers::GameStatus::InProgress) {
        QString winMsg = (m_checkersGame.status() == Checkers::GameStatus::WhiteWon) ? trText("white_won") : trText("black_won");
        QMessageBox::information(this, trText("game_checkers"), QString("<h3>%1</h3>").arg(winMsg));
    }
}

// -------------------------------------------------------------
// Go Handlers
// -------------------------------------------------------------
void MainWindow::onGoPointClicked(Go::Point p) {
    if (m_goGame.isAITurn() || m_goGame.status() != Go::GameStatus::InProgress) return;
    if (!m_goGame.isLegalMove(p)) return;

    m_goGame.playMove(p);
    m_goBoardWidget->setLastMove(p);
    updateUI();

    if (m_goGame.status() == Go::GameStatus::Finished) {
        QString winMsg = (m_goGame.winner() == Go::StoneColor::Black) ? trText("black_won") : trText("white_won");
        QMessageBox::information(this, trText("game_go"),
                                 QString("<h3>%1</h3><p>Score: Noirs %2 - Blancs %3</p>")
                                     .arg(winMsg)
                                     .arg(m_goGame.blackScore())
                                     .arg(m_goGame.whiteScore()));
        return;
    }

    if (m_goGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onGoAIMove);
    }
}

void MainWindow::onGoAIMove() {
    if (!m_goGame.isAITurn() || m_goGame.status() != Go::GameStatus::InProgress) return;

    auto aiMoveOpt = m_goAI.chooseMove(m_goGame.board(), Go::StoneColor::White, m_goGame.koPoint());
    if (aiMoveOpt) {
        m_goGame.playMove(*aiMoveOpt);
        m_goBoardWidget->setLastMove(*aiMoveOpt);
    } else {
        m_goGame.passTurn();
    }
    updateUI();

    if (m_goGame.status() == Go::GameStatus::Finished) {
        QString winMsg = (m_goGame.winner() == Go::StoneColor::Black) ? trText("black_won") : trText("white_won");
        QMessageBox::information(this, trText("game_go"),
                                 QString("<h3>%1</h3><p>Score: Noirs %2 - Blancs %3</p>")
                                     .arg(winMsg)
                                     .arg(m_goGame.blackScore())
                                     .arg(m_goGame.whiteScore()));
    }
}

void MainWindow::onGoPassTurn() {
    if (m_goGame.isAITurn() || m_goGame.status() != Go::GameStatus::InProgress) return;
    m_goGame.passTurn();
    updateUI();

    if (m_goGame.status() == Go::GameStatus::Finished) {
        QString winMsg = (m_goGame.winner() == Go::StoneColor::Black) ? trText("black_won") : trText("white_won");
        QMessageBox::information(this, trText("game_go"), QString("<h3>%1</h3>").arg(winMsg));
        return;
    }

    if (m_goGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onGoAIMove);
    }
}

// -------------------------------------------------------------
// Connect Four Handlers
// -------------------------------------------------------------
void MainWindow::onConnectFourColumnSelected(int col) {
    if (m_c4Game.isAITurn() || m_c4Game.status() != ConnectFour::GameStatus::InProgress) return;

    if (!m_c4Game.dropPiece(col)) return;

    auto last = m_c4Game.lastMove();
    m_c4Widget->setLastMove(last.first, last.second);
    m_c4Widget->setWinningLine(m_c4Game.winningLine());
    updateUI();

    if (m_c4Game.status() != ConnectFour::GameStatus::InProgress) {
        QString winMsg = (m_c4Game.status() == ConnectFour::GameStatus::RedWon) ? trText("red_won") :
                         (m_c4Game.status() == ConnectFour::GameStatus::YellowWon ? trText("yellow_won") : trText("draw"));
        QMessageBox::information(this, trText("game_connectfour"), QString("<h3>%1</h3>").arg(winMsg));
        return;
    }

    if (m_c4Game.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onConnectFourAIMove);
    }
}

void MainWindow::onConnectFourAIMove() {
    if (!m_c4Game.isAITurn() || m_c4Game.status() != ConnectFour::GameStatus::InProgress) return;

    int col = m_c4AI.chooseColumn(m_c4Game.board(), ConnectFour::Player::Yellow);
    if (col != -1) {
        m_c4Game.dropPiece(col);
        auto last = m_c4Game.lastMove();
        m_c4Widget->setLastMove(last.first, last.second);
        m_c4Widget->setWinningLine(m_c4Game.winningLine());
    }
    updateUI();

    if (m_c4Game.status() != ConnectFour::GameStatus::InProgress) {
        QString winMsg = (m_c4Game.status() == ConnectFour::GameStatus::RedWon) ? trText("red_won") :
                         (m_c4Game.status() == ConnectFour::GameStatus::YellowWon ? trText("yellow_won") : trText("draw"));
        QMessageBox::information(this, trText("game_connectfour"), QString("<h3>%1</h3>").arg(winMsg));
    }
}

// -------------------------------------------------------------
// Reversi Handlers
// -------------------------------------------------------------
void MainWindow::onReversiCellClicked(int r, int c) {
    if (m_reversiGame.isAITurn() || m_reversiGame.status() != Reversi::GameStatus::InProgress) return;

    if (!m_reversiGame.playMove(r, c)) return;

    m_reversiWidget->setLastMove(m_reversiGame.lastMove());
    updateUI();

    if (m_reversiGame.status() != Reversi::GameStatus::InProgress) {
        QString winMsg = (m_reversiGame.status() == Reversi::GameStatus::BlackWon) ? trText("black_won") :
                         (m_reversiGame.status() == Reversi::GameStatus::WhiteWon ? trText("white_won") : trText("draw"));
        QMessageBox::information(this, trText("game_reversi"), QString("<h3>%1</h3><p>Score: Noirs %2 - Blancs %3</p>")
                                 .arg(winMsg)
                                 .arg(m_reversiGame.blackScore())
                                 .arg(m_reversiGame.whiteScore()));
        return;
    }

    if (m_reversiGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onReversiAIMove);
    }
}

void MainWindow::onReversiAIMove() {
    if (!m_reversiGame.isAITurn() || m_reversiGame.status() != Reversi::GameStatus::InProgress) return;

    auto mv = m_reversiAI.chooseMove(m_reversiGame.board(), Reversi::Piece::White);
    if (mv) {
        m_reversiGame.playMove(mv->row, mv->col);
        m_reversiWidget->setLastMove(mv);
    }
    updateUI();

    if (m_reversiGame.status() != Reversi::GameStatus::InProgress) {
        QString winMsg = (m_reversiGame.status() == Reversi::GameStatus::BlackWon) ? trText("black_won") :
                         (m_reversiGame.status() == Reversi::GameStatus::WhiteWon ? trText("white_won") : trText("draw"));
        QMessageBox::information(this, trText("game_reversi"), QString("<h3>%1</h3><p>Score: Noirs %2 - Blancs %3</p>")
                                 .arg(winMsg)
                                 .arg(m_reversiGame.blackScore())
                                 .arg(m_reversiGame.whiteScore()));
    } else if (m_reversiGame.isAITurn()) {
        // Human had no moves and passed, AI plays again
        QTimer::singleShot(400, this, &MainWindow::onReversiAIMove);
    }
}

// -------------------------------------------------------------
// Quoridor Handlers
// -------------------------------------------------------------
void MainWindow::onQuoridorPawnMove(Quoridor::Position dest) {
    if (m_quoridorGame.isAITurn() || m_quoridorGame.status() != Quoridor::GameStatus::InProgress) return;

    if (!m_quoridorGame.movePawn(dest)) return;
    updateUI();

    if (m_quoridorGame.status() != Quoridor::GameStatus::InProgress) {
        QString winMsg = (m_quoridorGame.status() == Quoridor::GameStatus::Player1Won) ? trText("p1_won") : trText("p2_won");
        QMessageBox::information(this, trText("game_quoridor"), QString("<h3>%1</h3>").arg(winMsg));
        return;
    }

    if (m_quoridorGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onQuoridorAIMove);
    }
}

void MainWindow::onQuoridorWall(Quoridor::Wall wall) {
    if (m_quoridorGame.isAITurn() || m_quoridorGame.status() != Quoridor::GameStatus::InProgress) return;

    if (!m_quoridorGame.placeWall(wall)) return;
    updateUI();

    if (m_quoridorGame.status() != Quoridor::GameStatus::InProgress) {
        QString winMsg = (m_quoridorGame.status() == Quoridor::GameStatus::Player1Won) ? trText("p1_won") : trText("p2_won");
        QMessageBox::information(this, trText("game_quoridor"), QString("<h3>%1</h3>").arg(winMsg));
        return;
    }

    if (m_quoridorGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onQuoridorAIMove);
    }
}

void MainWindow::onQuoridorToggleMode() {
    auto curMode = m_quoridorWidget->uiMode();
    auto newMode = (curMode == Quoridor::UIMode::MovePawn) ? Quoridor::UIMode::PlaceWall : Quoridor::UIMode::MovePawn;
    m_quoridorWidget->setUIMode(newMode);
    updateTexts();
}

void MainWindow::onQuoridorAIMove() {
    if (!m_quoridorGame.isAITurn() || m_quoridorGame.status() != Quoridor::GameStatus::InProgress) return;

    auto action = m_quoridorAI.chooseAction(m_quoridorGame.board(), Quoridor::PlayerId::Player2, m_quoridorGame.p2Walls());
    if (action.type == Quoridor::AIAction::Type::MovePawn) {
        m_quoridorGame.movePawn(action.targetPos);
    } else {
        m_quoridorGame.placeWall(action.wall);
    }
    updateUI();

    if (m_quoridorGame.status() != Quoridor::GameStatus::InProgress) {
        QString winMsg = (m_quoridorGame.status() == Quoridor::GameStatus::Player1Won) ? trText("p1_won") : trText("p2_won");
        QMessageBox::information(this, trText("game_quoridor"), QString("<h3>%1</h3>").arg(winMsg));
    }
}

// -------------------------------------------------------------
// Nine Men's Morris Handlers
// -------------------------------------------------------------
void MainWindow::onMorrisPointClicked(int pt) {
    if (m_morrisGame.isAITurn() || m_morrisGame.status() != Morris::GameStatus::InProgress) return;
    if (m_morrisGame.isAwaitingCapture()) return;

    if (!m_morrisGame.placePiece(pt)) return;
    m_morrisWidget->setLastMove(m_morrisGame.lastMove());
    updateUI();

    if (m_morrisGame.status() != Morris::GameStatus::InProgress) {
        QString winMsg = (m_morrisGame.status() == Morris::GameStatus::WhiteWon) ? trText("white_won") : trText("black_won");
        QMessageBox::information(this, trText("game_morris"), QString("<h3>%1</h3>").arg(winMsg));
        return;
    }

    if (m_morrisGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onMorrisAIMove);
    }
}

void MainWindow::onMorrisMove(int from, int to) {
    if (m_morrisGame.isAITurn() || m_morrisGame.status() != Morris::GameStatus::InProgress) return;
    if (m_morrisGame.isAwaitingCapture()) return;

    if (!m_morrisGame.movePiece(from, to)) return;
    m_morrisWidget->setLastMove(m_morrisGame.lastMove());
    updateUI();

    if (m_morrisGame.status() != Morris::GameStatus::InProgress) {
        QString winMsg = (m_morrisGame.status() == Morris::GameStatus::WhiteWon) ? trText("white_won") : trText("black_won");
        QMessageBox::information(this, trText("game_morris"), QString("<h3>%1</h3>").arg(winMsg));
        return;
    }

    if (m_morrisGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onMorrisAIMove);
    }
}

void MainWindow::onMorrisCapture(int pt) {
    if (m_morrisGame.isAITurn() || m_morrisGame.status() != Morris::GameStatus::InProgress) return;

    if (!m_morrisGame.removeOpponentPiece(pt)) return;
    updateUI();

    if (m_morrisGame.status() != Morris::GameStatus::InProgress) {
        QString winMsg = (m_morrisGame.status() == Morris::GameStatus::WhiteWon) ? trText("white_won") : trText("black_won");
        QMessageBox::information(this, trText("game_morris"), QString("<h3>%1</h3>").arg(winMsg));
        return;
    }

    if (m_morrisGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onMorrisAIMove);
    }
}

void MainWindow::onMorrisAIMove() {
    if (!m_morrisGame.isAITurn() || m_morrisGame.status() != Morris::GameStatus::InProgress) return;

    if (m_morrisGame.isAwaitingCapture()) {
        int capPt = m_morrisAI.chooseCapture(m_morrisGame.board(), Morris::Piece::Black);
        if (capPt != -1) {
            m_morrisGame.removeOpponentPiece(capPt);
        }
    } else {
        auto phase = m_morrisGame.currentPhase(Morris::Piece::Black);
        if (phase == Morris::GamePhase::Placing) {
            int pt = m_morrisAI.choosePlacement(m_morrisGame.board(), Morris::Piece::Black);
            if (pt != -1) {
                m_morrisGame.placePiece(pt);
                m_morrisWidget->setLastMove(m_morrisGame.lastMove());
            }
        } else {
            bool isFlying = (phase == Morris::GamePhase::Flying);
            auto mv = m_morrisAI.chooseMove(m_morrisGame.board(), Morris::Piece::Black, isFlying);
            if (mv.from != -1) {
                m_morrisGame.movePiece(mv.from, mv.to);
                m_morrisWidget->setLastMove(m_morrisGame.lastMove());
            }
        }

        // If move completed a mill, immediately choose capture for AI
        if (m_morrisGame.isAwaitingCapture()) {
            int capPt = m_morrisAI.chooseCapture(m_morrisGame.board(), Morris::Piece::Black);
            if (capPt != -1) {
                m_morrisGame.removeOpponentPiece(capPt);
            }
        }
    }

    updateUI();

    if (m_morrisGame.status() != Morris::GameStatus::InProgress) {
        QString winMsg = (m_morrisGame.status() == Morris::GameStatus::WhiteWon) ? trText("white_won") : trText("black_won");
        QMessageBox::information(this, trText("game_morris"), QString("<h3>%1</h3>").arg(winMsg));
    }
}

// -------------------------------------------------------------
// Dots and Boxes Handlers
// -------------------------------------------------------------
void MainWindow::onDotsAndBoxesEdge(const DotsAndBoxes::Edge& edge) {
    if (m_dbGame.isAITurn() || m_dbGame.status() != DotsAndBoxes::GameStatus::InProgress) return;

    if (!m_dbGame.playEdge(edge)) return;

    m_dbWidget->setLastEdge(m_dbGame.lastEdge());
    updateUI();

    if (m_dbGame.status() != DotsAndBoxes::GameStatus::InProgress) {
        QString winMsg = (m_dbGame.status() == DotsAndBoxes::GameStatus::Player1Won) ? trText("blue_won") :
                         (m_dbGame.status() == DotsAndBoxes::GameStatus::Player2Won ? trText("red_won") : trText("draw"));
        QMessageBox::information(this, trText("game_dotsandboxes"),
                                 QString("<h3>%1</h3><p>Score: Bleu %2 - Rouge %3</p>")
                                     .arg(winMsg)
                                     .arg(m_dbGame.player1Score())
                                     .arg(m_dbGame.player2Score()));
        return;
    }

    if (m_dbGame.isAITurn()) {
        QTimer::singleShot(400, this, &MainWindow::onDotsAndBoxesAIMove);
    }
}

void MainWindow::onDotsAndBoxesAIMove() {
    if (!m_dbGame.isAITurn() || m_dbGame.status() != DotsAndBoxes::GameStatus::InProgress) return;

    auto edgeOpt = m_dbAI.chooseEdge(m_dbGame.board(), DotsAndBoxes::Player::Player2);
    if (edgeOpt) {
        m_dbGame.playEdge(*edgeOpt);
        m_dbWidget->setLastEdge(m_dbGame.lastEdge());
        updateUI();

        if (m_dbGame.status() != DotsAndBoxes::GameStatus::InProgress) {
            QString winMsg = (m_dbGame.status() == DotsAndBoxes::GameStatus::Player1Won) ? trText("blue_won") :
                             (m_dbGame.status() == DotsAndBoxes::GameStatus::Player2Won ? trText("red_won") : trText("draw"));
            QMessageBox::information(this, trText("game_dotsandboxes"),
                                     QString("<h3>%1</h3><p>Score: Bleu %2 - Rouge %3</p>")
                                         .arg(winMsg)
                                         .arg(m_dbGame.player1Score())
                                         .arg(m_dbGame.player2Score()));
            return;
        }
    }
}

// -------------------------------------------------------------
// UI Updates
// -------------------------------------------------------------
void MainWindow::updateUI() {
    if (m_activeGame == ActiveGame::Checkers) {
        m_checkersWidget->setBoard(m_checkersGame.board());
        m_checkersWidget->setActivePlayer(m_checkersGame.activePlayer());
        m_checkersWidget->setInteractive(!m_checkersGame.isAITurn() && m_checkersGame.status() == Checkers::GameStatus::InProgress);

        QString turnStr = (m_checkersGame.activePlayer() == Checkers::PieceColor::White) ? trText("white_turn") : trText("black_turn");
        m_checkersStatusLabel->setText(turnStr);
        m_checkersScoreLabel->setText(QString("%1: %2 %3 | %4 %5")
                                          .arg(trText("captures"))
                                          .arg(trText("white")).arg(m_checkersGame.whiteCaptures())
                                          .arg(trText("black")).arg(m_checkersGame.blackCaptures()));
        m_undoBtn->setEnabled(m_checkersGame.canUndo() && !m_checkersGame.isAITurn());
    }
    else if (m_activeGame == ActiveGame::Go) {
        m_goBoardWidget->setBoard(m_goGame.board());
        m_goBoardWidget->setActivePlayer(m_goGame.activePlayer());
        m_goBoardWidget->setInteractive(!m_goGame.isAITurn() && m_goGame.status() == Go::GameStatus::InProgress);

        QString turnStr = (m_goGame.activePlayer() == Go::StoneColor::Black) ? trText("black_turn") : trText("white_turn");
        m_goStatusLabel->setText(turnStr);
        m_goScoreLabel->setText(QString("%1: %2 %3 | %4 %5")
                                    .arg(trText("captures"))
                                    .arg(trText("black")).arg(m_goGame.blackCaptures())
                                    .arg(trText("white")).arg(m_goGame.whiteCaptures()));
        m_undoBtn->setEnabled(m_goGame.canUndo() && !m_goGame.isAITurn());
        m_goPassBtn->setEnabled(!m_goGame.isAITurn() && m_goGame.status() == Go::GameStatus::InProgress);
    }
    else if (m_activeGame == ActiveGame::ConnectFour) {
        m_c4Widget->setBoard(m_c4Game.board());
        m_c4Widget->setActivePlayer(m_c4Game.activePlayer());
        m_c4Widget->setInteractive(!m_c4Game.isAITurn() && m_c4Game.status() == ConnectFour::GameStatus::InProgress);

        QString turnStr = (m_c4Game.activePlayer() == ConnectFour::Player::Red) ? trText("red_turn") : trText("yellow_turn");
        m_c4StatusLabel->setText(turnStr);
        m_undoBtn->setEnabled(m_c4Game.canUndo() && !m_c4Game.isAITurn());
    }
    else if (m_activeGame == ActiveGame::Reversi) {
        m_reversiWidget->setBoard(m_reversiGame.board());
        m_reversiWidget->setActivePlayer(m_reversiGame.activePlayer());
        m_reversiWidget->setInteractive(!m_reversiGame.isAITurn() && m_reversiGame.status() == Reversi::GameStatus::InProgress);

        QString turnStr = (m_reversiGame.activePlayer() == Reversi::Piece::Black) ? trText("black_turn") : trText("white_turn");
        m_reversiStatusLabel->setText(turnStr);
        m_reversiScoreLabel->setText(QString("%1: %2 %3 | %4 %5")
                                         .arg(trText("score"))
                                         .arg(trText("black")).arg(m_reversiGame.blackScore())
                                         .arg(trText("white")).arg(m_reversiGame.whiteScore()));
        m_undoBtn->setEnabled(m_reversiGame.canUndo() && !m_reversiGame.isAITurn());
    }
    else if (m_activeGame == ActiveGame::Quoridor) {
        m_quoridorWidget->setBoard(m_quoridorGame.board());
        m_quoridorWidget->setActivePlayer(m_quoridorGame.activePlayer());
        m_quoridorWidget->setInteractive(!m_quoridorGame.isAITurn() && m_quoridorGame.status() == Quoridor::GameStatus::InProgress);

        QString turnStr = (m_quoridorGame.activePlayer() == Quoridor::PlayerId::Player1) ? trText("p1_turn") : trText("p2_turn");
        m_quoridorStatusLabel->setText(turnStr);
        m_quoridorWallsLabel->setText(QString("%1: J1 %2 | J2 %3")
                                          .arg(trText("walls_left"))
                                          .arg(m_quoridorGame.p1Walls())
                                          .arg(m_quoridorGame.p2Walls()));
        m_undoBtn->setEnabled(m_quoridorGame.canUndo() && !m_quoridorGame.isAITurn());
    }
    else if (m_activeGame == ActiveGame::Morris) {
        m_morrisWidget->setBoard(m_morrisGame.board());
        m_morrisWidget->setActivePlayer(m_morrisGame.activePlayer());
        m_morrisWidget->setGamePhase(m_morrisGame.currentPhase(m_morrisGame.activePlayer()));
        m_morrisWidget->setAwaitingCapture(m_morrisGame.isAwaitingCapture());
        m_morrisWidget->setInteractive(!m_morrisGame.isAITurn() && m_morrisGame.status() == Morris::GameStatus::InProgress);

        if (m_morrisGame.isAwaitingCapture()) {
            m_morrisStatusLabel->setText(trText("morris_remove"));
        } else {
            QString turnStr = (m_morrisGame.activePlayer() == Morris::Piece::White) ? trText("white_turn") : trText("black_turn");
            m_morrisStatusLabel->setText(turnStr);
        }

        auto phase = m_morrisGame.currentPhase(m_morrisGame.activePlayer());
        if (phase == Morris::GamePhase::Placing) {
            int unplaced = (m_morrisGame.activePlayer() == Morris::Piece::White) ? m_morrisGame.whiteUnplaced() : m_morrisGame.blackUnplaced();
            m_morrisPhaseLabel->setText(trText("morris_phase_place").arg(unplaced));
        } else if (phase == Morris::GamePhase::Flying) {
            m_morrisPhaseLabel->setText(trText("morris_phase_fly"));
        } else {
            m_morrisPhaseLabel->setText(trText("morris_phase_move"));
        }
        m_undoBtn->setEnabled(m_morrisGame.canUndo() && !m_morrisGame.isAITurn());
    }
    else if (m_activeGame == ActiveGame::DotsAndBoxes) {
        m_dbWidget->setBoard(m_dbGame.board());
        m_dbWidget->setActivePlayer(m_dbGame.activePlayer());
        m_dbWidget->setInteractive(!m_dbGame.isAITurn() && m_dbGame.status() == DotsAndBoxes::GameStatus::InProgress);

        QString turnStr = (m_dbGame.activePlayer() == DotsAndBoxes::Player::Player1) ? trText("blue_turn") : trText("red_turn");
        m_dbStatusLabel->setText(turnStr);
        m_dbScoreLabel->setText(QString("%1: %2 %3 | %4 %5")
                                    .arg(trText("score"))
                                    .arg(trText("blue")).arg(m_dbGame.player1Score())
                                    .arg(trText("red")).arg(m_dbGame.player2Score()));
        m_undoBtn->setEnabled(m_dbGame.canUndo() && !m_dbGame.isAITurn());
    }
}

void MainWindow::updateTexts() {
    setWindowTitle(trText("app_title"));

    m_homeBtn->setText(trText("main_menu"));
    m_newGameBtn->setText(trText("new_game"));
    m_undoBtn->setText(trText("undo"));
    m_goPassBtn->setText(trText("pass_turn"));
    m_rulesBtn->setText(trText("rules"));
    m_aboutBtn->setText(trText("about"));
    m_themeBtn->setText(m_isDarkMode ? QString::fromUtf8("☀️ ") + trText("theme_light") : QString::fromUtf8("🌙 ") + trText("theme_dark"));

    if (m_quoridorModeBtn) {
        QString modeText = (m_quoridorWidget->uiMode() == Quoridor::UIMode::MovePawn) ? trText("mode_wall") : trText("mode_move");
        m_quoridorModeBtn->setText(modeText);
    }

    int curModeIdx = m_modeCombo->currentIndex();
    if (curModeIdx < 0) curModeIdx = 0;
    m_modeCombo->blockSignals(true);
    m_modeCombo->clear();
    m_modeCombo->addItem(trText("mode_2p"));
    m_modeCombo->addItem(trText("mode_ai"));
    m_modeCombo->setCurrentIndex(curModeIdx);
    m_modeCombo->blockSignals(false);

    int curDiffIdx = m_diffCombo->currentIndex();
    if (curDiffIdx < 0) curDiffIdx = 1;
    m_diffCombo->blockSignals(true);
    m_diffCombo->clear();
    m_diffCombo->addItem(trText("easy"));
    m_diffCombo->addItem(trText("medium"));
    m_diffCombo->addItem(trText("hard"));
    m_diffCombo->setCurrentIndex(curDiffIdx);
    m_diffCombo->blockSignals(false);

    updateUI();
}

void MainWindow::onToggleTheme() {
    m_isDarkMode = !m_isDarkMode;
    applyTheme();
    updateTexts();
}

void MainWindow::applyTheme() {
    if (m_isDarkMode) {
        QFile file(":/src/style/style.qss");
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            qApp->setStyleSheet(file.readAll());
        }
    } else {
        QString lightStyle =
            "QMainWindow { background-color: #F4F4F6; color: #18181B; }"
            "QWidget { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; font-size: 13px; color: #18181B; }"
            "QToolBar { background-color: #FFFFFF; border-bottom: 1px solid #E4E4E7; padding: 6px 12px; spacing: 8px; }"
            "QPushButton { background-color: #F4F4F5; color: #18181B; border: 1px solid #D4D4D8; border-radius: 6px; padding: 6px 14px; font-weight: 500; }"
            "QPushButton:hover { background-color: #E4E4E7; border-color: #71717A; color: #000000; }"
            "QPushButton:pressed { background-color: #D4D4D8; }"
            "QPushButton:disabled { background-color: #F4F4F5; color: #A1A1AA; border-color: #E4E4E7; }"
            "QComboBox { background-color: #FFFFFF; color: #18181B; border: 1px solid #D4D4D8; border-radius: 6px; padding: 5px 12px; min-width: 120px; }"
            "QComboBox:hover { border-color: #71717A; }"
            "QComboBox QAbstractItemView { background-color: #FFFFFF; color: #18181B; border: 1px solid #D4D4D8; selection-background-color: #E4E4E7; selection-color: #000000; }"
            "#statusBanner { background-color: #FFFFFF; border: 1px solid #E4E4E7; border-radius: 10px; }"
            "QTabWidget::pane { border: 1px solid #E4E4E7; background-color: #FFFFFF; border-radius: 8px; }"
            "QTabBar::tab { background-color: #F4F4F5; color: #71717A; padding: 8px 18px; border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 4px; }"
            "QTabBar::tab:selected { background-color: #FFFFFF; color: #18181B; font-weight: bold; }"
            "QTextBrowser { background-color: #FFFFFF; border: 1px solid #E4E4E7; border-radius: 8px; color: #18181B; padding: 14px; }";
        qApp->setStyleSheet(lightStyle);
    }
    m_homeWidget->setDarkMode(m_isDarkMode);
}

void MainWindow::onLanguageChanged(Language /*lang*/) {
    updateTexts();
}

void MainWindow::onShowRules() {
    int tab = 0;
    switch (m_activeGame) {
        case ActiveGame::Checkers: tab = 0; break;
        case ActiveGame::Go: tab = 1; break;
        case ActiveGame::ConnectFour: tab = 2; break;
        case ActiveGame::Reversi: tab = 3; break;
        case ActiveGame::Quoridor: tab = 4; break;
        case ActiveGame::Morris: tab = 5; break;
        case ActiveGame::DotsAndBoxes: tab = 6; break;
        default: tab = 0; break;
    }
    RulesDialog dlg(tab, this);
    dlg.exec();
}

void MainWindow::onShowAbout() {
    AboutDialog dlg(this);
    dlg.exec();
}

} // namespace Strategix
