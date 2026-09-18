#ifndef STRATEGIX_RULESDIALOG_H
#define STRATEGIX_RULESDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QTextBrowser>

namespace Strategix {

class RulesDialog : public QDialog {
    Q_OBJECT

public:
    explicit RulesDialog(int initialTab = 0, QWidget *parent = nullptr);

    void updateTexts();

private:
    QTabWidget *m_tabs;
    QTextBrowser *m_checkersBrowser;
    QTextBrowser *m_goBrowser;
    QTextBrowser *m_connectFourBrowser;
    QTextBrowser *m_reversiBrowser;
    QTextBrowser *m_quoridorBrowser;
    QTextBrowser *m_morrisBrowser;
    QTextBrowser *m_dotsAndBoxesBrowser;
};

} // namespace Strategix

#endif // STRATEGIX_RULESDIALOG_H
