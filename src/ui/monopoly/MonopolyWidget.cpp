#include "MonopolyWidget.h"
#include "../../localization/LanguageManager.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QtMath>
#include <algorithm>
#include <cmath>

namespace Strategix::Monopoly {

MonopolyWidget::MonopolyWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    // Default 2-player setup: Human vs AI (or Human vs Human)
    m_setups.resize(2);
    m_setups[0] = {"", TokenType::Dog, false, AIDifficulty::Easy};
    m_setups[1] = {"", TokenType::Battleship, true, AIDifficulty::Easy};

    // Load 3D golden token figurines
    for (int i = 0; i < 8; ++i) {
        QString resPath = QString(":/img/token_%1.png").arg(i);
        QPixmap px(resPath);
        if (px.isNull()) {
            px.load(QString("img/token_%1.png").arg(i));
        }
        m_tokenPixmaps[i] = px;
    }

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16); // ~60 FPS
    connect(m_animTimer, &QTimer::timeout, this, &MonopolyWidget::onAnimTick);
    m_animTimer->start();

    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, &MonopolyWidget::onGameClockTick);
    m_clockTimer->start();

    m_aiTimer = new QTimer(this);
    m_aiTimer->setSingleShot(true);
    connect(m_aiTimer, &QTimer::timeout, this, &MonopolyWidget::onAIStep);

    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, [this]() {
        update();
    });
}

void MonopolyWidget::setGame(MonopolyGame *game, MonopolyAI *ai) {
    m_game = game;
    m_ai = ai;
    if (m_game) {
        m_displayedMoney.resize(m_game->numPlayers(), 1500.0);
        for (int i = 0; i < m_game->numPlayers(); ++i) {
            m_displayedMoney[i] = m_game->player(i).money;
        }
    }
    update();
}

void MonopolyWidget::setDarkMode(bool dark) {
    m_isDarkMode = dark;
    update();
}

void MonopolyWidget::showLobbySetup() {
    m_screen = ViewScreen::LobbySetup;
    m_overlayMode = ActionOverlayMode::None;
    m_showRulesOverlay = false;
    update();
}

void MonopolyWidget::syncFromMainWindowToolbar(GameMode mode, AIDifficulty diff) {
    if (m_setups.size() < 2) m_setups.resize(2);
    m_setups[0].isAI = false;
    m_setups[1].isAI = (mode == GameMode::PlayerVsAI);
    for (auto& s : m_setups) {
        if (s.isAI) s.difficulty = diff;
    }
    if (m_screen == ViewScreen::InGame && m_game) {
        startNewConfiguredGame();
    } else {
        update();
    }
}

void MonopolyWidget::onExternalNewGame() {
    if (m_screen == ViewScreen::InGame) {
        startNewConfiguredGame();
    } else {
        showLobbySetup();
    }
}

void MonopolyWidget::startNewConfiguredGame() {
    if (!m_game) return;
    m_game->startCustomGame(m_setups);
    m_displayedMoney.assign(m_game->numPlayers(), 1500.0);
    m_moneyAnims.clear();
    m_diceAnimActive = false;
    m_tokenAnimActive = false;
    m_cardAnim.active = false;
    m_expandedPlayerId = 0;
    m_historyScrollOffset = 0;
    m_overlayMode = ActionOverlayMode::None;
    m_showRulesOverlay = false;
    m_screen = ViewScreen::InGame;
    emit gameStateUpdated();
    update();
}

// -----------------------------------------------------------------------------
// Formatting & Translations
// -----------------------------------------------------------------------------
QString MonopolyWidget::tokenName(TokenType token) const {
    return trText(QString("mono_token_%1").arg(static_cast<int>(token)));
}

QString MonopolyWidget::setupPlayerDisplayName(int index) const {
    if (index < 0 || index >= static_cast<int>(m_setups.size())) return QString();
    if (!m_setups[index].isAI && index == 0) {
        return trText("mono_player_default").arg(1);
    }
    return tokenName(m_setups[index].token);
}

QString MonopolyWidget::playerDisplayName(int playerId) const {
    if (!m_game || playerId < 0 || playerId >= m_game->numPlayers()) return QString();
    const Player& p = m_game->player(playerId);
    if (!p.customName.empty()) return QString::fromStdString(p.customName);
    if (!p.isAI && playerId == 0) {
        return trText("mono_player_default").arg(1);
    }
    return tokenName(p.token);
}

QString MonopolyWidget::spaceName(int spaceIndex) const {
    if (spaceIndex < 0 || spaceIndex >= MonopolyBoard::NUM_SPACES) return QString();
    return trText(QString("mono_space_%1").arg(spaceIndex));
}

QString MonopolyWidget::formatMoney(int amount) const {
    // Format with thousands separator and Monopoly M symbol (₩/M)
    bool neg = amount < 0;
    int absVal = std::abs(amount);
    QString numStr;
    if (absVal >= 1000) {
        numStr = QString("%1,%2").arg(absVal / 1000).arg(absVal % 1000, 3, 10, QChar('0'));
    } else {
        numStr = QString::number(absVal);
    }
    return QString("%1M%2").arg(neg ? "-" : "").arg(numStr);
}

QString MonopolyWidget::formatHistoryEntry(const HistoryEntry& e) const {
    QString tpl = trText(QString::fromStdString(e.key));
    if (e.key == "mono_log_welcome") return tpl;

    QString p1 = (e.player1Id >= 0) ? playerDisplayName(e.player1Id) : "";
    QString p2 = (e.player2Id >= 0) ? playerDisplayName(e.player2Id) : "";
    QString sp = (e.spaceIndex >= 0) ? spaceName(e.spaceIndex) : "";

    if (e.key == "mono_log_moved" || e.key == "mono_log_moved_double") {
        return tpl.arg(p1, sp);
    }
    if (e.key == "mono_log_bought_property" || e.key == "mono_log_paid_tax" ||
        e.key == "mono_log_auction_won" || e.key == "mono_log_built_house" ||
        e.key == "mono_log_built_hotel" || e.key == "mono_log_sold_building" ||
        e.key == "mono_log_mortgaged" || e.key == "mono_log_unmortgaged") {
        return tpl.arg(p1, sp, QString::number(e.amount));
    }
    if (e.key == "mono_log_paid_rent") {
        return tpl.arg(p1, p2, QString::number(e.amount));
    }
    if (e.key == "mono_log_auction_started") {
        return tpl.arg(sp);
    }
    if (e.key == "mono_log_card_collect" || e.key == "mono_log_card_pay") {
        return tpl.arg(p1, QString::number(e.amount));
    }
    if (e.key == "mono_log_trade_completed") {
        return tpl.arg(p1, p2);
    }
    return tpl.arg(p1);
}

QColor MonopolyWidget::groupColor(ColorGroup group) const {
    switch (group) {
        case ColorGroup::Brown:     return QColor("#7B4B27");
        case ColorGroup::LightBlue: return QColor("#58B9E8");
        case ColorGroup::Pink:      return QColor("#D63088");
        case ColorGroup::Orange:    return QColor("#F27822");
        case ColorGroup::Red:       return QColor("#E32929");
        case ColorGroup::Yellow:    return QColor("#F2C918");
        case ColorGroup::Green:     return QColor("#1F9D4C");
        case ColorGroup::DarkBlue:  return QColor("#1C52A6");
        case ColorGroup::Railroad:  return QColor("#52525B");
        case ColorGroup::Utility:   return QColor("#A1A1AA");
        default:                    return QColor("#D4AF37");
    }
}

QColor MonopolyWidget::playerAccentColor(int playerId) const {
    static const QColor cols[4] = {
        QColor("#D4AF37"), // Player 1: Gold
        QColor("#38BDF8"), // Player 2: Cyan Blue
        QColor("#F43F5E"), // Player 3: Ruby Rose
        QColor("#34D399")  // Player 4: Emerald
    };
    return cols[std::clamp(playerId, 0, 3)];
}

// -----------------------------------------------------------------------------
// Geometry Calculators (Board fills 100% of available space!)
// -----------------------------------------------------------------------------
QRectF MonopolyWidget::bottomBarRect() const {
    double h = 46.0;
    return QRectF(4.0, height() - h - 4.0, width() - 8.0, h);
}

QRectF MonopolyWidget::boardOuterRect() const {
    QRectF bar = bottomBarRect();
    double availH = bar.top() - 8.0;
    double availW = width() - 8.0;
    // Ensure right panel has at least 270px width, while board takes maximum square space
    double maxBoardW = std::max(360.0, availW - 290.0);
    double side = std::min(availH, maxBoardW);
    return QRectF(4.0, 4.0, side, side);
}

QRectF MonopolyWidget::rightPanelRect() const {
    QRectF b = boardOuterRect();
    QRectF bar = bottomBarRect();
    double left = b.right() + 6.0;
    return QRectF(left, 4.0, width() - left - 4.0, bar.top() - 8.0);
}

QRectF MonopolyWidget::spaceRect(int spaceIndex) const {
    QRectF b = boardOuterRect().adjusted(3, 3, -3, -3);
    // Corner ratio = 1.52 * regular tile width => total = 2 * 1.52 + 9 = 12.04 units
    double unit = b.width() / 12.04;
    double corner = unit * 1.52;
    double reg = (b.width() - 2.0 * corner) / 9.0;

    if (spaceIndex == 0) {
        return QRectF(b.right() - corner, b.bottom() - corner, corner, corner);
    } else if (spaceIndex > 0 && spaceIndex < 10) {
        int k = spaceIndex; // 1..9 from right to left
        return QRectF(b.right() - corner - k * reg, b.bottom() - corner, reg, corner);
    } else if (spaceIndex == 10) {
        return QRectF(b.left(), b.bottom() - corner, corner, corner);
    } else if (spaceIndex > 10 && spaceIndex < 20) {
        int k = spaceIndex - 10; // 1..9 from bottom to top
        return QRectF(b.left(), b.bottom() - corner - k * reg, corner, reg);
    } else if (spaceIndex == 20) {
        return QRectF(b.left(), b.top(), corner, corner);
    } else if (spaceIndex > 20 && spaceIndex < 30) {
        int k = spaceIndex - 20; // 1..9 from left to right
        return QRectF(b.left() + corner + (k - 1) * reg, b.top(), reg, corner);
    } else if (spaceIndex == 30) {
        return QRectF(b.right() - corner, b.top(), corner, corner);
    } else if (spaceIndex > 30 && spaceIndex < 40) {
        int k = spaceIndex - 30; // 1..9 from top to bottom
        return QRectF(b.right() - corner, b.top() + corner + (k - 1) * reg, corner, reg);
    }
    return QRectF();
}

QRectF MonopolyWidget::boardCenterRect() const {
    QRectF b = boardOuterRect().adjusted(3, 3, -3, -3);
    double unit = b.width() / 12.04;
    double corner = unit * 1.52;
    return b.adjusted(corner, corner, -corner, -corner);
}

QPointF MonopolyWidget::spaceCenter(int spaceIndex) const {
    return spaceRect(spaceIndex).center();
}

QPointF MonopolyWidget::playerCardAnchor(int playerId) const {
    QRectF rp = rightPanelRect();
    if (!m_game || playerId < 0 || playerId >= m_game->numPlayers()) {
        return QPointF(rp.center().x(), rp.top() + 180.0); // Bank position
    }
    double y = rp.top() + 25.0 + playerId * 42.0;
    return QPointF(rp.left() + 80.0, y);
}

// -----------------------------------------------------------------------------
// Animation & Timer Ticks
// -----------------------------------------------------------------------------
void MonopolyWidget::onGameClockTick() {
    if (m_screen == ViewScreen::InGame && m_game && m_game->status() == GameStatus::InProgress) {
        m_game->setElapsedSeconds(m_game->elapsedSeconds() + 1);
        update();
    }
}

void MonopolyWidget::onAnimTick() {
    bool needsRepaint = false;

    // 1. Dice roll animation (halved speed: 0.018 per tick)
    if (m_diceAnimActive) {
        m_diceAnimProgress += 0.018;
        // Cycle faces while tumbling
        if (m_diceAnimProgress < 0.82) {
            int tick = static_cast<int>(m_diceAnimProgress * 32.0);
            m_animDieFace1 = (tick % 6) + 1;
            m_animDieFace2 = ((tick * 3 + 2) % 6) + 1;
        } else if (m_game) {
            m_animDieFace1 = m_game->die1();
            m_animDieFace2 = m_game->die2();
        }
        needsRepaint = true;

        if (m_diceAnimProgress >= 1.0) {
            m_diceAnimActive = false;
            m_diceAnimProgress = 1.0;
            if (m_game && m_game->lastMovePath().size() > 1) {
                startMovePathAnimation(m_game->lastMovePath(), m_game->lastMoveWasJailSend());
            } else {
                processRecentMoneyTransfers();
                if (m_game && m_game->phase() == TurnPhase::CardModal) {
                    triggerFlyingCard(m_game->drawnCard().isChance);
                } else {
                    scheduleAIIfNeeded();
                }
            }
        }
    }

    // 2. Token movement animation (halved speed: 0.08 per tile, 0.022 for jail flight)
    if (m_tokenAnimActive) {
        double speed = m_tokenAnimIsJailFlight ? 0.022 : 0.08;
        m_tokenAnimStepProgress += speed;
        needsRepaint = true;

        if (m_tokenAnimStepProgress >= 1.0) {
            m_tokenAnimStepProgress = 0.0;
            m_tokenAnimStepIdx += 1;
            if (m_tokenAnimStepIdx + 1 >= m_tokenAnimPath.size()) {
                m_tokenAnimActive = false;
                processRecentMoneyTransfers();
                if (m_game && m_game->phase() == TurnPhase::CardModal) {
                    triggerFlyingCard(m_game->drawnCard().isChance);
                } else {
                    scheduleAIIfNeeded();
                }
            }
        }
    }

    // 3. Card flip / fly animation (halved speed: 0.027 per tick)
    if (m_cardAnim.active) {
        m_cardAnim.progress += 0.027;
        needsRepaint = true;
        if (m_cardAnim.progress >= 1.0) {
            m_cardAnim.progress = 1.0;
            m_cardAnim.active = false;
            scheduleAIIfNeeded();
        }
    }

    // 4. Floating money animations (halved speed: 0.0125 per tick)
    for (auto it = m_moneyAnims.begin(); it != m_moneyAnims.end(); ) {
        it->progress += 0.0125;
        needsRepaint = true;
        if (it->progress >= 1.0) {
            it = m_moneyAnims.erase(it);
        } else {
            ++it;
        }
    }

    // 5. Smooth rolling cash counters
    if (m_game) {
        if (static_cast<int>(m_displayedMoney.size()) != m_game->numPlayers()) {
            m_displayedMoney.resize(m_game->numPlayers(), 1500.0);
        }
        for (int i = 0; i < m_game->numPlayers(); ++i) {
            double target = m_game->player(i).money;
            double diff = target - m_displayedMoney[i];
            if (std::abs(diff) > 0.5) {
                m_displayedMoney[i] += diff * 0.10;
                if (std::abs(target - m_displayedMoney[i]) < 1.0) {
                    m_displayedMoney[i] = target;
                }
                needsRepaint = true;
            }
        }
    }

    if (needsRepaint) {
        update();
    }
}

bool MonopolyWidget::isAnimating() const {
    return m_diceAnimActive || m_tokenAnimActive || m_cardAnim.active;
}

void MonopolyWidget::skipActiveAnimations() {
    if (!isAnimating()) return;
    m_diceAnimActive = false;
    m_diceAnimProgress = 1.0;
    m_tokenAnimActive = false;
    m_cardAnim.active = false;
    m_cardAnim.progress = 1.0;
    m_moneyAnims.clear();

    if (m_game) {
        m_game->clearRecentTransfers();
        m_displayedMoney.resize(m_game->numPlayers(), 1500.0);
        for (int i = 0; i < m_game->numPlayers(); ++i) {
            m_displayedMoney[i] = m_game->player(i).money;
        }
    }
    scheduleAIIfNeeded();
    update();
}

void MonopolyWidget::triggerRollAndMoveAnimation() {
    if (!m_game) return;
    m_tokenAnimPlayerId = m_game->activePlayerIndex();
    m_game->rollDice();
    m_diceAnimActive = true;
    m_diceAnimProgress = 0.0;
    m_animDieFace1 = 1;
    m_animDieFace2 = 6;
    emit gameStateUpdated();
    update();
}

void MonopolyWidget::startMovePathAnimation(const std::vector<int>& path, bool jailFlight) {
    if (path.size() < 2) return;
    m_tokenAnimPath = path;
    m_tokenAnimStepIdx = 0;
    m_tokenAnimStepProgress = 0.0;
    m_tokenAnimIsJailFlight = jailFlight;
    m_tokenAnimActive = true;
}

void MonopolyWidget::processRecentMoneyTransfers() {
    if (!m_game) return;
    QRectF cRect = boardCenterRect();
    QPointF bankPos(cRect.center().x(), cRect.top() + 60.0);

    for (const auto& tr : m_game->recentTransfers()) {
        QPointF fromP = (tr.fromPlayerId >= 0) ? playerCardAnchor(tr.fromPlayerId) :
                        ((tr.spaceIndex >= 0) ? spaceCenter(tr.spaceIndex) : bankPos);
        QPointF toP = (tr.toPlayerId >= 0) ? playerCardAnchor(tr.toPlayerId) : bankPos;

        FloatingMoneyAnim anim;
        anim.startPos = fromP;
        anim.endPos = toP;
        anim.amount = tr.amount;
        anim.isPositive = (tr.toPlayerId >= 0);
        anim.progress = 0.0;
        m_moneyAnims.push_back(anim);
    }
    m_game->clearRecentTransfers();
}

void MonopolyWidget::triggerFlyingCard(bool isChance) {
    QRectF cRect = boardCenterRect();
    m_cardAnim.active = true;
    m_cardAnim.isChance = isChance;
    m_cardAnim.isDeedToSidebar = false;
    m_cardAnim.progress = 0.0;
    if (isChance) {
        m_cardAnim.startPos = QPointF(cRect.right() - cRect.width() * 0.22, cRect.center().y());
    } else {
        m_cardAnim.startPos = QPointF(cRect.left() + cRect.width() * 0.22, cRect.center().y());
    }
    m_cardAnim.endPos = cRect.center();
}

void MonopolyWidget::triggerDeedFlyToPlayer(int spaceIndex, int playerId) {
    QRectF cRect = boardCenterRect();
    m_cardAnim.active = true;
    m_cardAnim.isDeedToSidebar = true;
    m_cardAnim.spaceIndex = spaceIndex;
    m_cardAnim.targetPlayerId = playerId;
    m_cardAnim.startPos = cRect.center();
    m_cardAnim.endPos = playerCardAnchor(playerId);
    m_cardAnim.progress = 0.0;
}

// -----------------------------------------------------------------------------
// AI Controller Loop
// -----------------------------------------------------------------------------
void MonopolyWidget::scheduleAIIfNeeded(int delayMs) {
    if (!m_game || !m_ai || m_screen != ViewScreen::InGame) return;
    if (m_diceAnimActive || m_tokenAnimActive || m_cardAnim.active) return;

    if (m_game->phase() == TurnPhase::WaitingForStartModal) {
        return; // Let human click FERMER on the initial start banner
    }

    if (m_game->isAITurn()) {
        int effectiveDelay = delayMs;
        if (effectiveDelay < 0) {
            switch (m_game->phase()) {
                case TurnPhase::CardModal:        effectiveDelay = 3600; break;
                case TurnPhase::RentPaymentModal: effectiveDelay = 2600; break;
                case TurnPhase::BuyOrAuction:     effectiveDelay = 2400; break;
                case TurnPhase::Auction:          effectiveDelay = 1600; break;
                case TurnPhase::WaitingForRoll:   effectiveDelay = 1500; break;
                case TurnPhase::WaitingForEndTurn:effectiveDelay = 1800; break;
                default:                          effectiveDelay = 1800; break;
            }
        }
        m_aiStepPending = true;
        m_aiTimer->start(effectiveDelay);
    }
}

void MonopolyWidget::onAIStep() {
    m_aiStepPending = false;
    if (!m_game || !m_ai || m_screen != ViewScreen::InGame) return;
    if (m_diceAnimActive || m_tokenAnimActive || m_cardAnim.active) return;
    if (!m_game->isAITurn()) return;

    int actorId = m_game->activeActorId();
    TurnPhase ph = m_game->phase();

    switch (ph) {
        case TurnPhase::WaitingForRoll: {
            if (m_game->player(actorId).inJail) {
                if (m_ai->shouldPayToLeaveJail(*m_game, actorId)) {
                    if (m_game->player(actorId).jailFreeCards() > 0) {
                        m_game->useJailFreeCard();
                    } else if (m_game->player(actorId).money >= 50) {
                        m_game->payJailFine();
                        processRecentMoneyTransfers();
                    }
                }
            }
            triggerRollAndMoveAnimation();
            break;
        }

        case TurnPhase::BuyOrAuction: {
            int pos = m_game->player(actorId).position;
            if (m_ai->shouldBuyProperty(*m_game, actorId, pos)) {
                m_game->buyCurrentProperty();
                processRecentMoneyTransfers();
                triggerDeedFlyToPlayer(pos, actorId);
                scheduleAIIfNeeded(1800);
            } else {
                m_game->startAuctionForCurrentProperty();
                scheduleAIIfNeeded(1600);
            }
            break;
        }

        case TurnPhase::Auction: {
            int bid = m_ai->chooseAuctionBid(*m_game, actorId);
            int propIdx = m_game->auctionState().spaceIndex;
            if (bid > 0) {
                m_game->placeAuctionBid(actorId, bid);
            } else {
                m_game->foldAuction(actorId);
            }
            if (m_game->phase() != TurnPhase::Auction) {
                processRecentMoneyTransfers();
                int finalWinner = m_game->board().space(propIdx).ownerId;
                if (finalWinner >= 0) {
                    triggerDeedFlyToPlayer(propIdx, finalWinner);
                }
            }
            scheduleAIIfNeeded(1600);
            break;
        }

        case TurnPhase::RentPaymentModal: {
            m_game->dismissRentModal();
            processRecentMoneyTransfers();
            scheduleAIIfNeeded(1800);
            break;
        }

        case TurnPhase::CardModal: {
            int prevPos = m_game->player(actorId).position;
            m_game->dismissCardModal();
            if (m_game->player(actorId).position != prevPos && m_game->lastMovePath().size() > 1) {
                m_tokenAnimPlayerId = actorId;
                startMovePathAnimation(m_game->lastMovePath(), m_game->lastMoveWasJailSend());
            } else {
                processRecentMoneyTransfers();
                scheduleAIIfNeeded(1800);
            }
            break;
        }

        case TurnPhase::DebtResolution: {
            m_ai->resolveDebt(*m_game, actorId);
            processRecentMoneyTransfers();
            scheduleAIIfNeeded(1800);
            break;
        }

        case TurnPhase::WaitingForEndTurn: {
            m_ai->performTurnManagement(*m_game, actorId);
            processRecentMoneyTransfers();
            if (m_game->canRollAgain()) {
                triggerRollAndMoveAnimation();
            } else {
                m_game->endTurn();
                m_expandedPlayerId = m_game->activePlayerIndex();
                scheduleAIIfNeeded(1500);
            }
            break;
        }

        default:
            break;
    }

    emit gameStateUpdated();
    update();
}

// -----------------------------------------------------------------------------
// Top-level Paint Event
// -----------------------------------------------------------------------------
void MonopolyWidget::paintEvent(QPaintEvent * /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    m_buttons.clear();

    if (m_screen == ViewScreen::LobbySetup) {
        paintLobbySetup(p);
    } else if (m_screen == ViewScreen::TokenSelect) {
        paintTokenSelect(p);
    } else {
        paintInGame(p);
    }

    if (m_showRulesOverlay) {
        paintRulesCarouselOverlay(p);
    }
}

// -----------------------------------------------------------------------------
// Reusable Vector Art & Gold/Obsidian/White UI Primitives
// -----------------------------------------------------------------------------
void MonopolyWidget::paintGoldButton(QPainter& p, const QString& id, const QRectF& rect,
                                     const QString& text, bool enabled, bool primary, int fontPx) {
    m_buttons.push_back({id, rect, enabled});
    bool hovered = enabled && (m_hoveredButtonId == id);

    p.save();
    QLinearGradient bg(rect.topLeft(), rect.bottomLeft());
    if (!enabled) {
        if (m_isDarkMode) {
            bg.setColorAt(0.0, QColor("#1F1F24"));
            bg.setColorAt(1.0, QColor("#141418"));
        } else {
            bg.setColorAt(0.0, QColor("#E4E2DC"));
            bg.setColorAt(1.0, QColor("#D5D2C8"));
        }
    } else if (primary) {
        if (hovered) {
            bg.setColorAt(0.0, QColor("#FCE89C"));
            bg.setColorAt(0.5, QColor("#E2BC42"));
            bg.setColorAt(1.0, QColor("#B88917"));
        } else {
            bg.setColorAt(0.0, QColor("#F3E5AB"));
            bg.setColorAt(0.5, QColor("#D4AF37"));
            bg.setColorAt(1.0, QColor("#9E7612"));
        }
    } else {
        if (m_isDarkMode) {
            if (hovered) {
                bg.setColorAt(0.0, QColor("#2E2A1E"));
                bg.setColorAt(1.0, QColor("#1D1A12"));
            } else {
                bg.setColorAt(0.0, QColor("#22201A"));
                bg.setColorAt(1.0, QColor("#141310"));
            }
        } else {
            if (hovered) {
                bg.setColorAt(0.0, QColor("#FFFDF5"));
                bg.setColorAt(1.0, QColor("#EFE4C2"));
            } else {
                bg.setColorAt(0.0, QColor("#FFFFFF"));
                bg.setColorAt(1.0, QColor("#F3EBD3"));
            }
        }
    }

    QColor borderCol = !enabled ? QColor("#6E6A5E") :
                       (hovered ? QColor("#FFE885") : QColor("#D4AF37"));
    p.setPen(QPen(borderCol, hovered ? 2.0 : 1.5));
    p.setBrush(bg);
    p.drawRoundedRect(rect, 7.0, 7.0);

    // Subtle inner bevel highlight
    if (enabled) {
        p.setPen(QPen(QColor(255, 255, 255, m_isDarkMode ? 40 : 140), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect.adjusted(1.5, 1.5, -1.5, -1.5), 5.5, 5.5);
    }

    QFont f = p.font();
    f.setPixelSize(fontPx);
    f.setBold(true);
    p.setFont(f);

    QColor textCol;
    if (!enabled) {
        textCol = m_isDarkMode ? QColor("#5E5D59") : QColor("#9E9B91");
    } else if (primary) {
        textCol = QColor("#11100C");
    } else {
        textCol = m_isDarkMode ? QColor("#F5E7B2") : QColor("#241E0E");
    }
    p.setPen(textCol);
    p.drawText(rect.adjusted(4, 0, -4, 0), Qt::AlignCenter | Qt::TextWordWrap, text);
    p.restore();
}

void MonopolyWidget::paintModalFrame(QPainter& p, const QRectF& rect, const QString& headerTitle) {
    p.save();

    // Outer drop shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 120));
    p.drawRoundedRect(rect.translated(0, 5), 16.0, 16.0);

    // Main modal body
    QLinearGradient bg(rect.topLeft(), rect.bottomLeft());
    if (m_isDarkMode) {
        bg.setColorAt(0.0, QColor("#1A191E"));
        bg.setColorAt(0.5, QColor("#121216"));
        bg.setColorAt(1.0, QColor("#0B0B0E"));
    } else {
        bg.setColorAt(0.0, QColor("#FFFFFF"));
        bg.setColorAt(0.5, QColor("#FCF9F0"));
        bg.setColorAt(1.0, QColor("#F4ECD6"));
    }

    // Double Metallic Gold border (characteristic of the online version's rounded frames + Mega Gold theme)
    p.setPen(QPen(QColor("#D4AF37"), 3.0));
    p.setBrush(bg);
    p.drawRoundedRect(rect, 15.0, 15.0);

    p.setPen(QPen(m_isDarkMode ? QColor("#8A6D1B") : QColor("#C59B27"), 1.2));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect.adjusted(5, 5, -5, -5), 11.0, 11.0);

    if (!headerTitle.isEmpty()) {
        QFont hf = p.font();
        hf.setPixelSize(20);
        hf.setBold(true);
        p.setFont(hf);
        p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#8A6608"));
        QRectF titleRect(rect.left() + 16, rect.top() + 12, rect.width() - 32, 28);
        p.drawText(titleRect, Qt::AlignCenter, headerTitle);
    }

    p.restore();
}

void MonopolyWidget::paintTokenIcon(QPainter& p, const QRectF& rect, TokenType token, bool withBox) {
    p.save();

    if (withBox) {
        QLinearGradient boxGrad(rect.topLeft(), rect.bottomRight());
        if (m_isDarkMode) {
            boxGrad.setColorAt(0.0, QColor("#2B2618"));
            boxGrad.setColorAt(0.5, QColor("#191710"));
            boxGrad.setColorAt(1.0, QColor("#100E0A"));
        } else {
            boxGrad.setColorAt(0.0, QColor("#FFFDF7"));
            boxGrad.setColorAt(0.5, QColor("#F7EED2"));
            boxGrad.setColorAt(1.0, QColor("#E8D6A0"));
        }
        p.setPen(QPen(QColor("#D4AF37"), 1.6));
        p.setBrush(boxGrad);
        p.drawRoundedRect(rect, 7.0, 7.0);
    }

    QRectF r = withBox ? rect.adjusted(4, 4, -4, -4) : rect;
    int tokIdx = std::clamp(static_cast<int>(token), 0, 7);
    const QPixmap& px = m_tokenPixmaps[tokIdx];

    if (!px.isNull()) {
        p.drawPixmap(r.toRect(), px);
        p.restore();
        return;
    }

    // Fallback vector drawing if pixmap not loaded
    QLinearGradient goldGrad(r.topLeft(), r.bottomRight());
    goldGrad.setColorAt(0.0, QColor("#FFF4C2"));
    goldGrad.setColorAt(0.5, QColor("#D4AF37"));
    goldGrad.setColorAt(1.0, QColor("#7D5D0B"));
    p.setPen(QPen(QColor("#523B04"), 1.3));
    p.setBrush(goldGrad);
    p.drawEllipse(r);
    p.restore();
}

void MonopolyWidget::paintMrMonopoly(QPainter& p, const QRectF& rect) {
    p.save();
    double cx = rect.center().x();
    double cy = rect.center().y();
    double s = std::min(rect.width(), rect.height()) * 0.42;

    // Golden aura burst behind Mr. Monopoly
    QRadialGradient aura(QPointF(cx, cy), s * 1.15);
    aura.setColorAt(0.0, QColor(212, 175, 55, 75));
    aura.setColorAt(0.6, QColor(212, 175, 55, 25));
    aura.setColorAt(1.0, Qt::transparent);
    p.setPen(Qt::NoPen);
    p.setBrush(aura);
    p.drawEllipse(QPointF(cx, cy), s * 1.15, s * 1.15);

    // Tuxedo body
    p.setPen(QPen(QColor("#D4AF37"), 1.6));
    p.setBrush(QColor("#18181C"));
    QPainterPath suit;
    suit.moveTo(cx - s * 0.38, cy + s * 0.82);
    suit.quadTo(cx - s * 0.42, cy + s * 0.18, cx, cy + s * 0.15);
    suit.quadTo(cx + s * 0.42, cy + s * 0.18, cx + s * 0.38, cy + s * 0.82);
    suit.closeSubpath();
    p.drawPath(suit);

    // White shirt V & gold bowtie
    p.setBrush(QColor("#FFFFFF"));
    QPolygonF shirt;
    shirt << QPointF(cx - s * 0.15, cy + s * 0.18)
          << QPointF(cx + s * 0.15, cy + s * 0.18)
          << QPointF(cx, cy + s * 0.52);
    p.drawPolygon(shirt);

    p.setBrush(QColor("#E32929"));
    p.drawEllipse(QPointF(cx, cy + s * 0.25), s * 0.08, s * 0.04);

    // Head
    p.setBrush(QColor("#FAD6B8"));
    p.setPen(QPen(QColor("#C59B27"), 1.4));
    p.drawEllipse(QPointF(cx, cy - s * 0.08), s * 0.32, s * 0.30);

    // Eyes
    p.setBrush(QColor("#18181B"));
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(cx - s * 0.11, cy - s * 0.12), s * 0.035, s * 0.035);
    p.drawEllipse(QPointF(cx + s * 0.11, cy - s * 0.12), s * 0.035, s * 0.035);

    // Iconic white handlebars mustache
    p.setBrush(QColor("#FFFFFF"));
    p.setPen(QPen(QColor("#A1A1AA"), 1.0));
    QPainterPath stache;
    stache.moveTo(cx, cy - s * 0.01);
    stache.quadTo(cx - s * 0.18, cy - s * 0.08, cx - s * 0.30, cy + s * 0.01);
    stache.quadTo(cx - s * 0.16, cy + s * 0.10, cx, cy + s * 0.03);
    stache.quadTo(cx + s * 0.16, cy + s * 0.10, cx + s * 0.30, cy + s * 0.01);
    stache.quadTo(cx + s * 0.18, cy - s * 0.08, cx, cy - s * 0.01);
    p.drawPath(stache);

    // Black & Gold Top Hat
    p.setBrush(QColor("#141418"));
    p.setPen(QPen(QColor("#D4AF37"), 1.6));
    p.drawEllipse(QRectF(cx - s * 0.42, cy - s * 0.40, s * 0.84, s * 0.14));
    p.drawRoundedRect(QRectF(cx - s * 0.28, cy - s * 0.82, s * 0.56, s * 0.46), 4, 4);
    p.setBrush(QColor("#D4AF37"));
    p.drawRect(QRectF(cx - s * 0.28, cy - s * 0.44, s * 0.56, s * 0.07));

    // Golden Cane
    p.setPen(QPen(QColor("#D4AF37"), 2.5, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(cx + s * 0.42, cy + s * 0.30), QPointF(cx + s * 0.26, cy + s * 0.88));

    p.restore();
}

void MonopolyWidget::paint3DDie(QPainter& p, const QRectF& rect, int value, double angleDeg) {
    p.save();
    p.translate(rect.center());
    if (std::abs(angleDeg) > 0.01) {
        p.rotate(angleDeg);
    }
    QRectF r(-rect.width() / 2.0, -rect.height() / 2.0, rect.width(), rect.height());

    // Drop shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 90));
    p.drawRoundedRect(r.translated(2, 3), r.width() * 0.22, r.height() * 0.22);

    // Die body (Ivory Pearl with Gold border & Gold/Black pips)
    QLinearGradient g(r.topLeft(), r.bottomRight());
    if (m_isDarkMode) {
        g.setColorAt(0.0, QColor("#262420"));
        g.setColorAt(0.5, QColor("#181612"));
        g.setColorAt(1.0, QColor("#0E0D0A"));
    } else {
        g.setColorAt(0.0, QColor("#FFFFFF"));
        g.setColorAt(0.6, QColor("#F8F4E6"));
        g.setColorAt(1.0, QColor("#E5DAC0"));
    }
    p.setPen(QPen(QColor("#D4AF37"), 1.6));
    p.setBrush(g);
    p.drawRoundedRect(r, r.width() * 0.20, r.height() * 0.20);

    // Pips
    p.setPen(Qt::NoPen);
    p.setBrush(m_isDarkMode ? QColor("#F3E5AB") : QColor("#18181B"));
    double pr = r.width() * 0.095;
    double off = r.width() * 0.25;

    auto drawPip = [&](double dx, double dy) {
        p.drawEllipse(QPointF(dx, dy), pr, pr);
    };

    int v = std::clamp(value, 1, 6);
    if (v % 2 == 1) drawPip(0, 0);
    if (v >= 2) { drawPip(-off, -off); drawPip(off, off); }
    if (v >= 4) { drawPip(off, -off); drawPip(-off, off); }
    if (v == 6) { drawPip(-off, 0); drawPip(off, 0); }

    p.restore();
}

void MonopolyWidget::paintHouseIcon(QPainter& p, const QRectF& rect, bool isHotel) {
    p.save();
    QColor mainCol = isHotel ? QColor("#DC2626") : QColor("#16A34A");
    QColor lightCol = isHotel ? QColor("#F87171") : QColor("#4ADE80");
    QColor darkCol = isHotel ? QColor("#7F1D1D") : QColor("#14532D");

    double x = rect.left(), y = rect.top(), w = rect.width(), h = rect.height();

    // 3D Isometric House / Hotel matching screenshots 182347 & 182404
    QPainterPath wall;
    wall.addRect(x + w * 0.16, y + h * 0.44, w * 0.68, h * 0.46);
    QLinearGradient wg(x, y + h * 0.44, x + w, y + h * 0.90);
    wg.setColorAt(0.0, mainCol);
    wg.setColorAt(1.0, darkCol);
    p.setPen(QPen(QColor("#111111"), 1.0));
    p.setBrush(wg);
    p.drawPath(wall);

    // Pitched roof
    QPainterPath roof;
    roof.moveTo(x + w * 0.08, y + h * 0.46);
    roof.lineTo(x + w * 0.50, y + h * 0.12);
    roof.lineTo(x + w * 0.92, y + h * 0.46);
    roof.closeSubpath();
    QLinearGradient rg(x, y + h * 0.12, x, y + h * 0.46);
    rg.setColorAt(0.0, lightCol);
    rg.setColorAt(1.0, mainCol);
    p.setBrush(rg);
    p.drawPath(roof);

    // Chimney
    p.setBrush(lightCol);
    p.drawEllipse(QPointF(x + w * 0.38, y + h * 0.25), w * 0.07, h * 0.05);

    p.restore();
}

void MonopolyWidget::paintLocomotiveIcon(QPainter& p, const QRectF& rect, const QColor& col) {
    p.save();
    double x = rect.left(), y = rect.top(), w = rect.width(), h = rect.height();
    p.setPen(QPen(col, 1.2));
    p.setBrush(col);

    // Boiler & Cabin
    p.drawRect(QRectF(x + w * 0.18, y + h * 0.40, w * 0.44, h * 0.26));
    p.drawRect(QRectF(x + w * 0.58, y + h * 0.22, w * 0.26, h * 0.44));
    // Smokestack
    QPolygonF stack;
    stack << QPointF(x + w * 0.22, y + h * 0.40)
          << QPointF(x + w * 0.16, y + h * 0.18)
          << QPointF(x + w * 0.32, y + h * 0.18)
          << QPointF(x + w * 0.28, y + h * 0.40);
    p.drawPolygon(stack);
    // Cowcatcher
    QPolygonF front;
    front << QPointF(x + w * 0.18, y + h * 0.50)
          << QPointF(x + w * 0.06, y + h * 0.68)
          << QPointF(x + w * 0.18, y + h * 0.68);
    p.drawPolygon(front);
    // Wheels
    p.drawEllipse(QPointF(x + w * 0.28, y + h * 0.74), w * 0.10, w * 0.10);
    p.drawEllipse(QPointF(x + w * 0.50, y + h * 0.74), w * 0.10, w * 0.10);
    p.drawEllipse(QPointF(x + w * 0.72, y + h * 0.74), w * 0.12, w * 0.12);
    p.restore();
}

void MonopolyWidget::paintChestIcon(QPainter& p, const QRectF& rect) {
    p.save();
    double x = rect.left(), y = rect.top(), w = rect.width(), h = rect.height();
    p.setPen(QPen(QColor("#D4AF37"), 1.4));
    p.setBrush(QColor("#1D70B8"));
    // Chest base
    p.drawRoundedRect(QRectF(x + w * 0.15, y + h * 0.42, w * 0.70, h * 0.42), 3, 3);
    // Open lid & golden coins inside
    p.setBrush(QColor("#F3E5AB"));
    p.drawEllipse(QRectF(x + w * 0.25, y + h * 0.30, w * 0.50, h * 0.22));
    p.setBrush(QColor("#38BDF8"));
    p.drawChord(QRectF(x + w * 0.15, y + h * 0.14, w * 0.70, h * 0.42), 0, 180 * 16);
    p.restore();
}

void MonopolyWidget::paintQuestionIcon(QPainter& p, const QRectF& rect, const QColor& col) {
    p.save();
    QFont f = p.font();
    f.setPixelSize(static_cast<int>(rect.height() * 0.88));
    f.setBold(true);
    p.setFont(f);
    p.setPen(col);
    p.drawText(rect, Qt::AlignCenter, "?");
    p.restore();
}

void MonopolyWidget::paintLightbulbIcon(QPainter& p, const QRectF& rect) {
    p.save();
    double cx = rect.center().x(), cy = rect.center().y();
    double r = std::min(rect.width(), rect.height()) * 0.30;
    p.setPen(QPen(QColor("#D4AF37"), 1.5));
    p.setBrush(QColor("#FACC15"));
    p.drawEllipse(QPointF(cx, cy - r * 0.2), r, r);
    p.setBrush(QColor("#A1A1AA"));
    p.drawRect(QRectF(cx - r * 0.45, cy + r * 0.65, r * 0.9, r * 0.5));
    p.restore();
}

void MonopolyWidget::paintWaterTapIcon(QPainter& p, const QRectF& rect) {
    p.save();
    double x = rect.left(), y = rect.top(), w = rect.width(), h = rect.height();
    p.setPen(QPen(QColor("#D4AF37"), 2.0, Qt::SolidLine, Qt::RoundCap));
    p.drawLine(QPointF(x + w * 0.2, y + h * 0.45), QPointF(x + w * 0.68, y + h * 0.45));
    p.drawLine(QPointF(x + w * 0.68, y + h * 0.45), QPointF(x + w * 0.68, y + h * 0.62));
    p.drawLine(QPointF(x + w * 0.45, y + h * 0.26), QPointF(x + w * 0.45, y + h * 0.45));
    p.drawLine(QPointF(x + w * 0.32, y + h * 0.26), QPointF(x + w * 0.58, y + h * 0.26));
    // Water drop
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#38BDF8"));
    p.drawEllipse(QPointF(x + w * 0.68, y + h * 0.78), w * 0.08, h * 0.11);
    p.restore();
}

void MonopolyWidget::paintRingIcon(QPainter& p, const QRectF& rect) {
    p.save();
    double cx = rect.center().x(), cy = rect.center().y();
    double r = std::min(rect.width(), rect.height()) * 0.28;
    p.setPen(QPen(QColor("#D4AF37"), 2.4));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QPointF(cx, cy + r * 0.2), r, r);
    // Diamond gem on top
    p.setPen(QPen(QColor("#38BDF8"), 1.2));
    p.setBrush(QColor("#E0F2FE"));
    QPolygonF gem;
    gem << QPointF(cx, cy - r * 0.65)
        << QPointF(cx + r * 0.5, cy - r * 1.15)
        << QPointF(cx, cy - r * 1.45)
        << QPointF(cx - r * 0.5, cy - r * 1.15);
    p.drawPolygon(gem);
    p.restore();
}

// -----------------------------------------------------------------------------
// Screen 1: Lobby Setup Screen (Replicating swappy-20261120-182034.png in Gold Theme)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintLobbySetup(QPainter& p) {
    QRectF full = rect();

    // Background
    QLinearGradient bg(full.topLeft(), full.bottomLeft());
    if (m_isDarkMode) {
        bg.setColorAt(0.0, QColor("#121216"));
        bg.setColorAt(0.6, QColor("#0C0C0F"));
        bg.setColorAt(1.0, QColor("#18150E"));
    } else {
        bg.setColorAt(0.0, QColor("#FFFFFF"));
        bg.setColorAt(0.6, QColor("#FAF6EA"));
        bg.setColorAt(1.0, QColor("#EFE5C8"));
    }
    p.fillRect(full, bg);

    // Outer Gold frame
    p.setPen(QPen(QColor("#D4AF37"), 2.5));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(full.adjusted(4, 4, -4, -4), 10, 10);

    // Top Header Stripe + MONOPOLY Gold Logo Box
    QRectF topStripe(full.left() + 6, full.top() + 18, full.width() - 12, 26);
    QLinearGradient stripeGrad(topStripe.left(), topStripe.top(), topStripe.right(), topStripe.top());
    stripeGrad.setColorAt(0.0, QColor(212, 175, 55, 40));
    stripeGrad.setColorAt(0.5, QColor(212, 175, 55, 140));
    stripeGrad.setColorAt(1.0, QColor(212, 175, 55, 40));
    p.fillRect(topStripe, stripeGrad);
    p.setPen(QPen(QColor("#D4AF37"), 1.5));
    p.drawLine(topStripe.topLeft(), topStripe.topRight());
    p.drawLine(topStripe.bottomLeft(), topStripe.bottomRight());

    QRectF logoBox(full.center().x() - 165, full.top() + 8, 330, 54);
    p.setPen(QPen(QColor("#D4AF37"), 3.0));
    p.setBrush(m_isDarkMode ? QColor("#0B0B0E") : QColor("#181612"));
    p.drawRoundedRect(logoBox, 6, 6);
    p.setPen(QPen(QColor("#F3E5AB"), 1.2));
    p.drawRoundedRect(logoBox.adjusted(4, 4, -4, -4), 4, 4);

    QFont logoFont = p.font();
    logoFont.setPixelSize(32);
    logoFont.setBold(true);
    logoFont.setLetterSpacing(QFont::AbsoluteSpacing, 3.0);
    p.setFont(logoFont);
    p.setPen(QColor("#F3E5AB"));
    p.drawText(logoBox, Qt::AlignCenter, "MONOPOLY");

    // If only 2 players, draw Mr. Monopoly in the center between the two player cards (just like 182034.png!)
    int nPlayers = static_cast<int>(m_setups.size());
    if (nPlayers == 2) {
        QRectF mrRect(full.center().x() - 105, full.top() + 90, 210, 360);
        paintMrMonopoly(p, mrRect);
    }

    // Player Setup Cards (2 to 4 cards with arched token dome at the top!)
    double cardW = (nPlayers <= 2) ? 215.0 : std::min(200.0, (full.width() - 100.0) / nPlayers - 16.0);
    double cardH = 340.0;
    double cardY = full.top() + 115.0;

    for (int i = 0; i < nPlayers; ++i) {
        double cardX = 0.0;
        if (nPlayers == 2) {
            cardX = (i == 0) ? (full.center().x() - 125.0 - cardW) : (full.center().x() + 125.0);
        } else {
            double totalW = nPlayers * cardW + (nPlayers - 1) * 18.0;
            cardX = full.center().x() - totalW / 2.0 + i * (cardW + 18.0);
        }

        QRectF cRect(cardX, cardY, cardW, cardH);

        // Arched top dome for token preview
        QRectF domeRect(cRect.center().x() - 48, cRect.top() - 36, 96, 72);
        p.setPen(QPen(QColor("#D4AF37"), 2.2));
        p.setBrush(m_isDarkMode ? QColor("#18171C") : QColor("#F5EED8"));
        p.drawChord(domeRect, 0, 180 * 16);

        // Main card frame
        paintModalFrame(p, cRect);
        paintTokenIcon(p, QRectF(cRect.center().x() - 16, cRect.top() - 30, 32, 26), m_setups[i].token, false);

        // Card header bar with Player Name
        QRectF hdrRect(cRect.left() + 8, cRect.top() + 8, cRect.width() - 16, 30);
        p.setPen(QPen(QColor("#D4AF37"), 1.2));
        p.setBrush(m_isDarkMode ? QColor("#221F17") : QColor("#EFE4C2"));
        p.drawRoundedRect(hdrRect, 6, 6);

        QFont hf = p.font();
        hf.setPixelSize(14);
        hf.setBold(true);
        p.setFont(hf);
        p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#241E0E"));
        p.drawText(hdrRect, Qt::AlignCenter, setupPlayerDisplayName(i));

        // Human vs Computer toggle buttons
        QRectF humBtn(cRect.left() + 16, hdrRect.bottom() + 14, cRect.width() - 32, 30);
        paintGoldButton(p, QString("setup_human_%1").arg(i), humBtn,
                        trText("mono_human_player"), true, !m_setups[i].isAI, 11);

        QRectF aiBtn(cRect.left() + 16, humBtn.bottom() + 8, cRect.width() - 32, 30);
        paintGoldButton(p, QString("setup_ai_%1").arg(i), aiBtn,
                        trText("mono_computer_player"), true, m_setups[i].isAI, 11);

        if (m_setups[i].isAI) {
            // Difficulty selector (Agent immobilier / Entrepreneur / Magnat de l'immobilier)
            QFont df = p.font();
            df.setPixelSize(11);
            df.setBold(false);
            p.setFont(df);
            p.setPen(m_isDarkMode ? QColor("#D4AF37") : QColor("#7A5B08"));
            p.drawText(QRectF(cRect.left() + 12, aiBtn.bottom() + 6, cRect.width() - 24, 18),
                       Qt::AlignCenter, trText("mono_choose_diff"));

            const QString diffKeys[3] = {"mono_diff_easy", "mono_diff_medium", "mono_diff_hard"};
            for (int d = 0; d < 3; ++d) {
                QRectF dRect(cRect.left() + 16, aiBtn.bottom() + 26 + d * 26, cRect.width() - 32, 22);
                bool sel = (static_cast<int>(m_setups[i].difficulty) == d);
                paintGoldButton(p, QString("setup_diff_%1_%2").arg(i).arg(d), dRect,
                                trText(diffKeys[d]), true, sel, 10);
            }
        } else {
            // Hint text for human player
            QFont Tf = p.font();
            Tf.setPixelSize(11);
            Tf.setBold(false);
            p.setFont(Tf);
            p.setPen(m_isDarkMode ? QColor("#D4D4D8") : QColor("#3F3F46"));
            p.drawText(QRectF(cRect.left() + 16, aiBtn.bottom() + 18, cRect.width() - 32, 75),
                       Qt::AlignCenter | Qt::TextWordWrap, trText("mono_click_token_hint"));
        }

        // Token choice square button at bottom of card
        QRectF tokBtn(cRect.center().x() - 38, cRect.bottom() - 86, 76, 72);
        m_buttons.push_back({QString("setup_pick_token_%1").arg(i), tokBtn, true});
        paintTokenIcon(p, tokBtn, m_setups[i].token, true);
        if (m_hoveredButtonId == QString("setup_pick_token_%1").arg(i)) {
            p.setPen(QPen(QColor("#FFE885"), 2.5));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(tokBtn, 7, 7);
        }

        // If player >= 2, show SUPPRIMER button below card
        if (i >= 2) {
            QRectF remBtn(cRect.center().x() - 55, cRect.bottom() + 6, 110, 26);
            paintGoldButton(p, QString("setup_remove_%1").arg(i), remBtn,
                            trText("mono_remove_player"), true, false, 10);
        }
    }

    // Bottom Navigation Bar (RETOUR | RÈGLES | AJOUTER JOUEUR | JOUER)
    QRectF botBar(full.left() + 8, full.bottom() - 68, full.width() - 16, 54);
    p.fillRect(botBar, m_isDarkMode ? QColor(24, 21, 15, 220) : QColor(242, 233, 208, 230));
    p.setPen(QPen(QColor("#D4AF37"), 1.5));
    p.drawLine(botBar.topLeft(), botBar.topRight());
    p.drawLine(botBar.bottomLeft(), botBar.bottomRight());

    double bw = 175.0, bh = 38.0;
    double spacing = (botBar.width() - 4 * bw) / 5.0;
    double by = botBar.center().y() - bh / 2.0;

    paintGoldButton(p, "lobby_back", QRectF(botBar.left() + spacing, by, bw, bh),
                    trText("mono_back"), true, false, 13);
    paintGoldButton(p, "lobby_rules", QRectF(botBar.left() + spacing * 2 + bw, by, bw, bh),
                    trText("mono_rules_btn"), true, false, 13);
    paintGoldButton(p, "lobby_add_player", QRectF(botBar.left() + spacing * 3 + bw * 2, by, bw, bh),
                    trText("mono_add_player"), nPlayers < 4, false, 13);
    paintGoldButton(p, "lobby_play", QRectF(botBar.left() + spacing * 4 + bw * 3, by, bw, bh),
                    trText("mono_play_btn"), true, true, 14);
}

// -----------------------------------------------------------------------------
// Screen 2: Token Selection Screen (Replicating swappy-20261120-182452.png)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintTokenSelect(QPainter& p) {
    QRectF full = rect();
    QLinearGradient bg(full.topLeft(), full.bottomLeft());
    if (m_isDarkMode) {
        bg.setColorAt(0.0, QColor("#121216"));
        bg.setColorAt(1.0, QColor("#1A1710"));
    } else {
        bg.setColorAt(0.0, QColor("#FFFFFF"));
        bg.setColorAt(1.0, QColor("#F3EBD3"));
    }
    p.fillRect(full, bg);

    // Top banner: CHOISISSEZ UN PION
    QRectF hdrBar(full.left() + 6, full.top() + 24, full.width() - 12, 38);
    p.fillRect(hdrBar, m_isDarkMode ? QColor("#201D14") : QColor("#EFE4C2"));
    p.setPen(QPen(QColor("#D4AF37"), 1.8));
    p.drawLine(hdrBar.topLeft(), hdrBar.topRight());
    p.drawLine(hdrBar.bottomLeft(), hdrBar.bottomRight());

    QFont hf = p.font();
    hf.setPixelSize(20);
    hf.setBold(true);
    p.setFont(hf);
    p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#7A5B08"));
    p.drawText(hdrBar, Qt::AlignCenter, trText("mono_choose_token_title"));

    // Left column: Player cards
    int nPlayers = static_cast<int>(m_setups.size());
    for (int i = 0; i < nPlayers; ++i) {
        QRectF pCard(full.left() + 18, hdrBar.bottom() + 18 + i * 118, 195, 108);
        paintModalFrame(p, pCard);
        if (i == m_editingPlayerIdx) {
            p.setPen(QPen(QColor("#FFE885"), 2.8));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(pCard, 15, 15);
        }
        m_buttons.push_back({QString("tokscreen_player_%1").arg(i), pCard, true});

        QRectF tBox(pCard.center().x() - 32, pCard.top() + 12, 64, 58);
        paintTokenIcon(p, tBox, m_setups[i].token, true);

        QFont pf = p.font();
        pf.setPixelSize(13);
        pf.setBold(true);
        p.setFont(pf);
        p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#241E0E"));
        p.drawText(QRectF(pCard.left(), tBox.bottom() + 4, pCard.width(), 24),
                   Qt::AlignCenter, setupPlayerDisplayName(i));
    }

    // Center large frame: PIONS DISPONIBLES
    QRectF centerBox(full.left() + 235, hdrBar.bottom() + 20, full.width() - 275, 360);
    paintModalFrame(p, centerBox, trText("mono_free_tokens"));

    // 8 Golden Tokens grid (2 rows of 4 or 1 row of 5 + 3)
    double tSize = 84.0;
    double gap = 22.0;
    double startX = centerBox.center().x() - (4 * tSize + 3 * gap) / 2.0;
    double startY = centerBox.top() + 72.0;

    for (int t = 0; t < 8; ++t) {
        int row = t / 4;
        int col = t % 4;
        QRectF tRect(startX + col * (tSize + gap), startY + row * (tSize + 34), tSize, tSize);
        TokenType tok = static_cast<TokenType>(t);

        bool takenByOther = false;
        for (int i = 0; i < nPlayers; ++i) {
            if (i != m_editingPlayerIdx && m_setups[i].token == tok) {
                takenByOther = true;
            }
        }

        m_buttons.push_back({QString("tokscreen_choose_%1").arg(t), tRect, !takenByOther});
        paintTokenIcon(p, tRect, tok, true);

        if (m_setups[m_editingPlayerIdx].token == tok) {
            p.setPen(QPen(QColor("#FFE885"), 3.0));
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(tRect.adjusted(-2, -2, 2, 2), 8, 8);
        } else if (takenByOther) {
            p.fillRect(tRect, QColor(0, 0, 0, 130));
        }

        QFont tf = p.font();
        tf.setPixelSize(11);
        tf.setBold(true);
        p.setFont(tf);
        p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#241E0E"));
        p.drawText(QRectF(tRect.left() - 10, tRect.bottom() + 4, tRect.width() + 20, 20),
                   Qt::AlignCenter, tokenName(tok));
    }

    // RETOUR button inside centerBox bottom-left
    QRectF backBtn(centerBox.left() + 20, centerBox.bottom() - 48, 120, 32);
    paintGoldButton(p, "tokscreen_back", backBtn, trText("mono_back"), true, false, 12);

    // Mr. Monopoly on bottom right
    paintMrMonopoly(p, QRectF(full.right() - 170, full.bottom() - 230, 155, 210));
}

// -----------------------------------------------------------------------------
// Screen 3: In-Game Screen (Board + Right Sidebar + Bottom Bar + Modals)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintInGame(QPainter& p) {
    QRectF full = rect();

    // Ambient outer container surface (Warm Bronze-Gold border so black board never merges with black background!)
    QLinearGradient outerBg(full.topLeft(), full.bottomRight());
    if (m_isDarkMode) {
        outerBg.setColorAt(0.0, QColor("#262014"));
        outerBg.setColorAt(0.5, QColor("#1B170F"));
        outerBg.setColorAt(1.0, QColor("#282114"));
    } else {
        outerBg.setColorAt(0.0, QColor("#F4ECD6"));
        outerBg.setColorAt(0.5, QColor("#EFE4C6"));
        outerBg.setColorAt(1.0, QColor("#E5D5AC"));
    }
    p.fillRect(full, outerBg);
    p.setPen(QPen(QColor("#D4AF37"), 2.0));
    p.setBrush(Qt::NoBrush);
    p.drawRect(full.adjusted(1, 1, -1, -1));

    QRectF bRect = boardOuterRect();
    QRectF sRect = rightPanelRect();
    QRectF barRect = bottomBarRect();

    paintBoard(p, bRect);
    paintRightSidebar(p, sRect);
    paintBottomActionBar(p, barRect);

    // Center turn modals or action overlays
    QRectF cRect = boardCenterRect();
    if (m_overlayMode != ActionOverlayMode::None) {
        if (m_overlayMode == ActionOverlayMode::Trade) {
            paintTradeOverlay(p, cRect);
        } else {
            paintActionOverlay(p, cRect);
        }
    } else {
        paintCenterTurnModals(p, cRect);
    }

    // Active animations (dice tumbling, card flying/flipping, floating money)
    paintAnimationsOverlay(p);
}

void MonopolyWidget::paintBoard(QPainter& p, const QRectF& bRect) {
    p.save();

    // Outer double gold frame around the board
    p.setPen(QPen(QColor("#D4AF37"), 3.0));
    p.setBrush(m_isDarkMode ? QColor("#0B0B0D") : QColor("#FAF8F2"));
    p.drawRect(bRect);

    // Paint center area first
    QRectF cRect = boardCenterRect();
    paintBoardCenter(p, cRect);

    // Paint all 40 spaces
    for (int i = 0; i < MonopolyBoard::NUM_SPACES; ++i) {
        QRectF sr = spaceRect(i);
        if (i == 0 || i == 10 || i == 20 || i == 30) {
            paintCornerTile(p, i, sr);
        } else {
            paintSpaceTile(p, i, sr);
        }
    }

    // Paint player tokens on the board
    paintTokensOnBoard(p);

    p.restore();
}

void MonopolyWidget::paintBoardCenter(QPainter& p, const QRectF& cRect) {
    p.save();

    // Center fill: Deep Obsidian in Dark Mode, Crisp Pearl White in Light Mode
    QRadialGradient centerGrad(cRect.center(), cRect.width() * 0.65);
    if (m_isDarkMode) {
        centerGrad.setColorAt(0.0, QColor("#171510"));
        centerGrad.setColorAt(0.7, QColor("#0D0D10"));
        centerGrad.setColorAt(1.0, QColor("#09090B"));
    } else {
        centerGrad.setColorAt(0.0, QColor("#FFFFFF"));
        centerGrad.setColorAt(0.7, QColor("#FCF9F0"));
        centerGrad.setColorAt(1.0, QColor("#F3EBD3"));
    }
    p.fillRect(cRect, centerGrad);
    p.setPen(QPen(QColor("#D4AF37"), 2.0));
    p.setBrush(Qt::NoBrush);
    p.drawRect(cRect);

    // 1. Motto above MONOPOLY banner
    QFont mf = p.font();
    mf.setPixelSize(std::max(10, static_cast<int>(cRect.width() * 0.025)));
    mf.setBold(true);
    p.setFont(mf);
    p.setPen(m_isDarkMode ? QColor("#D4AF37") : QColor("#8A6608"));
    QRectF mottoRect(cRect.left(), cRect.top() + cRect.height() * 0.08, cRect.width(), 22);
    p.drawText(mottoRect, Qt::AlignCenter, trText("mono_motto"));

    // 2. Rectangular Gold-Framed MONOPOLY Banner (NO "MÉGA"!)
    double banW = cRect.width() * 0.62;
    double banH = cRect.height() * 0.14;
    QRectF bannerRect(cRect.center().x() - banW / 2.0, mottoRect.bottom() + 4, banW, banH);

    p.setPen(QPen(QColor("#D4AF37"), 3.0));
    p.setBrush(m_isDarkMode ? QColor("#0B0B0D") : QColor("#FFFFFF"));
    p.drawRect(bannerRect);
    p.setPen(QPen(QColor("#D4AF37"), 1.2));
    p.drawRect(bannerRect.adjusted(4, 4, -4, -4));

    QFont bf = p.font();
    bf.setPixelSize(std::max(18, static_cast<int>(banH * 0.62)));
    bf.setBold(true);
    bf.setLetterSpacing(QFont::AbsoluteSpacing, 3.0);
    p.setFont(bf);
    p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#B88917"));
    p.drawText(bannerRect, Qt::AlignCenter, "MONOPOLY");

    // 3. Left & Right Dotted Gold Card Slots (CAISSE DE COMMUNAUTÉ & CHANCE, just like on the Black & Gold board!)
    double deckW = cRect.width() * 0.18;
    double deckH = cRect.height() * 0.27;
    QRectF chestSlot(cRect.left() + cRect.width() * 0.04, cRect.center().y() - deckH * 0.35, deckW, deckH);
    QRectF chanceSlot(cRect.right() - cRect.width() * 0.04 - deckW, cRect.center().y() - deckH * 0.35, deckW, deckH);

    QPen dottedGold(QColor("#D4AF37"), 1.6, Qt::DotLine);
    p.setPen(dottedGold);
    p.setBrush(m_isDarkMode ? QColor(212, 175, 55, 14) : QColor(212, 175, 55, 22));
    p.drawRoundedRect(chestSlot, 6, 6);
    p.drawRoundedRect(chanceSlot, 6, 6);

    paintChestIcon(p, QRectF(chestSlot.center().x() - 22, chestSlot.center().y() - 22, 44, 44));
    paintQuestionIcon(p, QRectF(chanceSlot.center().x() - 22, chanceSlot.center().y() - 26, 44, 52), QColor("#D4AF37"));

    p.restore();
}

void MonopolyWidget::paintCornerTile(QPainter& p, int index, const QRectF& r) {
    p.save();
    QColor bgCol = m_isDarkMode ? QColor("#111115") : QColor("#FFFFFF");
    if (m_hoveredSpaceIndex == index) {
        bgCol = m_isDarkMode ? QColor("#221E14") : QColor("#FAF0D2");
    }
    p.setPen(QPen(QColor("#D4AF37"), 1.5));
    p.setBrush(bgCol);
    p.drawRect(r);

    QColor txtCol = m_isDarkMode ? QColor("#F3E5AB") : QColor("#1A1814");
    QFont f = p.font();

    if (index == 0) {
        // DÉPART / GO
        f.setPixelSize(std::max(10, static_cast<int>(r.width() * 0.17)));
        f.setBold(true);
        p.setFont(f);
        p.setPen(txtCol);
        p.drawText(QRectF(r.left() + 4, r.top() + 6, r.width() - 8, r.height() * 0.25),
                   Qt::AlignCenter, trText("mono_go_title"));

        f.setPixelSize(std::max(7, static_cast<int>(r.width() * 0.085)));
        f.setBold(false);
        p.setFont(f);
        p.drawText(QRectF(r.left() + 4, r.top() + r.height() * 0.28, r.width() - 8, r.height() * 0.42),
                   Qt::AlignCenter | Qt::TextWordWrap, trText("mono_go_sub"));

        // Iconic Red & Gold Arrow at bottom
        double ay = r.bottom() - r.height() * 0.16;
        p.setPen(QPen(QColor("#D4AF37"), 1.2));
        p.setBrush(QColor("#E32929"));
        QPolygonF arrow;
        arrow << QPointF(r.left() + r.width() * 0.12, ay)
              << QPointF(r.left() + r.width() * 0.28, ay - 6)
              << QPointF(r.left() + r.width() * 0.28, ay - 2.5)
              << QPointF(r.right() - r.width() * 0.12, ay - 2.5)
              << QPointF(r.right() - r.width() * 0.12, ay + 2.5)
              << QPointF(r.left() + r.width() * 0.28, ay + 2.5)
              << QPointF(r.left() + r.width() * 0.28, ay + 6);
        p.drawPolygon(arrow);
    } else if (index == 10) {
        // PRISON / SIMPLE VISITE
        QRectF jailBox(r.right() - r.width() * 0.68, r.top(), r.width() * 0.68, r.height() * 0.68);
        p.setBrush(QColor("#F27822"));
        p.setPen(QPen(QColor("#D4AF37"), 1.4));
        p.drawRect(jailBox);

        // Prisoner bars inside orange box
        QRectF barsBox = jailBox.adjusted(8, 8, -8, -8);
        p.setBrush(m_isDarkMode ? QColor("#18181B") : QColor("#FFFFFF"));
        p.drawRect(barsBox);
        p.setPen(QPen(QColor("#D4AF37"), 1.4));
        for (int b = 1; b <= 3; ++b) {
            double bx = barsBox.left() + b * (barsBox.width() / 4.0);
            p.drawLine(QPointF(bx, barsBox.top()), QPointF(bx, barsBox.bottom()));
        }

        f.setPixelSize(std::max(7, static_cast<int>(r.width() * 0.10)));
        f.setBold(true);
        p.setFont(f);
        p.setPen(txtCol);
        p.drawText(QRectF(r.left(), r.bottom() - r.height() * 0.28, r.width(), r.height() * 0.26),
                   Qt::AlignCenter, trText("mono_jail_visiting"));
    } else if (index == 20) {
        // PARC GRATUIT
        f.setPixelSize(std::max(8, static_cast<int>(r.width() * 0.12)));
        f.setBold(true);
        p.setFont(f);
        p.setPen(txtCol);
        p.drawText(QRectF(r.left() + 2, r.top() + 4, r.width() - 4, 16),
                   Qt::AlignCenter, trText("mono_free_parking"));

        // Red Free Parking car emblem
        QRectF carR(r.center().x() - r.width() * 0.26, r.center().y() - r.height() * 0.18,
                    r.width() * 0.52, r.height() * 0.36);
        p.setPen(QPen(QColor("#D4AF37"), 1.3));
        p.setBrush(QColor("#E32929"));
        p.drawRoundedRect(carR, 5, 5);
        p.setBrush(QColor("#F3E5AB"));
        p.drawEllipse(QPointF(carR.left() + carR.width() * 0.25, carR.bottom()), 4, 4);
        p.drawEllipse(QPointF(carR.right() - carR.width() * 0.25, carR.bottom()), 4, 4);

        p.setPen(txtCol);
        p.drawText(QRectF(r.left() + 2, r.bottom() - 20, r.width() - 4, 16),
                   Qt::AlignCenter, trText("mono_free_parking_2"));
    } else if (index == 30) {
        // ALLEZ EN PRISON
        f.setPixelSize(std::max(8, static_cast<int>(r.width() * 0.11)));
        f.setBold(true);
        p.setFont(f);
        p.setPen(txtCol);
        p.drawText(QRectF(r.left() + 2, r.top() + 4, r.width() - 4, 16),
                   Qt::AlignCenter, trText("mono_go_to_jail_1"));

        // Policeman badge / hat icon
        QPointF c = r.center();
        double rad = r.width() * 0.20;
        p.setPen(QPen(QColor("#D4AF37"), 1.5));
        p.setBrush(QColor("#1D4ED8"));
        p.drawEllipse(c, rad, rad);
        p.setBrush(QColor("#F3E5AB"));
        p.drawEllipse(c, rad * 0.4, rad * 0.4);

        p.setPen(txtCol);
        p.drawText(QRectF(r.left() + 2, r.bottom() - 20, r.width() - 4, 16),
                   Qt::AlignCenter, trText("mono_go_to_jail_2"));
    }

    p.restore();
}

void MonopolyWidget::paintSpaceTile(QPainter& p, int index, const QRectF& r) {
    if (!m_game) return;
    const Space& s = m_game->board().space(index);

    p.save();
    QColor bgCol = m_isDarkMode ? QColor("#111115") : QColor("#FFFFFF");
    if (m_hoveredSpaceIndex == index) {
        bgCol = m_isDarkMode ? QColor("#242015") : QColor("#FAF0D2");
    }

    // Highlight eligible tiles during Build / Sell / Mortgage / Unmortgage overlay
    bool eligibleHighlight = false;
    int actId = m_game->activePlayerIndex();
    if (m_overlayMode == ActionOverlayMode::Build) {
        eligibleHighlight = m_game->board().canBuildOn(actId, index, m_game->player(actId).money);
    } else if (m_overlayMode == ActionOverlayMode::Sell) {
        eligibleHighlight = m_game->board().canSellBuildingOn(actId, index);
    } else if (m_overlayMode == ActionOverlayMode::Mortgage) {
        eligibleHighlight = m_game->board().canMortgage(actId, index);
    } else if (m_overlayMode == ActionOverlayMode::Unmortgage) {
        eligibleHighlight = m_game->board().canUnmortgage(actId, index, m_game->player(actId).money);
    }

    p.setPen(QPen(eligibleHighlight ? QColor("#FFE885") : QColor("#D4AF37"), eligibleHighlight ? 2.4 : 1.3));
    p.setBrush(bgCol);
    p.drawRect(r);

    // Determine orientation side: 0=Bottom(1..9), 1=Left(11..19), 2=Top(21..29), 3=Right(31..39)
    int side = index / 10;

    // Color group band on inner edge facing the board center
    QRectF bandRect;
    QRectF contentRect = r;
    double bandThick = (side == 0 || side == 2) ? (r.height() * 0.24) : (r.width() * 0.24);

    if (s.type == SpaceType::Property) {
        if (side == 0) { // Bottom row: band at top
            bandRect = QRectF(r.left(), r.top(), r.width(), bandThick);
            contentRect.setTop(bandRect.bottom());
        } else if (side == 1) { // Left col: band at right
            bandRect = QRectF(r.right() - bandThick, r.top(), bandThick, r.height());
            contentRect.setRight(bandRect.left());
        } else if (side == 2) { // Top row: band at bottom
            bandRect = QRectF(r.left(), r.bottom() - bandThick, r.width(), bandThick);
            contentRect.setBottom(bandRect.top());
        } else { // Right col: band at left
            bandRect = QRectF(r.left(), r.top(), bandThick, r.height());
            contentRect.setLeft(bandRect.right());
        }

        p.setPen(QPen(QColor("#D4AF37"), 1.1));
        p.setBrush(groupColor(s.group));
        p.drawRect(bandRect);

        // Draw houses (1..4) or hotel (5) inside the color band!
        if (s.buildings == 5) {
            double hs = std::min(bandRect.width(), bandRect.height()) * 0.85;
            QRectF hr(bandRect.center().x() - hs / 2.0, bandRect.center().y() - hs / 2.0, hs, hs);
            paintHouseIcon(p, hr, true);
        } else if (s.buildings >= 1 && s.buildings <= 4) {
            double hs = std::min(bandRect.width(), bandRect.height()) * 0.72;
            for (int b = 0; b < s.buildings; ++b) {
                QRectF hr;
                if (side == 0 || side == 2) {
                    double step = bandRect.width() / (s.buildings + 1.0);
                    hr = QRectF(bandRect.left() + step * (b + 1) - hs / 2.0,
                                bandRect.center().y() - hs / 2.0, hs, hs);
                } else {
                    double step = bandRect.height() / (s.buildings + 1.0);
                    hr = QRectF(bandRect.center().x() - hs / 2.0,
                                bandRect.top() + step * (b + 1) - hs / 2.0, hs, hs);
                }
                paintHouseIcon(p, hr, false);
            }
        }
    }

    // Owner indicator strip on outer edge if owned
    if (s.ownerId >= 0) {
        QColor ownCol = playerAccentColor(s.ownerId);
        if (s.mortgaged) ownCol = QColor("#71717A");
        p.setPen(Qt::NoPen);
        p.setBrush(ownCol);
        if (side == 0) p.drawRect(QRectF(r.left() + 1, r.bottom() - 3.5, r.width() - 2, 3.5));
        else if (side == 1) p.drawRect(QRectF(r.left(), r.top() + 1, 3.5, r.height() - 2));
        else if (side == 2) p.drawRect(QRectF(r.left() + 1, r.top(), r.width() - 2, 3.5));
        else p.drawRect(QRectF(r.right() - 3.5, r.top() + 1, 3.5, r.height() - 2));
    }

    // Draw text & icon in local coordinates facing upright or readable per side
    p.save();
    p.translate(contentRect.center());
    double w = contentRect.width();
    double h = contentRect.height();
    if (side == 1) {
        p.rotate(90);
        std::swap(w, h);
    } else if (side == 3) {
        p.rotate(-90);
        std::swap(w, h);
    }
    QRectF local(-w / 2.0, -h / 2.0, w, h);

    QColor txtCol = m_isDarkMode ? QColor("#F4F4F5") : QColor("#18181B");
    QColor priceCol = m_isDarkMode ? QColor("#F3E5AB") : QColor("#8A6608");

    QFont nf = p.font();
    nf.setPixelSize(std::max(6, static_cast<int>(std::min(w, h) * 0.145)));
    nf.setBold(true);
    p.setFont(nf);
    p.setPen(txtCol);

    QString sName = spaceName(index);
    QRectF nameBox(local.left() + 2, local.top() + 2, local.width() - 4, local.height() * 0.52);
    p.drawText(nameBox, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, sName);

    // Special icons for Railroad, Utility, Chance, Community Chest, Tax
    QRectF iconBox(local.center().x() - local.width() * 0.26,
                   local.top() + local.height() * 0.36,
                   local.width() * 0.52,
                   local.height() * 0.36);

    if (s.type == SpaceType::Railroad) {
        paintLocomotiveIcon(p, iconBox, m_isDarkMode ? QColor("#F3E5AB") : QColor("#18181B"));
    } else if (s.type == SpaceType::CommunityChest) {
        paintChestIcon(p, iconBox.adjusted(-2, 2, 2, 6));
    } else if (s.type == SpaceType::Chance) {
        QColor qCol = (index == 7) ? QColor("#D63088") : ((index == 22) ? QColor("#38BDF8") : QColor("#F27822"));
        paintQuestionIcon(p, iconBox.adjusted(-2, 0, 2, 8), qCol);
    } else if (s.type == SpaceType::Utility) {
        if (index == 12) paintLightbulbIcon(p, iconBox);
        else paintWaterTapIcon(p, iconBox);
    } else if (s.type == SpaceType::Tax) {
        if (index == 38) {
            paintRingIcon(p, iconBox);
        } else {
            p.setPen(QPen(QColor("#D4AF37"), 1.2));
            p.setBrush(QColor("#D4AF37"));
            QPointF ic = iconBox.center();
            QPolygonF dia;
            dia << QPointF(ic.x(), ic.y() - 5) << QPointF(ic.x() + 5, ic.y())
                << QPointF(ic.x(), ic.y() + 5) << QPointF(ic.x() - 5, ic.y());
            p.drawPolygon(dia);
        }
    }

    // Price or Tax label at bottom of tile
    QFont pf = p.font();
    pf.setPixelSize(std::max(7, static_cast<int>(std::min(w, h) * 0.16)));
    pf.setBold(true);
    p.setFont(pf);
    p.setPen(priceCol);

    QRectF priceBox(local.left() + 2, local.bottom() - local.height() * 0.25, local.width() - 4, local.height() * 0.24);
    if (s.price > 0) {
        p.drawText(priceBox, Qt::AlignCenter, formatMoney(s.price));
    } else if (s.taxAmount > 0) {
        p.drawText(priceBox, Qt::AlignCenter, formatMoney(s.taxAmount));
    }

    p.restore();
    p.restore();
}

void MonopolyWidget::paintTokensOnBoard(QPainter& p) {
    if (!m_game) return;

    int n = m_game->numPlayers();
    for (int i = 0; i < n; ++i) {
        const Player& pl = m_game->player(i);
        if (pl.bankrupt) continue;

        QPointF pos;
        double jumpOffset = 0.0;

        if (m_tokenAnimActive && i == m_tokenAnimPlayerId && m_tokenAnimStepIdx + 1 < m_tokenAnimPath.size()) {
            int fromIdx = m_tokenAnimPath[m_tokenAnimStepIdx];
            int toIdx = m_tokenAnimPath[m_tokenAnimStepIdx + 1];
            QPointF pA = spaceCenter(fromIdx);
            QPointF pB = spaceCenter(toIdx);
            double t = m_tokenAnimStepProgress;
            pos = pA * (1.0 - t) + pB * t;
            // Parabolic hop arc!
            double arcHeight = m_tokenAnimIsJailFlight ? 55.0 : 18.0;
            jumpOffset = 4.0 * t * (1.0 - t) * arcHeight;
        } else {
            QRectF sr = spaceRect(pl.position);
            // Offset multiple tokens on the same space so all are clearly visible
            static const QPointF offsets[4] = {
                QPointF(-0.17, -0.14),
                QPointF(0.17, -0.14),
                QPointF(-0.17, 0.17),
                QPointF(0.17, 0.17)
            };
            pos = sr.center() + QPointF(sr.width() * offsets[i % 4].x(), sr.height() * offsets[i % 4].y());
        }

        // Size of the 3D figurine standing directly on the board (no circular badge!)
        double tokRadius = std::max(21.0, boardOuterRect().width() * 0.036);

        // Soft 3D cast shadow on the board under the figurine
        p.save();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, m_isDarkMode ? 160 : 110));
        p.drawEllipse(pos + QPointF(2, tokRadius * 0.45), tokRadius * 0.78, tokRadius * 0.34);

        // Draw 3D golden figurine directly on the board without any circle around it!
        QPointF drawCenter = pos - QPointF(0, jumpOffset);
        QRectF figRect(drawCenter.x() - tokRadius, drawCenter.y() - tokRadius, tokRadius * 2.0, tokRadius * 2.0);
        paintTokenIcon(p, figRect, pl.token, false);
        p.restore();
    }
}

// -----------------------------------------------------------------------------
// Right Sidebar (Player Accordion Panels + BANQUE + HISTORIQUE)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintRightSidebar(QPainter& p, const QRectF& sRect) {
    if (!m_game) return;
    p.save();

    double curY = sRect.top();
    double w = sRect.width();
    int n = m_game->numPlayers();

    // 1. Player Panels (Expanded active/selected player + Collapsed bars for others)
    for (int i = 0; i < n; ++i) {
        const Player& pl = m_game->player(i);
        bool isExpanded = (i == m_expandedPlayerId);
        double cardH = isExpanded ? 182.0 : 40.0;
        QRectF pRect(sRect.left(), curY, w, cardH);

        // Panel background
        QLinearGradient pGrad(pRect.topLeft(), pRect.bottomLeft());
        if (m_isDarkMode) {
            if (i == m_game->activePlayerIndex()) {
                pGrad.setColorAt(0.0, QColor("#282315"));
                pGrad.setColorAt(1.0, QColor("#16140D"));
            } else {
                pGrad.setColorAt(0.0, QColor("#1A1A20"));
                pGrad.setColorAt(1.0, QColor("#121216"));
            }
        } else {
            if (i == m_game->activePlayerIndex()) {
                pGrad.setColorAt(0.0, QColor("#FFFDF5"));
                pGrad.setColorAt(1.0, QColor("#F2E5BC"));
            } else {
                pGrad.setColorAt(0.0, QColor("#FFFFFF"));
                pGrad.setColorAt(1.0, QColor("#F5F0E1"));
            }
        }

        QColor borderCol = (i == m_game->activePlayerIndex()) ? QColor("#FFE066") : QColor("#D4AF37");
        p.setPen(QPen(borderCol, (i == m_game->activePlayerIndex()) ? 2.2 : 1.5));
        p.setBrush(pGrad);
        p.drawRoundedRect(pRect, 9.0, 9.0);

        // Register click to expand this player panel
        m_buttons.push_back({QString("sidebar_player_%1").arg(i), QRectF(pRect.left(), pRect.top(), pRect.width(), 38.0), true});

        // Token icon
        QRectF tokBox(pRect.left() + 8, pRect.top() + 5, 30, 30);
        paintTokenIcon(p, tokBox, pl.token, true);

        // Player name
        QFont nf = p.font();
        nf.setPixelSize(14);
        nf.setBold(true);
        p.setFont(nf);
        p.setPen(pl.bankrupt ? QColor("#EF4444") : (m_isDarkMode ? QColor("#F4F4F5") : QColor("#18181B")));
        p.drawText(QRectF(tokBox.right() + 8, pRect.top() + 6, w * 0.42, 26),
                   Qt::AlignVCenter | Qt::AlignLeft, playerDisplayName(i));

        // Animated Money balance
        int dispCash = (i < static_cast<int>(m_displayedMoney.size()))
                       ? static_cast<int>(std::round(m_displayedMoney[i])) : pl.money;
        QFont mf = p.font();
        mf.setPixelSize(15);
        mf.setBold(true);
        p.setFont(mf);
        p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
        p.drawText(QRectF(pRect.right() - 165, pRect.top() + 6, 100, 26),
                   Qt::AlignVCenter | Qt::AlignRight, formatMoney(dispCash));

        // If active player & expanded, draw the 2 dice on top-right (just like 182642.png!)
        if (isExpanded && i == m_game->activePlayerIndex()) {
            paint3DDie(p, QRectF(pRect.right() - 56, pRect.top() + 7, 22, 22), m_game->die1());
            paint3DDie(p, QRectF(pRect.right() - 30, pRect.top() + 7, 22, 22), m_game->die2());
        } else {
            // Dropdown triangle arrow
            p.setPen(QPen(QColor("#D4AF37"), 1.2));
            p.setBrush(QColor("#D4AF37"));
            QPointF ac(pRect.right() - 20, pRect.top() + 20);
            QPolygonF tri;
            tri << QPointF(ac.x() - 6, ac.y() - 3) << QPointF(ac.x() + 6, ac.y() - 3) << QPointF(ac.x(), ac.y() + 4);
            p.drawPolygon(tri);
        }

        if (isExpanded) {
            // Divider line
            p.setPen(QPen(QColor(212, 175, 55, 90), 1.0));
            p.drawLine(QPointF(pRect.left() + 8, pRect.top() + 40), QPointF(pRect.right() - 8, pRect.top() + 40));

            // Mini Property Grid (matching the exact layout in 182516.png - 182709.png!)
            // Row 1: Brown (2), LightBlue (3), Pink (3), Orange (3)
            // Row 2: Red (3), Yellow (3), Green (3), DarkBlue (2)
            // Row 3: Utilities (2), Railroads (4)
            auto drawMiniGroup = [&](ColorGroup grp, double gx, double gy) {
                auto indices = m_game->board().groupSpaces(grp);
                double mw = 13.0, mh = 18.0, mgap = 2.0;
                for (size_t k = 0; k < indices.size(); ++k) {
                    int spIdx = indices[k];
                    const Space& sp = m_game->board().space(spIdx);
                    QRectF mr(gx + k * (mw + mgap), gy, mw, mh);
                    m_buttons.push_back({QString("inspect_deed_%1").arg(spIdx), mr, true});

                    bool owned = (sp.ownerId == i);
                    QColor grpCol = groupColor(grp);
                    if (owned) {
                        p.setPen(QPen(QColor("#D4AF37"), 1.2));
                        p.setBrush(sp.mortgaged ? QColor("#52525B") : (m_isDarkMode ? QColor("#F4F4F5") : QColor("#FFFFFF")));
                        p.drawRect(mr);
                        p.fillRect(QRectF(mr.left() + 1, mr.top() + 1, mr.width() - 2, 5), grpCol);
                    } else {
                        p.setPen(QPen(grpCol.darker(m_isDarkMode ? 115 : 105), 1.1));
                        p.setBrush(Qt::NoBrush);
                        p.drawRect(mr);
                    }
                }
            };

            double r1Y = pRect.top() + 48.0;
            double r2Y = r1Y + 24.0;
            double r3Y = r2Y + 24.0;
            double lx = pRect.left() + 14.0;

            drawMiniGroup(ColorGroup::Brown,     lx,         r1Y);
            drawMiniGroup(ColorGroup::LightBlue, lx + 48.0,  r1Y);
            drawMiniGroup(ColorGroup::Pink,      lx + 106.0, r1Y);
            drawMiniGroup(ColorGroup::Orange,    lx + 164.0, r1Y);

            drawMiniGroup(ColorGroup::Red,       lx,         r2Y);
            drawMiniGroup(ColorGroup::Yellow,    lx + 58.0,  r2Y);
            drawMiniGroup(ColorGroup::Green,     lx + 116.0, r2Y);
            drawMiniGroup(ColorGroup::DarkBlue,  lx + 178.0, r2Y);

            drawMiniGroup(ColorGroup::Utility,   lx + 58.0,  r3Y);
            drawMiniGroup(ColorGroup::Railroad,  lx + 116.0, r3Y);

            // Bottom strip of expanded player card: House count, Hotel count, Jail cards
            p.setPen(QPen(QColor(212, 175, 55, 90), 1.0));
            p.drawLine(QPointF(pRect.left() + 8, pRect.bottom() - 36), QPointF(pRect.right() - 8, pRect.bottom() - 36));

            paintHouseIcon(p, QRectF(pRect.left() + 12, pRect.bottom() - 30, 22, 22), false);
            p.setFont(nf);
            p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
            p.drawText(QRectF(pRect.left() + 38, pRect.bottom() - 30, 30, 22),
                       Qt::AlignVCenter | Qt::AlignLeft,
                       QString::number(m_game->board().playerHouseCount(i)));

            paintHouseIcon(p, QRectF(pRect.left() + 74, pRect.bottom() - 30, 22, 22), true);
            p.setPen(m_isDarkMode ? QColor("#F87171") : QColor("#B91C1C"));
            p.drawText(QRectF(pRect.left() + 100, pRect.bottom() - 30, 30, 22),
                       Qt::AlignVCenter | Qt::AlignLeft,
                       QString::number(m_game->board().playerHotelCount(i)));

            if (pl.jailFreeCards() > 0) {
                QRectF jcRect(pRect.right() - 105, pRect.bottom() - 29, 92, 20);
                p.setPen(QPen(QColor("#D4AF37"), 1.0));
                p.setBrush(m_isDarkMode ? QColor("#262216") : QColor("#F7EED2"));
                p.drawRoundedRect(jcRect, 4, 4);
                QFont jf = p.font();
                jf.setPixelSize(10);
                jf.setBold(true);
                p.setFont(jf);
                p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#7A5B08"));
                p.drawText(jcRect, Qt::AlignCenter, QString("🗝 x%1").arg(pl.jailFreeCards()));
            }
        }

        curY += cardH + 6.0;
    }

    // 2. BANQUE Bar (matching 182516.png - 182709.png!)
    QRectF bankRect(sRect.left(), curY, w, 38.0);
    m_buttons.push_back({"sidebar_bank", bankRect, true});
    QLinearGradient bGrad(bankRect.topLeft(), bankRect.bottomLeft());
    if (m_isDarkMode) {
        bGrad.setColorAt(0.0, QColor("#2A2518"));
        bGrad.setColorAt(1.0, QColor("#18150E"));
    } else {
        bGrad.setColorAt(0.0, QColor("#F7EED2"));
        bGrad.setColorAt(1.0, QColor("#E5D4A0"));
    }
    p.setPen(QPen(QColor("#D4AF37"), 1.6));
    p.setBrush(bGrad);
    p.drawRoundedRect(bankRect, 8.0, 8.0);

    QFont bFont = p.font();
    bFont.setPixelSize(18);
    bFont.setBold(true);
    bFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
    p.setFont(bFont);
    p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#6E5106"));
    p.drawText(bankRect, Qt::AlignCenter, trText("mono_bank"));

    // House & Hotel bank stock badges on left of BANQUE bar
    QFont sf = p.font();
    sf.setPixelSize(11);
    sf.setBold(true);
    p.setFont(sf);
    paintHouseIcon(p, QRectF(bankRect.left() + 10, bankRect.top() + 10, 16, 16), false);
    p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
    p.drawText(QRectF(bankRect.left() + 28, bankRect.top() + 10, 24, 16), Qt::AlignVCenter,
               QString::number(m_game->board().bankHousesAvailable()));

    paintHouseIcon(p, QRectF(bankRect.right() - 52, bankRect.top() + 10, 16, 16), true);
    p.setPen(m_isDarkMode ? QColor("#F87171") : QColor("#B91C1C"));
    p.drawText(QRectF(bankRect.right() - 34, bankRect.top() + 10, 24, 16), Qt::AlignVCenter,
               QString::number(m_game->board().bankHotelsAvailable()));

    curY += 44.0;

    // 3. HISTORIQUE Panel (fills remaining height down to bottom bar!)
    double histH = std::max(120.0, sRect.bottom() - curY);
    QRectF histRect(sRect.left(), curY, w, histH);
    p.setPen(QPen(QColor("#D4AF37"), 1.6));
    p.setBrush(m_isDarkMode ? QColor("#101014") : QColor("#FFFFFF"));
    p.drawRoundedRect(histRect, 9.0, 9.0);

    // Historique header bar with live elapsed timer 00:01:34
    QRectF histHdr(histRect.left() + 2, histRect.top() + 2, histRect.width() - 4, 28);
    p.fillRect(histHdr, m_isDarkMode ? QColor("#1F1C14") : QColor("#F3EBD3"));
    p.setPen(QPen(QColor("#D4AF37"), 1.0));
    p.drawLine(histHdr.bottomLeft(), histHdr.bottomRight());

    QFont hhf = p.font();
    hhf.setPixelSize(13);
    hhf.setBold(true);
    p.setFont(hhf);
    p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#241E0E"));
    p.drawText(histHdr.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignLeft, trText("mono_history"));

    int secs = m_game->elapsedSeconds();
    QString timeStr = QString("%1:%2:%3")
                          .arg(secs / 3600, 2, 10, QChar('0'))
                          .arg((secs % 3600) / 60, 2, 10, QChar('0'))
                          .arg(secs % 60, 2, 10, QChar('0'));
    p.drawText(histHdr.adjusted(12, 0, -12, 0), Qt::AlignVCenter | Qt::AlignRight, timeStr);

    // Log rows
    QRectF logArea(histRect.left() + 6, histHdr.bottom() + 4, histRect.width() - 30, histRect.height() - 36);
    p.save();
    p.setClipRect(logArea);

    const auto& logs = m_game->historyLog();
    double rowH = 38.0;
    int maxOffset = std::max(0, static_cast<int>(logs.size()) - static_cast<int>(logArea.height() / rowH));
    int offset = std::clamp(m_historyScrollOffset, 0, maxOffset);

    QFont lf = p.font();
    lf.setPixelSize(11);
    lf.setBold(false);
    p.setFont(lf);

    for (size_t idx = static_cast<size_t>(offset); idx < logs.size(); ++idx) {
        double ry = logArea.top() + (idx - offset) * rowH;
        if (ry > logArea.bottom()) break;

        const auto& item = logs[idx];
        QString tStamp = QString("%1:%2:%3")
                             .arg(item.elapsedSeconds / 3600, 2, 10, QChar('0'))
                             .arg((item.elapsedSeconds % 3600) / 60, 2, 10, QChar('0'))
                             .arg(item.elapsedSeconds % 60, 2, 10, QChar('0'));

        p.setPen(m_isDarkMode ? QColor("#D4AF37") : QColor("#8A6608"));
        p.drawText(QRectF(logArea.left() + 2, ry + 2, 58, rowH - 4), Qt::AlignTop | Qt::AlignLeft, tStamp);

        p.setPen(m_isDarkMode ? QColor("#E4E4E7") : QColor("#27272A"));
        p.drawText(QRectF(logArea.left() + 62, ry + 2, logArea.width() - 64, rowH - 4),
                   Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, formatHistoryEntry(item));

        p.setPen(QPen(QColor(212, 175, 55, 45), 1.0));
        p.drawLine(QPointF(logArea.left(), ry + rowH - 1), QPointF(logArea.right(), ry + rowH - 1));
    }
    p.restore();

    // Up/Down scroll buttons on right edge of HISTORIQUE panel
    QRectF upBtn(histRect.right() - 22, histHdr.bottom() + 4, 18, 22);
    QRectF dnBtn(histRect.right() - 22, histRect.bottom() - 26, 18, 22);
    paintGoldButton(p, "hist_scroll_up", upBtn, "▲", offset > 0, false, 9);
    paintGoldButton(p, "hist_scroll_down", dnBtn, "▼", offset < maxOffset, false, 9);

    p.restore();
}

// -----------------------------------------------------------------------------
// Bottom Action Bar (MENU | CONSTRUIRE | VENDRE | HYPOTHÉQUER | LEVER L'HYPOTHÈQUE | ÉCHANGE | MONOPOLY)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintBottomActionBar(QPainter& p, const QRectF& barRect) {
    if (!m_game) return;
    p.save();

    p.setPen(QPen(QColor("#D4AF37"), 1.6));
    p.setBrush(m_isDarkMode ? QColor("#12110E") : QColor("#F5EED8"));
    p.drawRoundedRect(barRect, 8.0, 8.0);

    bool humanCanManage = !m_game->isAITurn() &&
                          (m_game->phase() == TurnPhase::WaitingForRoll ||
                           m_game->phase() == TurnPhase::WaitingForEndTurn ||
                           m_game->phase() == TurnPhase::DebtResolution);

    int actId = m_game->activePlayerIndex();
    int cash = m_game->player(actId).money;
    bool canBuildAny = false, canSellAny = false, canMortAny = false, canUnmortAny = false;
    for (int idx : m_game->board().playerProperties(actId)) {
        if (m_game->board().canBuildOn(actId, idx, cash)) canBuildAny = true;
        if (m_game->board().canSellBuildingOn(actId, idx)) canSellAny = true;
        if (m_game->board().canMortgage(actId, idx)) canMortAny = true;
        if (m_game->board().canUnmortgage(actId, idx, cash)) canUnmortAny = true;
    }

    double logoW = 165.0;
    double availW = barRect.width() - logoW - 24.0;
    double gap = 6.0;
    double btnW = (availW - 5 * gap) / 6.0;
    double btnH = barRect.height() - 10.0;
    double by = barRect.top() + 5.0;
    double bx = barRect.left() + 6.0;

    paintGoldButton(p, "bar_menu",       QRectF(bx + 0 * (btnW + gap), by, btnW, btnH), trText("mono_menu_btn"), true, m_overlayMode == ActionOverlayMode::Menu, 11);
    paintGoldButton(p, "bar_build",      QRectF(bx + 1 * (btnW + gap), by, btnW, btnH), trText("mono_build_btn"), humanCanManage && canBuildAny, m_overlayMode == ActionOverlayMode::Build, 11);
    paintGoldButton(p, "bar_sell",       QRectF(bx + 2 * (btnW + gap), by, btnW, btnH), trText("mono_sell_btn"), humanCanManage && canSellAny, m_overlayMode == ActionOverlayMode::Sell, 11);
    paintGoldButton(p, "bar_mortgage",   QRectF(bx + 3 * (btnW + gap), by, btnW, btnH), trText("mono_mortgage_btn"), humanCanManage && canMortAny, m_overlayMode == ActionOverlayMode::Mortgage, 11);
    paintGoldButton(p, "bar_unmortgage", QRectF(bx + 4 * (btnW + gap), by, btnW, btnH), trText("mono_unmortgage_btn"), humanCanManage && canUnmortAny, m_overlayMode == ActionOverlayMode::Unmortgage, 10);
    paintGoldButton(p, "bar_trade",      QRectF(bx + 5 * (btnW + gap), by, btnW, btnH), trText("mono_trade_btn"), humanCanManage, m_overlayMode == ActionOverlayMode::Trade, 11);

    // Bottom-right MONOPOLY Gold Emblem
    QRectF badgeRect(barRect.right() - logoW - 6.0, by, logoW, btnH);
    p.setPen(QPen(QColor("#D4AF37"), 2.0));
    p.setBrush(m_isDarkMode ? QColor("#0B0B0D") : QColor("#181612"));
    p.drawRoundedRect(badgeRect, 6.0, 6.0);
    QFont lf = p.font();
    lf.setPixelSize(19);
    lf.setBold(true);
    lf.setLetterSpacing(QFont::AbsoluteSpacing, 2.0);
    p.setFont(lf);
    p.setPen(QColor("#F3E5AB"));
    p.drawText(badgeRect, Qt::AlignCenter, "MONOPOLY");

    p.restore();
}

// -----------------------------------------------------------------------------
// Title Deed Card Renderer (Front & Mortgaged Back, matching photos & 182642.png!)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintTitleDeedCard(QPainter& p, const QRectF& cardRect, int spaceIndex, bool showBackIfMortgaged) {
    if (!m_game || spaceIndex < 0 || spaceIndex >= MonopolyBoard::NUM_SPACES) return;
    const Space& s = m_game->board().space(spaceIndex);

    p.save();
    // Card shadow
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 100));
    p.drawRoundedRect(cardRect.translated(2, 3), 6, 6);

    if (showBackIfMortgaged && s.mortgaged) {
        // Black & Gold Mortgaged Side (as shown in monopoly-mega-paris...(7).jpg!)
        p.setPen(QPen(QColor("#D4AF37"), 2.0));
        p.setBrush(QColor("#0E0E12"));
        p.drawRoundedRect(cardRect, 5, 5);
        p.setPen(QPen(QColor("#D4AF37"), 1.0));
        p.drawRect(cardRect.adjusted(6, 6, -6, -6));

        QFont tf = p.font();
        tf.setPixelSize(12);
        tf.setBold(true);
        p.setFont(tf);
        p.setPen(QColor("#F3E5AB"));
        p.drawText(QRectF(cardRect.left() + 12, cardRect.top() + 16, cardRect.width() - 24, 38),
                   Qt::AlignCenter | Qt::TextWordWrap, spaceName(spaceIndex));

        p.drawLine(QPointF(cardRect.left() + 16, cardRect.top() + 60),
                   QPointF(cardRect.right() - 16, cardRect.top() + 60));

        tf.setPixelSize(13);
        p.setFont(tf);
        p.setPen(QColor("#FFE066"));
        p.drawText(QRectF(cardRect.left() + 12, cardRect.top() + 72, cardRect.width() - 24, 42),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   trText("mono_mortgaged_for").arg(s.mortgageValue));

        p.drawLine(QPointF(cardRect.left() + 16, cardRect.top() + 120),
                   QPointF(cardRect.right() - 16, cardRect.top() + 120));

        tf.setPixelSize(9);
        tf.setBold(false);
        p.setFont(tf);
        p.setPen(QColor("#D4D4D8"));
        p.drawText(QRectF(cardRect.left() + 14, cardRect.top() + 130, cardRect.width() - 28, cardRect.height() - 140),
                   Qt::AlignCenter | Qt::TextWordWrap, trText("mono_mortgaged_note"));
        p.restore();
        return;
    }

    // Front of Title Deed Card
    p.setPen(QPen(QColor("#D4AF37"), 1.8));
    p.setBrush(m_isDarkMode ? QColor("#18181D") : QColor("#FFFFFF"));
    p.drawRoundedRect(cardRect, 5, 5);

    QRectF inner = cardRect.adjusted(5, 5, -5, -5);
    p.setPen(QPen(QColor("#D4AF37"), 1.0));
    p.drawRect(inner);

    QColor txtCol = m_isDarkMode ? QColor("#F4F4F5") : QColor("#18181B");
    QColor subCol = m_isDarkMode ? QColor("#D4D4D8") : QColor("#3F3F46");

    if (s.type == SpaceType::Property) {
        QRectF hdr(inner.left() + 3, inner.top() + 3, inner.width() - 6, inner.height() * 0.23);
        p.setPen(QPen(QColor("#18181B"), 1.0));
        p.setBrush(groupColor(s.group));
        p.drawRect(hdr);

        QColor hdrTxt = (s.group == ColorGroup::Yellow || s.group == ColorGroup::LightBlue)
                        ? QColor("#111111") : QColor("#FFFFFF");
        QFont f = p.font();
        f.setPixelSize(8);
        f.setBold(true);
        p.setFont(f);
        p.setPen(hdrTxt);
        p.drawText(QRectF(hdr.left(), hdr.top() + 2, hdr.width(), 12), Qt::AlignCenter, trText("mono_deed_title"));

        f.setPixelSize(11);
        p.setFont(f);
        p.drawText(QRectF(hdr.left() + 4, hdr.top() + 13, hdr.width() - 8, hdr.height() - 14),
                   Qt::AlignCenter | Qt::TextWordWrap, spaceName(spaceIndex));

        double ry = hdr.bottom() + 4;
        double rh = (inner.height() * 0.45) / 6.0;
        const QString labels[6] = {
            trText("mono_deed_rent_base"),
            trText("mono_deed_house_1"),
            trText("mono_deed_house_2"),
            trText("mono_deed_house_3"),
            trText("mono_deed_house_4"),
            trText("mono_deed_hotel")
        };

        f.setPixelSize(9);
        f.setBold(false);
        p.setFont(f);
        p.setPen(txtCol);
        for (int i = 0; i < 6; ++i) {
            QRectF rowR(inner.left() + 8, ry + i * rh, inner.width() - 16, rh);
            p.drawText(rowR, Qt::AlignVCenter | Qt::AlignLeft, labels[i]);
            p.drawText(rowR, Qt::AlignVCenter | Qt::AlignRight, formatMoney(s.rents[i]));
        }

        double fy = ry + 6 * rh + 3;
        p.setPen(QPen(QColor("#D4AF37"), 0.8));
        p.drawLine(QPointF(inner.left() + 10, fy), QPointF(inner.right() - 10, fy));

        f.setPixelSize(8);
        p.setFont(f);
        p.setPen(subCol);
        QRectF footR(inner.left() + 6, fy + 3, inner.width() - 12, inner.bottom() - fy - 4);
        QString footText = QString("%1\n%2\n%3")
                               .arg(trText("mono_deed_mortgage_val").arg(s.mortgageValue))
                               .arg(trText("mono_deed_house_cost").arg(s.houseCost))
                               .arg(trText("mono_deed_hotel_cost").arg(s.houseCost));
        p.drawText(footR, Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, footText);
    } else if (s.type == SpaceType::Railroad) {
        paintLocomotiveIcon(p, QRectF(inner.center().x() - 24, inner.top() + 8, 48, 36),
                            m_isDarkMode ? QColor("#F3E5AB") : QColor("#18181B"));
        QFont f = p.font();
        f.setPixelSize(11);
        f.setBold(true);
        p.setFont(f);
        p.setPen(txtCol);
        p.drawText(QRectF(inner.left() + 6, inner.top() + 46, inner.width() - 12, 28),
                   Qt::AlignCenter | Qt::TextWordWrap, spaceName(spaceIndex));

        f.setPixelSize(9);
        f.setBold(false);
        p.setFont(f);
        const QString rrKeys[4] = {"mono_deed_rr_1", "mono_deed_rr_2", "mono_deed_rr_3", "mono_deed_rr_4"};
        const int rrRents[4] = {25, 50, 100, 200};
        for (int i = 0; i < 4; ++i) {
            QRectF rowR(inner.left() + 8, inner.top() + 82 + i * 18, inner.width() - 16, 18);
            p.drawText(rowR, Qt::AlignVCenter | Qt::AlignLeft, trText(rrKeys[i]));
            p.drawText(rowR, Qt::AlignVCenter | Qt::AlignRight, formatMoney(rrRents[i]));
        }
        p.setPen(subCol);
        p.drawText(QRectF(inner.left() + 6, inner.bottom() - 26, inner.width() - 12, 22),
                   Qt::AlignCenter, trText("mono_deed_mortgage_val").arg(s.mortgageValue));
    } else if (s.type == SpaceType::Utility) {
        QRectF icR(inner.center().x() - 22, inner.top() + 8, 44, 38);
        if (spaceIndex == 12) paintLightbulbIcon(p, icR);
        else paintWaterTapIcon(p, icR);

        QFont f = p.font();
        f.setPixelSize(11);
        f.setBold(true);
        p.setFont(f);
        p.setPen(txtCol);
        p.drawText(QRectF(inner.left() + 6, inner.top() + 48, inner.width() - 12, 28),
                   Qt::AlignCenter | Qt::TextWordWrap, spaceName(spaceIndex));

        f.setPixelSize(8);
        f.setBold(false);
        p.setFont(f);
        p.setPen(subCol);
        p.drawText(QRectF(inner.left() + 8, inner.top() + 80, inner.width() - 16, 85),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   trText("mono_deed_util_1") + "\n\n" + trText("mono_deed_util_2"));
        p.drawText(QRectF(inner.left() + 6, inner.bottom() - 24, inner.width() - 12, 20),
                   Qt::AlignCenter, trText("mono_deed_mortgage_val").arg(s.mortgageValue));
    }

    p.restore();
}

// -----------------------------------------------------------------------------
// Center Board Modals (Start, Roll Dice, Buy/Auction, Auction, Rent, Card, Debt, End Turn)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintCenterTurnModals(QPainter& p, const QRectF& cRect) {
    if (!m_game) return;
    if (m_diceAnimActive || m_tokenAnimActive) return;

    TurnPhase ph = m_game->phase();
    const Player& actP = m_game->activePlayer();

    if (ph == TurnPhase::WaitingForStartModal) {
        // Replicating swappy-20261120-182516.png: "Cuirassé commence en premier."
        QRectF mRect(cRect.center().x() - 165, cRect.center().y() - 95, 330, 190);
        paintModalFrame(p, mRect);

        QFont f = p.font();
        f.setPixelSize(15);
        f.setBold(true);
        p.setFont(f);
        p.setPen(m_isDarkMode ? QColor("#F4F4F5") : QColor("#18181B"));
        p.drawText(QRectF(mRect.left() + 20, mRect.top() + 20, mRect.width() - 40, 32),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   trText("mono_starts_first").arg(playerDisplayName(actP.id)));

        paintTokenIcon(p, QRectF(mRect.left() + 24, mRect.top() + 62, 68, 64), actP.token, true);

        QRectF closeBtn(mRect.center().x() - 55, mRect.bottom() - 48, 110, 34);
        paintGoldButton(p, "modal_start_close", closeBtn, trText("mono_close_btn"), true, true, 13);
    }
    else if (ph == TurnPhase::WaitingForRoll) {
        // Replicating swappy-20261120-182544.png: Player Name, Token Icon, [JETER LES DÉS]
        double mh = actP.inJail ? 148.0 : 116.0;
        QRectF mRect(cRect.center().x() - 160, cRect.center().y() - mh / 2.0, 320, mh);
        paintModalFrame(p, mRect, playerDisplayName(actP.id));

        paintTokenIcon(p, QRectF(mRect.left() + 22, mRect.top() + 44, 62, 58), actP.token, true);

        QRectF rollBtn(mRect.left() + 98, mRect.top() + 50, mRect.width() - 118, 44);
        paintGoldButton(p, "modal_roll_dice", rollBtn, trText("mono_roll_dice"), !actP.isAI, true, 15);

        if (actP.inJail) {
            QRectF payBtn(mRect.left() + 22, mRect.bottom() - 36, 132, 26);
            QRectF cardBtn(mRect.right() - 154, mRect.bottom() - 36, 132, 26);
            paintGoldButton(p, "modal_pay_jail", payBtn, trText("mono_pay_jail_50"),
                            !actP.isAI && actP.money >= 50, false, 11);
            paintGoldButton(p, "modal_card_jail", cardBtn, trText("mono_use_jail_card"),
                            !actP.isAI && actP.jailFreeCards() > 0, false, 11);
        }
    }
    else if (ph == TurnPhase::BuyOrAuction) {
        // Replicating swappy-20261120-182642.png: À VENDRE | M160 | Title Deed Card | [ACHETER] [ENCHÈRE]
        int spIdx = actP.position;
        const Space& s = m_game->board().space(spIdx);
        double mw = std::min(340.0, cRect.width() - 16.0);
        double mh = std::min(365.0, cRect.height() - 12.0);
        QRectF mRect(cRect.center().x() - mw / 2.0, cRect.center().y() - mh / 2.0, mw, mh);
        paintModalFrame(p, mRect, trText("mono_for_sale"));

        // Price banner
        QRectF priceBar(mRect.left() + 18, mRect.top() + 42, mRect.width() - 36, 34);
        p.setPen(QPen(QColor("#D4AF37"), 1.3));
        p.setBrush(m_isDarkMode ? QColor("#0B0B0E") : QColor("#F3EBD3"));
        p.drawRoundedRect(priceBar, 6, 6);

        QFont pf = p.font();
        pf.setPixelSize(22);
        pf.setBold(true);
        p.setFont(pf);
        p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
        p.drawText(priceBar, Qt::AlignCenter, formatMoney(s.price));

        // Title Deed card in center
        double cw = 152.0;
        double ch = mh - 142.0;
        QRectF cardR(mRect.center().x() - cw / 2.0, priceBar.bottom() + 8, cw, ch);
        paintTitleDeedCard(p, cardR, spIdx, false);

        // ACHETER & ENCHÈRE buttons
        double bw = 118.0, bh = 36.0;
        QRectF buyBtn(mRect.left() + 26, mRect.bottom() - bh - 12, bw, bh);
        QRectF aucBtn(mRect.right() - 26 - bw, mRect.bottom() - bh - 12, bw, bh);
        paintGoldButton(p, "modal_buy_prop", buyBtn, trText("mono_buy_btn"),
                        !actP.isAI && actP.money >= s.price, true, 13);
        paintGoldButton(p, "modal_auction_prop", aucBtn, trText("mono_auction_btn"),
                        !actP.isAI, false, 13);
    }
    else if (ph == TurnPhase::Auction) {
        // Replicating swappy-20261120-182204.png right-side auction UI!
        const AuctionState& auc = m_game->auctionState();
        double mw = std::min(345.0, cRect.width() - 16.0);
        double mh = std::min(370.0, cRect.height() - 12.0);
        QRectF mRect(cRect.center().x() - mw / 2.0, cRect.center().y() - mh / 2.0, mw, mh);
        paintModalFrame(p, mRect, trText("mono_auction_btn"));

        // Highest bid banner
        QRectF bidBar(mRect.left() + 18, mRect.top() + 40, mRect.width() - 36, 28);
        p.setPen(QPen(QColor("#D4AF37"), 1.2));
        p.setBrush(m_isDarkMode ? QColor("#0B0B0E") : QColor("#F3EBD3"));
        p.drawRoundedRect(bidBar, 5, 5);

        QString highTxt = (auc.highestBidderId >= 0)
                          ? trText("mono_highest_bid").arg(auc.currentBid).arg(playerDisplayName(auc.highestBidderId))
                          : formatMoney(m_game->board().space(auc.spaceIndex).price);
        QFont bf = p.font();
        bf.setPixelSize(13);
        bf.setBold(true);
        p.setFont(bf);
        p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
        p.drawText(bidBar, Qt::AlignCenter, highTxt);

        // Title Deed Card
        double cw = 146.0;
        double ch = mh - 166.0;
        QRectF cardR(mRect.center().x() - cw / 2.0, bidBar.bottom() + 6, cw, ch);
        paintTitleDeedCard(p, cardR, auc.spaceIndex, false);

        // Bottom auction controls bar: Active Bidder Token + Name | Spin box | MISER / SE COUCHER
        QRectF botArea(mRect.left() + 14, cardR.bottom() + 8, mRect.width() - 28, 76);
        p.setPen(QPen(QColor("#D4AF37"), 1.0));
        p.setBrush(m_isDarkMode ? QColor("#181612") : QColor("#F5EED8"));
        p.drawRoundedRect(botArea, 6, 6);

        const Player& bidder = m_game->player(auc.activeBidderId);
        paintTokenIcon(p, QRectF(botArea.left() + 8, botArea.top() + 8, 36, 34), bidder.token, true);

        QFont tf = p.font();
        tf.setPixelSize(12);
        tf.setBold(true);
        p.setFont(tf);
        p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#241E0E"));
        p.drawText(QRectF(botArea.left() + 50, botArea.top() + 6, 140, 18),
                   Qt::AlignLeft | Qt::AlignVCenter, playerDisplayName(bidder.id));
        tf.setPixelSize(11);
        tf.setBold(false);
        p.setFont(tf);
        p.drawText(QRectF(botArea.left() + 50, botArea.top() + 22, 140, 18),
                   Qt::AlignLeft | Qt::AlignVCenter, trText("mono_bid_turn"));

        // Bid amount display + Up/Down arrows
        QRectF spinBox(botArea.left() + 8, botArea.bottom() - 30, 120, 24);
        p.setPen(QPen(QColor("#D4AF37"), 1.2));
        p.setBrush(m_isDarkMode ? QColor("#0B0B0D") : QColor("#FFFFFF"));
        p.drawRoundedRect(spinBox, 4, 4);
        tf.setBold(true);
        p.setFont(tf);
        p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
        p.drawText(spinBox.adjusted(6, 0, -28, 0), Qt::AlignCenter, formatMoney(auc.inputBid));

        paintGoldButton(p, "auc_inc_bid", QRectF(spinBox.right() - 24, spinBox.top() + 1, 22, 11),
                        "▲", !bidder.isAI && auc.inputBid + 10 <= bidder.money, false, 8);
        paintGoldButton(p, "auc_dec_bid", QRectF(spinBox.right() - 24, spinBox.top() + 12, 22, 11),
                        "▼", !bidder.isAI && auc.inputBid > auc.currentBid + 1, false, 8);

        // MISER & SE COUCHER buttons on right of botArea
        QRectF bidBtn(botArea.right() - 116, botArea.top() + 8, 108, 28);
        QRectF foldBtn(botArea.right() - 116, botArea.bottom() - 34, 108, 26);
        paintGoldButton(p, "auc_place_bid", bidBtn, trText("mono_bid_btn"),
                        !bidder.isAI && auc.inputBid > auc.currentBid && auc.inputBid <= bidder.money, true, 12);
        paintGoldButton(p, "auc_fold", foldBtn, trText("mono_fold_btn"),
                        !bidder.isAI, false, 11);
    }
    else if (ph == TurnPhase::RentPaymentModal) {
        // Replicating swappy-20261120-182222.png: PAYEZ LOYER | LOYER 6 M | Payer -> Owner
        const RentNotification& rn = m_game->rentNotification();
        QRectF mRect(cRect.center().x() - 155, cRect.center().y() - 105, 310, 210);
        paintModalFrame(p, mRect, trText("mono_pay_rent_title"));

        QFont f = p.font();
        f.setPixelSize(13);
        f.setBold(true);
        p.setFont(f);
        p.setPen(m_isDarkMode ? QColor("#F4F4F5") : QColor("#18181B"));
        p.drawText(QRectF(mRect.left(), mRect.top() + 42, mRect.width(), 18),
                   Qt::AlignCenter, trText("mono_rent_label"));

        f.setPixelSize(18);
        p.setFont(f);
        p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
        p.drawText(QRectF(mRect.left(), mRect.top() + 60, mRect.width(), 24),
                   Qt::AlignCenter, formatMoney(rn.amount));

        p.setPen(QPen(QColor("#D4AF37"), 1.0));
        p.drawLine(QPointF(mRect.left() + 16, mRect.top() + 88), QPointF(mRect.right() - 16, mRect.top() + 88));

        // Payer token on left, arrow in middle, Owner token on right
        const Player& pFrom = m_game->player(rn.fromPlayerId);
        const Player& pTo = m_game->player(rn.toPlayerId);

        f.setPixelSize(12);
        p.setFont(f);
        p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#241E0E"));
        p.drawText(QRectF(mRect.left() + 18, mRect.top() + 92, 100, 18), Qt::AlignCenter, playerDisplayName(pFrom.id));
        p.drawText(QRectF(mRect.right() - 118, mRect.top() + 92, 100, 18), Qt::AlignCenter, playerDisplayName(pTo.id));

        paintTokenIcon(p, QRectF(mRect.left() + 42, mRect.top() + 112, 52, 48), pFrom.token, true);
        paintTokenIcon(p, QRectF(mRect.right() - 94, mRect.top() + 112, 52, 48), pTo.token, true);

        // Glowing arrow between tokens
        p.setPen(QPen(QColor("#D4AF37"), 1.4));
        p.setBrush(QColor("#4ADE80"));
        QPointF ac(mRect.center().x(), mRect.top() + 136);
        QPolygonF arr;
        arr << QPointF(ac.x() - 12, ac.y() - 12) << QPointF(ac.x() + 14, ac.y()) << QPointF(ac.x() - 12, ac.y() + 12);
        p.drawPolygon(arr);

        QRectF okBtn(mRect.center().x() - 52, mRect.bottom() - 38, 104, 28);
        paintGoldButton(p, "modal_rent_close", okBtn, trText("mono_close_btn"), true, true, 12);
    }
    else if (ph == TurnPhase::CardModal && !m_cardAnim.active) {
        // Replicating monopoly-mega-paris...(6).jpg & 182238.png: Black & Gold / White & Gold Chance & Community Chest card!
        const Card& c = m_game->drawnCard();
        QRectF cardR(cRect.center().x() - 165, cRect.center().y() - 110, 330, 220);

        // Card drop shadow
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 130));
        p.drawRoundedRect(cardR.translated(3, 5), 10, 10);

        p.setPen(QPen(QColor("#D4AF37"), 2.5));
        p.setBrush(m_isDarkMode ? QColor("#0B0B0E") : QColor("#FFFFFF"));
        p.drawRoundedRect(cardR, 10, 10);
        p.setPen(QPen(QColor("#D4AF37"), 1.0));
        p.drawRoundedRect(cardR.adjusted(6, 6, -6, -6), 7, 7);

        QString title = c.isChance ? trText("mono_space_7") : trText("mono_space_2");
        QFont tf = p.font();
        tf.setPixelSize(17);
        tf.setBold(true);
        p.setFont(tf);
        p.setPen(QColor("#D4AF37"));
        p.drawText(QRectF(cardR.left() + 16, cardR.top() + 14, cardR.width() - 32, 24),
                   Qt::AlignCenter, title);

        // Illustration on right side of card (Mr. Monopoly)
        paintMrMonopoly(p, QRectF(cardR.right() - 102, cardR.top() + 44, 88, 118));

        // Card text on left side
        tf.setPixelSize(13);
        tf.setBold(true);
        p.setFont(tf);
        p.setPen(m_isDarkMode ? QColor("#F4F4F5") : QColor("#18181B"));
        p.drawText(QRectF(cardR.left() + 20, cardR.top() + 46, cardR.width() - 126, 118),
                   Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap,
                   trText(QString::fromStdString(c.textKey)));

        QRectF okBtn(cardR.center().x() - 55, cardR.bottom() - 44, 110, 32);
        paintGoldButton(p, "modal_card_close", okBtn, trText("mono_close_btn"), true, true, 12);
    }
    else if (ph == TurnPhase::DebtResolution) {
        QRectF mRect(cRect.center().x() - 165, cRect.center().y() - 95, 330, 190);
        paintModalFrame(p, mRect, trText("mono_debt_title").arg(m_game->pendingDebtAmount()));

        QFont f = p.font();
        f.setPixelSize(12);
        f.setBold(false);
        p.setFont(f);
        p.setPen(m_isDarkMode ? QColor("#E4E4E7") : QColor("#27272A"));
        p.drawText(QRectF(mRect.left() + 18, mRect.top() + 50, mRect.width() - 36, 75),
                   Qt::AlignCenter | Qt::TextWordWrap, trText("mono_debt_hint"));

        QRectF bkBtn(mRect.center().x() - 65, mRect.bottom() - 46, 130, 34);
        paintGoldButton(p, "modal_declare_bankrupt", bkBtn, trText("mono_bankrupt_btn"), !actP.isAI, false, 13);
    }
    else if (ph == TurnPhase::WaitingForEndTurn) {
        // Replicating swappy-20261120-182709.png: Player Name, Token Icon, [FINIR TOUR] (or [JETER LES DÉS] if double!)
        QRectF mRect(cRect.center().x() - 160, cRect.center().y() - 58, 320, 116);
        paintModalFrame(p, mRect, playerDisplayName(actP.id));

        paintTokenIcon(p, QRectF(mRect.left() + 22, mRect.top() + 44, 62, 58), actP.token, true);

        QRectF endBtn(mRect.left() + 98, mRect.top() + 50, mRect.width() - 118, 44);
        QString btnLabel = m_game->canRollAgain() ? trText("mono_roll_dice") : trText("mono_end_turn");
        QString btnId = m_game->canRollAgain() ? "modal_roll_dice" : "modal_end_turn";
        paintGoldButton(p, btnId, endBtn, btnLabel, !actP.isAI, true, 15);
    }
    else if (ph == TurnPhase::GameOver) {
        int winId = m_game->winnerId();
        QString winName = (winId >= 0) ? playerDisplayName(winId) : "";
        QRectF mRect(cRect.center().x() - 170, cRect.center().y() - 110, 340, 220);
        paintModalFrame(p, mRect, trText("mono_winner_title").arg(winName));

        if (winId >= 0) {
            paintTokenIcon(p, QRectF(mRect.center().x() - 36, mRect.top() + 52, 72, 68),
                           m_game->player(winId).token, true);
            QFont f = p.font();
            f.setPixelSize(14);
            f.setBold(true);
            p.setFont(f);
            p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
            p.drawText(QRectF(mRect.left(), mRect.top() + 128, mRect.width(), 24),
                       Qt::AlignCenter,
                       trText("mono_winner_sub").arg(m_game->playerNetWorth(winId)));
        }

        QRectF newBtn(mRect.center().x() - 75, mRect.bottom() - 46, 150, 34);
        paintGoldButton(p, "menu_new_game", newBtn, trText("mono_new_game_btn"), true, true, 13);
    }
}

// -----------------------------------------------------------------------------
// Action Overlays (MENU, CONSTRUIRE, VENDRE, HYPOTHÉQUER, LEVER L'HYPOTHÈQUE, INSPECT DEED)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintActionOverlay(QPainter& p, const QRectF& cRect) {
    if (!m_game) return;

    if (m_overlayMode == ActionOverlayMode::Menu) {
        QRectF mRect(cRect.center().x() - 145, cRect.center().y() - 135, 290, 270);
        paintModalFrame(p, mRect, trText("mono_menu_btn"));

        double bw = 220.0, bh = 34.0, bx = mRect.center().x() - bw / 2.0;
        paintGoldButton(p, "overlay_close",   QRectF(bx, mRect.top() + 52,  bw, bh), trText("mono_resume_btn"), true, true, 13);
        paintGoldButton(p, "menu_new_game",   QRectF(bx, mRect.top() + 94,  bw, bh), trText("mono_new_game_btn"), true, false, 12);
        paintGoldButton(p, "menu_lobby",      QRectF(bx, mRect.top() + 136, bw, bh), trText("mono_setup_lobby_btn"), true, false, 12);
        paintGoldButton(p, "lobby_rules",     QRectF(bx, mRect.top() + 178, bw, bh), trText("mono_rules_btn"), true, false, 12);
        paintGoldButton(p, "lobby_back",      QRectF(bx, mRect.top() + 220, bw, bh), trText("main_menu"), true, false, 12);
        return;
    }

    if (m_overlayMode == ActionOverlayMode::InspectDeed && m_inspectedSpaceIndex >= 0) {
        QRectF mRect(cRect.center().x() - 130, cRect.center().y() - 165, 260, 330);
        paintModalFrame(p, mRect);
        paintTitleDeedCard(p, QRectF(mRect.center().x() - 95, mRect.top() + 16, 190, 250),
                           m_inspectedSpaceIndex, true);
        paintGoldButton(p, "overlay_close", QRectF(mRect.center().x() - 55, mRect.bottom() - 46, 110, 32),
                        trText("mono_close_btn"), true, true, 12);
        return;
    }

    if (m_overlayMode == ActionOverlayMode::BankInfo) {
        QRectF mRect(cRect.center().x() - 155, cRect.center().y() - 100, 310, 200);
        paintModalFrame(p, mRect, trText("mono_bank"));
        paintHouseIcon(p, QRectF(mRect.left() + 45, mRect.top() + 65, 36, 36), false);
        paintHouseIcon(p, QRectF(mRect.right() - 125, mRect.top() + 65, 36, 36), true);

        QFont f = p.font();
        f.setPixelSize(18);
        f.setBold(true);
        p.setFont(f);
        p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
        p.drawText(QRectF(mRect.left() + 90, mRect.top() + 65, 60, 36), Qt::AlignVCenter,
                   QString("%1 / 32").arg(m_game->board().bankHousesAvailable()));
        p.setPen(m_isDarkMode ? QColor("#F87171") : QColor("#B91C1C"));
        p.drawText(QRectF(mRect.right() - 82, mRect.top() + 65, 65, 36), Qt::AlignVCenter,
                   QString("%1 / 12").arg(m_game->board().bankHotelsAvailable()));

        paintGoldButton(p, "overlay_close", QRectF(mRect.center().x() - 55, mRect.bottom() - 46, 110, 32),
                        trText("mono_close_btn"), true, true, 12);
        return;
    }

    // Build / Sell / Mortgage / Unmortgage interactive property list modal
    QString titleKey = "mono_build_btn";
    if (m_overlayMode == ActionOverlayMode::Sell) titleKey = "mono_sell_btn";
    else if (m_overlayMode == ActionOverlayMode::Mortgage) titleKey = "mono_mortgage_btn";
    else if (m_overlayMode == ActionOverlayMode::Unmortgage) titleKey = "mono_unmortgage_btn";

    double mw = std::min(350.0, cRect.width() - 16.0);
    double mh = std::min(340.0, cRect.height() - 16.0);
    QRectF mRect(cRect.center().x() - mw / 2.0, cRect.center().y() - mh / 2.0, mw, mh);
    paintModalFrame(p, mRect, trText(titleKey));

    int actId = m_game->activePlayerIndex();
    int cash = m_game->player(actId).money;
    std::vector<int> eligible;
    for (int idx : m_game->board().playerProperties(actId)) {
        if (m_overlayMode == ActionOverlayMode::Build && m_game->board().canBuildOn(actId, idx, cash)) eligible.push_back(idx);
        else if (m_overlayMode == ActionOverlayMode::Sell && m_game->board().canSellBuildingOn(actId, idx)) eligible.push_back(idx);
        else if (m_overlayMode == ActionOverlayMode::Mortgage && m_game->board().canMortgage(actId, idx)) eligible.push_back(idx);
        else if (m_overlayMode == ActionOverlayMode::Unmortgage && m_game->board().canUnmortgage(actId, idx, cash)) eligible.push_back(idx);
    }

    double ry = mRect.top() + 50.0;
    for (size_t i = 0; i < eligible.size() && i < 7; ++i) {
        int spIdx = eligible[i];
        const Space& s = m_game->board().space(spIdx);
        QRectF rowR(mRect.left() + 14, ry + i * 33.0, mRect.width() - 28, 29.0);

        p.setPen(QPen(QColor("#D4AF37"), 1.0));
        p.setBrush(m_isDarkMode ? QColor("#18181F") : QColor("#FFFFFF"));
        p.drawRoundedRect(rowR, 5, 5);

        p.fillRect(QRectF(rowR.left() + 4, rowR.top() + 4, 10, rowR.height() - 8), groupColor(s.group));

        QFont rf = p.font();
        rf.setPixelSize(11);
        rf.setBold(true);
        p.setFont(rf);
        p.setPen(m_isDarkMode ? QColor("#F4F4F5") : QColor("#18181B"));
        p.drawText(QRectF(rowR.left() + 20, rowR.top(), rowR.width() - 115, rowR.height()),
                   Qt::AlignVCenter | Qt::AlignLeft, spaceName(spIdx));

        int amount = s.houseCost;
        if (m_overlayMode == ActionOverlayMode::Sell) amount = s.houseCost / 2;
        else if (m_overlayMode == ActionOverlayMode::Mortgage) amount = s.mortgageValue;
        else if (m_overlayMode == ActionOverlayMode::Unmortgage) amount = m_game->board().unmortgageCost(spIdx);

        QRectF actBtn(rowR.right() - 88, rowR.top() + 3, 84, rowR.height() - 6);
        paintGoldButton(p, QString("prop_action_%1").arg(spIdx), actBtn, formatMoney(amount), true, true, 11);
    }

    paintGoldButton(p, "overlay_close", QRectF(mRect.center().x() - 55, mRect.bottom() - 42, 110, 30),
                    trText("mono_close_btn"), true, false, 12);
}

// -----------------------------------------------------------------------------
// Trade Overlay (Replicating swappy-20261120-182328.png 2-column ÉCHANGE dialog!)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintTradeOverlay(QPainter& p, const QRectF& cRect) {
    if (!m_game) return;
    double mw = std::min(370.0, cRect.width() - 8.0);
    double mh = std::min(370.0, cRect.height() - 8.0);
    QRectF mRect(cRect.center().x() - mw / 2.0, cRect.center().y() - mh / 2.0, mw, mh);
    paintModalFrame(p, mRect, trText("mono_trade_btn"));

    int fromId = m_tradeDraft.fromPlayerId;
    int toId = m_tradeDraft.toPlayerId;
    const Player& pFrom = m_game->player(fromId);
    const Player& pTo = m_game->player(toId);

    // Left column (Active Player) & Right column (Opponent Player)
    QRectF leftCol(mRect.left() + 12, mRect.top() + 42, 108, mh - 54);
    QRectF rightCol(mRect.right() - 120, mRect.top() + 42, 108, mh - 54);
    QRectF midCol(leftCol.right() + 6, mRect.top() + 42, rightCol.left() - leftCol.right() - 12, mh - 54);

    // Player headers
    paintTokenIcon(p, QRectF(leftCol.left() + 4, leftCol.top(), 28, 26), pFrom.token, true);
    paintTokenIcon(p, QRectF(rightCol.right() - 32, rightCol.top(), 28, 26), pTo.token, true);

    QFont f = p.font();
    f.setPixelSize(11);
    f.setBold(true);
    p.setFont(f);
    p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#241E0E"));
    p.drawText(QRectF(leftCol.left() + 36, leftCol.top(), 70, 26), Qt::AlignVCenter | Qt::AlignLeft, playerDisplayName(fromId));

    // Clicking opponent name cycles opponent if >2 players
    QRectF oppNameRect(rightCol.left(), rightCol.top(), 72, 26);
    m_buttons.push_back({"trade_cycle_opp", oppNameRect, m_game->numPlayers() > 2});
    p.drawText(oppNameRect, Qt::AlignVCenter | Qt::AlignRight, playerDisplayName(toId));

    // Mini property grids for both players (click owned property to toggle in trade!)
    auto drawTradePropColumn = [&](int pid, const QRectF& colR, const std::vector<int>& selectedList, bool isLeft) {
        const ColorGroup groups[10] = {
            ColorGroup::Brown, ColorGroup::LightBlue, ColorGroup::Pink, ColorGroup::Orange,
            ColorGroup::Red, ColorGroup::Yellow, ColorGroup::Green, ColorGroup::DarkBlue,
            ColorGroup::Railroad, ColorGroup::Utility
        };
        double gy = colR.top() + 32.0;
        for (int g = 0; g < 10; ++g) {
            auto spList = m_game->board().groupSpaces(groups[g]);
            double gx = (g % 2 == 0) ? colR.left() + 4 : colR.left() + 56;
            double rowY = gy + (g / 2) * 34.0;
            for (size_t k = 0; k < spList.size(); ++k) {
                int spIdx = spList[k];
                const Space& sp = m_game->board().space(spIdx);
                QRectF mr(gx + k * 12.0, rowY, 11.0, 22.0);

                bool owned = (sp.ownerId == pid) && !(sp.type == SpaceType::Property && m_game->board().groupHasAnyBuildings(sp.group));
                bool isSel = std::find(selectedList.begin(), selectedList.end(), spIdx) != selectedList.end();

                if (owned) {
                    m_buttons.push_back({QString("trade_toggle_%1_%2").arg(isLeft ? "L" : "R").arg(spIdx), mr, true});
                    p.setPen(QPen(isSel ? QColor("#4ADE80") : QColor("#D4AF37"), isSel ? 2.0 : 1.0));
                    p.setBrush(isSel ? QColor("#4ADE80") : (m_isDarkMode ? QColor("#F4F4F5") : QColor("#FFFFFF")));
                    p.drawRect(mr);
                    p.fillRect(QRectF(mr.left() + 1, mr.top() + 1, mr.width() - 2, 5), groupColor(groups[g]));
                } else {
                    p.setPen(QPen(groupColor(groups[g]).darker(130), 0.9));
                    p.setBrush(Qt::NoBrush);
                    p.drawRect(mr);
                }
            }
        }
    };

    drawTradePropColumn(fromId, leftCol, m_tradeDraft.offeredProperties, true);
    drawTradePropColumn(toId, rightCol, m_tradeDraft.requestedProperties, false);

    // Cash transfer controls at bottom of Left & Right columns
    QRectF lCashBox(leftCol.left(), leftCol.bottom() - 36, 76, 26);
    p.setPen(QPen(QColor("#D4AF37"), 1.1));
    p.setBrush(m_isDarkMode ? QColor("#0B0B0D") : QColor("#FFFFFF"));
    p.drawRoundedRect(lCashBox, 4, 4);
    p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
    p.drawText(lCashBox, Qt::AlignCenter, formatMoney(m_tradeDraft.offeredCash));
    paintGoldButton(p, "trade_l_cash_up", QRectF(lCashBox.right() + 2, lCashBox.top(), 24, 12), "▲", m_tradeDraft.offeredCash + 20 <= pFrom.money, false, 8);
    paintGoldButton(p, "trade_l_cash_dn", QRectF(lCashBox.right() + 2, lCashBox.top() + 14, 24, 12), "▼", m_tradeDraft.offeredCash >= 20, false, 8);

    QRectF rCashBox(rightCol.left() + 28, rightCol.bottom() - 36, 76, 26);
    p.setPen(QPen(QColor("#D4AF37"), 1.1));
    p.setBrush(m_isDarkMode ? QColor("#0B0B0D") : QColor("#FFFFFF"));
    p.drawRoundedRect(rCashBox, 4, 4);
    p.setPen(m_isDarkMode ? QColor("#4ADE80") : QColor("#15803D"));
    p.drawText(rCashBox, Qt::AlignCenter, formatMoney(m_tradeDraft.requestedCash));
    paintGoldButton(p, "trade_r_cash_up", QRectF(rightCol.left() + 2, rCashBox.top(), 24, 12), "▲", m_tradeDraft.requestedCash + 20 <= pTo.money, false, 8);
    paintGoldButton(p, "trade_r_cash_dn", QRectF(rightCol.left() + 2, rCashBox.top() + 14, 24, 12), "▼", m_tradeDraft.requestedCash >= 20, false, 8);

    // Center column: Title Deed preview or trade instructions + PROPOSITION & ANNULER buttons
    if (m_tradePreviewSpace >= 0) {
        paintTitleDeedCard(p, QRectF(midCol.left() + 2, midCol.top() + 12, midCol.width() - 4, 185), m_tradePreviewSpace, false);
    } else {
        f.setPixelSize(10);
        f.setBold(false);
        p.setFont(f);
        p.setPen(m_isDarkMode ? QColor("#D4D4D8") : QColor("#3F3F46"));
        p.drawText(QRectF(midCol.left(), midCol.top() + 30, midCol.width(), 120),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   m_tradeFeedbackMsg.isEmpty() ? trText("mono_trade_hint") : m_tradeFeedbackMsg);
    }

    bool hasAnyContent = !m_tradeDraft.offeredProperties.empty() || !m_tradeDraft.requestedProperties.empty() ||
                         m_tradeDraft.offeredCash > 0 || m_tradeDraft.requestedCash > 0;

    paintGoldButton(p, "trade_propose", QRectF(midCol.left() + 4, midCol.bottom() - 68, midCol.width() - 8, 28),
                    trText("mono_propose_btn"), hasAnyContent, true, 11);
    paintGoldButton(p, "overlay_close", QRectF(midCol.left() + 4, midCol.bottom() - 36, midCol.width() - 8, 28),
                    trText("mono_cancel_btn"), true, false, 11);
}

// -----------------------------------------------------------------------------
// 11-Page Interactive Visual Rules Carousel Overlay (Replicating 182107.png - 182435.png)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintRulesCarouselOverlay(QPainter& p) {
    p.save();
    p.fillRect(rect(), QColor(0, 0, 0, 155));

    double mw = std::min(760.0, width() - 40.0);
    double mh = std::min(470.0, height() - 40.0);
    QRectF mRect(rect().center().x() - mw / 2.0, rect().center().y() - mh / 2.0, mw, mh);
    paintModalFrame(p, mRect);

    int page = std::clamp(m_rulesPage, 0, 10);
    QString title = trText(QString("mono_rule_title_%1").arg(page));
    QString body  = trText(QString("mono_rule_body_%1").arg(page));

    // Title
    QFont tf = p.font();
    tf.setPixelSize(22);
    tf.setBold(true);
    p.setFont(tf);
    p.setPen(m_isDarkMode ? QColor("#F3E5AB") : QColor("#8A6608"));
    p.drawText(QRectF(mRect.left() + 36, mRect.top() + 26, mRect.width() - 72, 32),
               Qt::AlignLeft | Qt::AlignVCenter, title);

    // Page counter badge on top-right
    tf.setPixelSize(13);
    p.setFont(tf);
    p.drawText(QRectF(mRect.right() - 110, mRect.top() + 26, 74, 32),
               Qt::AlignRight | Qt::AlignVCenter, QString("%1 / 11").arg(page + 1));

    // Body text on left 60%, visual diagram on right 35% (just like the screenshots!)
    QRectF textRect(mRect.left() + 36, mRect.top() + 74, mRect.width() * 0.56, mRect.height() - 150);
    QRectF illustRect(textRect.right() + 20, mRect.top() + 74, mRect.right() - textRect.right() - 50, mRect.height() - 150);

    QFont bf = p.font();
    bf.setPixelSize(14);
    bf.setBold(false);
    p.setFont(bf);
    p.setPen(m_isDarkMode ? QColor("#F4F4F5") : QColor("#18181B"));
    p.drawText(textRect, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, body);

    // Visual illustration per rule slide matching the screenshots!
    if (page == 0 || page == 1) {
        paintMrMonopoly(p, illustRect);
    } else if (page == 2) {
        // 3D Dice pair (182146.png)
        paint3DDie(p, QRectF(illustRect.center().x() - 75, illustRect.center().y() - 55, 68, 68), 5, -14.0);
        paint3DDie(p, QRectF(illustRect.center().x() + 5,  illustRect.center().y() - 25, 68, 68), 3, 12.0);
    } else if (page == 3 || page == 9) {
        // Title Deed Card preview (182204.png)
        if (m_game) {
            paintTitleDeedCard(p, QRectF(illustRect.center().x() - 75, illustRect.center().y() - 105, 150, 210),
                               6, page == 9);
        }
    } else if (page == 4) {
        // Pay Rent preview box (182222.png)
        QRectF prBox(illustRect.center().x() - 95, illustRect.center().y() - 65, 190, 130);
        paintModalFrame(p, prBox, trText("mono_pay_rent_title"));
        paintTokenIcon(p, QRectF(prBox.left() + 20, prBox.bottom() - 58, 44, 42), TokenType::Boot, true);
        paintTokenIcon(p, QRectF(prBox.right() - 64, prBox.bottom() - 58, 44, 42), TokenType::TopHat, true);
    } else if (page == 5) {
        // Chance & Community Chest cards (182238.png)
        QRectF c1(illustRect.left() + 10, illustRect.center().y() - 70, 150, 95);
        QRectF c2(illustRect.left() + 45, illustRect.center().y() - 10, 150, 95);
        paintModalFrame(p, c1, trText("mono_space_7"));
        paintModalFrame(p, c2, trText("mono_space_2"));
    } else if (page == 6) {
        // Case Départ & Prison tiles (182254.png & 182310.png)
        paintCornerTile(p, 0, QRectF(illustRect.center().x() - 85, illustRect.top() + 10, 110, 110));
        paintCornerTile(p, 10, QRectF(illustRect.center().x() - 25, illustRect.top() + 130, 110, 110));
    } else if (page == 7) {
        // Trade button & tokens
        paintTokenIcon(p, QRectF(illustRect.center().x() - 80, illustRect.center().y() - 35, 64, 64), TokenType::Racecar, true);
        paintTokenIcon(p, QRectF(illustRect.center().x() + 16, illustRect.center().y() - 35, 64, 64), TokenType::Dog, true);
    } else if (page == 8) {
        // 3D Green House & Red Hotel (182347.png & 182404.png)
        paintHouseIcon(p, QRectF(illustRect.center().x() - 95, illustRect.center().y() - 50, 85, 85), false);
        paintHouseIcon(p, QRectF(illustRect.center().x() + 10, illustRect.center().y() - 55, 92, 92), true);
    } else {
        paintMrMonopoly(p, illustRect);
    }

    // Bottom navigation: [<] (left) | [FERMER] (center) | [>] (right)
    QRectF prevBtn(mRect.left() + 28, mRect.bottom() - 54, 48, 38);
    QRectF closeBtn(mRect.center().x() - 65, mRect.bottom() - 54, 130, 38);
    QRectF nextBtn(mRect.right() - 76, mRect.bottom() - 54, 48, 38);

    if (page > 0) {
        paintGoldButton(p, "rules_prev", prevBtn, "◀", true, false, 16);
    }
    paintGoldButton(p, "rules_close", closeBtn, trText("mono_close_btn"), true, true, 14);
    if (page < 10) {
        paintGoldButton(p, "rules_next", nextBtn, "▶", true, false, 16);
    }

    p.restore();
}

// -----------------------------------------------------------------------------
// Active Animations Overlay (3D Tumbling Dice, Flying/Flipping Cards, Floating Money)
// -----------------------------------------------------------------------------
void MonopolyWidget::paintAnimationsOverlay(QPainter& p) {
    QRectF cRect = boardCenterRect();

    // 1. Dice Tumbling Animation across board center
    if (m_diceAnimActive) {
        double t = m_diceAnimProgress;
        double bounce = std::abs(std::sin(t * M_PI * 3.0)) * (1.0 - t) * 55.0;
        double angle1 = (1.0 - t) * 540.0;
        double angle2 = -(1.0 - t) * 480.0;

        QPointF center = cRect.center() + QPointF(0, -bounce);
        paint3DDie(p, QRectF(center.x() - 62, center.y() - 27, 54, 54), m_animDieFace1, angle1);
        paint3DDie(p, QRectF(center.x() + 8,  center.y() - 27, 54, 54), m_animDieFace2, angle2);
    }

    // 2. Flying / 3D-Flipping Card Animation
    if (m_cardAnim.active) {
        double t = m_cardAnim.progress;
        // Smooth ease-out curve
        double ease = 1.0 - std::pow(1.0 - t, 3.0);
        QPointF curPos = m_cardAnim.startPos * (1.0 - ease) + m_cardAnim.endPos * ease;

        if (m_cardAnim.isDeedToSidebar) {
            double scale = 1.0 - 0.75 * ease;
            double w = 130.0 * scale;
            double h = 175.0 * scale;
            paintTitleDeedCard(p, QRectF(curPos.x() - w / 2.0, curPos.y() - h / 2.0, w, h),
                               m_cardAnim.spaceIndex, false);
        } else {
            // 3D horizontal flip from deck slot to center modal
            double scale = 0.35 + 0.65 * ease;
            double flipCos = std::cos(ease * M_PI);
            double w = 310.0 * scale * std::max(0.08, std::abs(flipCos));
            double h = 200.0 * scale;
            QRectF r(curPos.x() - w / 2.0, curPos.y() - h / 2.0, w, h);

            p.save();
            p.setPen(QPen(QColor("#D4AF37"), 2.2));
            p.setBrush(m_isDarkMode ? QColor("#0B0B0E") : QColor("#FFFFFF"));
            p.drawRoundedRect(r, 8, 8);
            if (ease < 0.5) {
                if (m_cardAnim.isChance) {
                    paintQuestionIcon(p, QRectF(r.center().x() - 20, r.center().y() - 24, 40, 48), QColor("#D4AF37"));
                } else {
                    paintChestIcon(p, QRectF(r.center().x() - 20, r.center().y() - 20, 40, 40));
                }
            }
            p.restore();
        }
    }

    // 3. Floating Golden Banknotes & Currency Badges
    for (const auto& ma : m_moneyAnims) {
        double t = ma.progress;
        double ease = 1.0 - std::pow(1.0 - t, 2.0);
        QPointF pos = ma.startPos * (1.0 - ease) + ma.endPos * ease - QPointF(0, std::sin(t * M_PI) * 28.0);
        double alpha = (t < 0.8) ? 1.0 : ((1.0 - t) / 0.2);

        p.save();
        p.setOpacity(std::clamp(alpha, 0.0, 1.0));

        QRectF billRect(pos.x() - 46, pos.y() - 16, 92, 32);
        QLinearGradient bGrad(billRect.topLeft(), billRect.bottomRight());
        if (ma.isPositive) {
            bGrad.setColorAt(0.0, QColor("#14532D"));
            bGrad.setColorAt(1.0, QColor("#064E3B"));
        } else {
            bGrad.setColorAt(0.0, QColor("#7F1D1D"));
            bGrad.setColorAt(1.0, QColor("#450A0A"));
        }
        p.setPen(QPen(QColor("#D4AF37"), 1.6));
        p.setBrush(bGrad);
        p.drawRoundedRect(billRect, 6, 6);

        QFont f = p.font();
        f.setPixelSize(13);
        f.setBold(true);
        p.setFont(f);
        p.setPen(QColor("#F3E5AB"));
        QString txt = (ma.isPositive ? "+" : "-") + formatMoney(ma.amount);
        p.drawText(billRect, Qt::AlignCenter, txt);
        p.restore();
    }
}

// -----------------------------------------------------------------------------
// Mouse & Wheel Input Handling
// -----------------------------------------------------------------------------
void MonopolyWidget::mouseMoveEvent(QMouseEvent *event) {
    QPointF pos = event->pos();
    QString newHover;
    for (auto it = m_buttons.rbegin(); it != m_buttons.rend(); ++it) {
        if (it->enabled && it->rect.contains(pos)) {
            newHover = it->id;
            break;
        }
    }

    int newSpaceHover = -1;
    if (m_screen == ViewScreen::InGame) {
        for (int i = 0; i < MonopolyBoard::NUM_SPACES; ++i) {
            if (spaceRect(i).contains(pos)) {
                newSpaceHover = i;
                break;
            }
        }
    }

    if (newHover != m_hoveredButtonId || newSpaceHover != m_hoveredSpaceIndex) {
        m_hoveredButtonId = newHover;
        m_hoveredSpaceIndex = newSpaceHover;
        setCursor((!m_hoveredButtonId.isEmpty() || m_hoveredSpaceIndex >= 0)
                  ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
}

void MonopolyWidget::leaveEvent(QEvent * /*event*/) {
    m_hoveredButtonId.clear();
    m_hoveredSpaceIndex = -1;
    update();
}

void MonopolyWidget::wheelEvent(QWheelEvent *event) {
    if (m_screen == ViewScreen::InGame && rightPanelRect().contains(event->position())) {
        int delta = event->angleDelta().y();
        if (delta < 0) m_historyScrollOffset += 1;
        else if (delta > 0 && m_historyScrollOffset > 0) m_historyScrollOffset -= 1;
        update();
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

void MonopolyWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;
    QPointF pos = event->pos();

    // If an animation (dice roll, token hop, or card flip) is playing and the user clicks on the board, skip it immediately!
    if (m_screen == ViewScreen::InGame && isAnimating() && boardOuterRect().contains(pos)) {
        skipActiveAnimations();
        return;
    }

    // 1. Check registered interactive buttons (reverse order so top overlays take precedence)
    for (auto it = m_buttons.rbegin(); it != m_buttons.rend(); ++it) {
        if (it->enabled && it->rect.contains(pos)) {
            QString clickedId = it->id;
            handleButtonClick(clickedId);
            return;
        }
    }

    // 2. If in-game and clicking a board tile directly
    if (m_screen == ViewScreen::InGame && m_game && !m_showRulesOverlay) {
        for (int i = 0; i < MonopolyBoard::NUM_SPACES; ++i) {
            if (spaceRect(i).contains(pos)) {
                int actId = m_game->activePlayerIndex();
                if (m_overlayMode == ActionOverlayMode::Build && m_game->board().canBuildOn(actId, i, m_game->player(actId).money)) {
                    m_game->buildHouseOrHotel(actId, i);
                    processRecentMoneyTransfers();
                    update();
                    return;
                } else if (m_overlayMode == ActionOverlayMode::Sell && m_game->board().canSellBuildingOn(actId, i)) {
                    m_game->sellHouseOrHotel(actId, i);
                    processRecentMoneyTransfers();
                    update();
                    return;
                } else if (m_overlayMode == ActionOverlayMode::Mortgage && m_game->board().canMortgage(actId, i)) {
                    m_game->mortgageProperty(actId, i);
                    processRecentMoneyTransfers();
                    update();
                    return;
                } else if (m_overlayMode == ActionOverlayMode::Unmortgage && m_game->board().canUnmortgage(actId, i, m_game->player(actId).money)) {
                    m_game->unmortgageProperty(actId, i);
                    processRecentMoneyTransfers();
                    update();
                    return;
                } else if (m_game->board().isPurchasable(i)) {
                    m_inspectedSpaceIndex = i;
                    m_overlayMode = ActionOverlayMode::InspectDeed;
                    update();
                    return;
                }
            }
        }
    }
}

void MonopolyWidget::handleButtonClick(const QString& id) {
    // Rules Carousel controls
    if (id == "lobby_rules") {
        m_showRulesOverlay = true;
        m_rulesPage = 0;
        update();
        return;
    }
    if (id == "rules_close") {
        m_showRulesOverlay = false;
        update();
        return;
    }
    if (id == "rules_prev") {
        m_rulesPage = std::max(0, m_rulesPage - 1);
        update();
        return;
    }
    if (id == "rules_next") {
        m_rulesPage = std::min(10, m_rulesPage + 1);
        update();
        return;
    }

    // Lobby Setup controls
    if (id == "lobby_back") {
        emit backToHomeRequested();
        return;
    }
    if (id == "lobby_add_player") {
        if (m_setups.size() < 4) {
            static const TokenType defTokens[4] = {TokenType::Dog, TokenType::Battleship, TokenType::TopHat, TokenType::Racecar};
            TokenType chosen = defTokens[m_setups.size() % 4];
            for (int t = 0; t < 8; ++t) {
                TokenType cand = static_cast<TokenType>(t);
                bool used = false;
                for (const auto& s : m_setups) if (s.token == cand) used = true;
                if (!used) { chosen = cand; break; }
            }
            m_setups.push_back({"", chosen, true, AIDifficulty::Easy});
            update();
        }
        return;
    }
    if (id.startsWith("setup_remove_")) {
        int idx = id.mid(13).toInt();
        if (idx >= 2 && idx < static_cast<int>(m_setups.size())) {
            m_setups.erase(m_setups.begin() + idx);
            update();
        }
        return;
    }
    if (id.startsWith("setup_human_")) {
        int idx = id.mid(12).toInt();
        if (idx >= 0 && idx < static_cast<int>(m_setups.size())) {
            m_setups[idx].isAI = false;
            update();
        }
        return;
    }
    if (id.startsWith("setup_ai_")) {
        int idx = id.mid(9).toInt();
        if (idx >= 0 && idx < static_cast<int>(m_setups.size())) {
            m_setups[idx].isAI = true;
            update();
        }
        return;
    }
    if (id.startsWith("setup_diff_")) {
        QStringList parts = id.split('_');
        if (parts.size() == 4) {
            int idx = parts[2].toInt();
            int d = parts[3].toInt();
            if (idx >= 0 && idx < static_cast<int>(m_setups.size())) {
                m_setups[idx].difficulty = static_cast<AIDifficulty>(std::clamp(d, 0, 2));
                update();
            }
        }
        return;
    }
    if (id.startsWith("setup_pick_token_")) {
        m_editingPlayerIdx = id.mid(17).toInt();
        m_screen = ViewScreen::TokenSelect;
        update();
        return;
    }
    if (id == "lobby_play" || id == "menu_new_game") {
        startNewConfiguredGame();
        return;
    }
    if (id == "menu_lobby") {
        showLobbySetup();
        return;
    }

    // Token Select Screen controls
    if (id == "tokscreen_back") {
        m_screen = ViewScreen::LobbySetup;
        update();
        return;
    }
    if (id.startsWith("tokscreen_player_")) {
        m_editingPlayerIdx = id.mid(17).toInt();
        update();
        return;
    }
    if (id.startsWith("tokscreen_choose_")) {
        int t = id.mid(17).toInt();
        if (m_editingPlayerIdx >= 0 && m_editingPlayerIdx < static_cast<int>(m_setups.size())) {
            m_setups[m_editingPlayerIdx].token = static_cast<TokenType>(std::clamp(t, 0, 7));
            update();
        }
        return;
    }

    if (!m_game) return;

    // Sidebar & Bottom Bar controls
    if (id.startsWith("sidebar_player_")) {
        m_expandedPlayerId = id.mid(15).toInt();
        update();
        return;
    }
    if (id == "sidebar_bank") {
        m_overlayMode = (m_overlayMode == ActionOverlayMode::BankInfo) ? ActionOverlayMode::None : ActionOverlayMode::BankInfo;
        update();
        return;
    }
    if (id == "hist_scroll_up") {
        m_historyScrollOffset = std::max(0, m_historyScrollOffset - 1);
        update();
        return;
    }
    if (id == "hist_scroll_down") {
        m_historyScrollOffset += 1;
        update();
        return;
    }
    if (id.startsWith("inspect_deed_")) {
        m_inspectedSpaceIndex = id.mid(13).toInt();
        m_overlayMode = ActionOverlayMode::InspectDeed;
        update();
        return;
    }
    if (id == "overlay_close") {
        m_overlayMode = ActionOverlayMode::None;
        update();
        return;
    }
    if (id == "bar_menu") {
        m_overlayMode = (m_overlayMode == ActionOverlayMode::Menu) ? ActionOverlayMode::None : ActionOverlayMode::Menu;
        update();
        return;
    }
    if (id == "bar_build") {
        m_overlayMode = (m_overlayMode == ActionOverlayMode::Build) ? ActionOverlayMode::None : ActionOverlayMode::Build;
        update();
        return;
    }
    if (id == "bar_sell") {
        m_overlayMode = (m_overlayMode == ActionOverlayMode::Sell) ? ActionOverlayMode::None : ActionOverlayMode::Sell;
        update();
        return;
    }
    if (id == "bar_mortgage") {
        m_overlayMode = (m_overlayMode == ActionOverlayMode::Mortgage) ? ActionOverlayMode::None : ActionOverlayMode::Mortgage;
        update();
        return;
    }
    if (id == "bar_unmortgage") {
        m_overlayMode = (m_overlayMode == ActionOverlayMode::Unmortgage) ? ActionOverlayMode::None : ActionOverlayMode::Unmortgage;
        update();
        return;
    }
    if (id == "bar_trade") {
        if (m_overlayMode == ActionOverlayMode::Trade) {
            m_overlayMode = ActionOverlayMode::None;
        } else {
            m_tradeDraft = TradeOffer{};
            m_tradeDraft.fromPlayerId = m_game->activePlayerIndex();
            m_tradeDraft.toPlayerId = (m_game->activePlayerIndex() + 1) % m_game->numPlayers();
            for (int step = 1; step < m_game->numPlayers(); ++step) {
                int cand = (m_game->activePlayerIndex() + step) % m_game->numPlayers();
                if (!m_game->player(cand).bankrupt) {
                    m_tradeDraft.toPlayerId = cand;
                    break;
                }
            }
            m_tradePreviewSpace = -1;
            m_tradeFeedbackMsg.clear();
            m_overlayMode = ActionOverlayMode::Trade;
        }
        update();
        return;
    }

    // Property Build / Sell / Mortgage / Unmortgage action button
    if (id.startsWith("prop_action_")) {
        int spIdx = id.mid(12).toInt();
        int actId = m_game->activePlayerIndex();
        if (m_overlayMode == ActionOverlayMode::Build) m_game->buildHouseOrHotel(actId, spIdx);
        else if (m_overlayMode == ActionOverlayMode::Sell) m_game->sellHouseOrHotel(actId, spIdx);
        else if (m_overlayMode == ActionOverlayMode::Mortgage) m_game->mortgageProperty(actId, spIdx);
        else if (m_overlayMode == ActionOverlayMode::Unmortgage) m_game->unmortgageProperty(actId, spIdx);
        processRecentMoneyTransfers();
        emit gameStateUpdated();
        update();
        return;
    }

    // Trade dialog buttons
    if (id == "trade_cycle_opp") {
        int n = m_game->numPlayers();
        for (int step = 1; step < n; ++step) {
            int cand = (m_tradeDraft.toPlayerId + step) % n;
            if (cand != m_tradeDraft.fromPlayerId && !m_game->player(cand).bankrupt) {
                m_tradeDraft.toPlayerId = cand;
                m_tradeDraft.requestedProperties.clear();
                m_tradeDraft.requestedCash = 0;
                break;
            }
        }
        update();
        return;
    }
    if (id.startsWith("trade_toggle_L_")) {
        int spIdx = id.mid(15).toInt();
        auto& vec = m_tradeDraft.offeredProperties;
        auto it = std::find(vec.begin(), vec.end(), spIdx);
        if (it != vec.end()) vec.erase(it);
        else vec.push_back(spIdx);
        m_tradePreviewSpace = spIdx;
        update();
        return;
    }
    if (id.startsWith("trade_toggle_R_")) {
        int spIdx = id.mid(15).toInt();
        auto& vec = m_tradeDraft.requestedProperties;
        auto it = std::find(vec.begin(), vec.end(), spIdx);
        if (it != vec.end()) vec.erase(it);
        else vec.push_back(spIdx);
        m_tradePreviewSpace = spIdx;
        update();
        return;
    }
    if (id == "trade_l_cash_up") {
        m_tradeDraft.offeredCash = std::min(m_game->player(m_tradeDraft.fromPlayerId).money, m_tradeDraft.offeredCash + 20);
        update();
        return;
    }
    if (id == "trade_l_cash_dn") {
        m_tradeDraft.offeredCash = std::max(0, m_tradeDraft.offeredCash - 20);
        update();
        return;
    }
    if (id == "trade_r_cash_up") {
        m_tradeDraft.requestedCash = std::min(m_game->player(m_tradeDraft.toPlayerId).money, m_tradeDraft.requestedCash + 20);
        update();
        return;
    }
    if (id == "trade_r_cash_dn") {
        m_tradeDraft.requestedCash = std::max(0, m_tradeDraft.requestedCash - 20);
        update();
        return;
    }
    if (id == "trade_propose") {
        bool accepted = true;
        if (m_game->player(m_tradeDraft.toPlayerId).isAI && m_ai) {
            accepted = m_ai->evaluateTradeOffer(*m_game, m_tradeDraft);
        }
        if (accepted && m_game->executeTrade(m_tradeDraft)) {
            processRecentMoneyTransfers();
            m_overlayMode = ActionOverlayMode::None;
        } else {
            m_tradePreviewSpace = -1;
            m_tradeFeedbackMsg = trText("mono_trade_rejected");
        }
        emit gameStateUpdated();
        update();
        return;
    }

    // Center Turn Modals
    if (id == "modal_start_close") {
        m_game->dismissStartModal();
        scheduleAIIfNeeded(350);
        update();
        return;
    }
    if (id == "modal_roll_dice") {
        triggerRollAndMoveAnimation();
        return;
    }
    if (id == "modal_pay_jail") {
        m_game->payJailFine();
        processRecentMoneyTransfers();
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "modal_card_jail") {
        m_game->useJailFreeCard();
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "modal_buy_prop") {
        int pos = m_game->activePlayer().position;
        int buyer = m_game->activePlayerIndex();
        if (m_game->buyCurrentProperty()) {
            processRecentMoneyTransfers();
            triggerDeedFlyToPlayer(pos, buyer);
        }
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "modal_auction_prop") {
        m_game->startAuctionForCurrentProperty();
        scheduleAIIfNeeded(450);
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "auc_inc_bid") {
        int maxM = m_game->player(m_game->auctionState().activeBidderId).money;
        m_game->setAuctionInputBid(std::min(maxM, m_game->auctionState().inputBid + 10));
        update();
        return;
    }
    if (id == "auc_dec_bid") {
        int minM = m_game->auctionState().currentBid + 1;
        m_game->setAuctionInputBid(std::max(minM, m_game->auctionState().inputBid - 10));
        update();
        return;
    }
    if (id == "auc_place_bid") {
        int bidder = m_game->auctionState().activeBidderId;
        int propIdx = m_game->auctionState().spaceIndex;
        m_game->placeAuctionBid(bidder, m_game->auctionState().inputBid);
        if (m_game->phase() != TurnPhase::Auction) {
            processRecentMoneyTransfers();
            int winner = m_game->board().space(propIdx).ownerId;
            if (winner >= 0) triggerDeedFlyToPlayer(propIdx, winner);
        }
        scheduleAIIfNeeded(450);
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "auc_fold") {
        int bidder = m_game->auctionState().activeBidderId;
        int propIdx = m_game->auctionState().spaceIndex;
        m_game->foldAuction(bidder);
        if (m_game->phase() != TurnPhase::Auction) {
            processRecentMoneyTransfers();
            int winner = m_game->board().space(propIdx).ownerId;
            if (winner >= 0) triggerDeedFlyToPlayer(propIdx, winner);
        }
        scheduleAIIfNeeded(450);
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "modal_rent_close") {
        m_aiTimer->stop();
        m_aiStepPending = false;
        m_game->dismissRentModal();
        processRecentMoneyTransfers();
        scheduleAIIfNeeded();
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "modal_card_close") {
        m_aiTimer->stop();
        m_aiStepPending = false;
        int prevPos = m_game->activePlayer().position;
        int actId = m_game->activePlayerIndex();
        m_game->dismissCardModal();
        if (m_game->activePlayer().position != prevPos && m_game->lastMovePath().size() > 1) {
            m_tokenAnimPlayerId = actId;
            startMovePathAnimation(m_game->lastMovePath(), m_game->lastMoveWasJailSend());
        } else {
            processRecentMoneyTransfers();
            scheduleAIIfNeeded();
        }
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "modal_declare_bankrupt") {
        m_game->declareBankruptcy(m_game->activePlayerIndex());
        processRecentMoneyTransfers();
        scheduleAIIfNeeded(450);
        emit gameStateUpdated();
        update();
        return;
    }
    if (id == "modal_end_turn") {
        m_game->endTurn();
        m_expandedPlayerId = m_game->activePlayerIndex();
        scheduleAIIfNeeded(450);
        emit gameStateUpdated();
        update();
        return;
    }
}

} // namespace Strategix::Monopoly
