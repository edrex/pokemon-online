#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QtCore>
#include <QTextEdit>

#include <QtScript>

#include <QScriptValueIterator>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class Server;
class ChallengeInfo;

class ScriptEngine : public QObject
{
    Q_OBJECT
public:
    ScriptEngine(Server *s);

    /* Events */
    bool beforeChatMessage(int src, const QString &message);
    void afterChatMessage(int src, const QString &message);
    bool beforeNewMessage(const QString &message);
    void afterNewMessage(const QString &message);
    void serverStartUp();
    void serverShutDown();
    void beforeLogOut(int src);
    void afterLogOut(int src);
    bool beforeLogIn(int src);
    void afterLogIn(int src);
    void beforeChangeTeam(int src);
    void afterChangeTeam(int src);
    bool beforeChangeTier(int src, const QString& oldTier, const QString &newTier);
    void afterChangeTier(int src, const QString& oldTier, const QString &newTier);
    bool beforeChallengeIssued(int src, int dest, const ChallengeInfo &desc);
    void afterChallengeIssued(int src, int dest, const ChallengeInfo &desc);
    bool beforeBattleMatchup(int src, int dest, const ChallengeInfo &desc);
    void afterBattleMatchup(int src, int dest, const ChallengeInfo &desc);
    void beforeBattleStarted(int src, int dest, const ChallengeInfo &desc);
    void afterBattleStarted(int winner, int loser, const ChallengeInfo &desc);
    void beforeBattleEnded(int winner, int loser, int desc);
    void afterBattleEnded(int winner, int loser, int desc);
    bool beforePlayerAway(int src, bool away);
    void afterPlayerAway(int src, bool away);
    bool beforePlayerKick(int src, int dest);
    void afterPlayerKick(int src, int dest);
    bool beforePlayerBan(int src, int dest);
    void afterPlayerBan(int src, int dest);

    /* Functions called in scripts */
    Q_INVOKABLE void sendAll(const QString &mess);
    Q_INVOKABLE void kick(int id);
    /* Prevents the event from happening.
       For exemple, if called in 'beforeChatMessage', the message won't appear.
       If called in 'beforeChallengeIssued', the challenge won't be issued.
       */
    Q_INVOKABLE void stopEvent();
    Q_INVOKABLE void shutDown();
    Q_INVOKABLE void sendMessage(int id, const QString &mess);
    /* Print on the server. Useful for debug purposes */
    Q_INVOKABLE void print(QScriptContext *context, QScriptEngine *engine);
    Q_INVOKABLE void clearPass(const QString &name);
    Q_INVOKABLE void changeAuth(int id, int auth);
    Q_INVOKABLE void changeAway(int id, bool away);
    Q_INVOKABLE void changeRating(const QString& name, const QString& tier, int newRating);
    Q_INVOKABLE void changePokeLevel(int id, int slot, int level);
    Q_INVOKABLE void changePokeNum(int id, int slot, int num);
    Q_INVOKABLE void changePokeItem(int id, int slot, int item);
    Q_INVOKABLE void changePokeMove(int id, int pokeslot, int moveslot, int move);
    Q_INVOKABLE void changeTier(int id, const QString &tier);
    /* Save vals using the QSettings (persistent vals, that stay after the shutdown of the server */
    Q_INVOKABLE void saveVal(const QString &key, const QVariant &val);
    Q_INVOKABLE void saveVal(const QString &file, const QString &key, const QVariant &val);
    Q_INVOKABLE void removeVal(const QString &key);
    Q_INVOKABLE void removeVal(const QString &file, const QString &key);
    /* Calls the underlying OS for a command */
    Q_INVOKABLE int system(const QString &command);
    /* Export the SQL databases to old style txt files */
    Q_INVOKABLE void exportMemberDatabase();
    Q_INVOKABLE void exportTierDatabase();
    Q_INVOKABLE void clearChat();
    Q_INVOKABLE void appendToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void writeToFile(const QString &fileName, const QString &content);
    Q_INVOKABLE void callLater(const QString &s, int delay);
    /* Evaluates the script given in parameter */
    Q_INVOKABLE QScriptValue eval(const QString &script);

    /* GET call */
    Q_INVOKABLE void webCall(const QString &urlstring, const QString &expr);
    /* POST call */
    Q_INVOKABLE void webCall(const QString &urlstring, const QString &expr, const QScriptValue &params_array);
    /* synchronous GET call */
    Q_INVOKABLE QScriptValue synchronousWebCall(const QString &urlstring);
    /* synchronous POST call */
    Q_INVOKABLE QScriptValue synchronousWebCall(const QString &urlstring, const QScriptValue &params_array);

    Q_INVOKABLE QScriptValue name(int id);
    Q_INVOKABLE QScriptValue id(const QString& name);
    Q_INVOKABLE QScriptValue auth(int id);
    Q_INVOKABLE QScriptValue battling(int id);
    Q_INVOKABLE QScriptValue away(int id);
    Q_INVOKABLE QScriptValue ip(int id); 
    Q_INVOKABLE QScriptValue dbAuth(const QString &name);
    Q_INVOKABLE QScriptValue dbIp(const QString &name);
    Q_INVOKABLE QScriptValue dbLastOn(const QString &name);
    Q_INVOKABLE bool dbRegistered(const QString &name);
    Q_INVOKABLE QScriptValue tier(int id);
    Q_INVOKABLE QScriptValue ranking(int id);
    Q_INVOKABLE QScriptValue ranking(const QString &name, const QString &tier);
    Q_INVOKABLE int maxAuth(const QString &ip);
    Q_INVOKABLE QScriptValue aliases(const QString &ip);
    Q_INVOKABLE QScriptValue totalPlayersByTier(const QString &tier);
    Q_INVOKABLE QScriptValue ladderEnabled(int id);
    Q_INVOKABLE QScriptValue ladderRating(int id);
    Q_INVOKABLE QScriptValue getVal(const QString &key);
    Q_INVOKABLE QScriptValue getVal(const QString &file, const QString &key);
    /* returns a state of the memory, useful to check for memory leaks and memory usage */
    Q_INVOKABLE QScriptValue memoryDump();
    Q_INVOKABLE bool hasLegalTeamForTier(int id, const QString &tier);

    Q_INVOKABLE QScriptValue pokemon(int num);
    Q_INVOKABLE QScriptValue pokeNum(const QString &name);
    Q_INVOKABLE QScriptValue move(int num);
    Q_INVOKABLE QScriptValue moveNum(const QString &name);
    Q_INVOKABLE QScriptValue item(int num);
    Q_INVOKABLE QScriptValue itemNum(const QString &item);
    Q_INVOKABLE QScriptValue nature(int num);
    Q_INVOKABLE QScriptValue natureNum(const QString &nature);
    Q_INVOKABLE QScriptValue ability(int num);
    Q_INVOKABLE QScriptValue abilityNum(const QString &nature);

    Q_INVOKABLE QScriptValue teamPokeLevel(int id, int slot);
    Q_INVOKABLE QScriptValue teamPoke(int id, int index);
    Q_INVOKABLE bool hasTeamPoke(int id, int pokemonnum);
    Q_INVOKABLE QScriptValue indexOfTeamPoke(int id, int pokenum);

    Q_INVOKABLE QScriptValue teamPokeMove(int id, int pokeindex, int moveindex);
    Q_INVOKABLE bool hasTeamPokeMove(int id, int pokeindex, int movenum);
    Q_INVOKABLE QScriptValue indexOfTeamPokeMove(int id, int pokeindex, int movenum);
    Q_INVOKABLE bool hasTeamMove(int id, int movenum);

    Q_INVOKABLE QScriptValue teamPokeItem(int id, int pokeindex);
    Q_INVOKABLE bool hasTeamItem(int id, int itemNum);

    Q_INVOKABLE QScriptValue teamPokeNature(int id, int slot);
    Q_INVOKABLE QScriptValue teamPokeEV(int id, int slot, int stat);
    Q_INVOKABLE QScriptValue teamPokeDV(int id, int slot, int stat);

    Q_INVOKABLE int numPlayers();
    Q_INVOKABLE bool loggedIn(int id);

    Q_INVOKABLE QScriptValue getFileContent(const QString &path);
    Q_INVOKABLE int rand(int min, int max);
    Q_INVOKABLE long time();
    Q_INVOKABLE QScriptValue getTierList();

    /* Set and unset player arrays:
       Those are arrays that, once a player logs off, have their content cleared
       for that offset. Useful as "session variables". */
    Q_INVOKABLE void setPA(const QString &name);
    Q_INVOKABLE void unsetPA(const QString &name);

    static QScriptValue nativePrint(QScriptContext *context, QScriptEngine *engine);

public slots:
    void changeScript(const QString &script);

private slots:
    void timer();
    void webCall_replyFinished(QNetworkReply* reply);
    void synchronousWebCall_replyFinished(QNetworkReply* reply);
    
private:
    Server *myserver;
    QScriptEngine myengine;
    QScriptValue myscript;
    QVector<bool> stopevents;

    QHash<QTimer*,QString> timerEvents;
    QHash<QNetworkReply*,QString> webCallEvents;

    void startStopEvent() {stopevents.push_back(false);}
    bool endStopEvent() {
        bool res = stopevents.back();
        stopevents.pop_back();
        return res;
    }

    QSet<QString> playerArrays;

    QEventLoop sync_loop;
    QString sync_data;

    void evaluate(const QScriptValue &expr);
    void printLine(const QString &s);
};

class ScriptWindow : public QWidget
{
    Q_OBJECT
public:
    ScriptWindow();
signals:
    void scriptChanged(const QString &script);
public slots:
    void okPressed();
private:
    QTextEdit *myedit;
};

#endif // SCRIPTENGINE_H
