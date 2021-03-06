#ifndef TIERMACHINE_H
#define TIERMACHINE_H

#include <QtGui>
#include "../Utilities/functions.h"

class Tier;
struct TeamBattle;
struct PokeBattle;
class WaitingObject;
class MemberRating;
class QSqlQuery;
class LoadThread;
template<class T> class InsertThread;

class TierMachine : public QObject
{
    Q_OBJECT

    friend class Tier;
    PROPERTY(QString, tierList);
public:
    enum QueryType {
        GetInfoOnUser
    };

    static void init();
    static TierMachine *obj();

    TierMachine();

    void clear();
    void save();

    QString toString()const;
    void fromString(const QString &s);

    const QStringList& tierNames() const;
    Tier& tier(const QString &name);
    const Tier& tier(const QString &name) const;
    bool exists(const QString &name) const;
    bool existsPlayer(const QString &name, const QString &player);
    bool isValid(const TeamBattle &t, const QString tier) const;
    bool isBanned(const PokeBattle &p, const QString &tier) const;

    void loadMemberInMemory(const QString &name, const QString &tier, QObject *o, const char *slot);
    void fetchRankings(const QString &tier, const QVariant &data, QObject *o, const char *slot);

    int rating(const QString &name, const QString &tier);
    int ranking(const QString &name, const QString &tier);
    int count (const QString &tier);
    void changeRating(const QString &winner, const QString &loser, const QString &tier);
    void changeRating(const QString &winner, const QString &tier, int newRating);

    QPair<int, int> pointChangeEstimate(const QString &player, const QString &foe, const QString &tier);
    QString findTier(const TeamBattle &t) const;

    void exportDatabase() const;

    static const int playersByPage = 40;

public slots:
    void processQuery(QSqlQuery*,const QVariant &,int,WaitingObject*);
    void insertMember(QSqlQuery*,void *,int);
private:
    QList<Tier*> m_tiers;
    QStringList m_tierNames;
    static TierMachine *inst;

    static const int loadThreadCount=2;
    int nextLoadThreadNumber;
    LoadThread *threads;
    InsertThread<MemberRating> *ithread;

    LoadThread * getThread();
};


class TierWindow : public QWidget
{
    Q_OBJECT
public:
    TierWindow(QWidget *parent = NULL);
signals:
    void tiersChanged();
private slots:
    void done();
private:
    QPlainTextEdit *m_editWindow;
};

/* For rankings */
typedef QVector<QPair<QString, int> > qvectorqpairqstringint;
Q_DECLARE_METATYPE(qvectorqpairqstringint)

#endif // TIERMACHINE_H
