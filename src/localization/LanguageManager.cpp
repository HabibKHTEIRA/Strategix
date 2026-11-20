#include "LanguageManager.h"
#include <QApplication>

namespace Strategix {

LanguageManager& LanguageManager::instance() {
    static LanguageManager s_instance;
    return s_instance;
}

LanguageManager::LanguageManager() {
    initTranslations();
}

void LanguageManager::setLanguage(Language lang) {
    if (m_currentLanguage != lang) {
        m_currentLanguage = lang;
        if (lang == Language::Arabic) {
            qApp->setLayoutDirection(Qt::RightToLeft);
        } else {
            qApp->setLayoutDirection(Qt::LeftToRight);
        }
        emit languageChanged(lang);
    }
}

QString LanguageManager::get(const QString& key) const {
    auto it = m_dict.find(key.toStdString());
    if (it != m_dict.end()) {
        auto lit = it->second.find(static_cast<int>(m_currentLanguage));
        if (lit != it->second.end()) {
            return lit->second;
        }
    }
    return key;
}

QString LanguageManager::gameNameCheckers() const {
    return get("game_checkers");
}

QString LanguageManager::gameNameGo() const {
    return get("game_go");
}

QString LanguageManager::gameNameConnectFour() const {
    return get("game_connectfour");
}

QString LanguageManager::gameNameReversi() const {
    return get("game_reversi");
}

QString LanguageManager::gameNameQuoridor() const {
    return get("game_quoridor");
}

QString LanguageManager::gameNameMorris() const {
    return get("game_morris");
}

QString LanguageManager::gameNameDotsAndBoxes() const {
    return get("game_dotsandboxes");
}

QString LanguageManager::gameNameMonopoly() const {
    return get("game_monopoly");
}

void LanguageManager::initTranslations() {
    auto add = [this](const std::string& key, const QString& fr, const QString& en, const QString& ar) {
        m_dict[key][static_cast<int>(Language::French)] = fr;
        m_dict[key][static_cast<int>(Language::English)] = en;
        m_dict[key][static_cast<int>(Language::Arabic)] = ar;
    };

    // App & Navigation
    add("app_title", "Stratégix - Jeux de Stratégie", "Strategix - Board Games", "ستراتيجي - ألعاب الذكاء الاستراتيجي");
    add("choose_game", "Choisissez votre jeu", "Choose Your Game", "اختر لعبتك");
    add("choose_game_subtitle", "Jeux de réflexion intemporels en C++ & Qt", "Timeless board games in modern C++ & Qt", "ألعاب كلاسيكية خالدة بتطوير حديث");
    add("game_checkers", "Jeu de Dames", "Checkers", "لعبة الدامة");
    add("game_checkers_desc", "Jeu classique sur damier 8x8 avec pions, dames et rafles stratégiques.",
        "Classic 8x8 draughts with pawns, queens, and capturing jumps.",
        "اللعبة الكلاسيكية الشهيرة على رقعة 8×8 مع الترقيات وحركات الأسر.");
    add("game_go", "Jeu de Go", "Go Game (Baduk)", "لعبة الغو (بادوك)");
    add("game_go_desc", "Le plus ancien jeu de stratégie au monde : encerclement de territoires et captures de pierres sur goban 9x9.",
        "The world's oldest strategy game: territory control and stone captures on a 9x9 goban.",
        "أقدم لعبة استراتيجية في العالم: إحاطة الأراضي وأسر الأحجار على لوحة 9×9.");
    add("game_connectfour", "Puissance 4", "Connect Four", "أربعة على التوالي");
    add("game_connectfour_desc", "Alignez 4 jetons de votre couleur horizontalement, verticalement ou en diagonale sur la grille verticale 7x6.",
        "Align 4 tokens of your color horizontally, vertically, or diagonally on the 7x6 vertical grid.",
        "قم بربط 4 قطع من لونك أفقياً أو عمودياً أو قطرياً على الشبكة الرأسية 7×6.");
    add("game_reversi", "Reversi (Othello)", "Reversi (Othello)", "ريفيرسي (عطيل)");
    add("game_reversi_desc", "Encadrez les pions adverses pour les retourner à votre couleur sur plateau 8x8. Une minute pour apprendre, une vie pour maîtriser !",
        "Trap and flip opponent discs to your color on an 8x8 board. A minute to learn, a lifetime to master!",
        "احصر قطع الخصم واقلبها لصالحك على رقعة 8×8. دقيقة لتتعلمها وحياة لتتقنها!");
    add("game_quoridor", "Quoridor", "Quoridor", "كوريدور");
    add("game_quoridor_desc", "Frayez-vous un chemin vers le bord opposé tout en érigeant des barrières en bois pour bloquer votre rival.",
        "Navigate your pawn to the opposite edge while placing wooden fences to hinder your opponent.",
        "شق طريقك نحو الطرف المقابل وضع الحواجز الخشبية لعرقلة تقدم خصمك.");
    add("game_morris", "Jeu du Moulin", "Nine Men's Morris", "لعبة الطاحونة (الموريس)");
    add("game_morris_desc", "Placez et déplacez vos 9 pions sur 3 carrés concentriques. Formez un alignement de 3 pour capturer une pièce adverse.",
        "Place and slide 9 pieces on 3 concentric squares. Form a row of 3 (a mill) to capture an opponent's piece.",
        "ضع وحرك قطعك التسع على المربعات متحدة المركز. كوّن طاحونة من 3 قطع لأسر قطع الخصم.");
    add("game_dotsandboxes", "La Pipopipette", "Dots and Boxes", "النقاط والمربعات");
    add("game_dotsandboxes_desc", "Tracez des lignes pour relier les points. Chaque carré de 1x1 complété vous rapporte 1 point et un coup rejoué !",
        "Connect adjacent dots with lines. Completing a 1x1 box awards 1 point and an extra bonus turn!",
        "صل النقاط بخطوط متتالية. غلق كل مربع 1×1 يمنحك نقطة ودوراً إضافياً فورياً!");

    add("play", "Jouer", "Play", "العب الآن");
    add("main_menu", "Menu principal", "Main Menu", "القائمة الرئيسية");
    add("back_to_menu", "Retour au menu", "Back to Menu", "العودة للقائمة");

    // Game Controls
    add("new_game", "Nouvelle partie", "New Game", "لعبة جديدة");
    add("undo", "Annuler le coup", "Undo Move", "تراجع عن النقلة");
    add("pass_turn", "Passer son tour", "Pass Turn", "تمرير الدور");
    add("rules", "Règles du jeu", "Game Rules", "قواعد اللعبة");
    add("about", "À propos", "About", "حول التطبيق");
    add("quit", "Quitter", "Quit", "خروج");
    add("mode", "Mode de jeu", "Game Mode", "نمط اللعب");
    add("mode_2p", "2 Joueurs (Local)", "2 Players (Local)", "لاعبان (محلي)");
    add("mode_ai", "1 Joueur vs Ordinateur", "1 Player vs Computer", "لاعب ضد الحاسوب");
    add("difficulty", "Niveau", "Difficulty", "المستوى");
    add("easy", "Débutant", "Easy", "مبتدئ");
    add("medium", "Intermédiaire", "Medium", "متوسط");
    add("hard", "Expert", "Expert", "خبير");
    add("language", "Langue", "Language", "اللغة");
    add("theme_dark", "Mode Sombre", "Dark Mode", "الوضع الليلي");
    add("theme_light", "Mode Clair", "Light Mode", "الوضع الفاتح");

    // In-game Status
    add("white_turn", "Au tour des Blancs", "White's turn", "دور القطع البيضاء");
    add("black_turn", "Au tour des Noirs", "Black's turn", "دور القطع السوداء");
    add("white_won", "Victoire des Blancs !", "White wins!", "فوز الأبيض!");
    add("black_won", "Victoire des Noirs !", "Black wins!", "فوز الأسود!");
    add("draw", "Match nul !", "Draw game!", "تعادل!");
    add("white", "Blancs", "White", "الأبيض");
    add("black", "Noirs", "Black", "الأسود");
    add("captures", "Prises", "Captures", "الأسر");
    add("score", "Score", "Score", "النتيجة");
    add("territory", "Territoire", "Territory", "المنطقة");
    add("board_size", "Taille du plateau", "Board Size", "حجم اللوحة");

    // Connect Four specific
    add("red_turn", "Au tour des Rouges", "Red's turn", "دور القطع الحمراء");
    add("yellow_turn", "Au tour des Jaunes", "Yellow's turn", "دور القطع الصفراء");
    add("red_won", "Victoire des Rouges !", "Red wins!", "فوز الأحمر!");
    add("yellow_won", "Victoire des Jaunes !", "Yellow wins!", "فوز الأصفر!");
    add("red", "Rouge", "Red", "أحمر");
    add("yellow", "Jaune", "Yellow", "أصفر");

    // Quoridor specific
    add("p1_turn", "Au tour du Joueur 1", "Player 1's turn", "دور اللاعب 1");
    add("p2_turn", "Au tour du Joueur 2", "Player 2's turn", "دور اللاعب 2");
    add("p1_won", "Victoire du Joueur 1 !", "Player 1 wins!", "فوز اللاعب 1!");
    add("p2_won", "Victoire du Joueur 2 !", "Player 2 wins!", "فوز اللاعب 2!");
    add("walls_left", "Barrières restantes", "Walls left", "الحواجز المتبقية");
    add("mode_move", "Mode Déplacement", "Move Mode", "نمط التحريك");
    add("mode_wall", "Mode Barrière", "Wall Mode", "نمط الحاجز");
    add("rotate_wall", "Tourner la barrière (Clic droit / Espace)", "Rotate Wall (Right Click / Space)", "تدوير الحاجز (زر يمين / مسافة)");

    // Nine Men's Morris specific
    add("morris_phase_place", "Phase 1 : Pose des pions (%1 restants)", "Phase 1: Placing pieces (%1 left)", "المرحلة 1: وضع القطع (%1 متبقية)");
    add("morris_phase_move", "Phase 2 : Déplacement des pions", "Phase 2: Moving pieces", "المرحلة 2: تحريك القطع");
    add("morris_phase_fly", "Phase 3 : Vol libre (3 pions restants)", "Phase 3: Flying (3 pieces left)", "المرحلة 3: الطيران الحر (3 قطع متبقية)");
    add("morris_remove", "Moulin formé ! Cliquez sur un pion adverse à retirer", "Mill formed! Click an opponent piece to capture", "طاحونة مكتملة! انقر على قطعة الخصم لأسرها");

    // Dots and Boxes specific
    add("blue_turn", "Au tour des Bleus", "Blue's turn", "دور الأزرق");
    add("blue_won", "Victoire des Bleus !", "Blue wins!", "فوز الأزرق!");
    add("blue", "Bleu", "Blue", "أزرق");
    add("boxes", "Cases fermées", "Boxes closed", "المربعات المغلقة");
    add("bonus_turn", "Case fermée ! Vous rejouez !", "Box closed! Bonus turn!", "مربع مغلق! دور إضافي لك!");

    // Monopoly - General & Setup
    add("game_monopoly", "Monopoly", "Monopoly", "مونوبولي (بنك الحظ)");
    add("game_monopoly_desc", "Achetez, vendez et négociez les avenues de Paris dans une édition moderne Noir & Or.",
        "Buy, sell, and trade iconic Parisian avenues in a modern Black & Gold edition.",
        "اشترِ وبِع وتفاوض على أشهر شوارع باريس في إصدار عصري بالأسود والذهبي.");
    add("mono_motto", "❖ Achetez. Vendez. Négociez. Gagnez ! ❖", "❖ Buy. Sell. Trade. Win! ❖", "❖ اشترِ. بِع. تفاوض. اربح! ❖");
    add("mono_new_game_btn", "N. PARTIE", "NEW GAME", "لعبة جديدة");
    add("mono_back", "RETOUR", "BACK", "رجوع");
    add("mono_rules_btn", "RÈGLES", "RULES", "القواعد");
    add("mono_add_player", "AJOUTER JOUEUR", "ADD PLAYER", "إضافة لاعب");
    add("mono_remove_player", "SUPPRIMER", "REMOVE", "حذف");
    add("mono_play_btn", "JOUER", "PLAY", "العب");
    add("mono_close_btn", "FERMER", "CLOSE", "إغلاق");
    add("mono_human_player", "JOUEUR HUMAIN", "HUMAN PLAYER", "لاعب بشري");
    add("mono_computer_player", "ORDINATEUR", "COMPUTER", "الكمبيوتر");
    add("mono_choose_diff", "Choisissez la difficulté", "Choose difficulty", "اختر مستوى الصعوبة");
    add("mono_diff_easy", "AGENT IMMOBILIER", "REAL ESTATE AGENT", "وكيل عقاري");
    add("mono_diff_medium", "ENTREPRENEUR", "ENTREPRENEUR", "مقاول");
    add("mono_diff_hard", "MAGNAT DE L'IMMOBILIER", "REAL ESTATE TYCOON", "قطب العقارات");
    add("mono_click_token_hint", "Cliquez sur l'icône d'un pion pour choisir votre propre pion.",
        "Click on a token icon to choose your own token.",
        "انقر على أيقونة البيدق لاختيار القطعة الخاصة بك.");
    add("mono_choose_token_title", "CHOISISSEZ UN PION", "CHOOSE A TOKEN", "اختر بيدقاً");
    add("mono_free_tokens", "PIONS DISPONIBLES", "AVAILABLE TOKENS", "البيادق المتاحة");
    add("mono_player_default", "Joueur %1", "Player %1", "اللاعب %1");
    add("mono_token_0", "Voiture", "Racecar", "سيارة سباق");
    add("mono_token_1", "Cuirassé", "Battleship", "بارجة");
    add("mono_token_2", "Haut-de-forme", "Top Hat", "قبعة عالية");
    add("mono_token_3", "Chien", "Scottie Dog", "كلب");
    add("mono_token_4", "Chaussure", "Boot", "حذاء");
    add("mono_token_5", "Chat", "Cat", "قط");
    add("mono_token_6", "Dé à coudre", "Thimble", "كشتبان");
    add("mono_token_7", "Brouette", "Wheelbarrow", "عربة يدوية");

    // Monopoly - In-game HUD & Bottom Bar
    add("mono_bank", "BANQUE", "BANK", "البنك");
    add("mono_history", "HISTORIQUE", "HISTORY", "السجل");
    add("mono_menu_btn", "MENU", "MENU", "القائمة");
    add("mono_build_btn", "CONSTRUIRE", "BUILD", "بناء");
    add("mono_sell_btn", "VENDRE", "SELL", "بيع");
    add("mono_mortgage_btn", "HYPOTHÉQUER", "MORTGAGE", "رهن");
    add("mono_unmortgage_btn", "LEVER L'HYPOTHÈQUE", "UNMORTGAGE", "فك الرهن");
    add("mono_trade_btn", "ÉCHANGE", "TRADE", "مقايضة");
    add("mono_starts_first", "%1 commence en premier.", "%1 goes first.", "%1 يبدأ أولاً.");
    add("mono_roll_dice", "JETER LES DÉS", "ROLL DICE", "رمي النرد");
    add("mono_end_turn", "FINIR TOUR", "END TURN", "إنهاء الدور");
    add("mono_pay_jail_50", "PAYER 50 ₩", "PAY 50 ₩", "دفع 50 ₩");
    add("mono_use_jail_card", "CARTE LIBÉRÉ", "USE JAIL CARD", "بطاقة الخروج");
    add("mono_for_sale", "À VENDRE", "FOR SALE", "للبيع");
    add("mono_buy_btn", "ACHETER", "BUY", "شراء");
    add("mono_auction_btn", "ENCHÈRE", "AUCTION", "مزاد");
    add("mono_bid_turn", "Tour de mise :", "Bidding turn:", "دور المزايدة:");
    add("mono_bid_btn", "MISER", "BID", "مزايدة");
    add("mono_fold_btn", "SE COUCHER", "FOLD", "انسحاب");
    add("mono_highest_bid", "Mise max : %1 ₩ (%2)", "High bid: %1 ₩ (%2)", "أعلى مزايدة: %1 ₩ (%2)");
    add("mono_pay_rent_title", "PAYEZ LOYER", "PAY RENT", "دفع الإيجار");
    add("mono_rent_label", "LOYER", "RENT", "الإيجار");
    add("mono_propose_btn", "PROPOSITION", "PROPOSE", "اقتراح");
    add("mono_cancel_btn", "ANNULER", "CANCEL", "إلغاء");
    add("mono_transfer_cash", "Transférer argent", "Transfer money", "تحويل الأموال");
    add("mono_trade_hint", "Cliquez sur les propriétés et définissez l'argent pour composer votre échange.",
        "Click properties and set money amounts to compose your trade.",
        "انقر على العقارات وحدد المبلغ المالي لتكوين عرض المقايضة.");
    add("mono_trade_accepted", "Échange accepté !", "Trade accepted!", "تم قبول المقايضة!");
    add("mono_trade_rejected", "L'adversaire a refusé votre proposition d'échange.", "Opponent declined your trade offer.", "رفض الخصم عرض المقايضة.");
    add("mono_bankrupt_btn", "FAILLITE", "BANKRUPT", "إفلاس");
    add("mono_debt_title", "DETTE À PAYER : %1 ₩", "DEBT TO PAY: %1 ₩", "دين مستحق الدفع: %1 ₩");
    add("mono_debt_hint", "Vendez des maisons, hypothéquez des terrains ou faites un échange pour réunir la somme.",
        "Sell houses, mortgage properties, or trade to raise the required funds.",
        "بِع المنازل أو ارهن العقارات أو قايض لجمع المبلغ المطلوب.");
    add("mono_winner_title", "VICTOIRE DE %1 !", "%1 WINS!", "فوز %1!");
    add("mono_winner_sub", "Fortune totale : %1 ₩", "Total Net Worth: %1 ₩", "إجمالي الثروة: %1 ₩");
    add("mono_setup_lobby_btn", "CONFIGURER JOUEURS", "PLAYER SETUP", "إعداد اللاعبين");
    add("mono_resume_btn", "REPRENDRE", "RESUME", "استئناف");

    // Monopoly - Title Deed Cards
    add("mono_deed_title", "TITRE DE PROPRIÉTÉ", "TITLE DEED", "سند ملكية");
    add("mono_deed_rent_base", "LOYER Terrain nu", "RENT Site only", "إيجار الأرض الفارغة");
    add("mono_deed_house_1", "Avec 1 Maison", "With 1 House", "مع منزل واحد");
    add("mono_deed_house_2", "Avec 2 Maisons", "With 2 Houses", "مع منزلين");
    add("mono_deed_house_3", "Avec 3 Maisons", "With 3 Houses", "مع 3 منازل");
    add("mono_deed_house_4", "Avec 4 Maisons", "With 4 Houses", "مع 4 منازل");
    add("mono_deed_hotel", "Avec HÔTEL", "With HOTEL", "مع فندق");
    add("mono_deed_mortgage_val", "Valeur Hypothécaire %1 ₩", "Mortgage Value %1 ₩", "قيمة الرهن العقاري %1 ₩");
    add("mono_deed_house_cost", "Prix des Maisons %1 ₩ chacune", "Houses cost %1 ₩ each", "تكلفة المنزل %1 ₩ لكل واحد");
    add("mono_deed_hotel_cost", "Prix d'un Hôtel %1 ₩ plus 4 maisons", "Hotels, %1 ₩ plus 4 houses", "تكلفة الفندق %1 ₩ + 4 منازل");
    add("mono_deed_group_rule", "Si un joueur possède TOUS les terrains d'un groupe de couleur quelconque, le loyer des terrains nus de ce groupe est doublé.",
        "If a player owns ALL the Lots of any Color-Group, the rent is Doubled on Unimproved Lots in that group.",
        "إذا امتلك اللاعب جميع أراضي مجموعة لونية كاملة، يتضاعف إيجار الأراضي غير المبنية فيها.");
    add("mono_deed_rr_1", "Loyer (1 gare)", "Rent (1 station)", "الإيجار (محطة واحدة)");
    add("mono_deed_rr_2", "Si vous avez 2 gares", "If 2 stations are owned", "إذا امتلكت محطتين");
    add("mono_deed_rr_3", "Si vous avez 3 gares", "If 3 stations are owned", "إذا امتلكت 3 محطات");
    add("mono_deed_rr_4", "Si vous avez 4 gares", "If 4 stations are owned", "إذا امتلكت 4 محطات");
    add("mono_deed_util_1", "Si l'on possède UNE compagnie de service public, le loyer est 4 fois le montant indiqué par les dés.",
        "If ONE Utility is owned, rent is 4 times amount shown on dice.",
        "إذا امتلكت شركة خدمات واحدة، يكون الإيجار 4 أضعاف مجموع النرد.");
    add("mono_deed_util_2", "Si l'on possède les DEUX compagnies de service public, le loyer est 10 fois le montant indiqué par les dés.",
        "If BOTH Utilities are owned, rent is 10 times amount shown on dice.",
        "إذا امتلكت كلتا شركتي الخدمات، يكون الإيجار 10 أضعاف مجموع النرد.");
    add("mono_mortgaged_for", "HYPOTHÉQUÉE POUR %1 ₩", "MORTGAGED FOR %1 ₩", "مرهونة مقابل %1 ₩");
    add("mono_mortgaged_note", "Si cette propriété est hypothéquée, ce côté de la carte doit être mis en évidence.",
        "Card must be turned this side up if property is mortgaged.",
        "إذا كان هذا العقار مرهوناً، يظهر هذا الوجه من البطاقة.");

    // Monopoly - Board corner & tile labels
    add("mono_go_title", "DÉPART", "GO", "الانطلاق");
    add("mono_go_sub", "RECEVEZ 200 ₩ CHAQUE FOIS QUE VOUS PASSEZ ICI", "COLLECT 200 ₩ SALARY AS YOU PASS", "احصل على 200 ₩ عند المرور");
    add("mono_jail_just", "SIMPLE", "JUST", "مجرد");
    add("mono_jail_visiting", "VISITE", "VISITING", "زيارة");
    add("mono_in_jail", "EN PRISON", "IN JAIL", "في السجن");
    add("mono_free_parking", "PARC", "FREE", "موقف");
    add("mono_free_parking_2", "GRATUIT", "PARKING", "مجاني");
    add("mono_go_to_jail_1", "ALLEZ EN", "GO TO", "اذهب إلى");
    add("mono_go_to_jail_2", "PRISON", "JAIL", "السجن");
    add("mono_pay_amount", "PAYEZ %1 ₩", "PAY %1 ₩", "ادفع %1 ₩");

    // Monopoly - 40 Board Spaces
    add("mono_space_0",  "DÉPART", "GO", "الانطلاق");
    add("mono_space_1",  "BOULEVARD DE BELLEVILLE", "BELLEVILLE BOULEVARD", "بوليفارد بيلفيل");
    add("mono_space_2",  "CAISSE DE COMMUNAUTÉ", "COMMUNITY CHEST", "صندوق الجماعة");
    add("mono_space_3",  "RUE LECOURBE", "LECOURBE STREET", "شارع لوكورب");
    add("mono_space_4",  "IMPÔTS SUR LE REVENU", "INCOME TAX", "ضريبة الدخل");
    add("mono_space_5",  "GARE MONTPARNASSE", "MONTPARNASSE STATION", "محطة مونبارناس");
    add("mono_space_6",  "RUE DE VAUGIRARD", "VAUGIRARD STREET", "شارع فوجيرار");
    add("mono_space_7",  "CHANCE", "CHANCE", "فرصة");
    add("mono_space_8",  "RUE DE COURCELLES", "COURCELLES STREET", "شارع كورسيل");
    add("mono_space_9",  "AVENUE DE LA RÉPUBLIQUE", "REPUBLIC AVENUE", "شارع الجمهورية");
    add("mono_space_10", "PRISON", "JAIL", "السجن");
    add("mono_space_11", "BOULEVARD DE LA VILLETTE", "LA VILLETTE BOULEVARD", "بوليفارد لافيليت");
    add("mono_space_12", "COMPAGNIE D'ÉLECTRICITÉ", "ELECTRIC COMPANY", "شركة الكهرباء");
    add("mono_space_13", "AVENUE DE NEUILLY", "NEUILLY AVENUE", "شارع نويي");
    add("mono_space_14", "RUE DE PARADIS", "PARADIS STREET", "شارع بارادي");
    add("mono_space_15", "GARE DE LYON", "LYON STATION", "محطة ليون");
    add("mono_space_16", "AVENUE MOZART", "MOZART AVENUE", "شارع موزارت");
    add("mono_space_17", "CAISSE DE COMMUNAUTÉ", "COMMUNITY CHEST", "صندوق الجماعة");
    add("mono_space_18", "BOULEVARD SAINT-MICHEL", "SAINT-MICHEL BOULEVARD", "بوليفارد سان ميشيل");
    add("mono_space_19", "PLACE PIGALLE", "PIGALLE SQUARE", "ساحة بيغال");
    add("mono_space_20", "PARC GRATUIT", "FREE PARKING", "موقف مجاني");
    add("mono_space_21", "AVENUE MATIGNON", "MATIGNON AVENUE", "شارع ماتينيون");
    add("mono_space_22", "CHANCE", "CHANCE", "فرصة");
    add("mono_space_23", "BOULEVARD MALESHERBES", "MALESHERBES BOULEVARD", "بوليفارد مالزيرب");
    add("mono_space_24", "AVENUE HENRI-MARTIN", "HENRI-MARTIN AVENUE", "شارع هنري مارتن");
    add("mono_space_25", "GARE DU NORD", "NORTH STATION", "محطة الشمال");
    add("mono_space_26", "FAUBOURG SAINT-HONORÉ", "FAUBOURG SAINT-HONORÉ", "فوبور سان أونوريه");
    add("mono_space_27", "PLACE DE LA BOURSE", "LA BOURSE SQUARE", "ساحة البورصة");
    add("mono_space_28", "COMPAGNIE DES EAUX", "WATER WORKS", "شركة المياه");
    add("mono_space_29", "RUE LA FAYETTE", "LA FAYETTE STREET", "شارع لافاييت");
    add("mono_space_30", "ALLEZ EN PRISON", "GO TO JAIL", "اذهب إلى السجن");
    add("mono_space_31", "AVENUE DE BRETEUIL", "BRETEUIL AVENUE", "شارع بروتوي");
    add("mono_space_32", "AVENUE FOCH", "FOCH AVENUE", "شارع فوش");
    add("mono_space_33", "CAISSE DE COMMUNAUTÉ", "COMMUNITY CHEST", "صندوق الجماعة");
    add("mono_space_34", "BOULEVARD DES CAPUCINES", "CAPUCINES BOULEVARD", "بوليفارد كابوسين");
    add("mono_space_35", "GARE SAINT-LAZARE", "SAINT-LAZARE STATION", "محطة سان لازار");
    add("mono_space_36", "CHANCE", "CHANCE", "فرصة");
    add("mono_space_37", "AVENUE DES CHAMPS-ÉLYSÉES", "CHAMPS-ÉLYSÉES AVENUE", "شارع الشانزليزيه");
    add("mono_space_38", "TAXE DE LUXE", "LUXURY TAX", "ضريبة الرفاهية");
    add("mono_space_39", "RUE DE LA PAIX", "RUE DE LA PAIX", "شارع السلام (لا بيه)");

    // Monopoly - 16 Chance Cards
    add("mono_chance_0",  "AVANCEZ JUSQU'À LA CASE DÉPART. (RECEVEZ 200 ₩).",
        "ADVANCE TO GO. (COLLECT 200 ₩).",
        "تقدم إلى خانة الانطلاق. (احصل على 200 ₩).");
    add("mono_chance_1",  "RENDEZ-VOUS RUE DE LA PAIX.",
        "ADVANCE TO RUE DE LA PAIX.",
        "توجه إلى شارع السلام (لا بيه).");
    add("mono_chance_2",  "RENDEZ-VOUS À L'AVENUE HENRI-MARTIN. SI VOUS PASSEZ PAR LA CASE DÉPART, RECEVEZ 200 ₩.",
        "ADVANCE TO HENRI-MARTIN AVENUE. IF YOU PASS GO, COLLECT 200 ₩.",
        "توجه إلى شارع هنري مارتن. إذا مررت بالانطلاق احصل على 200 ₩.");
    add("mono_chance_3",  "AVANCEZ AU BOULEVARD DE LA VILLETTE. SI VOUS PASSEZ PAR LA CASE DÉPART, RECEVEZ 200 ₩.",
        "ADVANCE TO LA VILLETTE BOULEVARD. IF YOU PASS GO, COLLECT 200 ₩.",
        "تقدم إلى بوليفارد لافيليت. إذا مررت بالانطلاق احصل على 200 ₩.");
    add("mono_chance_4",  "RENDEZ-VOUS À LA GARE DE LYON. SI VOUS PASSEZ PAR LA CASE DÉPART, RECEVEZ 200 ₩.",
        "TAKE A TRIP TO LYON STATION. IF YOU PASS GO, COLLECT 200 ₩.",
        "توجه إلى محطة ليون. إذا مررت بالانطلاق احصل على 200 ₩.");
    add("mono_chance_5",  "AVANCEZ JUSQU'À LA GARE LA PLUS PROCHE. SI ELLE APPARTIENT À UN JOUEUR, PAYEZ-LUI LE DOUBLE DU LOYER.",
        "ADVANCE TO THE NEAREST STATION. IF OWNED, PAY OWNER TWICE THE RENTAL.",
        "تقدم إلى أقرب محطة قطار. إذا كانت مملوكة، ادفع ضعف الإيجار.");
    add("mono_chance_6",  "AVANCEZ JUSQU'À LA COMPAGNIE DE SERVICE PUBLIC LA PLUS PROCHE.",
        "ADVANCE TOKEN TO NEAREST UTILITY.",
        "تقدم إلى أقرب شركة خدمات عامة.");
    add("mono_chance_7",  "RECULEZ DE TROIS CASES.",
        "GO BACK THREE SPACES.",
        "تراجع ثلاث خانات للخلف.");
    add("mono_chance_8",  "ALLEZ EN PRISON. NE FRANCHISSEZ PAS LA CASE DÉPART, NE TOUCHEZ PAS 200 ₩.",
        "GO DIRECTLY TO JAIL. DO NOT PASS GO, DO NOT COLLECT 200 ₩.",
        "اذهب مباشرة إلى السجن. لا تمر بالانطلاق ولا تحصل على 200 ₩.");
    add("mono_chance_9",  "VOUS ÊTES LIBÉRÉ DE PRISON. CETTE CARTE PEUT ÊTRE CONSERVÉE JUSQU'À CE QU'ELLE SOIT UTILISÉE OU VENDUE.",
        "GET OUT OF JAIL FREE. THIS CARD MAY BE KEPT UNTIL NEEDED OR TRADED.",
        "أنت حر من السجن. يمكنك الاحتفاظ بهذه البطاقة لاستخدامها أو مقايضتها.");
    add("mono_chance_10", "VOUS DEVEZ FAIRE DES TRAVAUX SUR VOS PROPRIÉTÉS : VERSEZ 25 ₩ POUR CHAQUE MAISON ET 100 ₩ POUR CHAQUE HÔTEL.",
        "MAKE GENERAL REPAIRS ON ALL YOUR PROPERTY: FOR EACH HOUSE PAY 25 ₩, FOR EACH HOTEL PAY 100 ₩.",
        "يجب إجراء صيانة لعقاراتك: ادفع 25 ₩ عن كل منزل و100 ₩ عن كل فندق.");
    add("mono_chance_11", "AMENDE POUR EXCÈS DE VITESSE SUR LE PÉRIPHÉRIQUE : PAYEZ 15 ₩.",
        "SPEEDING FINE ON THE RING ROAD: PAY 15 ₩.",
        "غرامة تجاوز السرعة على الطريق الدائري: ادفع 15 ₩.");
    add("mono_chance_12", "VOUS AIDEZ À RAMASSER LES DÉCHETS DANS PARIS. RECEVEZ 100 ₩.",
        "YOU HELP CLEAN UP PARIS STREETS. COLLECT 100 ₩.",
        "لقد ساعدت في تنظيف شوارع باريس. احصل على 100 ₩.");
    add("mono_chance_13", "LA BANQUE VOUS VERSE UN DIVIDENDE DE 50 ₩.",
        "BANK PAYS YOU DIVIDEND OF 50 ₩.",
        "يدفع لك البنك أرباح أسهم بقيمة 50 ₩.");
    add("mono_chance_14", "VOTRE IMMEUBLE ET VOTRE PRÊT RAPPORTENT. VOUS DEVEZ TOUCHER 150 ₩.",
        "YOUR BUILDING AND LOAN MATURES. COLLECT 150 ₩.",
        "استحقاق عوائد المباني والقروض الخاصة بك. احصل على 150 ₩.");
    add("mono_chance_15", "VOUS AVEZ ÉTÉ ÉLU PRÉSIDENT DU CONSEIL D'ADMINISTRATION. VERSEZ 50 ₩ À CHAQUE JOUEUR.",
        "YOU HAVE BEEN ELECTED CHAIRMAN OF THE BOARD. PAY EACH PLAYER 50 ₩.",
        "تم انتخابك رئيساً لمجلس الإدارة. ادفع 50 ₩ لكل لاعب.");

    // Monopoly - 16 Community Chest Cards
    add("mono_chest_0",  "AVANCEZ JUSQU'À LA CASE DÉPART. (RECEVEZ 200 ₩).",
        "ADVANCE TO GO. (COLLECT 200 ₩).",
        "تقدم إلى خانة الانطلاق. (احصل على 200 ₩).");
    add("mono_chest_1",  "ERREUR DE LA BANQUE EN VOTRE FAVEUR. RECEVEZ 200 ₩.",
        "BANK ERROR IN YOUR FAVOR. COLLECT 200 ₩.",
        "خطأ بنكي لصالحك. احصل على 200 ₩.");
    add("mono_chest_2",  "VISITEZ LE MÉDECIN : PAYEZ 50 ₩.",
        "DOCTOR'S FEES: PAY 50 ₩.",
        "رسوم زيارة الطبيب: ادفع 50 ₩.");
    add("mono_chest_3",  "LA VENTE DE VOTRE STOCK VOUS RAPPORTE 50 ₩.",
        "FROM SALE OF STOCK YOU GET 50 ₩.",
        "من بيع الأسهم الخاصة بك تحصل على 50 ₩.");
    add("mono_chest_4",  "VOUS ÊTES LIBÉRÉ DE PRISON. CETTE CARTE PEUT ÊTRE CONSERVÉE JUSQU'À CE QU'ELLE SOIT UTILISÉE OU VENDUE.",
        "GET OUT OF JAIL FREE. THIS CARD MAY BE KEPT UNTIL NEEDED OR TRADED.",
        "أنت حر من السجن. يمكنك الاحتفاظ بهذه البطاقة لاستخدامها أو مقايضتها.");
    add("mono_chest_5",  "ALLEZ EN PRISON. NE FRANCHISSEZ PAS LA CASE DÉPART, NE TOUCHEZ PAS 200 ₩.",
        "GO DIRECTLY TO JAIL. DO NOT PASS GO, DO NOT COLLECT 200 ₩.",
        "اذهب مباشرة إلى السجن. لا تمر بالانطلاق ولا تحصل على 200 ₩.");
    add("mono_chest_6",  "C'EST VOTRE ANNIVERSAIRE : CHAQUE JOUEUR DOIT VOUS DONNER 10 ₩.",
        "IT IS YOUR BIRTHDAY: COLLECT 10 ₩ FROM EVERY PLAYER.",
        "إنه عيد ميلادك: يعطيك كل لاعب 10 ₩.");
    add("mono_chest_7",  "VOTRE ASSURANCE VIE VOUS RAPPORTE 100 ₩.",
        "LIFE INSURANCE MATURES. COLLECT 100 ₩.",
        "استحقاق التأمين على الحياة. احصل على 100 ₩.");
    add("mono_chest_8",  "FRAIS D'HOSPITALISATION : PAYEZ 100 ₩.",
        "HOSPITAL FEES: PAY 100 ₩.",
        "رسوم المستشفى: ادفع 100 ₩.");
    add("mono_chest_9",  "FRAIS DE SCOLARITÉ : PAYEZ 50 ₩.",
        "SCHOOL FEES: PAY 50 ₩.",
        "الرسوم الدراسية: ادفع 50 ₩.");
    add("mono_chest_10", "RECEVEZ 25 ₩ D'HONORAIRES DE CONSEIL.",
        "RECEIVE 25 ₩ CONSULTANCY FEE.",
        "احصل على 25 ₩ أتعاب استشارات.");
    add("mono_chest_11", "VOUS ÊTES IMPOSÉ POUR LES RÉPARATIONS DE VOIRIE : 40 ₩ PAR MAISON ET 115 ₩ PAR HÔTEL.",
        "YOU ARE ASSESSED FOR STREET REPAIRS: 40 ₩ PER HOUSE AND 115 ₩ PER HOTEL.",
        "ضريبة إصلاح الطرقات: ادفع 40 ₩ عن كل منزل و115 ₩ عن كل فندق.");
    add("mono_chest_12", "VOUS AVEZ GAGNÉ LE DEUXIÈME PRIX DE BEAUTÉ. RECEVEZ 10 ₩.",
        "YOU HAVE WON SECOND PRIZE IN A BEAUTY CONTEST. COLLECT 10 ₩.",
        "فزت بالجائزة الثانية في مسابقة الجمال. احصل على 10 ₩.");
    add("mono_chest_13", "VOUS HÉRITEZ DE 100 ₩.",
        "YOU INHERIT 100 ₩.",
        "لقد ورثت 100 ₩.");
    add("mono_chest_14", "LES IMPÔTS VOUS REMBOURSENT. RECEVEZ 150 ₩.",
        "INCOME TAX REFUND. COLLECT 150 ₩.",
        "استرداد ضريبي. احصل على 150 ₩.");
    add("mono_chest_15", "RETOURNEZ À BELLEVILLE.",
        "GO BACK TO BELLEVILLE BOULEVARD.",
        "عد إلى بوليفارد بيلفيل.");

    // Monopoly - History Log Messages
    add("mono_log_welcome", "Bienvenue dans Monopoly", "Welcome to Monopoly", "مرحباً بكم في مونوبولي");
    add("mono_log_moved", "%1 s'est déplacé sur %2.", "%1 moved to %2.", "انتقل %1 إلى %2.");
    add("mono_log_moved_double", "%1 a fait un double et s'est déplacé sur %2.", "%1 rolled a double and moved to %2.", "حصل %1 على نرد مزدوج وانتقل إلى %2.");
    add("mono_log_passed_go", "%1 passe par la case Départ et reçoit 200 ₩.", "%1 passed GO and collected 200 ₩.", "مر %1 بخانة الانطلاق وحصل على 200 ₩.");
    add("mono_log_bought_property", "%1 a acheté %2 pour %3 ₩.", "%1 bought %2 for %3 ₩.", "اشترى %1 عقار %2 مقابل %3 ₩.");
    add("mono_log_paid_rent", "%1 a payé %3 ₩ de loyer à %2.", "%1 paid %3 ₩ rent to %2.", "دفع %1 إيجاراً بقيمة %3 ₩ إلى %2.");
    add("mono_log_paid_tax", "%1 a payé %3 ₩ en %2.", "%1 paid %3 ₩ in %2.", "دفع %1 مبلغ %3 ₩ في %2.");
    add("mono_log_sent_to_jail", "%1 est envoyé en prison !", "%1 was sent to Jail!", "تم إرسال %1 إلى السجن!");
    add("mono_log_three_doubles_jail", "%1 a fait 3 doubles d'affilée : direction la prison !", "%1 rolled 3 doubles in a row: go to Jail!", "حصل %1 على 3 أزواج متتالية: إلى السجن!");
    add("mono_log_paid_jail", "%1 a payé 50 ₩ de caution pour sortir de prison.", "%1 paid 50 ₩ fine to leave Jail.", "دفع %1 كفالة 50 ₩ للخروج من السجن.");
    add("mono_log_used_jail_card", "%1 a utilisé une carte Libéré de prison.", "%1 used a Get Out of Jail Free card.", "استخدم %1 بطاقة الخروج من السجن.");
    add("mono_log_jail_double_escape", "%1 a fait un double et sort de prison !", "%1 rolled a double and escaped Jail!", "حصل %1 على نرد مزدوج وخرج من السجن!");
    add("mono_log_stay_in_jail", "%1 reste en prison.", "%1 stays in Jail.", "يبقى %1 في السجن.");
    add("mono_log_auction_started", "Mise aux enchères de %1 !", "Auction started for %1!", "بدأ المزاد العلني على %1!");
    add("mono_log_auction_won", "%1 remporte l'enchère pour %2 à %3 ₩.", "%1 won the auction for %2 at %3 ₩.", "فاز %1 بالمزاد على %2 مقابل %3 ₩.");
    add("mono_log_card_collect", "%1 reçoit %2 ₩ grâce à une carte.", "%1 collected %2 ₩ from a card.", "حصل %1 على %2 ₩ من بطاقة.");
    add("mono_log_card_pay", "%1 paye %2 ₩ suite à une carte.", "%1 paid %2 ₩ from a card.", "دفع %1 مبلغ %2 ₩ بسبب بطاقة.");
    add("mono_log_got_jail_card", "%1 obtient une carte Vous êtes libéré de prison.", "%1 received a Get Out of Jail Free card.", "حصل %1 على بطاقة الخروج من السجن.");
    add("mono_log_built_house", "%1 a construit une maison sur %2 (%3 ₩).", "%1 built a house on %2 (%3 ₩).", "بنى %1 منزلاً على %2 (%3 ₩).");
    add("mono_log_built_hotel", "%1 a construit un hôtel sur %2 (%3 ₩).", "%1 built a hotel on %2 (%3 ₩).", "بنى %1 فندقاً على %2 (%3 ₩).");
    add("mono_log_sold_building", "%1 a vendu un bâtiment sur %2 (+%3 ₩).", "%1 sold a building on %2 (+%3 ₩).", "باع %1 مبنى على %2 (+%3 ₩).");
    add("mono_log_mortgaged", "%1 a hypothéqué %2 (+%3 ₩).", "%1 mortgaged %2 (+%3 ₩).", "رهن %1 عقار %2 (+%3 ₩).");
    add("mono_log_unmortgaged", "%1 a levé l'hypothèque de %2 (-%3 ₩).", "%1 unmortgaged %2 (-%3 ₩).", "فك %1 رهن عقار %2 (-%3 ₩).");
    add("mono_log_trade_completed", "Échange conclu entre %1 et %2.", "Trade completed between %1 and %2.", "تمت مقايضة ناجحة بين %1 و %2.");
    add("mono_log_bankrupt", "%1 est déclaré en faillite !", "%1 has declared bankruptcy!", "أعلن %1 إفلاسه!");

    // Monopoly - 11 Interactive Visual Rules Pages (matching the screenshots)
    add("mono_rule_title_0", "LE BUT & POUR COMMENCER", "THE GOAL & GETTING STARTED", "الهدف والبداية");
    add("mono_rule_body_0",
        "LE BUT : Remportez le plus d'argent possible et ruinez vos adversaires !\n\n"
        "POUR COMMENCER :\n"
        "Jouez contre un, deux ou trois joueurs en cliquant sur AJOUTER JOUEUR.\n"
        "Choisissez la difficulté de vos adversaires :\n"
        " - Agent immobilier = adversaire facile\n"
        " - Entrepreneur = adversaire intermédiaire\n"
        " - Magnat de l'immobilier = adversaire difficile\n\n"
        "Cliquez sur l'icône d'un pion pour choisir votre propre pion.\n"
        "Cliquez sur JOUER pour commencer à faire fortune !",
        "THE GOAL: Earn the most money possible and bankrupt your opponents!\n\n"
        "GETTING STARTED:\n"
        "Play against one, two, or three players by clicking ADD PLAYER.\n"
        "Choose your opponents' difficulty:\n"
        " - Real Estate Agent = easy opponent\n"
        " - Entrepreneur = medium opponent\n"
        " - Real Estate Tycoon = hard opponent\n\n"
        "Click on a token icon to choose your own token.\n"
        "Click PLAY to start building your fortune!",
        "الهدف: اجمع أكبر ثروة ممكنة وأفلس منافسيك!\n\n"
        "للبدء:\n"
        "العب ضد لاعب أو اثنين أو ثلاثة بالنقر على إضافة لاعب.\n"
        "اختر مستوى صعوبة منافسيك:\n"
        " - وكيل عقاري = منافس سهل\n"
        " - مقاول = منافس متوسط\n"
        " - قطب العقارات = منافس صعب\n\n"
        "انقر على أيقونة البيدق لاختيار قطعتك المفضلة، ثم انقر على العب للبدء!");

    add("mono_rule_title_1", "L'ÉCRAN DE JEU", "THE GAME SCREEN", "شاشة اللعب");
    add("mono_rule_body_1",
        "Plateau : c'est ici que se déroule l'action de la partie.\n\n"
        "Informations joueur : ici, vous pouvez consulter l'argent possédé par chaque joueur sur leurs comptes, les propriétés qu'ils possèdent, leurs maisons et hôtels, et les cartes Vous êtes libéré de prison possédées.\n\n"
        "Banque : consultez les réserves de maisons et d'hôtels.\n\n"
        "Historique : rendez-vous ici pour consulter tous les événements relatifs à la partie en temps réel.",
        "Board: This is where all the game action takes place.\n\n"
        "Player Info: Check each player's cash balance, the properties they own by color group, their houses and hotels, and any Get Out of Jail Free cards held.\n\n"
        "Bank: Check available houses and hotels.\n\n"
        "History: View all timestamped game events in real time.",
        "اللوحة: هنا تدور كافة أحداث المباراة.\n\n"
        "معلومات اللاعب: يمكنك متابعة الرصيد المالي لكل لاعب، والعقارات التي يمتلكها حسب المجموعات اللونية، والمنازل والفنادق، وبطاقات الخروج من السجن.\n\n"
        "السجل: تابع كافة أحداث اللعبة لحظة بلحظة.");

    add("mono_rule_title_2", "LANCER LES DÉS", "ROLLING THE DICE", "رمي النرد");
    add("mono_rule_body_2",
        "Chaque tour débute par un lancer de dés. À votre tour, cliquez sur JETER LES DÉS.\n\n"
        "Votre pion se déplacera automatiquement sur le plateau du nombre de cases correspondant au résultat de vos dés. Les cases du plateau peuvent être occupées par plusieurs pions en même temps.\n\n"
        "DOUBLE : si les deux dés indiquent le même chiffre, vous avez fait un double ! Vous relancez les dés. Cependant, au TROISIÈME double consécutif, vous allez directement en prison.",
        "Each turn begins with a dice roll. On your turn, click ROLL DICE.\n\n"
        "Your token automatically moves around the board by the number of spaces rolled. Multiple tokens can occupy the same space simultaneously.\n\n"
        "DOUBLES: If both dice show the same number, you rolled a double and get another roll! However, on your THIRD consecutive double, you go directly to Jail.",
        "يبدأ كل دور برمي النرد. في دورك، انقر على رمي النرد.\n\n"
        "سيتحرك بيدقك تلقائياً على اللوحة بعدد الخانات الموافق لمجموع النرد.\n\n"
        "النرد المزدوج (الدوبل): إذا أظهر النردان نفس الرقم، تحصل على رمية إضافية! لكن عند الحصول على المزدوج الثالث على التوالي، تذهب مباشرة إلى السجن.");

    add("mono_rule_title_3", "ACHETER UNE PROPRIÉTÉ", "BUYING A PROPERTY", "شراء عقار والمزاد");
    add("mono_rule_body_3",
        "Quand vous vous arrêtez sur une propriété qui n'appartient à personne, la Banque vous propose de l'acheter. Cliquez sur ACHETER pour l'acquérir au prix indiqué.\n\n"
        "Ou cliquez sur ENCHÈRE pour l'acquérir peut-être à un prix moindre ! Tous les joueurs peuvent miser dans une enchère. Le joueur qui remporte l'enchère paye le montant de la mise gagnante à la Banque et reçoit le Titre de propriété. Si une propriété n'est pas achetée, elle est obligatoirement mise aux enchères.",
        "When you land on an unowned property, the Bank offers it for sale. Click BUY to purchase it at the listed price.\n\n"
        "Or click AUCTION to start a bidding war! All players can bid in an auction. The highest bidder pays their winning bid to the Bank and receives the Title Deed. If a property is not bought directly, it must be auctioned.",
        "عندما تتوقف على عقار غير مملوك لأحد، يعرض عليك البنك شراءه. انقر على شراء للحصول عليه بالسعر المعلن.\n\n"
        "أو انقر على مزاد لطرحه في مزاد علني بين جميع اللاعبين! الفائز بأعلى مزايدة يدفع المبلغ للبنك ويستلم سند الملكية.");

    add("mono_rule_title_4", "PAYER LE LOYER", "PAYING RENT", "دفع الإيجار");
    add("mono_rule_body_4",
        "Quand vous vous arrêtez sur une propriété qui appartient à un autre joueur, son propriétaire vous demande de payer un loyer selon le Titre de propriété. Si la propriété est hypothéquée, aucun loyer n'est perçu.\n\n"
        "ASTUCE POUR DEVENIR RICHE !\n"
        "Essayez de posséder tous les Titres de propriété d'un même groupe de couleur. Quand vous possédez toutes les propriétés d'un groupe de couleur, les loyers perçus sur les terrains nus sont doublés !",
        "When you land on a property owned by another player, you must pay rent according to its Title Deed card. If the property is mortgaged, no rent is collected.\n\n"
        "TIP TO GET RICH!\n"
        "Try to collect all Title Deeds of the same color group. When you own a complete color group, rent on unimproved properties in that group is doubled!",
        "عندما تتوقف على عقار يملكه لاعب آخر، يتوجب عليك دفع الإيجار المحدد في سند الملكية. إذا كان العقار مرهوناً فلا يُدفع أي إيجار.\n\n"
        "نصيحة للثراء!\n"
        "احرص على امتلاك كافة عقارات المجموعة اللونية الواحدة، فعندها يتضاعف إيجار الأراضي غير المبنية في تلك المجموعة!");

    add("mono_rule_title_5", "CARTES CHANCE & COMMUNAUTÉ", "CHANCE & COMMUNITY CHEST", "بطاقات الفرصة وصندوق الجماعة");
    add("mono_rule_body_5",
        "Quand vous vous arrêtez sur l'une de ces cases, vous tirez la carte du dessus de la pile. La Banque se charge automatiquement de prendre ou donner l'argent ou de déplacer votre pion selon la carte.\n\n"
        "CARTES VOUS ÊTES LIBÉRÉ DE PRISON :\n"
        "Vous pouvez conserver cette carte jusqu'à ce qu'elle vous soit utile pour sortir gratuitement de prison, ou la vendre à un autre joueur lors d'un échange.",
        "When you land on one of these spaces, you draw the top card from the deck. The Bank automatically collects or pays money or moves your token as instructed.\n\n"
        "GET OUT OF JAIL FREE CARDS:\n"
        "Keep this card until you need it to escape Jail for free, or trade it to another player at an agreed price.",
        "عند التوقف على إحدى هذه الخانات، تسحب البطاقة العلوية وتُنفذ تعليماتها تلقائياً.\n\n"
        "بطاقات الخروج من السجن:\n"
        "يمكنك الاحتفاظ بهذه البطاقة لاستخدامها عند الحاجة للخروج مجاناً من السجن، أو مقايضتها مع لاعب آخر.");

    add("mono_rule_title_6", "CASE DÉPART & PRISON", "GO & JAIL", "خانة الانطلاق والسجن");
    add("mono_rule_body_6",
        "CASE DÉPART : À chaque fois que votre pion s'arrête ou passe par la case Départ, la Banque vous paye 200 ₩ !\n\n"
        "PRISON : Vous allez en prison si vous tombez sur Allez en prison, tirez une carte Allez en prison, ou faites 3 doubles de suite.\n"
        "En prison, vous percevez toujours vos loyers et pouvez construire ou échanger. Pour sortir : faites un double (3 essais max), utilisez une carte Libéré de prison, ou payez une caution de 50 ₩ avant de lancer les dés.",
        "GO SPACE: Every time your token lands on or passes GO, the Bank pays you 200 ₩!\n\n"
        "JAIL: You go to Jail if you land on Go to Jail, draw a Go to Jail card, or roll 3 doubles in a row.\n"
        "While in Jail, you still collect rent and can build or trade. To get out: roll a double (up to 3 turns), use a Get Out of Jail Free card, or pay a 50 ₩ fine before rolling.",
        "خانة الانطلاق: في كل مرة تمر بها أو تتوقف عليها يدفع لك البنك 200 ₩!\n\n"
        "السجن: تذهب للسجن إذا توقفت على خانة اذهب إلى السجن، أو سحبت بطاقة السجن، أو رميت 3 أزواج متتالية.\n"
        "للخروج من السجن: ارمِ نرداً مزدوجاً، أو استخدم بطاقة الخروج من السجن، أو ادفع كفالة 50 ₩.");

    add("mono_rule_title_7", "ÉCHANGER", "TRADING", "المقايضة والتبادل");
    add("mono_rule_body_7",
        "Les joueurs peuvent conclure des échanges en cliquant sur ÉCHANGE en bas de l'écran.\n\n"
        "Vous pouvez échanger des propriétés (sans bâtiments), de l'argent et des cartes Vous êtes libéré de prison.\n"
        " - Sélectionnez ce que vous proposez dans votre colonne de biens.\n"
        " - Sélectionnez ce que vous souhaitez obtenir chez votre adversaire.\n"
        " - Définissez l'argent éventuel à transférer et cliquez sur PROPOSITION.",
        "Players can make deals at any time on their turn by clicking TRADE at the bottom of the screen.\n\n"
        "You can trade unimproved properties, cash, and Get Out of Jail Free cards.\n"
        " - Select what you want to offer from your column.\n"
        " - Select what you want in return from your opponent's column.\n"
        " - Adjust any cash transfer and click PROPOSE.",
        "يمكن للاعبين إبرام صفقات تبادل بالنقر على زر مقايضة أسفل الشاشة.\n\n"
        "يمكنك مبادلة العقارات (غير المبنية) والأموال وبطاقات الخروج من السجن.\n"
        " - اختر ما تعرضه من عمود ممتلكاتك وما تطلبه من عمود الخصم ثم انقر على اقتراح.");

    add("mono_rule_title_8", "MAISONS & HÔTELS", "HOUSES & HOTELS", "المنازل والفنادق");
    add("mono_rule_body_8",
        "MAISONS : Quand vous possédez toutes les propriétés d'un groupe de couleur (sans hypothèque), cliquez sur CONSTRUIRE pour acheter des maisons à la Banque.\n"
        "Règle de construction égale : vous devez construire uniformément (1 maison sur chaque terrain du groupe avant de passer à la 2e maison, jusqu'à 4 maisons par terrain).\n\n"
        "HÔTELS : Quand vous avez 4 maisons sur chaque propriété d'un groupe, vous pouvez les améliorer en Hôtel ! La limite de la Banque est de 32 maisons et 12 hôtels.",
        "HOUSES: When you own an entire color group (unmortgaged), click BUILD to buy houses from the Bank.\n"
        "Even Building Rule: You must build evenly across the group (1 house on each property before building a 2nd house on any, up to 4 houses per property).\n\n"
        "HOTELS: Once you have 4 houses on every property of a group, you can upgrade to a Hotel! Bank limit: 32 houses and 12 hotels.",
        "المنازل: عندما تمتلك كافة عقارات مجموعة لونية كاملة، انقر على بناء لشراء المنازل من البنك.\n"
        "قاعدة البناء المتساوي: يجب البناء بالتساوي على عقارات المجموعة (منزل على كل عقار قبل بناء المنزل الثاني، حتى 4 منازل).\n\n"
        "الفنادق: بعد بناء 4 منازل على كل عقار في المجموعة، يمكنك ترقيتها إلى فندق! يمتلك البنك 32 منزلاً و12 فندقاً.");

    add("mono_rule_title_9", "VENDRE & HYPOTHÈQUES", "SELLING & MORTGAGES", "البيع والرهن العقاري");
    add("mono_rule_body_9",
        "VENDRE DES BÂTIMENTS : La Banque rachète vos maisons et hôtels à la moitié de leur prix d'achat (de manière uniforme sur le groupe).\n\n"
        "HYPOTHÈQUES : Vous manquez d'argent ? Cliquez sur HYPOTHÉQUER. La Banque vous avance la moitié de la valeur du terrain (tous les bâtiments du groupe doivent d'abord être vendus).\n"
        "Pour lever l'hypothèque, cliquez sur LEVER L'HYPOTHÈQUE et remboursez l'hypothèque majorée de 10 % d'intérêts.",
        "SELLING BUILDINGS: The Bank buys back houses and hotels at half their purchase price (evenly across the color group).\n\n"
        "MORTGAGES: Short on cash? Click MORTGAGE. The Bank lends you half the property's value (all buildings in the color group must be sold first).\n"
        "To unmortgage, click UNMORTGAGE and pay the mortgage value plus 10% interest.",
        "بيع المباني: يشتري البنك المنازل والفنادق بنصف سعر شرائها بالتساوي.\n\n"
        "الرهن العقاري: عند الحاجة للسيولة، انقر على رهن للحصول على نصف قيمة العقار من البنك (بشرط عدم وجود مباني في المجموعة).\n"
        "لفك الرهن، انقر على فك الرهن وادفع قيمة الرهن مضافاً إليها 10% فائدة.");

    add("mono_rule_title_10", "FAILLITE", "BANKRUPTCY", "الإفلاس ونهاية اللعبة");
    add("mono_rule_body_10",
        "FAILLITE :\n"
        "Si vous devez plus d'argent que vous ne pouvez en réunir en vendant vos bâtiments, en hypothéquant vos propriétés ou en échangeant, vous êtes déclaré en faillite et éliminé de la partie !\n\n"
        "Vos biens restants reviennent à votre créancier. Le dernier joueur en lice remporte la partie et devient le grand maître de Monopoly !",
        "BANKRUPTCY:\n"
        "If you owe more money than you can raise by selling buildings, mortgaging properties, or trading, you are declared bankrupt and eliminated!\n\n"
        "Your remaining assets are turned over to your creditor. The last remaining solvent player wins the game!",
        "الإفلاس:\n"
        "إذا عجزت عن سداد ديونك حتى بعد بيع المباني ورهن العقارات، يُعلن إفلاسك وتخرج من اللعبة!\n\n"
        "تنتقل ممتلكاتك المتبقية إلى الدائن، ويفوز باللعبة آخر لاعب صامد!");
}

QString LanguageManager::rulesCheckersHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>قواعد لعبة الدامة (8×8)</h2>"
            "<p><b>1. الهدف:</b> أسر جميع قطع الخصم أو محاصرتها لمنعه من التحرك.</p>"
            "<p><b>2. حركة القطع (البيادق):</b> تتحرك البيادق خطوة واحدة قطرياً للأمام على المربعات الداكنة.</p>"
            "<p><b>3. الأسر:</b> تقفز القطعة فوق قطعة الخصم المجاورة إلى المربع الفارغ الذي يليها. الأسر المتتالي (السلسلة) متاح ومطلوب.</p>"
            "<p><b>4. الترقية للدامة (الملك):</b> عندما يصل البيدق إلى الصف الأخير للخصم، يتحول إلى «دامة» يمكنها التحرك والأسر قطرياً للأمام والخلف عبر مسافات حرة.</p>"
            "<p><b>5. الفوز:</b> يفوز اللاعب الذي يأسر كافة قطع الخصم أو يجبره على العجز عن الحركة.</p>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Checkers Rules (8x8)</h2>"
            "<p><b>1. Goal:</b> Capture all opponent pieces or block them so they have no legal moves left.</p>"
            "<p><b>2. Pawn Moves:</b> Pawns move forward diagonally by one dark square.</p>"
            "<p><b>3. Captures:</b> Jumping over an adjacent opponent piece into an empty square behind it captures that piece. Multi-jumps are supported!</p>"
            "<p><b>4. King Promotion:</b> Reaching the opponent's back row promotes a pawn into a King (Queen), allowing backward and multi-direction movements.</p>"
            "<p><b>5. Victory:</b> The game ends when a player has no pieces left or has no legal moves available.</p>"
        );
    } else {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Règles du Jeu de Dames (8x8)</h2>"
            "<p><b>1. But du jeu :</b> Capturer tous les pions adverses ou les bloquer pour qu'ils ne puissent plus bouger.</p>"
            "<p><b>2. Déplacement des pions :</b> Les pions avancent d'une case en diagonale vers l'avant, uniquement sur les cases sombres.</p>"
            "<p><b>3. Prise & Rafle :</b> Un pion peut sauter par-dessus une pièce adverse adjacente pour atterrir sur la case libre située juste derrière. Les prises enchaînées (rafles) sont possibles !</p>"
            "<p><b>4. Promotion en Dame :</b> Tout pion atteignant la dernière rangée adverse est couronné en Dame. La Dame peut se déplacer en avant et en arrière en diagonale.</p>"
            "<p><b>5. Fin de partie :</b> Le premier joueur qui élimine ou bloque totalement son adversaire gagne la partie.</p>"
        );
    }
}

QString LanguageManager::rulesGoHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>قواعد لعبة الغو (Baduk)</h2>"
            "<p><b>1. الهدف:</b> إحاطة أكبر مساحة ممكنة من اللوحة بالحجارة الخاصة بك (الأسود والأبيض).</p>"
            "<p><b>2. وضع الحجارة:</b> يضع اللاعبون بالتناوب حجراً على تقاطعات الشبكة (9×9). لا تتحرك الحجارة بعد وضعها.</p>"
            "<p><b>3. الحريات والأسر:</b> كل حجر أو مجموعة متصلة تمتلك تقاطعات مجاورة فارغة تسمى «حريات». عندما تُحرم المجموعة من آخر حرية لها، تُؤسر وتُزال من اللوحة فوراً.</p>"
            "<p><b>4. قاعدة الكو (Kō):</b> يُمنع تكرار نفس شكل اللوحة مباشرة لمنع الحلقات اللانهائية.</p>"
            "<p><b>5. التمرير والنهاية:</b> يمكن للاعب تمرير دوره. إذا مرر كلا اللاعبين بالتتابع، تنتهي اللعبة ويُحسب الفائز بناءً على الأراضي والأسر.</p>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Go Game Rules (Baduk)</h2>"
            "<p><b>1. Goal:</b> Control more territory on the board than the opponent by placing Black and White stones.</p>"
            "<p><b>2. Placement:</b> Players take turns placing a stone on intersections of the 9x9 grid. Stones are never moved once placed.</p>"
            "<p><b>3. Liberties & Captures:</b> Stones and connected groups have orthogonally adjacent empty points called 'liberties'. When a group has 0 liberties left, it is captured and removed from the board.</p>"
            "<p><b>4. The Ko Rule:</b> Players cannot make a move that instantly repeats the exact previous board position.</p>"
            "<p><b>5. Passing & Scoring:</b> A player can pass their turn. When both players pass consecutively, the game ends and score is counted (territory + captured stones).</p>"
        );
    } else {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Règles du Jeu de Go (Weiqi / Baduk)</h2>"
            "<p><b>1. But du jeu :</b> Contrôler plus de territoire que l'adversaire en posant des pierres Noires et Blanches sur les intersections.</p>"
            "<p><b>2. Pose des pierres :</b> Les joueurs posent à tour de rôle une pierre sur les intersections de la grille 9x9. Les pierres ne bougent plus une fois posées.</p>"
            "<p><b>3. Libertés & Captures :</b> Les pierres et groupes reliés possèdent des intersections adjacentes libres appelées « libertés ». Un groupe dont la dernière liberté est comblée est capturé et retiré du goban.</p>"
            "<p><b>4. Règle du Kō :</b> Il est interdit de rejouer un coup qui recrée immédiatement la situation exacte du plateau précédent.</p>"
            "<p><b>5. Fin de partie :</b> Tout joueur peut passer son tour. Lorsque les deux joueurs passent consécutivement, la partie s'arrête et les territoires sont décomptés.</p>"
        );
    }
}

QString LanguageManager::rulesConnectFourHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>قواعد لعبة أربعة على التوالي (Connect Four)</h2>"
            "<p><b>1. الهدف:</b> أن تكون أول لاعب يصفّ 4 قطع من لونه في خط مستقيم (أفقياً، عمودياً، أو قطرياً).</p>"
            "<p><b>2. طريقة اللعب:</b> يتناوب اللاعبان (الأحمر والأصفر) على إسقاط قطعة في أحد الأعمدة السبعة (7 أعمدة × 6 صفوف). تسقط القطعة دائماً إلى أدنى خانة فارغة في العمود بفعل الجاذبية.</p>"
            "<p><b>3. شروط الفوز:</b> يفوز فوراً أول لاعب ينجح في محاذاة 4 قطع متتالية.</p>"
            "<p><b>4. التعادل:</b> إذا امتلأت جميع خانات اللوحة الـ 42 دون أن ينجح أي لاعب في محاذاة 4 قطع، تنتهي اللعبة بالتعادل.</p>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Connect Four Rules</h2>"
            "<p><b>1. Goal:</b> Be the first player to connect 4 of your colored discs in a line (horizontal, vertical, or diagonal).</p>"
            "<p><b>2. Gameplay:</b> Players take turns dropping one colored token (Red or Yellow) into one of the 7 columns. The disc falls down by gravity to the lowest unoccupied slot in that column.</p>"
            "<p><b>3. Victory:</b> The first player to form an unbroken horizontal, vertical, or diagonal line of 4 consecutive discs wins immediately!</p>"
            "<p><b>4. Draw:</b> If the 7x6 board is completely filled and neither player has achieved a line of 4, the game ends in a tie.</p>"
        );
    } else {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Règles du Puissance 4</h2>"
            "<p><b>1. But du jeu :</b> Être le premier joueur à aligner 4 jetons de sa couleur horizontalement, verticalement ou en diagonale.</p>"
            "<p><b>2. Déroulement :</b> Les joueurs déposent à tour de rôle un jeton (Rouge ou Jaune) dans l'une des 7 colonnes de la grille (7x6). Le jeton glisse par gravité jusqu'à la position libre la plus basse de la colonne.</p>"
            "<p><b>3. Victoire :</b> Dès qu'un joueur parvient à aligner 4 jetons consécutifs de sa couleur, il remporte instantanément la manche.</p>"
            "<p><b>4. Match nul :</b> Si toutes les cases de la grille sont remplies sans qu'aucun alignement de 4 ne soit formé, la partie se solde par un match nul.</p>"
        );
    }
}

QString LanguageManager::rulesReversiHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>قواعد لعبة ريفيرسي (Othello)</h2>"
            "<p><b>1. الهدف:</b> امتلاك أكبر عدد من القطع التي تظهر لونك (أسود أو أبيض) عند نهاية اللعبة على رقعة 8×8.</p>"
            "<p><b>2. البداية:</b> تبدأ اللعبة بـ 4 قطع في المركز، قطعتان سوداوان وقطعتان بيضاوان على شكل تقاطع قطري. يبدأ الأسود دائماً باللعب.</p>"
            "<p><b>3. حركة الانقلاب:</b> يجب وضع القطعة في خانة فارغة تحصر قطعة أو أكثر من قطع الخصم بين القطعة الجديدة وقطعة أخرى من نفس لون اللاعب (أفقياً، عمودياً، أو قطرياً). تُقلب كافة قطع الخصم المحصورة لتصبح بلونك فوراً!</p>"
            "<p><b>4. التمرير:</b> إذا لم يمتلك اللاعب أي حركة قانونية لحصر قطع الخصم، يمر دوره تلقائياً للخصم.</p>"
            "<p><b>5. النهاية:</b> تنتهي اللعبة عندما لا يمتلك أي من اللاعبين حركة صالحة، أو عندما تمتلئ الرقعة. يفوز صاحب العدد الأكبر من القطع.</p>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Reversi (Othello) Rules</h2>"
            "<p><b>1. Goal:</b> Have the majority of discs showing your color (Black or White) at the end of the game on an 8x8 grid.</p>"
            "<p><b>2. Initial Setup:</b> 4 discs placed in the center (two black, two white, diagonally opposite). Black moves first.</p>"
            "<p><b>3. Flanking & Flipping:</b> You must place your disc on an empty square such that it traps one or more continuous opponent discs between your newly placed disc and another disc of your color in any horizontal, vertical, or diagonal direction. All trapped opponent discs are flipped to your color!</p>"
            "<p><b>4. Passing:</b> If a player has no legal move that brackets an opponent disc, their turn is automatically passed.</p>"
            "<p><b>5. Victory:</b> The game ends when neither player has a legal move or the board is full. The player with the most discs of their color wins.</p>"
        );
    } else {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Règles du Reversi (Othello)</h2>"
            "<p><b>1. But du jeu :</b> Avoir une majorité de pions de sa couleur (Noir ou Blanc) sur le plateau 8x8 à la fin de la partie.</p>"
            "<p><b>2. Mise en place :</b> La partie débute avec 4 pions au centre (2 blancs et 2 noirs en diagonale). Les Noirs jouent en premier.</p>"
            "<p><b>3. Encadrement & Retournement :</b> Tout coup joué doit obligatoirement encadrer un ou plusieurs pions adverses en ligne droite (horizontale, verticale ou diagonale) entre le pion posé et un autre pion de sa couleur déjà présent. Tous les pions adverses encadrés sont alors retournés à votre couleur !</p>"
            "<p><b>4. Passe obligatoire :</b> Si un joueur n'a aucun coup légal permettant de retourner au moins un pion adverse, il passe obligatoirement son tour.</p>"
            "<p><b>5. Fin de partie :</b> Lorsque le plateau est plein ou qu'aucun joueur ne peut plus jouer, le joueur ayant le plus de pions l'emporte.</p>"
        );
    }
}

QString LanguageManager::rulesQuoridorHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>قواعد لعبة كوريدور (Quoridor)</h2>"
            "<p><b>1. الهدف:</b> أن تكون أول لاعب يوصل بَيْدَقه إلى أي خانة في الصف المقابل لبدايته على رقعة 9×9.</p>"
            "<p><b>2. التجهيز:</b> يبدأ اللاعب 1 في منتصف الصف السفلي (الهدف الصف 0)، واللاعب 2 في منتصف الصف العلوي (الهدف الصف 8). يمتلك كل لاعب 10 حواجز خشبية.</p>"
            "<p><b>3. الدور:</b> في دورك، تختار أحد أمرين: إما تحريك بَيْدَقك خانة واحدة (أفقياً أو عمودياً، مع إمكانية القفز فوق الخصم إذا كان مجاوراً)، أو وضع حاجز خشبي بين الخانات.</p>"
            "<p><b>4. قواعد الحواجز:</b> يغطي الحاجز حافتين بين المربعات (أفقياً أو عمودياً). يُمنع منعاً باتاً وضع حاجز يُغلق الطريق بالكامل ويمنع أي لاعب من الوصول لهدفه (يجب أن يبقى دائماً مسار حر واحد على الأقل).</p>"
            "<p><b>5. الفوز:</b> أول من يلمس صف النهاية يفوز بالمباراة فوراً.</p>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Quoridor Rules</h2>"
            "<p><b>1. Goal:</b> Be the first player to reach any square on the baseline opposite to your starting edge on a 9x9 board.</p>"
            "<p><b>2. Setup:</b> Player 1 starts at the middle of the bottom edge (targeting row 0), Player 2 at the middle of the top edge (targeting row 8). Each player has 10 wooden fences (walls).</p>"
            "<p><b>3. Turn Action:</b> On each turn, you choose to either move your pawn one square orthogonally (or jump over an adjacent opponent pawn), or place one fence between squares.</p>"
            "<p><b>4. Fence Placement:</b> Each fence spans 2 grid cells (horizontal or vertical). Golden rule: it is strictly forbidden to completely block the path to the goal for either player (at least one valid open path must always exist)!</p>"
            "<p><b>5. Victory:</b> The first pawn to land on its goal row wins immediately.</p>"
        );
    } else {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Règles de Quoridor</h2>"
            "<p><b>1. But du jeu :</b> Être le premier à atteindre l'une des cases de la ligne opposée sur le plateau 9x9.</p>"
            "<p><b>2. Position de départ :</b> Le Joueur 1 part du milieu de la rangée du bas (vers la rangée 0), et le Joueur 2 du milieu de la rangée du haut (vers la rangée 8). Chacun dispose d'une réserve de 10 barrières.</p>"
            "<p><b>3. Actions au tour :</b> À son tour, le joueur choisit soit de déplacer son pion d'une case orthogonale (ou sauter par-dessus l'adversaire s'il est adjacent), soit de poser une barrière en bois.</p>"
            "<p><b>4. Pose des barrières :</b> Une barrière mesure 2 cases de long (horizontale ou verticale). Règle absolue : il est strictement interdit de bloquer totalement l'accès à la ligne de but ; chaque joueur doit toujours conserver au moins un chemin praticable vers sa ligne d'arrivée !</p>"
            "<p><b>5. Victoire :</b> Le premier pion qui touche la ligne adverse gagne la partie.</p>"
        );
    }
}

QString LanguageManager::rulesMorrisHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>قواعد لعبة الطاحونة (Nine Men's Morris)</h2>"
            "<p><b>1. الهدف:</b> تقليص قطع الخصم إلى أقل من 3 قطع أو محاصرتها لمنعه من التحرك.</p>"
            "<p><b>2. اللوحة والقطع:</b> تتكون اللوحة من 3 مربعات متحدة المركز تربط بينها خطوط في منتصف الأضلاع (24 نقطة تقاطع). يمتلك كل لاعب 9 قطع (أبيض وأسود).</p>"
            "<p><b>3. المراحل:</b></p>"
            "<ul>"
            "<li><b>المرحلة 1 (الوضع):</b> يضع اللاعبون قطعهم التسع بالتناوب على التقاطعات الفارغة.</li>"
            "<li><b>المرحلة 2 (التحريك):</b> يتحرك كل لاعب بنقل قطعة واحدة إلى نقطة مجاورة فارغة على طول الخطوط.</li>"
            "<li><b>المرحلة 3 (الطيران):</b> عندما يتبقى للاعب 3 قطع فقط، يُسمح له بـ «الطيران» بنقل قطعته لأي نقطة فارغة في اللوحة.</li>"
            "</ul>"
            "<p><b>4. الطاحونة والأسر:</b> عندما يصطف 3 قطع من نفس اللون على خط واحد مستقيم، تتكون «طاحونة» ويحق للاعب إزالة قطعة واحدة من قطع الخصم (بشرط ألا تكون القطعة المستهدفة جزءاً من طاحونة مكتملة للخصم، إلا إذا كانت جميع قطعه في طواحين).</p>"
            "<p><b>5. الفوز:</b> يفوز اللاعب إذا بقي للخصم قطعتان فقط أو عجز عن القيام بأي حركة صالحة.</p>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Nine Men's Morris Rules</h2>"
            "<p><b>1. Goal:</b> Reduce the opponent to fewer than 3 pieces or leave them with no legal moves.</p>"
            "<p><b>2. Board:</b> 3 concentric squares connected at side midpoints (24 intersection points). Each player starts with 9 pieces (White and Black).</p>"
            "<p><b>3. Phases:</b></p>"
            "<ul>"
            "<li><b>Phase 1 (Placing):</b> Players take turns placing their 9 pieces onto vacant points.</li>"
            "<li><b>Phase 2 (Moving):</b> Players take turns sliding one piece along a line to an adjacent empty point.</li>"
            "<li><b>Phase 3 (Flying):</b> When a player is down to 3 pieces, their pieces can 'fly' to any vacant point on the board.</li>"
            "</ul>"
            "<p><b>4. Mills & Captures:</b> Aligning 3 pieces of the same color on a straight line forms a 'mill', allowing you to remove one opponent piece from the board (pieces inside an opponent mill cannot be removed unless all their pieces are in mills).</p>"
            "<p><b>5. Victory:</b> You win when the opponent is reduced to 2 pieces or cannot make any legal move.</p>"
        );
    } else {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Règles du Jeu du Moulin (Mérelles)</h2>"
            "<p><b>1. But du jeu :</b> Réduire l'adversaire à moins de 3 pions ou le bloquer totalement pour qu'il ne puisse plus se déplacer.</p>"
            "<p><b>2. Plateau :</b> 3 carrés concentriques reliés par les milieux de leurs côtés, offrant 24 points d'intersection. Chaque joueur dispose de 9 pions.</p>"
            "<p><b>3. Déroulement en 3 phases :</b></p>"
            "<ul>"
            "<li><b>Phase 1 (Pose) :</b> Les joueurs posent alternativement leurs 9 pions sur les points libres.</li>"
            "<li><b>Phase 2 (Déplacement) :</b> Chaque joueur glisse l'un de ses pions vers un point adjacent libre le long des lignes.</li>"
            "<li><b>Phase 3 (Vol) :</b> Dès qu'un joueur ne possède plus que 3 pions, il peut « voler » et téléporter son pion sur n'importe quel point libre du plateau.</li>"
            "</ul>"
            "<p><b>4. Moulin & Prise :</b> Aligner 3 pions de sa couleur forme un « moulin » et autorise à capturer immédiatement un pion adverse (un pion faisant partie d'un moulin adverse ne peut être capturé, sauf si tous les pions adverses sont dans des moulins).</p>"
            "<p><b>5. Victoire :</b> Le joueur gagne si l'adversaire n'a plus que 2 pions ou ne peut plus bouger.</p>"
        );
    }
}

QString LanguageManager::rulesDotsAndBoxesHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>قواعد لعبة النقاط والمربعات (La Pipopipette)</h2>"
            "<p><b>1. الهدف:</b> إغلاق أكبر عدد ممكن من المربعات (1×1) وتسجيل أعلى مجموع نقاط على شبكة النقاط.</p>"
            "<p><b>2. طريقة اللعب:</b> يتناوب اللاعبان على رسم خط واحد (حافة أفقية أو عمودية) يصل بين نقطتين متجاورتين.</p>"
            "<p><b>3. إغلاق المربع:</b> عندما يكتمل الضلع الرابع لأي مربع 1×1 بفضل خط رسمه اللاعب، يستولي اللاعب على ذلك المربع ويحصل على نقطة. ينتقل الدور مباشرة بعد ذلك إلى اللاعب التالي كالمعتاد (لا يوجد دور إضافي).</p>"
            "<p><b>4. النهاية والفوز:</b> تنتهي اللعبة بمجرد إغلاق كافة المربعات على الرقعة. يفوز اللاعب الذي جمع أكبر رصيد من المربعات.</p>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Dots and Boxes Rules</h2>"
            "<p><b>1. Goal:</b> Complete the most 1x1 boxes to earn the highest score on the grid.</p>"
            "<p><b>2. Gameplay:</b> Players take turns drawing a single horizontal or vertical line connecting two adjacent dots.</p>"
            "<p><b>3. Completing a Box:</b> Completing the fourth wall of a 1x1 box claims that box with your color and awards 1 point. The turn then passes to the next player as usual (no extra turn is granted).</p>"
            "<p><b>4. Victory:</b> The game ends when all boxes have been claimed. The player with the highest box count wins!</p>"
        );
    } else {
        return QString::fromUtf8(
            "<h2 style='color:#E5A93C;'>Règles de La Pipopipette (Dots & Boxes)</h2>"
            "<p><b>1. But du jeu :</b> Capturer le plus grand nombre de cases (carrés 1x1) sur la grille de points pour obtenir le score le plus élevé.</p>"
            "<p><b>2. Déroulement :</b> Les joueurs tracent à tour de rôle un segment (horizontal ou vertical) reliant deux points voisins non encore connectés.</p>"
            "<p><b>3. Fermeture de case :</b> Le joueur qui trace le quatrième côté fermant un carré capture cette case à sa couleur et marque 1 point. Le tour passe ensuite normalement au joueur adverse (aucun tour supplémentaire n'est accordé).</p>"
            "<p><b>4. Fin de partie :</b> La partie prend fin dès que tous les carrés de la grille sont fermés. Le joueur ayant conquis le plus de carrés est proclamé vainqueur.</p>"
        );
    }
}

QString LanguageManager::rulesMonopolyHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2 style='color:#D4AF37;'>قواعد لعبة مونوبولي (Monopoly)</h2>"
            "<p><b>1. الهدف:</b> جمع أكبر ثروة ممكنة وإفلاس جميع المنافسين عبر شراء وتطوير وتأجير العقارات.</p>"
            "<p><b>2. رمي النرد والحركة:</b> يبدأ كل دور برمي النردين. إذا حصلت على نرد مزدوج (Double) تلعب دوراً إضافياً، ولكن 3 أزواج متتالية ترسلك فوراً إلى السجن. عند المرور بخانة الانطلاق تحصل على 200 ₩.</p>"
            "<p><b>3. الشراء والمزاد العلني:</b> عند التوقف على عقار غير مملوك، يمكنك شراؤه بالسعر المعلن أو طرحه في مزاد علني يشارك فيه كافة اللاعبين.</p>"
            "<p><b>4. الإيجار والمجموعات اللونية:</b> يتضاعف إيجار الأراضي الفارغة إذا امتلك المالك جميع عقارات المجموعة اللونية الكاملة. لا يُدفع إيجار عن العقارات المرهونة.</p>"
            "<p><b>5. المنازل والفنادق:</b> عند امتلاك مجموعة لونية كاملة، يمكنك بناء المنازل بالتساوي (حتى 4 منازل لكل عقار) ثم ترقيتها إلى فندق.</p>"
            "<p><b>6. المقايضة والرهن والإفلاس:</b> يمكنك مقايضة العقارات والأموال وبطاقات الخروج من السجن مع اللاعبين الآخرين، أو رهن العقارات لدى البنك مقابل نصف قيمتها. اللاعب الذي يعجز عن سداد ديونه يعلن إفلاسه.</p>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2 style='color:#D4AF37;'>Monopoly Rules</h2>"
            "<p><b>1. Goal:</b> Earn the greatest fortune and bankrupt your opponents by buying, developing, and trading properties.</p>"
            "<p><b>2. Rolling & Moving:</b> Roll two dice to move around the board. Rolling doubles grants an extra turn, but 3 consecutive doubles send you straight to Jail! Collect 200 ₩ every time you pass GO.</p>"
            "<p><b>3. Buying & Auctions:</b> Land on an unowned property to buy it at face value or put it up for auction where all players bid.</p>"
            "<p><b>4. Rent & Color Monopolies:</b> Owning all properties in a color group doubles the rent on unimproved lots and unlocks house construction. No rent is collected on mortgaged properties.</p>"
            "<p><b>5. Houses & Hotels:</b> Build houses evenly across a completed color group (up to 4 per property), then upgrade 4 houses into a Hotel.</p>"
            "<p><b>6. Trading, Mortgages & Bankruptcy:</b> Trade properties, cash, and Get Out of Jail Free cards with other players, or mortgage properties to the Bank for half their price. A player who cannot pay their debts declares bankruptcy.</p>"
        );
    } else {
        return QString::fromUtf8(
            "<h2 style='color:#D4AF37;'>Règles du Monopoly</h2>"
            "<p><b>1. But du jeu :</b> Remporter le plus d'argent possible et mener vos adversaires à la faillite en achetant, construisant et négociant des propriétés.</p>"
            "<p><b>2. Lancer les dés & Case Départ :</b> Lancez les deux dés pour avancer. Un double vous permet de rejouer, mais 3 doubles consécutifs vous envoient directement en prison ! Recevez 200 ₩ à chaque passage par la case Départ.</p>"
            "<p><b>3. Achat & Enchères :</b> Sur une propriété libre, cliquez sur ACHETER au prix indiqué ou lancez une ENCHÈRE ouverte à tous les joueurs.</p>"
            "<p><b>4. Loyers & Groupes de couleur :</b> Posséder toutes les propriétés d'un même groupe de couleur double le loyer des terrains nus et permet de construire. Aucun loyer n'est dû sur une propriété hypothéquée.</p>"
            "<p><b>5. Maisons & Hôtels :</b> Construisez uniformément jusqu'à 4 maisons par terrain d'un groupe complet, puis améliorez-les en Hôtel (limite de la Banque : 32 maisons et 12 hôtels).</p>"
            "<p><b>6. Échanges, Hypothèques & Faillite :</b> Échangez terrains, argent et cartes Libéré de prison, ou hypothéquez vos biens pour la moitié de leur valeur. Tout joueur incapable de rembourser ses dettes est déclaré en faillite.</p>"
        );
    }
}

QString LanguageManager::aboutHtml() const {
    if (m_currentLanguage == Language::Arabic) {
        return QString::fromUtf8(
            "<h2>ستراتيجي - منصة ألعاب الذكاء والاستراتيجية</h2>"
            "<hr/>"
            "<p>منصة متكاملة لألعاب الطاولة والتفكير الاستراتيجي الكلاسيكية تتضمن:</p>"
            "<ul>"
            "<li><b>7 ألعاب كلاسيكية كاملة:</b> الدامة، الغو (بادوك)، أربعة على التوالي، ريفيرسي (عطيل)، كوريدور، لعبة الطاحونة (الموريس)، والنقاط والمربعات (البيبوتيبيت).</li>"
            "<li><b>أنماط لعب متعددة:</b> اللعب بين لاعبين محلياً أو اللعب ضد الكمبيوتر مع 3 مستويات من التحدي (مبتدئ، متوسط، خبير).</li>"
            "<li><b>التراجع التفاعلي (Undo):</b> إمكانية التراجع عن النقلات في أي وقت لكافة الألعاب.</li>"
            "<li><b>دعم لغات عالمي:</b> العربية، الفرنسية، والإنجليزية مع دعم كامل لاتجاه الواجهة (RTL).</li>"
            "<li><b>مظهر عصري ومريح:</b> تصميم إلكتروني أنيق يدعم الوضع الليلي والوضع الفاتح بلمسة واحدة.</li>"
            "</ul>"
        );
    } else if (m_currentLanguage == Language::English) {
        return QString::fromUtf8(
            "<h2>Strategix - Strategy Board Games Suite</h2>"
            "<hr/>"
            "<p>A comprehensive standalone suite of classic combinatorial and strategy board games featuring:</p>"
            "<ul>"
            "<li><b>7 Complete Strategy Games:</b> Checkers, Go (Baduk), Connect Four, Reversi (Othello), Quoridor, Nine Men's Morris, and Dots & Boxes.</li>"
            "<li><b>Versatile Game Modes:</b> 2 Players (Local pass-and-play) or 1 Player vs Computer across 3 levels (Easy, Medium, Expert).</li>"
            "<li><b>Move Undo:</b> Full snapshot history with unlimited undo across all games.</li>"
            "<li><b>Multilingual Interface:</b> English, French, and Arabic with native Right-to-Left (RTL) layout switching.</li>"
            "<li><b>Minimalist Modern Design:</b> Clean aesthetic with instantaneous Dark Mode and Light Mode switching.</li>"
            "</ul>"
        );
    } else {
        return QString::fromUtf8(
            "<h2>Stratégix - Suite de Jeux de Stratégie</h2>"
            "<hr/>"
            "<p>Une plateforme complète et autonome regroupant les plus grands classiques des jeux de réflexion et de stratégie combinatoire :</p>"
            "<ul>"
            "<li><b>7 Jeux Complets :</b> Jeu de Dames, Jeu de Go, Puissance 4, Reversi (Othello), Quoridor, Jeu du Moulin et La Pipopipette (Dots & Boxes).</li>"
            "<li><b>Modes de Jeu :</b> 2 Joueurs en local ou 1 Joueur contre l'ordinateur avec 3 niveaux de difficulté (Débutant, Intermédiaire, Expert).</li>"
            "<li><b>Annulation de coup (Undo) :</b> Historique complet des coups avec retour en arrière instantané sur chaque jeu.</li>"
            "<li><b>Support Multilingue :</b> Français, Anglais et Arabe avec bascule dynamique et prise en charge native du sens d'écriture (RTL).</li>"
            "<li><b>Interface Épurée & Thèmes :</b> Rendu vectoriel soigné avec bascule immédiate entre Mode Sombre et Mode Clair.</li>"
            "</ul>"
        );
    }
}

} // namespace Strategix
