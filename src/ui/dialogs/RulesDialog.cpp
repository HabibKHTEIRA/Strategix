#include "RulesDialog.h"
#include "../../localization/LanguageManager.h"

#include <QVBoxLayout>
#include <QPushButton>

namespace Strategix {

RulesDialog::RulesDialog(int initialTab, QWidget *parent)
    : QDialog(parent)
{
    resize(700, 560);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_tabs = new QTabWidget(this);

    m_checkersBrowser = new QTextBrowser(this);
    m_checkersBrowser->setOpenExternalLinks(true);
    m_tabs->addTab(m_checkersBrowser, "");

    m_goBrowser = new QTextBrowser(this);
    m_goBrowser->setOpenExternalLinks(true);
    m_tabs->addTab(m_goBrowser, "");

    m_connectFourBrowser = new QTextBrowser(this);
    m_connectFourBrowser->setOpenExternalLinks(true);
    m_tabs->addTab(m_connectFourBrowser, "");

    m_reversiBrowser = new QTextBrowser(this);
    m_reversiBrowser->setOpenExternalLinks(true);
    m_tabs->addTab(m_reversiBrowser, "");

    m_quoridorBrowser = new QTextBrowser(this);
    m_quoridorBrowser->setOpenExternalLinks(true);
    m_tabs->addTab(m_quoridorBrowser, "");

    m_morrisBrowser = new QTextBrowser(this);
    m_morrisBrowser->setOpenExternalLinks(true);
    m_tabs->addTab(m_morrisBrowser, "");

    m_dotsAndBoxesBrowser = new QTextBrowser(this);
    m_dotsAndBoxesBrowser->setOpenExternalLinks(true);
    m_tabs->addTab(m_dotsAndBoxesBrowser, "");

    m_tabs->setCurrentIndex(initialTab);
    layout->addWidget(m_tabs);

    QPushButton *closeBtn = new QPushButton(this);
    closeBtn->setObjectName("closeRulesBtn");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    updateTexts();

    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &RulesDialog::updateTexts);
}

void RulesDialog::updateTexts() {
    setWindowTitle(trText("rules"));
    m_tabs->setTabText(0, trText("game_checkers"));
    m_tabs->setTabText(1, trText("game_go"));
    m_tabs->setTabText(2, trText("game_connectfour"));
    m_tabs->setTabText(3, trText("game_reversi"));
    m_tabs->setTabText(4, trText("game_quoridor"));
    m_tabs->setTabText(5, trText("game_morris"));
    m_tabs->setTabText(6, trText("game_dotsandboxes"));

    m_checkersBrowser->setHtml(LanguageManager::instance().rulesCheckersHtml());
    m_goBrowser->setHtml(LanguageManager::instance().rulesGoHtml());
    m_connectFourBrowser->setHtml(LanguageManager::instance().rulesConnectFourHtml());
    m_reversiBrowser->setHtml(LanguageManager::instance().rulesReversiHtml());
    m_quoridorBrowser->setHtml(LanguageManager::instance().rulesQuoridorHtml());
    m_morrisBrowser->setHtml(LanguageManager::instance().rulesMorrisHtml());
    m_dotsAndBoxesBrowser->setHtml(LanguageManager::instance().rulesDotsAndBoxesHtml());

    QPushButton *closeBtn = findChild<QPushButton*>("closeRulesBtn");
    if (closeBtn) {
        closeBtn->setText(trText("quit") == "Quitter" ? "Fermer" : (trText("quit") == "Quit" ? "Close" : "إغلاق"));
    }
}

} // namespace Strategix
