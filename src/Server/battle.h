#ifndef BATTLE_H
#define BATTLE_H

#include <QtCore>
#include "../PokemonInfo/battlestructs.h"
#include "../Utilities/mtrand.h"
#include "../Utilities/contextswitch.h"
class Player;

class BattleSituation : public ContextCallee
{
    Q_OBJECT

    PROPERTY(int, turn);
    PROPERTY(int , publicId);
    PROPERTY(bool, rated);
    PROPERTY(QString, tier);
    PROPERTY(quint32, clauses);
    PROPERTY(int, attacker);
    PROPERTY(int, attacked);
    PROPERTY(bool, doubles);
    PROPERTY(int, numberOfSlots);
    PROPERTY(bool, blocked);
public:
    enum {
	AllButPlayer = -2,
	All = -1,
	Player1,
        Player2,
        Slot11,
        Slot21,
        Slot12,
        Slot22
    };
    typedef QVariantHash context;

    BattleSituation(Player &p1, Player &p2, const ChallengeInfo &additionnalData, int id);
    ~BattleSituation();

    const TeamBattle &pubteam(int id);
    /* returns 0 or 1, or -1 if that player is not involved */
    int spot(int id) const;
    /* The other player */
    int opponent(int player) const;
    int partner(int spot) const;
    QList<int> revs(int slot) const;
    QList<int> allRevs(int slot) const; //returns even koed opponents
    /* returns the id corresponding to that spot (spot is 0 or 1) */
    int id(int spot) const;
    /* Return the configuration of the players (1 refer to that player, 0 to that one... */
    BattleConfiguration configuration() const;

    bool acceptSpectator(int id, bool authed=false) const;
    void addSpectator(Player *p);

    bool sleepClause() const {
        return clauses() & ChallengeInfo::SleepClause;
    }
    bool OHKOClause() const {
        return clauses() & ChallengeInfo::OHKOClause;
    }

    void notifyClause(int clause, bool active = true);

    void removeSpectator(int id);

    /*
	Below Player is either 1 or 0, aka the spot of the id.
	Use the functions above to make conversions
    */
    TeamBattle &team(int player);
    const TeamBattle &team(int player) const;
    PokeBattle &poke(int player);
    const PokeBattle &poke(int player) const;
    PokeBattle &poke(int player, int poke);
    const PokeBattle &poke(int player, int poke) const;
    int currentPoke(int player) const;
    bool koed(int player) const;
    int player(int slot) const;
    /* Returns -1 if none */
    int randomOpponent(int slot) const;
    /* Returns a koed one if none */
    int randomValidOpponent(int slot) const;
    int slot(int player, int poke = 0) const;
    void changeCurrentPoke(int player, int poke);
    int countAlive(int player) const;
    int countBackUp(int player) const;

    /* Starts the battle -- use the time before to connect signals / slots */
    void start(ContextSwitcher &ctx);
    /* The battle runs in a different thread -- easier to interrutpt the battle & co */
    void run();
    /* requests choice of action from the player */
    bool requestChoice(int player, bool acq = true /*private arg */, bool custom = false); /* return true if the pokemon has a choice to make (including switching & struggle)*/
    void requestChoices(); /* request from both players */
    /* Shows what attacks are allowed or not */
    BattleChoices createChoice(int player);
    bool isMovePossible(int player, int slot);
    /* called just after requestChoice(s) */
    void analyzeChoice(int player);
    void analyzeChoices(); 
    std::vector<int> sortedBySpeed();

    /* Commands for the battle situation */
    void beginTurn();
    void endTurn();
    void endTurnStatus(int player);
    void endTurnWeather();
    void callForth(int weather, int turns);
    /* Attack... */
    /* if special occurence = true, then it means a move like mimic/copycat/metronome has been used. In that case attack does not
	represent the moveslot but rather than that it represents the move num, plus PP will not be lost */
    void useAttack(int player, int attack, bool specialOccurence = false, bool notify = true);
    /* Returns true or false if an attack is going on or not */
    bool attacking();
    /* Does not do extra operations,just a setter */
    void changeHp(int player, int newHp);
    /* Sends a poke back to his pokeball (not koed) */
    void sendBack(int player, bool silent = false);
    void notifyHits(int number);
    void sendPoke(int player, int poke, bool silent = false);
    void callEntryEffects(int player);
    void koPoke(int player, int source, bool straightattack = false);
    /* Does not do extra operations,just a setter */
    void changeStatMod(int player, int stat, int newstatmod);
    void changeForme(int player, int poke, int newform);
    void calculateTypeModStab();
    int forme(int player);
    void changeAForme(int player, int newforme);
    void gainStatMod(int player, int stat, int bonus, bool tell = true);
    void loseStatMod(int player, int stat, int malus, int attacker);
    bool canSendPreventMessage(int defender, int attacker);
    void preventStatMod(int player, int attacker);
    /* Does not do extra operations,just a setter */
    void changeStatus(int player, int status, bool tell = true);
    void changeStatus(int team, int poke, int status);
    void healStatus(int player, int status);
    void healConfused(int player);
    void healLife(int player, int healing);
    bool canGetStatus(int player, int status);
    void inflictStatus(int player, int Status, int inflicter);
    bool isConfused(int player);
    void inflictConfused(int player, bool tell=true);
    void inflictConfusedDamage(int player);
    void inflictRecoil(int source, int target);
    void inflictDamage(int player, int damage, int source, bool straightattack = false, bool goForSub = false);
    void inflictPercentDamage(int player, int percent, int source, bool straightattack = false);
    void inflictSubDamage(int player, int damage, int source);
    void disposeItem(int player);
    void eatBerry(int player, bool show=true);
    void acqItem(int player, int item);
    void loseItem(int player);
    /* Removes PP.. */
    void changePP(int player, int move, int PP);
    void losePP(int player, int move, int loss);
    void gainPP(int player, int move, int gain);
    //Uproarer
    bool isThereUproar();
    void addUproarer(int player);
    void removeUproarer(int player);

    int calculateDamage(int player, int target);
    void applyMoveStatMods(int player, int target);
    bool testAccuracy(int player, int target, bool silent = false);
    void testCritical(int player, int target);
    void testFlinch(int player, int target);
    bool testStatus(int player);
    bool testFail(int player);
    void failSilently(int player);
    void fail(int player, int move, int part=0, int type=0, int trueSource = -1);
    bool hasType(int player, int type);
    bool hasWorkingAbility(int play, int ability);
    void acquireAbility(int play, int ability);
    int ability(int player);
    int pokenum(int player);
    bool hasWorkingItem(int player, int item);
    bool isWeatherWorking(int weather);
    bool isSeductionPossible(int seductor, int naiveone);
    int move(int player, int slot);
    bool hasMove(int player, int move);
    int weather();
    int getType(int player, int slot);
    bool isFlying(int player);
    bool isOut(int player, int poke);
    bool hasSubstitute(int player);
    void requestSwitchIns();
    void requestSwitch(int player);
    bool linked(int linked, QString relationShip);
    void link(int linker, int linked, QString relationShip);
    void notifySub(int player, bool sub);
    int repeatNum(int player, context &move);
    PokeFraction getStatBoost(int player, int stat);
    int getStat(int player, int stat);
    /* conversion for sending a message */
    quint8 ypoke(int, int i) const { return i; } /* aka 'your poke', or what you need to know if it's your poke */
    ShallowBattlePoke opoke(int play, int i) const { return ShallowBattlePoke(poke(play, i));} /* aka 'opp poke', or what you need to know if it's your opponent's poke */
    BattleDynamicInfo constructInfo(int player);
    void notifyInfos();
    BattleStats constructStats(int player);

    void changeTempMove(int player, int slot, int move);
    void changeSprite(int player, int poke);
    /* Send a message to the outworld */
    enum BattleCommand
    {
	SendOut,
	SendBack,
	UseAttack,
	OfferChoice,
	BeginTurn,
	ChangePP,
	ChangeHp,
	Ko,
	Effective, /* to tell how a move is effective */
	Miss,
	CriticalHit,
	Hit, /* for moves like fury double kick etc. */
	StatChange,
	StatusChange,
	StatusMessage,
	Failed,
	BattleChat,
	MoveMessage,
	ItemMessage,
	NoOpponent,
	Flinch,
	Recoil,
	WeatherMessage,
        StraightDamage,
        AbilityMessage,
        AbsStatusChange,
        Substitute,
        BattleEnd,
        BlankMessage,
        CancelMove,
        Clause,
        DynamicInfo,
        DynamicStats,
        Spectating,
        SpectatorChat,
        AlreadyStatusMessage,
        ChangeTempPoke,
        ClockStart,
        ClockStop,
        Rated,
        TierSection,
        EndMessage,
        PointEstimate,
        StartChoices,
        Avoid
    };

    enum ChangeTempPoke {
        TempMove,
        TempAbility,
        TempItem,
        TempSprite,
        DefiniteForm,
        AestheticForme
    };

    enum WeatherM
    {
	ContinueWeather,
	EndWeather,
	HurtWeather
    };

    enum Weather
    {
	NormalWeather = 0,
	Hail = 1,
	Rain = 2,
	SandStorm = 3,
	Sunny = 4
    };

    enum StatusFeeling
    {
	FeelConfusion,
	HurtConfusion,
	FreeConfusion,
	PrevParalysed,
	PrevFrozen,
	FreeFrozen,
	FeelAsleep,
	FreeAsleep,
	HurtBurn,
	HurtPoison
    };

    void sendMoveMessage(int move, int part=0, int src=0, int type=0, int foe=-1, int other=-1, const QString &q="");
    void sendAbMessage(int move, int part=0, int src=0, int foe=-1, int type=0, int other=-1);
    void sendItemMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1);
    void sendBerryMessage(int item, int src, int part = 0, int foe = -1, int berry = -1, int num=-1);

    void notifyFail(int p);
    /* Here C++0x would make it so much better looking with variadic templates! */
    void notify(int player, int command, int who);
    template<class T>
    void notify(int player, int command, int who, const T& param);
    template<class T1, class T2>
    void notify(int player, int command, int who, const T1& param1, const T2& param2);
    template<class T1, class T2, class T3>
    void notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3);
    template<class T1, class T2, class T3, class T4>
    void notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4);
    template<class T1, class T2, class T3, class T4, class T5>
    void notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4, const T5 &param5);
    template<class T1, class T2, class T3, class T4, class T5, class T6>
    void notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4, const T5 &param5, const T6 &param6);
public slots:
    void battleChoiceReceived(int id, const BattleChoice &b);
    void battleChat(int id, const QString &str);
public:
    void spectatingChat(int id, const QString &str);
private:
    bool canCancel(int player);
    void cancel(int player);

    bool validChoice(const BattleChoice &b);
    void storeChoice(const BattleChoice &b);
    bool allChoicesOkForPlayer(int player);
    bool allChoicesSet();
signals:
    /* Due to threading issue, and the signal not being direct,
       The battle might already be deleted when the signal is received.

       So the parameter "publicId" is for the server to not to have to use
       sender(); */
    void battleInfo(int publicId, int id, const QByteArray &info);
    void battleFinished(int result, int winner, int loser);
private:
    mutable QMutex spectatorMutex;

    /* if battle ends, stop the battle thread */
    void testWin();
    int spectatorKey(int id) const {
        return 10000 + id;
    }

    /* What choice we allow the players to have */
    QList<BattleChoices> options;
    /* Is set to false once a player choses it move */
    QList<int> hasChoice;
    /* just indicates if the player could originally move or not */
    QList<bool> couldMove;
    QList<QPointer<Player> > pendingSpectators;

    TeamBattle team1, team2;

    QList<int> mycurrentpoke; /* -1 for koed */
    /* timers */
    QAtomicInt timeleft[2];
    QAtomicInt startedAt[2];
    bool timeStopped[2];
    QBasicTimer *timer;
    /*.*/
    int myid[2];
    QString winMessage[2];
    QString loseMessage[2];
protected:
    void timerEvent(QTimerEvent *);

    void startClock(int player, bool broadCoast = true);
    void stopClock(int player, bool broadCoast = false);
    int timeLeft(int player);

    void yield();
    void schedule();
public:
    std::vector<int> targetList;
    /* Calls the effects of source reacting to name */
    void calleffects(int source, int target, const QString &name);
    /* This time the pokelong effects */
    void callpeffects(int source, int target, const QString &name);
    /* this time the general battle effects (imprison, ..) */
    void callbeffects(int source, int target, const QString &name);
    /* The team zone effects */
    void callzeffects(int source, int target, const QString &name);
    /* The slot effects */
    void callseffects(int source, int target, const QString &name);
    /* item effects */
    void callieffects(int source, int target, const QString &name);
    /* Ability effects */
    void callaeffects(int source, int target, const QString &name);

    void emitCommand(int player, int players, const QByteArray &data);
public:
    /**************************************/
    /*** VIVs: very important variables ***/
    /**************************************/
    /* Those variable are used throughout the battle to describe the situation.
       These are 'dynamic', in contrary to 'static'. For exemple when a pokemon
       is switched in, its type and ability and moves are stored in there, taken
       from their 'static' value that are in the TeamBattle struct. Then they
       can be changed (like with ability swap) but when the poke is sent back then
       back in the dynamic value is restored to the static one. */

    /* Variables that are reset when the poke is switched out.
	Like for exemple a Requiem one... */
    QList<context> pokelong;
    /* Variables that are reset every turn right before everything else happens
	at the very beginning of a turn */
    QList<context> turnlong;
    /* General things like last move ever used, etc. */
    context battlelong;
    /* Moves that affect a team */
    context teamzone[2];
    /* Moves that affect a particular Slot (wish, ...) */
    QList<context> slotzone;

    /* The choice of a player, accessed by move ENCORE */
    QList<BattleChoice> choice;

    /* Sleep clause necessity: only pokes asleep because of something else than rest are put there */
    // Public because used by Yawn
    int currentForcedSleepPoke[2];

    /* Generator of random numbers */
    mutable MTRand_int32 true_rand2;
    unsigned true_rand() const {
        return unsigned(true_rand2());
    }
private:
    QHash<int,int> spectators;
public:
    QHash<int, int> getSpectators() {
        QMutexLocker m(&spectatorMutex);
        return spectators;
    }

    struct QuitException {};
};

inline void BattleSituation::notify(int player, int command, int who)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who);

    emitCommand(who, player, tosend);
}

template<class T>
void BattleSituation::notify(int player, int command, int who, const T& param)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who) << param;

    emitCommand(who, player, tosend);
}

template<class T1, class T2>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who) << param1 << param2;

    emitCommand(who, player, tosend);
}

template<class T1, class T2, class T3>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who) << param1 << param2 << param3;

    emitCommand(who, player, tosend);
}

template<class T1, class T2, class T3, class T4>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who) << param1 << param2 << param3 << param4;

    emitCommand(who, player, tosend);
}

template<class T1, class T2, class T3, class T4, class T5>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4, const T5 &param5)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who) << param1 << param2 << param3 << param4 << param5;

    emitCommand(who, player, tosend);
}

template<class T1, class T2, class T3, class T4, class T5, class T6>
void BattleSituation::notify(int player, int command, int who, const T1& param1, const T2& param2, const T3 &param3, const T4 &param4, const T5 &param5, const T6 &param6)
{
    QByteArray tosend;
    QDataStream out(&tosend, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << uchar(command) << qint8(who) << param1 << param2 << param3 << param4 << param5 << param6;

    emitCommand(who, player, tosend);
}

Q_DECLARE_METATYPE(QSharedPointer<QSet<int> >)

#endif // BATTLE_H
