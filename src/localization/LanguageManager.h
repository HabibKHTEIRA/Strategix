#ifndef STRATEGIX_LANGUAGEMANAGER_H
#define STRATEGIX_LANGUAGEMANAGER_H

#include <QObject>
#include <QString>
#include <unordered_map>
#include <string>

namespace Strategix {

enum class Language {
    French,
    English,
    Arabic
};

class LanguageManager : public QObject {
    Q_OBJECT

public:
    static LanguageManager& instance();

    Language currentLanguage() const { return m_currentLanguage; }
    void setLanguage(Language lang);

    bool isRTL() const { return m_currentLanguage == Language::Arabic; }

    QString get(const QString& key) const;

    // Direct helper functions
    QString gameNameCheckers() const;
    QString gameNameGo() const;
    QString gameNameConnectFour() const;
    QString gameNameReversi() const;
    QString gameNameQuoridor() const;
    QString gameNameMorris() const;
    QString gameNameDotsAndBoxes() const;

    QString rulesCheckersHtml() const;
    QString rulesGoHtml() const;
    QString rulesConnectFourHtml() const;
    QString rulesReversiHtml() const;
    QString rulesQuoridorHtml() const;
    QString rulesMorrisHtml() const;
    QString rulesDotsAndBoxesHtml() const;
    QString aboutHtml() const;

signals:
    void languageChanged(Language lang);

private:
    LanguageManager();
    void initTranslations();

    Language m_currentLanguage = Language::French;
    std::unordered_map<std::string, std::unordered_map<int, QString>> m_dict;
};

// Global inline shorthand
inline QString trText(const QString& key) {
    return LanguageManager::instance().get(key);
}

} // namespace Strategix

#endif // STRATEGIX_LANGUAGEMANAGER_H
