#pragma once
#include <QObject>
#include <QSettings>
#include <QDate>
#include <QString>
class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY darkModeChanged FINAL)
    Q_PROPERTY(QString themeMode READ themeMode  WRITE setThemeMode  NOTIFY themeModeChanged  FINAL)
    Q_PROPERTY(int accentIndex READ accentIndex WRITE setAccentIndex NOTIFY accentIndexChanged FINAL)
    Q_PROPERTY(double fontSizeScale READ fontSizeScale WRITE setFontSizeScale NOTIFY fontSizeScaleChanged FINAL)


public:
    explicit AppSettings(QObject *parent = nullptr);

    bool    darkMode() const;
    void    setDarkMode(bool dark);

    QString themeMode() const;
    void setThemeMode(const QString &mode);

    int accentIndex() const;
    void setAccentIndex(int index);

    double fontSizeScale() const;
    void setFontSizeScale(double scale);

    ///
    /// \brief themeInitialized
    /// \return Returns true if the user is set theme inside settings(profile) page
    ///
    bool themeInitialized() const;


signals:
    void darkModeChanged();
    void themeModeChanged();

    void accentIndexChanged();

    void fontSizeScaleChanged();

private:
    QSettings m_settings;
    bool    m_darkMode;
};
