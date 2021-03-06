#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <QtGui>
#include "security.h"

class ServerWindow : public QWidget
{
    Q_OBJECT
public:
    ServerWindow(QWidget *parent = 0);
signals:
    void privacyChanged(int priv);
    void nameChanged(const QString &name);
    void descChanged(const QString &desc);
    void announcementChanged(const QString &ann);
    void maxChanged(int num);
    void logSavingChanged(bool logSaving);
private slots:
    void apply();
private:
    QComboBox *serverPrivate;
    QLineEdit *serverName;
    QPlainTextEdit *serverDesc;
    QPlainTextEdit *serverAnnouncement;
    QSpinBox *serverPlayerMax;
    QSpinBox *serverPort;
    QCheckBox *saveLogs;
};

#endif // SERVERCONFIG_H
