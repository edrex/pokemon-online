#include "player.h"
#include "security.h"
#include "challenge.h"
#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/pokemoninfo.h"
#include "tiermachine.h"
#include "waitingobject.h"
#include "server.h"


Player::Player(QTcpSocket *sock, int id) : myrelay(sock, id), myid(id)
{
    lockCount = 0;
    battleSearch() = false;
    myip = relay().ip();
    rating() = -1;
    waiting_team = NULL;

    m_state = NotLoggedIn;
    myauth = 0;

    connect(&relay(), SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&relay(), SIGNAL(loggedIn(TeamInfo,bool,bool,QColor)), SLOT(loggedIn(TeamInfo,bool,bool,QColor)));
    connect(&relay(), SIGNAL(messageReceived(QString)), SLOT(recvMessage(QString)));
    connect(&relay(), SIGNAL(teamReceived(TeamInfo)), SLOT(recvTeam(TeamInfo)));
    connect(&relay(), SIGNAL(challengeStuff(ChallengeInfo)), SLOT(challengeStuff(ChallengeInfo)));
    connect(&relay(), SIGNAL(forfeitBattle(int)), SLOT(battleForfeited(int)));
    connect(&relay(), SIGNAL(battleMessage(int,BattleChoice)), SLOT(battleMessage(int,BattleChoice)));
    connect(&relay(), SIGNAL(battleChat(int,QString)), SLOT(battleChat(int,QString)));
    connect(&relay(), SIGNAL(sentHash(QString)), SLOT(hashReceived(QString)));
    connect(&relay(), SIGNAL(wannaRegister()), SLOT(registerRequest()));
    connect(&relay(), SIGNAL(kick(int)), SLOT(playerKick(int)));
    connect(&relay(), SIGNAL(ban(int)), SLOT(playerBan(int)));
    connect(&relay(), SIGNAL(banRequested(QString)), SLOT(CPBan(QString)));
    connect(&relay(), SIGNAL(tempBanRequested(QString,int)), SLOT(CPTBan(QString,int)));
    connect(&relay(), SIGNAL(unbanRequested(QString)), SLOT(CPUnban(QString)));
    connect(&relay(), SIGNAL(PMsent(int,QString)), SLOT(receivePM(int,QString)));
    connect(&relay(), SIGNAL(getUserInfo(QString)), SLOT(userInfoAsked(QString)));
    connect(&relay(), SIGNAL(banListRequested()), SLOT(giveBanList()));
    connect(&relay(), SIGNAL(awayChange(bool)), SLOT(awayChange(bool)));
    connect(&relay(), SIGNAL(battleSpectateRequested(int)), SLOT(spectatingRequested(int)));
    connect(&relay(), SIGNAL(battleSpectateEnded(int)), SLOT(quitSpectating(int)));
    connect(&relay(), SIGNAL(battleSpectateChat(int,QString)), SLOT(spectatingChat(int,QString)));
    connect(&relay(), SIGNAL(ladderChange(bool)), SLOT(ladderChange(bool)));
    connect(&relay(), SIGNAL(showTeamChange(bool)), SLOT(showTeamChange(bool)));
    connect(&relay(), SIGNAL(tierChanged(QString)), SLOT(changeTier(QString)));
    connect(&relay(), SIGNAL(findBattle(FindBattleData)), SLOT(findBattle(FindBattleData)));
    connect(&relay(), SIGNAL(showRankings(QString,int)), SLOT(getRankingsByPage(QString, int)));
    connect(&relay(), SIGNAL(showRankings(QString,QString)), SLOT(getRankingsByName(QString, QString)));
    /* To avoid threading / simulateneous calls problems, it's queued */
    connect(this, SIGNAL(unlocked()), &relay(), SLOT(undelay()),Qt::QueuedConnection);
}

Player::~Player()
{
    removeWaitingTeam();
}

void Player::ladderChange(bool n)
{
    if (!isLoggedIn())
        return;//INV BEHAV
    ladder() = n;
    emit updated(id());
}

void Player::showTeamChange(bool n)
{
    if (!isLoggedIn())
        return; //INV BEHAV
    showteam() = n;
    emit updated(id());
}

void Player::cancelBattleSearch()
{
    if (!inSearchForBattle())
        return;
    emit battleSearchCancelled(id());
}

void Player::lock()
{
    lockCount += 1;
    relay().delay();
}

void Player::unlock()
{
    lockCount -= 1;
    if (lockCount >= 0)
        emit unlocked();
    else
        lockCount = 0;
}

void Player::changeTier(const QString &newtier)
{
    if (tier() == newtier)
        return;
    if (battling()) {
        sendMessage(tr("You can't change tiers while battling."));
        return;
    }
    if (!TierMachine::obj()->exists(newtier)) {
        sendMessage(tr("The tier %1 doesn't exist!").arg(newtier));
        return;
    }
    if (!TierMachine::obj()->isValid(team(), newtier)) {
        QString pokeList = "";
        for(int i = 0; i < 6; i++) {
            if (TierMachine::obj()->isBanned(team().poke(i),newtier)) {
                pokeList += PokemonInfo::Name(team().poke(i).num()) + ", ";
            }
        }
        if (pokeList.length() >= 2)
            pokeList.resize(pokeList.size()-2);

        sendMessage(tr("The following pokemons are banned in %1, hence you can't choose that tier: %2.").arg(newtier,pokeList));
        return;
    }
    if (Server::serverIns->beforeChangeTier(id(), tier(), newtier)) {
        QString oldtier = tier();
        executeTierChange(newtier);
        Server::serverIns->afterChangeTier(id(), oldtier, tier());
    }
}

void Player::executeTierChange(const QString &newtier)
{
    tier() = newtier;
    findRating();
    cancelChallenges();
}

void Player::doWhenDC()
{
    relay().stopReceiving();
    cancelChallenges();
    cancelBattleSearch();

    foreach(int id, battles) {
        battleForfeited(id);
    }
    foreach(int id, battlesSpectated) {
        quitSpectating(id);
    }
}

void Player::quitSpectating(int battleId)
{
    if (battlesSpectated.contains(battleId)) {
        battlesSpectated.remove(battleId);
        emit spectatingStopped(this->id(), battleId);
    }
}

void Player::spectateBattle(const QString &name0, const QString &name1, int battleId, bool doubles)
{
    battlesSpectated.insert(battleId);
    relay().notify(NetworkServ::SpectateBattle, name0, name1, qint32(battleId), doubles);
}

void Player::cancelChallenges()
{
    foreach(Challenge *c, challengedBy) {
        c->cancel(this);
    }
    while (challenged.size() != 0) {
        (*challenged.begin())->cancel(this);
    }
}

void Player::removeChallenge(Challenge *c)
{
    challengedBy.remove(c);
    challenged.remove(c);
}

void Player::addChallenge(Challenge *c, bool youarechallenged)
{
    if (youarechallenged) {
        challengedBy.insert(c);
    } else {
        challenged.insert(c);
    }
}

bool Player::okForChallenge(int) const
{
    if (!isLoggedIn() || battling() || away() || challengedBy.size() >= 3)
        return false;

    return true;
}

bool Player::okForBattle() const
{
    return isLoggedIn();
}

void Player::awayChange(bool away)
{
    if (away == this->away()) {
        return;
    }

    if (!isLoggedIn() || battling()) {
        return;
    }

    if (Server::serverIns->beforePlayerAway(id(), away)) {
        executeAwayChange(away);
        Server::serverIns->afterPlayerAway(id(), away);
    }
}

void Player::executeAwayChange(bool away)
{
    changeState(Away, away);
    emit awayChange(id(), away);
}

void Player::changeState(int newstate, bool on)
{
    if (on) {
        m_state |= newstate;
    } else {
        m_state &= 0xFF ^ newstate;
    }
}

int Player::auth() const {
    return myauth;
}

void Player::setAuth(int auth)  {
    myauth = auth;
}

void Player::setName(const QString &newname)  {
    team().name = newname;
}

void Player::kick() {
    relay().close();
}

void Player::disconnected()
{
    emit disconnected(id());
}

int Player::firstBattleId()
{
    return *battles.begin();
}

void Player::battleChat(int bid, const QString &s)
{
    if (!hasBattle(bid))
        return; //INVALID BEHAVIOR

    emit battleChat(id(), bid, s);
}

void Player::spectatingChat(int id, const QString &chat)
{
    if (!battlesSpectated.contains(id)) {
        return; //INVALID BEHAVIOR
    }
    emit spectatingChat(this->id(), id, chat);
}

void Player::battleMessage(int bid, const BattleChoice &b)
{
    if (!hasBattle(bid))
        return; //INVALID BEHAVIOR

    emit battleMessage(id(), bid, b);
}

void Player::recvMessage(const QString &mess)
{
    if (!isLoggedIn())
        return; //INVALID BEHAVIOR
    /* for now we just emit the signal, but later we'll do more, such as flood count */
    emit recvMessage(id(), mess);
}

void Player::battleForfeited(int bid)
{
    if (!hasBattle(bid))
        return; //INVALID BEHAVIOR

    emit battleFinished(bid, Forfeit, 0, id());
}

void Player::battleResult(int battleid, int result, int winner, int loser)
{
    relay().sendBattleResult(battleid, result, winner, loser);
}

void Player::addBattle(int battleid)
{
    battles.insert(battleid);
}

void Player::removeBattle(int battleid)
{
    battles.remove(battleid);
}

void Player::getRankingsByName(const QString &tier, const QString &name)
{
    if (!TierMachine::obj()->exists(tier))
        return;
    TierMachine::obj()->fetchRankings(tier, name, this, SLOT(displayRankings()));
}

void Player::getRankingsByPage(const QString &tier, int page)
{
    if (!TierMachine::obj()->exists(tier))
        return;
    /* A page is 40 players */
    TierMachine::obj()->fetchRankings(tier, page, this, SLOT(displayRankings()));
}

void Player::displayRankings()
{
    WaitingObject *src = (WaitingObject*) (sender());

    int page = src->data["rankingpage"].toInt();
    int startingRank = (page-1) * TierMachine::playersByPage + 1;

    int count = TierMachine::obj()->count(src->data.value("tier").toString());
    relay().startRankings(page, startingRank, (count-1) / TierMachine::playersByPage + 1);

    typedef QPair<QString, int> p;
    QVector<p> results = src->data["rankingdata"].value<QVector<p> >();

    /* Removing the properties to clear memory */
    src->data.clear();

    foreach(p pair, results) {
        relay().sendRanking(pair.first, pair.second);
    }
}

void Player::receivePM(int id, const QString &pm)
{
    if (!isLoggedIn()) {
        //INVALID BEHAVIOR
        return;
    }

    QString str = pm.trimmed();

    if (str.length() == 0) {
        //INVALID BEHAVIOR
        return;
    }

    emit PMReceived(this->id(), id, str);
}

void Player::playerBan(int p) {
    if (!isLoggedIn()) {
        emit info(id(), "Tried to ban while not logged in");
        kick();
        return;
    }

    if (auth() < 2) {
        return; //INVALID BEHAVIOR
    }

    emit playerBan(id(),p);
}

void Player::CPBan(const QString &name)
{
    if (auth() < 2) {
        return; //INVALID BEHAVIOR
    }
    int maxAuth = SecurityManager::maxAuth(SecurityManager::ip(name));
    if (maxAuth >= auth()) {
        sendMessage(name + " has authority equal or superior to yours under another nick.");
        return;
    }
    SecurityManager::ban(name);
    emit info(id(), "Banned player " + name + " with CP.");
}

void Player::CPUnban(const QString &name)
{
    if (auth() < 2) {
        return; //INVALID BEHAVIOR
    }
    SecurityManager::unban(name);
    emit info(id(), "Unbanned player " + name + " with CP.");
}

void Player::CPTBan(const QString &name, int time)
{
    if (auth() < 1) {
        return; //INVALID BEHAVIOR
    }
    int maxAuth = SecurityManager::maxAuth(SecurityManager::ip(name));
    if (maxAuth >= auth()) {
        sendMessage(name + " has authority " + maxAuth + " under another nick.");
        return;
    }
    /* Checking the time boundaries */
    time = std::max(1, std::min(time, 1440));
    SecurityManager::ban(name);
    TempBan *tBan = new TempBan(name,time);
    tBan->start();
    connect(tBan,SIGNAL(end(QString)),this,SLOT(tUnban(QString)));
    emit info(id(), "Temporarily Banned player " + name + " with CP for " + int(time) + " minutes.");
}

void Player::playerKick(int p) {
    if (!isLoggedIn()) {
        emit info(id(), "Tried to kick while not logged in");
        kick();
        return;
    }

    if (auth() < 1) {
        return; //INVALID BEHAVIOR
    }

    emit playerKick(id(),p);
}

void Player::challengeStuff(const ChallengeInfo &c)
{
    int id = c.opponent();

    if (id == 0) {
        /* Find Battle Cancel */
        if (inSearchForBattle())
            cancelBattleSearch();
        return;
    }

    if (!isLoggedIn() || id == this->id()) {
        // INVALID BEHAVIOR
        return;
    }

    int desc = c.desc();

    if (desc < ChallengeInfo::Sent || desc  >= ChallengeInfo::ChallengeDescLast) {
        // INVALID BEHAVIOR
        return;
    }

    if (desc == ChallengeInfo::Sent)
    {
        if (team().invalid() && ! (c.clauses & ChallengeInfo::ChallengeCup)) {
            sendMessage("Your team is invalid, you can't challenge except for Challenge Cup!");
            return;
        }
        if (challenged.size() >= 10) {
            sendMessage("You already have challenge 10 people, you can't challenge more!");
            return;
        }
        emit sendChallenge(this->id(), id, c);
    } else {
        if (desc == ChallengeInfo::Accepted && !okForBattle()) {
            return;
        }

        foreach (Challenge *_c, challengedBy) {
            if (_c->challenger() == id) {
                _c->manageStuff(this, c);
                return;
            }
        }
        foreach (Challenge *_c, challenged) {
            if (_c->challenged() == id) {
                _c->manageStuff(this, c);
                return;
            }
        }
    }
}

void Player::findBattle(const FindBattleData& f)
{
    if (!isLoggedIn()) {
        // INVALID BEHAVIOR
        return;
    }

    if (battles.size() >= 3) {
        // INVALID BEHAVIOR
        return;
    }

    if (team().invalid())
    {
        sendMessage("Your team is invalid, you can't find battles!");
        return;
    }

    cancelBattleSearch();

    emit findBattle(id(),f);
}

void Player::sendChallengeStuff(const ChallengeInfo &c)
{
    relay().sendChallengeStuff(c);
}

void Player::startBattle(int battleid, int id, const TeamBattle &team, const BattleConfiguration &conf, bool doubles)
{
    relay().engageBattle(battleid, this->id(), id, team, conf, doubles);

    cancelChallenges();
    cancelBattleSearch();
}

void Player::giveBanList()
{
    if (myauth == 0) {
        return; //INVALID BEHAVIOR
    }
    QHash<QString, QString> bannedMembers = SecurityManager::banList();

    QHashIterator<QString, QString> it(bannedMembers);

    while (it.hasNext()) {
        it.next();
        relay().notify(NetworkServ::GetBanList, it.key(), it.value());
    }
}

TeamBattle & Player::team()
{
    return myteam;
}

const TeamBattle & Player::team() const
{
    return myteam;
}

Analyzer & Player::relay()
{
    return myrelay;
}

const Analyzer & Player::relay() const
{
    return myrelay;
}

bool Player::battling() const
{
    return battles.size() > 0;
}

bool Player::hasBattle(int battleId) const
{
    return battles.contains(battleId);
}

bool Player::away() const
{
    return state() & Away;
}

int Player::state() const
{
    return m_state;
}

bool Player::connected() const
{
    return relay().isConnected();
}

PlayerInfo Player::bundle() const
{
    PlayerInfo p;
    p.auth = myauth;
    p.flags = state() | (Battling && battling());
    p.id = id();
    p.team = basicInfo();
    p.rating = ladder() ? rating() : -1;
    p.tier = tier();
    p.avatar = avatar();
    p.color = color();

    if (showteam()) {
        for(int i = 0; i < 6; i++) {
            p.pokes[i] = team().poke(i).num();
        }
    } else {
        for(int i = 0; i < 6; i++) {
            p.pokes[i] = 0;
        }
    }

    return p;
}

bool Player::isLoggedIn() const
{
    return m_state != NotLoggedIn;
}

int Player::id() const
{
    return myid;
}

BasicInfo Player::basicInfo() const
{
    BasicInfo ret = {team().name, team().info};
    return ret;
}

void Player::loggedIn(const TeamInfo &team,bool ladder, bool showteam, QColor c)
{
    if (isLoggedIn())
        return;

    if (!testNameValidity(team.name)) {
        sendMessage("Invalid name");
        kick();
        return;
    }

    this->ladder() = ladder;
    this->showteam() = showteam;

    assignNewColor(c);
    assignTeam(team);

    testAuthentification(team.name);
}

void Player::loginSuccess()
{
    if (waiting_team) {
        /* Don't get the new name yet, wait till fully logged in (with rating and all) */
        if (isLoggedIn()) {
            waiting_team->name = name();
        }
        assignTeam(*waiting_team);
        removeWaitingTeam();
    }

    ontologin = true;
    findTierAndRating();
}

void Player::testAuthentification(const QString &name)
{
    lock();

    waiting_name = name;
    SecurityManager::loadMemberInMemory(name, this, SLOT(testAuthentificationLoaded()));
}

void Player::testAuthentificationLoaded()
{
    unlock();
    QString name = waiting_name;

    if (SecurityManager::exist(name)) {
        SecurityManager::Member m = SecurityManager::member(name);
        if (m.isBanned()) {
            sendMessage("You are banned!");
            kick();
            return;
        }

        if (m.isProtected()) {
            relay().notify(NetworkServ::AskForPass, QString(m.salt));
            return;
        }

        myauth = m.authority();

        m.modifyIP(relay().ip().toAscii());
        m.modifyDate(QDate::currentDate().toString(Qt::ISODate).toAscii());
        SecurityManager::updateMember(m);

        /* To tell the player he's not registered */
        relay().notify(NetworkServ::Register);
        loginSuccess();
    } else {
        myauth = 0;
        rating() = 1000;

        SecurityManager::create(name, QDate::currentDate().toString(Qt::ISODate), relay().ip());
        /* To tell the player he's not registered */
        relay().notify(NetworkServ::Register);
        loginSuccess();
    }
}

void Player::findTierAndRating()
{
    tier() = TierMachine::obj()->findTier(team());
    findRating();
}

void Player::findRating()
{
    lock();
    TierMachine::obj()->loadMemberInMemory(name(), tier(), this, SLOT(ratingLoaded()));
}

void Player::ratingLoaded()
{
    unlock();
    rating() = TierMachine::obj()->rating(name(), tier());

    if (ontologin) {
        ontologin = false;
        if (waiting_name.length() > 0 && (waiting_name != name() || !isLoggedIn()))
            emit loggedIn(id(), waiting_name);
        else
            emit recvTeam(id(), name());
        waiting_name.clear();
    } else {
        emit updated(id());
    }
}

void Player::assignNewColor(const QColor &c)
{
    if (c.lightness() <= 140 && c.green() <= 180)
        color() = c;
}

void Player::assignTeam(const TeamInfo &team)
{
    avatar() = team.avatar;
    this->team() = team;
    winningMessage() = team.win;
    losingMessage() = team.lose;
}

void Player::changeWaitingTeam(const TeamInfo &t)
{
    delete waiting_team;
    waiting_team = new TeamInfo(t);
}

void Player::removeWaitingTeam()
{
    delete waiting_team;
    waiting_team=NULL;
    return;
}

bool Player::isLocked() const
{
    return lockCount > 0;
}

bool Player::testNameValidity(const QString &name)
{
    if(!SecurityManager::isValid(name)) {
        emit info(id(), "invalid name: \"" + name + "\"");
        sendMessage("Invalid name. Change your name.");
        kick();
        return false;
    }
    return true;
}

void Player::registerRequest() {
    /* If not logged in or in the middle of an authentification, we quit */
    if (!isLoggedIn() || waiting_name.length() > 0)
        return; //INVALID BEHAVIOR
    SecurityManager::Member m = SecurityManager::member(name());

    if (m.isProtected())
        return; //INVALID BEHAVIOR

    for (int i = 0; i < SecurityManager::Member::saltLength; i++) {
        m.salt[i] = uchar((true_rand() % (90-49)) + 49);
    }

    SecurityManager::updateMember(m);
    relay().notify(NetworkServ::AskForPass, QString(m.salt));
}

void Player::userInfoAsked(const QString &name)
{
    if (myauth == 0) {
        return; //INVALID BEHAVIOR
    }

    if (!SecurityManager::exist(name)) {
        relay().sendUserInfo(UserInfo(name, UserInfo::NonExistant));
        return;
    }

    SecurityManager::Member m = SecurityManager::member(name);

    UserInfo ret(name, m.isBanned() ? UserInfo::Banned : 0, m.authority(), m.ip, m.date);
    relay().sendUserInfo(ret);

    if (SecurityManager::maxAuth(m.ip) > auth()) {
        relay().notify(NetworkServ::GetUserAlias, m.name);
        return;
    }

    QList<QString> aliases = SecurityManager::membersForIp(m.ip);

    foreach(QString alias, aliases) {
        relay().notify(NetworkServ::GetUserAlias, alias);
    }
}

void Player::hashReceived(const QString &_hash) {
    QByteArray hash = md5_hash(_hash.toAscii());
    if (waiting_name.length() > 0) {
        if (battling()) {
            sendMessage("You can't change teams while battling.");
            return;
        }
        if (hash == SecurityManager::member(waiting_name).hash) {
            SecurityManager::Member m = SecurityManager::member(waiting_name);

            m.modifyIP(relay().ip().toAscii());
            m.modifyDate(QDate::currentDate().toString(Qt::ISODate).toAscii());
            m.hash = hash;
            myauth = m.authority();
            SecurityManager::updateMember(m);

            loginSuccess();
        } else {
            emit info(id(), tr("authentification failed for %1").arg(waiting_name));
            kick();
            return;
        }
    } else {
        SecurityManager::Member m = SecurityManager::member(name());
        if (m.isProtected()) {
            return; //Invalid behavior
        }

        m.hash = hash;
        SecurityManager::updateMember(m);
        emit info(id(), tr("%1 registered.").arg(name()));
    }
}

QString Player::name() const
{
    return team().name;
}

QString Player::ip() const
{
    return myip;
}

void Player::recvTeam(const TeamInfo &team)
{
    /* If the guy is not logged in, obvious. If he is battling, he could make it so the points lost are on his other team */
    if (!isLoggedIn())
        return;

    QString oldName = name();

    if (team.name != oldName && battling()) {
        sendMessage("You can't change names while battling.");
        return;
    }

    cancelChallenges();
    cancelBattleSearch();

    if (team.name.toLower() == oldName.toLower()) {
        assignTeam(team);

        //Still needs to deal with afterChangeTeam event
        ontologin = true;
        findTierAndRating();
        return;
    }

    changeWaitingTeam(team);
    testAuthentification(team.name);
}

void Player::spectatingRequested(int id)
{
    if (!isLoggedIn()) {
        return; //INVALID BEHAVIOR
    }
    if (battlesSpectated.size() >= 2) {
        sendMessage(tr("You're already watching %1 battles!").arg(battlesSpectated.size()));
        return;
    }
    emit spectatingRequested(this->id(), id);
}

void Player::sendMessage(const QString &mess)
{
    relay().sendMessage(mess);
}

void Player::tUnban(QString name)
{
    SecurityManager::unban(name);
}

TempBan::TempBan(const QString& na,const int& ti) : myname(na), mytime(ti)
{
}
TempBan::~TempBan()
{
}
void TempBan::start()
{
    //    mytimer = new QTimer();
    //    mytimer->start(mytime*60*1000);
    //    connect(mytimer,SIGNAL(timeout()),this,SLOT(done()));
}

QString TempBan::name() const
{
    return myname;
}

int TempBan::time() const
{
    return mytime;
}

void TempBan::done()
{
    emit end(myname);
}
