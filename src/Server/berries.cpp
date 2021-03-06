#include "berries.h"
#include "items.h"
#include "../PokemonInfo/pokemonstructs.h"

typedef BerryMechanics BM;
typedef BattleSituation BS;

struct BMStatusBerry : public BM
{
    BMStatusBerry() {
        functions["AfterStatusChange"] = &asc;
    }

    static void asc(int s, int, BS &b) {
        if (b.koed(s))
            return;

        int status = b.poke(s).status();
        bool conf = b.isConfused(s);
        int arg = poke(b,s)["ItemArg"].toInt();

        /* Confusion berry */
        if (arg == -1) {
            if (conf) {
                b.eatBerry(s);
                b.healConfused(s);
                b.sendBerryMessage(1, s, 0);
            }
            return;
        }

        /* Lum berry */
        if (conf && arg == 0) {
            b.healConfused(s);
            goto end;
        }

        if (status == Pokemon::Fine) {
            return;
        }

        /* LumBerry */
        if (arg == 0) {
            goto end;
        }    /* Poison Berry */
        else if (arg == Pokemon::Poisoned) {
            if (status == Pokemon::Poisoned || status == Pokemon::DeeplyPoisoned) {
                goto end;
            }
        } else { /* Other Status Berry */
            if (status == arg) {
                goto end;
            }
        }

        return;

        end:
        b.eatBerry(s);
        b.healStatus(s, status);
        b.sendBerryMessage(1, s, arg + 1);
    }
};

struct BMLeppa : public BM
{
    BMLeppa() {
        functions["BeforeEnding"] = &appl;
    }

    static void appl(int s, int, BS &b) {
        int minmove = 0;
        int minPP = 100;
        bool init = false;
        bool zeroPP = false;
        for (int i = 0; i < 4; i++) {
            if (b.move(s, i) == 0) {
                continue;
            }
            if (b.poke(s).move(i).PP() == 0) {
                zeroPP = true;
                init = true;
                minmove = i;
                minPP = 0;
                break;
            }
            if (b.poke(s).move(i).PP() < minPP && b.poke(s).move(i).PP() < b.poke(s).move(i).totalPP()) {
                minmove = i;
                init = true;
                minPP = b.poke(s).move(i).PP();
            }
        }
            
        
        if (init && (zeroPP || turn(b,s).value("BugBiter").toBool())) {
            b.eatBerry(s);
            b.sendBerryMessage(2,s,0,0,0,b.move(s,minmove));
            b.gainPP(s,minmove,10);
        }
    }
};

struct BMPinch : public BM
{
    static bool testpinch(int s, int , BS &b, int ratio) {
        if (turn(b,s).value("BugBiter").toBool()) {
            b.eatBerry(s);
            return true;
        }
        //Gluttony
        if (b.hasWorkingAbility(s, 30))
            ratio = 2;

        bool ret = b.poke(s).lifePoints()*ratio <= b.poke(s).totalLifePoints() && !b.koed(s);

        if (ret)
            b.eatBerry(s);

        return ret;
    }
};

struct BMPinchHP : public BMPinch
{
    BMPinchHP() {
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int s, int, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == s && turn(b,b.attacker())["Power"].toInt() > 0)
            return;
        tp(s, s, b);
    }

    static void tp(int s, int, BS &b) {
        if (!testpinch(s, s, b,2))
            return;

        b.sendBerryMessage(3,s,0);
        int arg = poke(b,s)["ItemArg"].toInt();
        if (arg == 10) /* oran berry */
            b.healLife(s, 10);
        else /* Sitrus Berry */
            b.healLife(s, b.poke(s).totalLifePoints()/4);
    }
};

struct BMAntiSuperEffective : public BM
{
    BMAntiSuperEffective() {
        functions["Mod3Items"] = &m3b;
    }

    static void m3b(int s, int t, BS &b) {
        if (!b.hasSubstitute(s) && turn(b,t)["TypeMod"].toInt() > 4 && turn(b,t)["Type"].toInt() == poke(b,s)["ItemArg"].toInt()) {
            b.sendBerryMessage(4,s,0,t,b.poke(s).item(),move(b,t));
            b.eatBerry(s,false);

            turn(b,t)["Mod3Berry"] = -5;
        }
    }
};

struct BMAntiNormal : public BM
{
    BMAntiNormal() {
        functions["Mod3Items"] = &m3b;
    }

    static void m3b(int s, int t, BS &b) {
        /* Normal moves */
        if (!b.hasSubstitute(s) && turn(b,t)["Type"].toInt() == 0) {
            b.sendBerryMessage(4,s,0,t,b.poke(s).item(),turn(b,t)["Move"].toInt());
            b.eatBerry(s,false);

            turn(b,t)["Mod3Berry"] = -5;
        }
    }
};

struct BMSuperHP : public BM
{
    BMSuperHP() {
        functions["TestPinch"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        if (b.koed(s))
            return;
        if (turn(b,t)["TypeMod"].toInt() <= 4)
            return;
        if (b.poke(s).isFull())
            return;
        b.eatBerry(s);
        b.sendBerryMessage(6,s,0);
        b.healLife(s, b.poke(s).totalLifePoints()/5);
    }
};

struct BMPinchStat : public BMPinch
{
    BMPinchStat() {
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int s, int, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == s && turn(b,b.attacker())["Power"].toInt() > 0)
            return;
        tp(s, s, b);
    }

    static void tp(int s, int, BS &b) {
        /* The berry may change after the call to test pinch (eaten),
           so saved before. */
        int berry = b.poke(s).item();

        if (!testpinch(s, s, b,4))
            return;

        int arg = poke(b,s)["ItemArg"].toInt();
        b.sendBerryMessage(7,s,0,s,berry, arg);
        b.gainStatMod(s, arg, 1,false);
    }
};

struct BMCriticalPinch : public BMPinch
{
    BMCriticalPinch() {
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int s, int, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == s && turn(b,b.attacker())["Power"].toInt() > 0)
            return;
        tp(s, s, b);
    }

    static void tp(int s, int, BS &b) {
        if (!testpinch(s, s, b,4))
            return;

        uas(s,s,b);
    }

    /* ripped off from focus energy */
    static void uas(int s, int, BS &b) {
        addFunction(poke(b,s), "TurnSettings", "FocusEnergy", &ts);
        b.sendMoveMessage(46,0,s);
    }
    static void ts(int s, int, BS &b) {
        addFunction(turn(b,s), "BeforeTargetList", "FocusEnergy", &btl);
    }
    static void btl(int s, int, BS &b) {
        if (turn(b,s)["Power"].toInt() > 0) {
            inc(turn(b,s)["CriticalRaise"], 2);
        }
    }
};

struct BMStarf : public BMPinch
{
    BMStarf() {
        functions["AfterHPChange"] = &ahpc;
        functions["TestPinch"] = &tp;
    }

    static void ahpc(int s, int, BS &b) {
        /* Those berries don't activate immediately when attacked by offensive moves,
           but only after side effects applied. At that time, the battle thread will call
           the effect "TestPinch"
        */
        if (b.attacked() == s && turn(b,b.attacker())["Power"].toInt() > 0)
            return;
        tp(s, s, b);
    }

    static void tp(int s, int, BS &b) {
        int berry = b.poke(s).item();

        if (!testpinch(s, s, b, 4))
            return;

        int stat = (b.true_rand()%5) +1;
        b.gainStatMod(s, stat, 2,false);
        b.sendBerryMessage(9,s,0,s,berry, stat);
    }
};

struct BMBerryLock : public BMPinch
{
    BMBerryLock() {
        functions["BeforeTargetList"] = &btl;
    }

    static void btl(int s, int, BS &b) {
        if (!testpinch(s, s, b,4))
            return;

        poke(b,s)["BerryLock"] = true;
        b.sendBerryMessage(10,s,0);
    }
};

struct BMCustap : public BMPinch
{
    BMCustap() {
        functions["TurnOrder"] = &to;
    }

    static void to (int s, int, BS &b) {
        if (!testpinch(s, s, b,4))
            return;

        b.sendBerryMessage(11,s,0);
        turn(b,s)["TurnOrder"] = 3;
    }
};

struct BMBerryRecoil : public BM
{
    BMBerryRecoil() {
        functions["UponOffensiveDamageReceived"] = &uodr;
    }

    static void uodr(int s, int t, BS &b) {
        //Magic Guard
        if (turn(b,t)["Category"].toInt() != poke(b,s)["ItemArg"].toInt() || b.koed(t) || b.hasWorkingAbility(t, Ability::MagicGuard)) {
            return;
        }
        b.eatBerry(s);
        b.sendBerryMessage(12,s,0,t);
        b.inflictDamage(t, b.poke(t).lifePoints()/8,s,false);
    }
};

#define REGISTER_BERRY(num, name) mechanics[num+8000] = BM##name(); names[num+8000] = #name; nums[#name] = num+8000;

void ItemEffect::initBerries()
{
    REGISTER_BERRY(1, StatusBerry);
    REGISTER_BERRY(2, Leppa);
    REGISTER_BERRY(3, PinchHP);
    REGISTER_BERRY(4, AntiSuperEffective);
    REGISTER_BERRY(5, AntiNormal);
    REGISTER_BERRY(6, SuperHP);
    REGISTER_BERRY(7, PinchStat);
    REGISTER_BERRY(8, CriticalPinch);
    REGISTER_BERRY(9, Starf);
    REGISTER_BERRY(10, BerryLock);
    REGISTER_BERRY(11, Custap);
    REGISTER_BERRY(12, BerryRecoil);
}
