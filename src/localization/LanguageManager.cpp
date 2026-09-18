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
