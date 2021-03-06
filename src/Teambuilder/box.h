#ifndef BOX_H
#define BOX_H
#include <QtGui>

class PokeTeam;
class AvatarBox;
class Team;
class TeamBuilder;

class TB_PokemonDetail : public QFrame
{
    Q_OBJECT
public:
    TB_PokemonDetail();

    void changePoke(PokeTeam *poke, int num);
    void updatePoke();
private:
    PokeTeam *poke;
    int num;

    QLabel *m_name, *m_nick, *m_num, *m_gender, *m_level, *m_type1, *m_type2, *m_nature, *m_item;
    QLabel *m_moves[4];

    AvatarBox *m_pic;
};

class PokemonBoxButton : public QPushButton
{
    Q_OBJECT
public:
    PokemonBoxButton(int num);

    void setIcon(const QIcon &ic) {
        QPushButton::setIcon(ic);
    }
    void setIcon(const QPixmap &px) {
        this->px = px;
        QPushButton::setIcon(QIcon(px));
    }

    static QPixmap *theicon;
    static QPixmap *theglowedicon;
signals:
    void pokeSwitched(int first, int second);
    void dragStarted(int num);
    void switchWithBox(int box, int boxSlot, int teamSlot);
protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void startDrag();

    QPoint startPos;
    int num;
    QPixmap px;
};

class TB_PokemonButtons : public QFrame
{
    Q_OBJECT
public:
    TB_PokemonButtons();

signals:
    void buttonChecked(int button);
public slots:
    void check(int index);
public:
    PokemonBoxButton *buttons[6];
};

class PokemonBox;

class TB_PokemonItem : public QGraphicsPixmapItem
{
public:
    TB_PokemonItem(PokeTeam *item, PokemonBox *box);
    ~TB_PokemonItem();

    void changePoke(PokeTeam *poke);
    void setBox(PokemonBox * newbox) {
        box = newbox;
    }

    PokeTeam *poke;
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
    void mousePressEvent(QGraphicsSceneMouseEvent *e);
    void startDrag();

    QPointF startPos;
private:
    PokemonBox *box;
};

class PokemonBox : public QGraphicsView
{
    Q_OBJECT
public:
    PokemonBox(int num);

    void addPokemon(const PokeTeam &poke, int slot = -1) throw (QString);

    /* Gets the team held by the current item */
    PokeTeam *getCurrent() throw (QString);
    /* Deletes the current item  */
    void deleteCurrent() throw (QString);

    void changeCurrent(const PokeTeam &poke) throw (QString);
    void changeCurrentSpot(int newspot);

    bool isFull() const;
    bool isEmpty() const;
    int freeSpot() const;
    /* Sets currentPoke as the first spot available */
    void updateCurrentPoke();

    int getNum() const {
        return num;
    }

    int getNumOf(const TB_PokemonItem*) const;

    void save();
    void load();
signals:
    void switchWithTeam(int box,int boxslot,int teamslot);
    void show(PokeTeam *team);
protected:
    void addGraphicsItem(int spot);

    QPointF calculatePos(int spot, const QSize& itemSize = QSize(32,32));
    /*
     * Gets the spot corresponding to that pos.
     * Returns -1 on failure
    */
    int calculateSpot(const QPoint &graphViewPos);

    void drawBackground(QPainter *painter, const QRectF &rect);
    TB_PokemonItem* currentItem();

    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

    static QPixmap *selBg;
private:
    QVector<TB_PokemonItem*> pokemons;
    int num;
    int currentPoke;
};

class TB_BoxContainer : public QTabWidget
{
public:
    TB_BoxContainer();
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
};

class TB_PokemonBoxes : public QWidget
{
    Q_OBJECT
public:
    TB_PokemonBoxes(TeamBuilder *parent);

    void updateBox();
    void updateSpot(int i);
public slots:
    void changeCurrentTeamPokemon(int newpoke);
    void store();
    void withdraw();
    void switchP();
    void deleteP();
    void switchWithinTeam(int poke1, int poke2);
    void switchBoxTeam(int box, int boxslot, int teamslot);
    void showPoke(PokeTeam *poke);
signals:
    void pokeChanged(int poke);
protected:
    PokeTeam *currentPokeTeam();
private:
    TB_PokemonDetail *m_details;
    TB_PokemonButtons *m_buttons;
    Team *m_team;
    int currentPoke;
    TB_BoxContainer *m_boxes;
    PokemonBox * boxes[26];

    PokemonBox *currentBox() {
        return boxes[m_boxes->currentIndex()];
    }
};

#endif // BOX_H
