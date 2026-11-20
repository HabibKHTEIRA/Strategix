#ifndef STRATEGIX_MONOPOLYWIDGET_H
#define STRATEGIX_MONOPOLYWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QRectF>
#include <QPointF>
#include <vector>
#include <string>
#include "../../core/monopoly/MonopolyGame.h"
#include "../../core/monopoly/MonopolyAI.h"

namespace Strategix::Monopoly {

enum class ViewScreen {
    LobbySetup,
    TokenSelect,
    InGame
};

enum class ActionOverlayMode {
    None,
    Menu,
    Build,
    Sell,
    Mortgage,
    Unmortgage,
    Trade,
    InspectDeed,
    BankInfo
};

struct HitButton {
    QString id;
    QRectF rect;
    bool enabled = true;
};

struct FloatingMoneyAnim {
    QPointF startPos;
    QPointF endPos;
    int amount = 0;
    bool isPositive = true;
    double progress = 0.0; // 0.0 .. 1.0
};

struct FlyingCardAnim {
    bool active = false;
    bool isChance = true;
    bool isDeedToSidebar = false;
    int spaceIndex = -1;
    int targetPlayerId = 0;
    QPointF startPos;
    QPointF endPos;
    double progress = 0.0; // 0.0 .. 1.0
};

class MonopolyWidget : public QWidget {
    Q_OBJECT

public:
    explicit MonopolyWidget(QWidget *parent = nullptr);

    void setGame(MonopolyGame *game, MonopolyAI *ai);
    void setDarkMode(bool dark);
    bool isDarkMode() const { return m_isDarkMode; }

    void showLobbySetup();
    void startNewConfiguredGame();
    void syncFromMainWindowToolbar(GameMode mode, AIDifficulty diff);
    void onExternalNewGame();

    QSize sizeHint() const override { return QSize(1020, 700); }
    QSize minimumSizeHint() const override { return QSize(820, 560); }

signals:
    void backToHomeRequested();
    void gameStateUpdated();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

private slots:
    void onAnimTick();
    void onGameClockTick();
    void onAIStep();

private:
    // Helper translations & formatting
    QString playerDisplayName(int playerId) const;
    QString setupPlayerDisplayName(int index) const;
    QString tokenName(TokenType token) const;
    QString spaceName(int spaceIndex) const;
    QString formatMoney(int amount) const;
    QString formatHistoryEntry(const HistoryEntry& entry) const;
    QColor groupColor(ColorGroup group) const;
    QColor playerAccentColor(int playerId) const;

    // Geometry calculators
    QRectF boardOuterRect() const;
    QRectF boardCenterRect() const;
    QRectF spaceRect(int spaceIndex) const;
    QPointF spaceCenter(int spaceIndex) const;
    QRectF rightPanelRect() const;
    QRectF bottomBarRect() const;
    QPointF playerCardAnchor(int playerId) const;

    // Painting sub-routines
    void paintLobbySetup(QPainter& p);
    void paintTokenSelect(QPainter& p);
    void paintInGame(QPainter& p);
    void paintBoard(QPainter& p, const QRectF& bRect);
    void paintBoardCenter(QPainter& p, const QRectF& cRect);
    void paintParisSkyline(QPainter& p, const QRectF& skyRect, const QColor& goldLight, const QColor& goldDark);
    void paintSpaceTile(QPainter& p, int index, const QRectF& r);
    void paintCornerTile(QPainter& p, int index, const QRectF& r);
    void paintTokensOnBoard(QPainter& p);
    void paintRightSidebar(QPainter& p, const QRectF& sRect);
    void paintBottomActionBar(QPainter& p, const QRectF& barRect);
    void paintCenterTurnModals(QPainter& p, const QRectF& cRect);
    void paintActionOverlay(QPainter& p, const QRectF& cRect);
    void paintTradeOverlay(QPainter& p, const QRectF& cRect);
    void paintRulesCarouselOverlay(QPainter& p);
    void paintAnimationsOverlay(QPainter& p);

    // Reusable vector art & UI components
    void paintTitleDeedCard(QPainter& p, const QRectF& cardRect, int spaceIndex, bool showBackIfMortgaged = false);
    void paintGoldButton(QPainter& p, const QString& id, const QRectF& rect, const QString& text,
                         bool enabled = true, bool primary = false, int fontPx = 12);
    void paintModalFrame(QPainter& p, const QRectF& rect, const QString& headerTitle = QString());
    void paintTokenIcon(QPainter& p, const QRectF& rect, TokenType token, bool withBox = true);
    void paintMrMonopoly(QPainter& p, const QRectF& rect);
    void paint3DDie(QPainter& p, const QRectF& rect, int value, double angleDeg = 0.0);
    void paintHouseIcon(QPainter& p, const QRectF& rect, bool isHotel);
    void paintLocomotiveIcon(QPainter& p, const QRectF& rect, const QColor& col);
    void paintChestIcon(QPainter& p, const QRectF& rect);
    void paintQuestionIcon(QPainter& p, const QRectF& rect, const QColor& col);
    void paintLightbulbIcon(QPainter& p, const QRectF& rect);
    void paintWaterTapIcon(QPainter& p, const QRectF& rect);
    void paintRingIcon(QPainter& p, const QRectF& rect);

    // Animation triggers
    void triggerRollAndMoveAnimation();
    void startMovePathAnimation(const std::vector<int>& path, bool jailFlight);
    void processRecentMoneyTransfers();
    void triggerFlyingCard(bool isChance);
    void triggerDeedFlyToPlayer(int spaceIndex, int playerId);
    void scheduleAIIfNeeded(int delayMs = 550);

    // Button interaction handling
    void handleButtonClick(const QString& id);

    MonopolyGame *m_game = nullptr;
    MonopolyAI *m_ai = nullptr;
    bool m_isDarkMode = true;

    ViewScreen m_screen = ViewScreen::LobbySetup;
    ActionOverlayMode m_overlayMode = ActionOverlayMode::None;
    bool m_showRulesOverlay = false;
    int m_rulesPage = 0;

    // Lobby Setup state
    std::vector<PlayerSetup> m_setups;
    int m_editingPlayerIdx = 0;

    // In-game UI state
    int m_expandedPlayerId = 0;
    int m_inspectedSpaceIndex = -1;
    int m_historyScrollOffset = 0;
    QString m_tradeFeedbackMsg;

    // Trade Builder state
    TradeOffer m_tradeDraft;
    int m_tradePreviewSpace = -1;

    // Hit detection registry rebuilt each paint
    std::vector<HitButton> m_buttons;
    QString m_hoveredButtonId;
    int m_hoveredSpaceIndex = -1;

    // Animations (60 FPS)
    QTimer *m_animTimer = nullptr;
    QTimer *m_clockTimer = nullptr;
    QTimer *m_aiTimer = nullptr;
    bool m_aiStepPending = false;

    // Dice roll animation
    bool m_diceAnimActive = false;
    double m_diceAnimProgress = 0.0;
    int m_animDieFace1 = 1;
    int m_animDieFace2 = 1;

    // Token step-by-step movement animation
    bool m_tokenAnimActive = false;
    int m_tokenAnimPlayerId = 0;
    std::vector<int> m_tokenAnimPath;
    size_t m_tokenAnimStepIdx = 0;
    double m_tokenAnimStepProgress = 0.0;
    bool m_tokenAnimIsJailFlight = false;

    // Smooth cash counters & floating money particles
    std::vector<double> m_displayedMoney;
    std::vector<FloatingMoneyAnim> m_moneyAnims;

    // Card flip / fly animation
    FlyingCardAnim m_cardAnim;
};

} // namespace Strategix::Monopoly

#endif // STRATEGIX_MONOPOLYWIDGET_H
