#ifndef STRATEGIX_ABOUTDIALOG_H
#define STRATEGIX_ABOUTDIALOG_H

#include <QDialog>
#include <QTextBrowser>

namespace Strategix {

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

    void updateTexts();

private:
    QTextBrowser *m_browser;
};

} // namespace Strategix

#endif // STRATEGIX_ABOUTDIALOG_H
