#include "AboutDialog.h"
#include "../../localization/LanguageManager.h"

#include <QVBoxLayout>
#include <QPushButton>

namespace Strategix {

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    resize(580, 420);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_browser = new QTextBrowser(this);
    m_browser->setOpenExternalLinks(true);
    layout->addWidget(m_browser);

    QPushButton *closeBtn = new QPushButton(this);
    closeBtn->setObjectName("closeAboutBtn");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    updateTexts();

    connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this, &AboutDialog::updateTexts);
}

void AboutDialog::updateTexts() {
    setWindowTitle(trText("about"));
    m_browser->setHtml(LanguageManager::instance().aboutHtml());

    QPushButton *closeBtn = findChild<QPushButton*>("closeAboutBtn");
    if (closeBtn) {
        closeBtn->setText(trText("quit") == "Quitter" ? "Fermer" : (trText("quit") == "Quit" ? "Close" : "إغلاق"));
    }
}

} // namespace Strategix
