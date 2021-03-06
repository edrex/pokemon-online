#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include "../PokemonInfo/pokemoninfo.h"

class TB_Menu;
class TeamBuilder;
class Client;
class ServerChoice;

/* The main window!

   All the general initialization is done here, the persistent data (i.e the team) too.
   The main window manages the sub-modules. */

class MainEngine : public QObject {
    Q_OBJECT
public:
    MainEngine();

    void loadTeam(const QString &path);
    template<class T>
    void setDefaultValue(const QString &key, T value);
public slots:
    void launchMenu();
    void launchCredits();
    void launchTeamBuilder();
    void goOnline(const QString &url, const quint16 port);
    void launchServerChoice();
    void changeStyle();
    void changeLanguage();
    void updateMenuBar();
    void quit();

    /* slots called by subwindows when they need it */
    void loadTeamDialog();
    void loadStyleSheet();
private:
    QMainWindow *displayer;

    TrainerTeam m_team;
    TrainerTeam *trainerTeam() {
        return &m_team;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:


    TB_Menu *m_menu;
    TeamBuilder *m_TB;
    Client *m_client;
    ServerChoice *m_choice;
};

template<class T>
void MainEngine::setDefaultValue(const QString &key, T value)
{
    QSettings s;
    if (s.value(key).isNull())
        s.setValue(key, value);
}

#endif // MAINWINDOW_H
