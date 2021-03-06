#include "challenge.h"
#include "client.h"
#include "../Utilities/otherwidgets.h"
#include "../PokemonInfo/battlestructs.h"
#include "../PokemonInfo/pokemoninfo.h"

BaseChallengeWindow::BaseChallengeWindow(const PlayerInfo &p, const QString &windowTitle, const QString &buttonOk, const QString &buttonNo, QWidget *parent)
        : emitOnClose(true)
{
    setPixmap(QPixmap("db/Challenge Window/ChallengeBG.png"));
    setFixedSize(pixmap()->size());

    setParent(parent);

    setWindowTitle(windowTitle.arg(p.team.name));

    setAttribute(Qt::WA_DeleteOnClose, true);

    QColor grey = "#414141";

    QLabel *name = new QLabel(toColor(p.team.name, grey),this);
    name->setGeometry(54,0,290,52);
    name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    name->setObjectName("Title");

    QLabel *trainerPic = new QLabel(this);
    trainerPic->move(13,85);
    QPixmap px (QString("db/Trainer Sprites/%1.png").arg(p.avatar));
    if (px.isNull())
        px = QString("db/Trainer Sprites/%1.png").arg(167);
    trainerPic->setPixmap(px);

    bool hidden = p.pokes[0]==0;

    if (hidden) {
        QLabel *hiddenTeam = new QLabel(this);
        hiddenTeam->move(163,82);
        hiddenTeam->setPixmap(QPixmap("db/Challenge Window/HiddenInnerBall.png"));
    } else {
        for (int i = 0; i < 6; i++) {
            QLabel *icon = new QLabel(this);
            icon->move(168+i*51,84);
            icon->setPixmap(PokemonInfo::Icon(p.pokes[i]));
        }
    }

    QFont treb("Trebuchet MS", 10);

    QLabel *pinfo = new QLabel(toColor(p.team.info, grey), this);
    pinfo->setGeometry(18,197,280,55);
    pinfo->setWordWrap(true);
    pinfo->setFont(treb);
    pinfo->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    battleMode = new QComboBox(this);
    battleMode->move(18,270);
    battleMode->addItem(tr("Singles"));
    battleMode->addItem(tr("Doubles"));

    QLabel *ladder = new QLabel(toColor(p.rating == -1 ? "unknown" : QString::number(p.rating), grey),this);
    ladder->setFont(treb);
    ladder->setGeometry(100,148,83,18);
    ladder->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    QLabel *tier = new QLabel(toBoldColor(p.tier, Qt::white),this);
    tier->setFont(QFont("Trebuchet MS", 10, QFont::Bold));
    tier->setGeometry(203,148,94,18);
    ladder->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

    QWidget *container = new QWidget(this);
    container->setGeometry(QRect(322,157,136,136));
    QVBoxLayout *clausesL= new QVBoxLayout(container);
    clausesL->setSpacing(0);

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i] = new QCheckBox(ChallengeInfo::clause(i));
        clauses[i]->setFont(treb);
        clauses[i]->setStyleSheet("color: #414141");
        clauses[i]->setToolTip(ChallengeInfo::description(i));
        clausesL->addWidget(clauses[i]);
    }

    //so when the challenge window pops up and the guy is talking and press the space bar
    //he doesn't activate the decline button
    QWidget *w = new QDummyGrabber();
    w->setParent(this);

    QImageButton *goback;
    goback = new QImageButton("db/Challenge Window/Buttons/" + buttonNo + "ButtonNormal.png", "db/Challenge Window/Buttons/" + buttonNo + "ButtonGlow.png");
    goback->setParent(this);
    goback->move(182,330);

    challenge_b = new QImageButton("db/Challenge Window/Buttons/" + buttonOk + "ButtonNormal.png", "db/Challenge Window/Buttons/" + buttonOk + "ButtonGlow.png");
    challenge_b->setParent(this);
    challenge_b->move(25,330);

    connect(goback, SIGNAL(clicked()), SLOT(onCancel()));

    myid = p.id;

    show();
}

void BaseChallengeWindow::closeEvent(QCloseEvent *)
{
    if (emitOnClose)
        onCancel();
}

void BaseChallengeWindow::forcedClose()
{
    emitOnClose = false;
    close();
}

int BaseChallengeWindow::id()
{
    return myid;
}


void BaseChallengeWindow::onChallenge()
{
    emit challenge(id());
    myid = -1;
    close();
}

void BaseChallengeWindow::onCancel()
{
    if (id() != -1) {
	emit cancel(id());
    }
    close();
}

ChallengeWindow::ChallengeWindow(const PlayerInfo &p, QWidget *parent)
        : BaseChallengeWindow(p, tr("%1's Info"), "Chall", "GoBack", parent)
{
    QSettings s;

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        clauses[i]->setChecked(s.value("clause_"+ChallengeInfo::clause(i)).toBool());
    }

    battleMode->setCurrentIndex(s.value("challenge_with_doubles").toInt());

    //This is necessary to do that here because this is the function of the derived class that is connected then
    connect(challenge_b, SIGNAL(clicked()), SLOT(onChallenge()));
}

void ChallengeWindow::onChallenge()
{
    QSettings s;

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        s.setValue("clause_"+ChallengeInfo::clause(i), clauses[i]->isChecked());
    }

    s.setValue("challenge_with_doubles", battleMode->currentIndex());

    emit challenge(id());
    close();
}

void ChallengeWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        event->accept();
        close();
    }
    else
    {
        BaseChallengeWindow::keyPressEvent(event);
    }
}

ChallengedWindow::ChallengedWindow(const PlayerInfo &p, const ChallengeInfo &c, QWidget *parent)
        : BaseChallengeWindow(p, tr("%1 challenged you!"), "Accept", "Decline", parent)
{
    quint32 clauses = c.clauses;

    connect(challenge_b, SIGNAL(clicked()), SLOT(onChallenge()));

    for (int i = 0; i < ChallengeInfo::numberOfClauses; i++) {
        this->clauses[i]->setChecked(clauses & (1 << i));
        this->clauses[i]->setDisabled(true);
    }

    battleMode->setCurrentIndex(c.mode);
    battleMode->setDisabled(true);
}
