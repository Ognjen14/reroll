#include "include/AppSettings.h"
#include <QCoreApplication>
#include <QStringList>

AppSettings::AppSettings(QObject *parent)
    : QObject(parent),
    m_settings("TopicDev", "Reroll"),
    m_darkMode(m_settings.value("darkMode", false).toBool())
{

}

bool AppSettings::darkMode() const
{
    return m_darkMode;
}

void AppSettings::setDarkMode(bool dark)
{
    if(m_darkMode == dark){
        return;
    }
    m_darkMode = dark;
    m_settings.setValue("darkMode",dark);
    emit darkModeChanged();
}
QString AppSettings::themeMode() const
{
    return m_settings.value("themeMode", "light").toString();
}

void AppSettings::setThemeMode(const QString &mode)
{
    if (themeMode() == mode){
        return;
    }
    m_settings.setValue("themeMode",mode);
    emit themeModeChanged();
}


bool AppSettings::themeInitialized() const
{
    return m_settings.contains("darkMode") || m_settings.contains("themeMode");
}


int AppSettings::accentIndex() const
{
    return m_settings.value("accentIndex", 0).toInt();
}

void AppSettings::setAccentIndex(int index)
{
    if (accentIndex() == index)
    {
        return;
    }

    m_settings.setValue("accentIndex", index);
    emit accentIndexChanged();
}

double AppSettings::fontSizeScale() const
{
    return m_settings.value("fontSize",1.0).toDouble();
}

void AppSettings::setFontSizeScale(double scale)
{
    if (qFuzzyCompare(fontSizeScale(), scale))
    {
        return;
    }

    m_settings.setValue("fontSize", scale);
    emit fontSizeScaleChanged();
}

QString AppSettings::appVersion() const
{
    return QCoreApplication::applicationVersion();
}
