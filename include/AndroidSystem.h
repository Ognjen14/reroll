#pragma once
#include <QObject>
#include <QString>
#include <QColor>
#include <QLocale>

class AndroidSystem : public QObject
{
    Q_OBJECT
public:
    explicit AndroidSystem(QObject * parent = nullptr);

    ///
    /// \brief setLightStatusBar
    /// \param light
    ///
    Q_INVOKABLE void setLightStatusBar(bool light);
    ///
    /// \brief setBarBackgroud
    /// \param color
    ///
    Q_INVOKABLE void setBarBackgroud(const QColor &color);
    ///
    /// \brief isSystemDarkMode
    /// \return Returns is android phone set to dark theme or not
    ///
    Q_INVOKABLE bool isSystemDarkMode() const;

};
