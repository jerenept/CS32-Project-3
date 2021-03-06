#include "Actor.h"



// Students:  Add code to this file (if you wish), Actor.h, StudentWorld.h, and StudentWorld.cpp
/*//////////////////////Actor////////////////////////////////
 *
 *
 */
Actor::Actor(int imageID, int startX, int startY, GraphObject::Direction startDir, float size, unsigned int depth,
             StudentWorld *sw)
        : GraphObject(imageID, startX, startY, startDir, size, depth), m_world(sw), m_toBeRemoved(false), m_value(0),
          m_ticksBetweenActions(30),
          m_ticksUntilAction(30) {

}

bool Actor::obstructsProtesters(int x, int y) {
    //if it obstructs the movement of the Protestors
    //int tmpX = getX() - x;
    //int tmpY = getY() - y;
    //return tmpX < SPRITE_WIDTH && tmpX > 0 && tmpY < SPRITE_HEIGHT && tmpY > 0;
    return false; //(sqrtf(sqr(getX() - x) + sqr(getY() - y)) < SPRITE_WIDTH);
    //if tmpX in [0..3] and tmpY in [0..3], the object is blocking the way of the protesters/FrackMan.
    // the "object", of course being just boulders.
}

Actor::~Actor() { }

void Actor::markRemoved() {
    m_toBeRemoved = true;
    this->setVisible(false);
}

bool Actor::toBeRemoved() {
    return m_toBeRemoved;
}



bool Actor::actThisTick() {
    bool act = (m_ticksUntilAction == 0);
    if (act) {
        m_ticksUntilAction = m_ticksBetweenActions;
    } else {
        m_ticksUntilAction--;
    }
    return act;
}

StudentWorld *Actor::getWorld(void) {
    return m_world;
}


/*///////////////////////Person////////////////////
 */
Person::Person(int imageId, int startX, int startY, GraphObject::Direction startDir, int hitPoints, StudentWorld *sw)
        : Actor(imageId, startX, startY, startDir, DEFAULT_SIZE, FOREGROUND, sw), m_hitPoints(hitPoints) {
    setVisible(true);
}

bool Person::obstructsProtesters(int x, int y) {
    return false;
    //Protesters' pathing isn't blocked by each other.
    //They have special interactions with the FrackMan, though I can't see why they would not
    // be allowed to walk through him, they just (should) never actually try to.
    //Also, the FrackMan can pass through protesters.
}


int Person::getHealth() {
    return m_hitPoints;
}

void Person::hurt(int damage) {
    m_hitPoints -= damage;
}

void Protester::hurt(int damage, bool play = true) {
    \
    if (play) {
    getWorld()->playSound(SOUND_PROTESTER_ANNOYED);
    }
    stun();
    Person::hurt(damage);
    if (getHealth() <= 0) {
        getWorld()->killedProtester();
    }
}

void FrackMan::hurt(int damage) {
    getWorld()->playSound(SOUND_PLAYER_ANNOYED);
    Person::hurt(damage);
}

bool Actor::validMovement(int &x, int &y, GraphObject::Direction direction) {
    //determine if an Actor at a particular place can make a particular movement, even into dirt
    //(i.e. only checks for Boulders and walls)
    //the integers passed in by reference actually change, to represent the movement.
    //this should be in Actor, and, actually, for Part 2, it will be.
    switch (direction) {
        case GraphObject::up:
            if ((y < VIEW_HEIGHT - SPRITE_HEIGHT) && !getWorld()->obstructionAt(x, y + 1)) {
                y = y + 1;
                return true;
            }
            break;
        case GraphObject::down:
            if ((y > 0) && !getWorld()->obstructionAt(x, y - 1)) {
                y = y - 1;
                return true;
            }
            break;
        case GraphObject::left:
            if ((x > 0) && !getWorld()->obstructionAt(x - 1, y)) {
                x = x - 1;
                return true;
            }
            break;
        case GraphObject::right:
            if ((x < VIEW_WIDTH - SPRITE_WIDTH) && !getWorld()->obstructionAt(x + 1, y)) {
                x = x + 1;
                return true;
            }
            break;
        default:
            return false;
    }
    return false;
}

/*//////////////////////////Discovery/////////////////////////
 */

Discovery::Discovery(int imageId, int locX, int locY, StudentWorld *sw)
        : Actor(imageId, locX, locY, DISCOVERY_START_DIR, DEFAULT_SIZE, DISCOVERY_DEPTH, sw) {

}

/* ////////////////FrackMan/////////////////
 */
FrackMan::FrackMan(StudentWorld *sw)
        : Person(IID_PLAYER, FRACKMAN_START_X, FRACKMAN_START_Y, FRACKMAN_START_DIR, FRACKMAN_HITPOINTS, sw),
          m_gold(FRACKMAN_START_GOLD), m_sonar(FRACKMAN_START_SONAR), m_water(FRACKMAN_START_WATER) {
}

void FrackMan::doSomething() {
    int keyp;
    int newX = getX();
    int newY = getY();
    getWorld()->deleteDirtAt(newX, newY);
    if (!getWorld()->getKey(keyp)) {//If I don't get a keypress, then nothing's going on
        getWorld()->clearDead();
        //and I can delete all the garbage I've been saving up
        return; //and quit early.
    }
    GraphObject::Direction dir = getDirection();
    switch (keyp) { //select the direction we wish to go. Also set the frackman's direction.
        case KEY_PRESS_LEFT:
            dir = GraphObject::left;
            break;
        case KEY_PRESS_RIGHT:
            dir = GraphObject::right;
            break;
        case KEY_PRESS_UP:
            dir = GraphObject::up;
            break;
        case KEY_PRESS_DOWN:
            dir = GraphObject::down;
            break;
        case KEY_PRESS_SPACE: {
            if (m_water > 0) {
                m_water--;
                validMovement(newX, newY, getDirection());
                Squirt *squirt = new Squirt(newX, newY, getDirection(), getWorld());
                getWorld()->insertActor(squirt);
            }
            return;
        }
        case KEY_PRESS_ESCAPE:
            this->hurt(this->getHealth());
            //completely annoy the frackman (me_irl)
            return;
        case 'z':
        case 'Z':
            if (m_sonar > 0) {
                getWorld()->sonarPulse(getX(), getY());
                m_sonar--;
                getWorld()->playSound(SOUND_SONAR);
            }
            return;
        case KEY_PRESS_TAB:
            if (m_gold > 0) {
                m_gold--;
                GoldNugget *bribe = new GoldNugget(getX(), getY(), getWorld(), true);
                getWorld()->insertActor(bribe);
            }
            break;
        default:
            dir = GraphObject::none;
            break; //???how??? did you get here???
    }
    if (dir == getDirection())
        validMovement(newX, newY, dir);
    setDirection(dir);
    moveTo(newX, newY);
}

/* ////////////////OilBarrel//////////////////
 */
OilBarrel::OilBarrel(int locX, int locY, StudentWorld *sw)
        : Discovery(IID_BARREL, locX, locY, sw) {
    setVisible(false);
}

void OilBarrel::doSomething() {
    if (this->toBeRemoved())
        return; //well if it's dead I don't really expect a lot out of it.
    if (this->pickedUp())
        getWorld()->gotOil();
}


//Protester - incidentally, these were initially named "Protestors", as the git history can tell you.
//post-post script Protestor is actually a valid spelling and I want to change it back,
//but I also really want that interview with Symantec
Protester::Protester(int imageId, int startX, int startY, StudentWorld *sw)
        : Person(imageId, startX, startY, PROTESTER_START_DIR, PROTESTER_START_HITPOINTS, sw), m_ticksBetweenYells(0),
          m_lastPerpendicularTurn(0), m_first_run(true) {
    setVisible(true);
    setValue(REGULAR_PROTESTER_VALUE);
    int ticksBetweenMoves = max(0, 3 - getWorld()->getLevel() / 4);
    this->setTicks(ticksBetweenMoves);
}

void Protester::doSomething() {
    if (toBeRemoved())
        return;
    if (getHealth() <= 0) {
        if (getX() != 60 || getY() != 60) {
            int x = getX();
            int y = getY();
            //GraphObject:
            Direction dir = getWorld()->leaveOilField(x, y);
            setDirection(dir);
            moveTo(x, y);
            //getWorld()->setGameStatText("XY: " + std::to_string(x) + std::to_string(y));
        } else {
            markRemoved();
        }
        return;
    }
    if (m_stunDuration > 0) {
        m_stunDuration--;
        return;
    }
    //GraphObject::Direction fm_dir = getWorld()->frackManDirection();]
    if (!actThisTick() && !m_first_run) {
        return;
    }

    if (m_first_run)
        m_first_run = false;


    float fm_dist = getWorld()->frackManCornerRadius(this);
    m_lastPerpendicularTurn--; //note: this may be the source of bugs //todo::
    if (getWorld()->lineOfSightWithFrackMan(this) && fm_dist >= 4.0) {
        int x = getX();
        int y = getY();

        setDirection(getWorld()->directionToFrackMan(this));
        if (validMovement(x, y, getWorld()->directionToFrackMan(this))) {
            moveTo(x, y);
            m_nSquaresToMove = 0;
            return;
        }
    }
    if (getWorld()->frackManCornerRadius(this) <= 4.0 && yellThisTick()) {
        getWorld()->playSound(SOUND_PROTESTER_YELL);
        getWorld()->damageFrackMan(PROTESTER_DAMAGE);
        return;
    }
    m_nSquaresToMove--;
    if (m_nSquaresToMove <= 0) {
        setDirection(getValidRandomDirection());
        m_nSquaresToMove = (rand() % 54) + 8;
    }


    int x = getX();
    int y = getY();
    if (m_lastPerpendicularTurn <= 0) {
        if (getDirection() == right || getDirection() == left) {
            if (validMovement(x, y, up) || validMovement(x, y, down)) { //note: it seems like the value would change,
                // but due to short circuited logic, if validmovement returns false the x and y are unchanged.
                // the protester will also have a tendency to turn up more often (turn down for what)
                x = getX();
                y = getY();
                int r = rand() % 2;
                if (r == 0) {
                    if (validMovement(x, y, up)) {
                        setDirection(up);
                        moveTo(x, y);
                    } else if (validMovement(x, y, down)) {
                        setDirection(down);
                        moveTo(x, y);
                    }
                }
                if (r == 1) {
                    if (validMovement(x, y, down)) {
                        setDirection(down);
                        moveTo(x, y);
                    } else {
                        validMovement(x, y, up);
                        setDirection(up);
                        moveTo(x, y);
                    }
                }
            } else {
                x = getX();
                y = getY();
                int r = rand() % 2;
                if (r == 0) {
                    if (validMovement(x, y, left)) {
                        setDirection(left);
                        moveTo(x, y);
                    } else if (validMovement(x, y, right)) {
                        setDirection(right);
                        moveTo(x, y);
                    }
                }
                if (r == 1) {
                    if (validMovement(x, y, right)) {
                        setDirection(right);
                        moveTo(x, y);
                    } else if (validMovement(x, y, left)) {
                        setDirection(left);
                        moveTo(x, y);
                    }
                }
            }
        }
    } else {
        setDirection(getValidRandomDirection());
        validMovement(x, y, getDirection());
        moveTo(x, y);
    }

    x = getX();
    y = getY();
    m_nSquaresToMove = (rand() % 54) + 8;
    m_lastPerpendicularTurn = 200;
    if (!validMovement(x, y, getDirection())) {
        m_nSquaresToMove = 0;
    }
}
//Dirt

void Dirt::doSomething() { } //dirt does nothing

//Dirt, when made, sits there and is dirt
Dirt::Dirt(int locX, int locY, StudentWorld *sw)
        : Actor(IID_DIRT, locX, locY, DIRT_DIR, DIRT_SIZE, BACKGROUND, sw) {
    setVisible(true);
}

Dirt::~Dirt() {}

bool Dirt::obstructsProtesters(int x, int y) {
    return (x == getX() && y == getY()) && !toBeRemoved();
} //dirt obstructs the protesters, but only in its small square.

bool Discovery::obstructsProtesters(int x, int y) {
    return false;
} //Protesters' pathing isn't blocked by discoveries.



Squirt::Squirt(int startX, int startY, GraphObject::Direction dir, StudentWorld *sw)
        : Actor(IID_WATER_SPURT, startX, startY, dir, SQUIRT_SIZE, SQUIRT_DEPTH, sw), m_distanceRemaining(8) {

    setVisible(true);
    this->getWorld()->playSound(SOUND_PLAYER_SQUIRT);
}


void Squirt::doSomething() {
    //if (!actThisTick() && m_distanceRemaining!=0){
    //''      return;
    //  }
    if (toBeRemoved())
        return;
    if (m_distanceRemaining == 0) {
        this->markRemoved();
        return;
    }
    int newX = getX();
    int newY = getY();
    if (!validMovement(newX, newY, getDirection())) {
        this->markRemoved();
        return;
    }
    --m_distanceRemaining;
    this->moveTo(newX, newY);
    getWorld()->damageOneActorAt(getX(), getY(), this);

}

Boulder::Boulder(int startX, int startY, StudentWorld *sw)
        : Actor(IID_BOULDER, startX, startY, BOULDER_START_DIR, BOULDER_SIZE, BOULDER_DEPTH, sw), m_mobile(false) {
    this->setVisible(true);
    getWorld()->deleteDirtAt(getX(), getY(), false);

}

void Boulder::doSomething() {
    if (toBeRemoved())
        return;
    if (dirtOrRockBelow()) {
        if (m_mobile) {
            this->markRemoved();
            return;
        }
    } else if (m_mobile || actThisTick()) {
        if (!m_mobile)
            getWorld()->playSound(SOUND_FALLING_ROCK);
        moveTo(getX(), getY() - 1);
        getWorld()->damageAllActorsAt(getX(), getY());
        m_mobile = true;
    }
}

bool Boulder::dirtOrRockBelow() {
    return getWorld()->dirtAt(getX(), getY() - 1) || getY() <= 0;
    //       return true; //if the boul}
    //return false;
}

bool Boulder::obstructsProtesters(int x, int y) {
    return (abs(getX() - x) < SPRITE_WIDTH && abs(getY() - y) < SPRITE_HEIGHT) &&
           !toBeRemoved(); //-1 to account for edge of boulder being smaller?
}
bool FrackMan::validMovement(int &x, int &y, GraphObject::Direction dir) {
    int nextX = x;
    int nextY = y;
    if (Actor::validMovement(nextX, nextY, dir)) {
        x = nextX;
        y = nextY;
        return true;
    }
    return false;
}

bool Squirt::validMovement(int &x, int &y, GraphObject::Direction dir) {
    int newX = x;
    int newY = y;
    if (Actor::validMovement(newX, newY, dir) && !getWorld()->dirtAt(x, y)) {
        x = newX;
        y = newY;
        return true;
    } else {
        return false;
    }
}

Sonar::Sonar(StudentWorld *sw) : Discovery(IID_SONAR, SONAR_START_X, SONAR_START_Y, sw) {
    setVisible(true);
    setTicks(min(100, 300 - 100 * (getWorld()->getLevel())));
    setValue(SONAR_VALUE);
}

void Sonar::doSomething() {
    if (actThisTick()) {
        this->markRemoved();
        getWorld()->sonarDespawned(false);
    }
    if (this->pickedUp()) {
        getWorld()->sonarDespawned(true);
    }
}

void Actor::setTicks(int ticks) {
    m_ticksBetweenActions = ticks;
}

bool Discovery::pickedUp() {
    if (getWorld()->frackManCornerRadius(this) <= 3.0 && !toBeRemoved()) {
        getWorld()->playSound(SOUND_GOT_GOODIE);
        this->markRemoved();
        return true;
    }
    if (getWorld()->frackManCornerRadius(this) <= 4.0 && !toBeRemoved()) {
        setVisible(true);
        return false;
    }
    return false;

}

void FrackMan::addSonar() {
    m_sonar++;
}


GoldNugget::GoldNugget(int locX, int locY, StudentWorld *sw, bool isBribe) : Discovery(IID_GOLD, locX, locY, sw),
                                                                             m_isBribe(isBribe) {
    setVisible(isBribe);
    if (isBribe) {
        setTicks(100);
    }
    setValue(GOLD_VALUE);
}

void GoldNugget::doSomething() {
    if (this->toBeRemoved())
        return; //well if it's dead I don't really expect a lot out of it.
    if (!m_isBribe && this->pickedUp())
        getWorld()->gotGold();
    if (m_isBribe) {
        if (actThisTick()) {
            //getWorld()->playSound(SOUND_PROTESTER_FOUND_GOLD);
            markRemoved();
        }
    }
}

void FrackMan::addGold() {
    m_gold++;
}


bool Protester::yellThisTick() {
    if (m_ticksBetweenYells == 0) {
        m_ticksBetweenYells = 15;
        return true;
    } else {
        m_ticksBetweenYells--;
        return false;
    }

}

GraphObject::Direction Protester::getValidRandomDirection() {
    //assumes that at least one of the directions is valid
    //(which is valid)
    int x = getX();
    int y = getY();
    GraphObject::Direction dir = getDirection();
    do {
        switch (rand() % 4) {
            case 0:
                dir = (GraphObject::up);
                break;
            case 1:
                dir = (GraphObject::down);
                break;
            case 2:
                dir = (GraphObject::left);
                break;
            case 3:
                dir = (GraphObject::right);
                break;
                //goto case_zero; //fuckin please help me, this feels so wrong
        }
    } while (!validMovement(x, y, dir) || getWorld()->dirtAt(x, y));
    return dir;
}

//GraphObject::Direction Person::getValidRandomDirectionLR() { }


void Actor::hurt(int damage) { } //most Actors can't be hurt.
int Actor::getHealth() {
    return 0;
}


bool Protester::validMovement(int &x, int &y, GraphObject::Direction dir) {
    int tempx = x;
    int tempy = y;
    bool valid = Actor::validMovement(tempx, tempy, dir);
    if (getWorld()->dirtAt(tempx, tempy)) {
        return false;
    }
    x = tempx;
    y = tempy;
    return valid;
}

Water::Water(int locX, int locY, StudentWorld *sw) : Discovery(IID_WATER_POOL, locX, locY, sw) {
    setVisible(true);
    setTicks(max(100, 300 - 10 * (getWorld()->getLevel())));

}

void Water::doSomething() {
    if (this->toBeRemoved())
        return;
    if (this->pickedUp()) {
        getWorld()->gotWater();
    }
    if (actThisTick())
        markRemoved();
}

void FrackMan::addWater() {
    m_water += 5;
}

void Protester::stun() {
    m_stunDuration = max(50, 100 - ((getWorld()->getLevel() * 10)));
}

int FrackMan::amtWater() {
    return m_water;
}

int FrackMan::amtGold() {
    return m_gold;
}

int FrackMan::amtSonar() {
    return m_sonar;
}

void Actor::setValue(int val) {
    m_value = val;
}

HardcoreProtester::HardcoreProtester(int imageId, int startX, int startY, StudentWorld *sw) : Protester(
        IID_HARD_CORE_PROTESTER, 60, 60, sw) {
    setVisible(true);
    setValue(HARDCORE_PROTESTER_VALUE);
}

void HardcoreProtester::doSomething() {
    if (toBeRemoved())
        return;
    if (getHealth() <= 0) {
        int x = getX();
        int y = getY();
        if (x != 60 || y != 60) {
            //GraphObject:
            Direction dir = getWorld()->leaveOilField(x, y);
            setDirection(dir);
            moveTo(x, y);
            //getWorld()->setGameStatText("XY: " + std::to_string(x) + std::to_string(y));
        } else {
            markRemoved();
        }
        return;
    }
    if (stunned()) {
        return;
    }
    //GraphObject::Direction fm_dir = getWorld()->frackManDirection();]
    if (!actThisTick()) {
        return;
    }

    if (getWorld()->lineOfSightWithFrackMan(this) && getWorld()->frackManCornerRadius(this) >= 4.0) {
        int x = getX();
        int y = getY();

        setDirection(getWorld()->directionToFrackMan(this));
        if (validMovement(x, y, getWorld()->directionToFrackMan(this))) {
            moveTo(x, y);
            resetMoveForward();
            return;
        }
    }
    float fm_dist = getWorld()->frackManCornerRadius(this);
    if (getWorld()->pathingDistanceFromFrackMan(getX(), getY()) <= (16 + getWorld()->getLevel() * 2)) {
        int x = getX();
        int y = getY();
        GraphObject::Direction dir = getWorld()->getFrackMan(x, y);
        setDirection(dir);
        moveTo(x, y);
    }
    if (getWorld()->lineOfSightWithFrackMan(this) && fm_dist >= 4.0) {
        int x = getX();
        int y = getY();
        setDirection(getWorld()->directionToFrackMan(this));
        if (validMovement(x, y, getWorld()->directionToFrackMan(this))) {
            moveTo(x, y);
            resetMoveForward();
            return;
        }
    }
    if (getWorld()->frackManCornerRadius(this) <= 4.0 && yellThisTick()) {
        getWorld()->playSound(SOUND_PROTESTER_YELL);
        getWorld()->damageFrackMan(PROTESTER_DAMAGE);
        return;
    }
    if (moveForward()) {
        setDirection(getValidRandomDirection());
    }


    int x = getX();
    int y = getY();
    if (makePerpendicularTurn()) {
        if (getDirection() == right || getDirection() == left) {
            if (validMovement(x, y, up) || validMovement(x, y, down)) { //note: it seems like the value would change,
                // but due to short circuited logic, if validmovement returns false the x and y are unchanged.
                // the protester will also have a tendency to turn up more often (turn down for what)
                x = getX();
                y = getY();
                int r = rand() % 2;
                if (r == 0) {
                    if (validMovement(x, y, up)) {
                        setDirection(up);
                        moveTo(x, y);
                    } else if (validMovement(x, y, down)) {
                        setDirection(down);
                        moveTo(x, y);
                    }
                }
                if (r == 1) {
                    if (validMovement(x, y, down)) {
                        setDirection(down);
                        moveTo(x, y);
                    } else {
                        validMovement(x, y, up);
                        setDirection(up);
                        moveTo(x, y);
                    }
                }
            } else {
                x = getX();
                y = getY();
                int r = rand() % 2;
                if (r == 0) {
                    if (validMovement(x, y, left)) {
                        setDirection(left);
                        moveTo(x, y);
                    } else if (validMovement(x, y, right)) {
                        setDirection(right);
                        moveTo(x, y);
                    }
                }
                if (r == 1) {
                    if (validMovement(x, y, right)) {
                        setDirection(right);
                        moveTo(x, y);
                    } else if (validMovement(x, y, left)) {
                        setDirection(left);
                        moveTo(x, y);
                    }
                }
            }
        }
    } else {
        setDirection(getValidRandomDirection());
        validMovement(x, y, getDirection());
        moveTo(x, y);
    }

    x = getX();
    y = getY();
    if (!validMovement(x, y, getDirection())) {
        resetMoveForward();
    }
}

bool Protester::stunned() {
    bool stunned = m_stunDuration > 0;
    if (m_stunDuration > 0) {
        m_stunDuration--;
    }
    return stunned;
}


bool Protester::makePerpendicularTurn() {
    m_lastPerpendicularTurn--;
    if (m_lastPerpendicularTurn <= 0) {
        m_lastPerpendicularTurn = 200;
        return true;
    }
    return false;
}


bool Protester::moveForward() {
    m_nSquaresToMove--;
    if (m_nSquaresToMove <= 0) {
        m_nSquaresToMove = (rand() % 54) + 8;
        return true;
    }
    else {
        return false;
    }
}

void Protester::resetMoveForward() {
    m_nSquaresToMove = 0;
}

void Protester::markRemoved() {
    getWorld()->increaseScore(REGULAR_PROTESTER_VALUE);
    getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
    Actor::markRemoved();
}

void HardcoreProtester::markRemoved() {
    getWorld()->increaseScore(HARDCORE_PROTESTER_VALUE);
    getWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
    Actor::markRemoved();
}

bool GoldNugget::isBribe() {
    return m_isBribe;
}

bool OilBarrel::pickedUp() {
    if (getWorld()->frackManCornerRadius(this) <= 3.0 && !toBeRemoved()) {
        getWorld()->playSound(SOUND_FOUND_OIL);
        this->markRemoved();
        return true;
    }
    if (getWorld()->frackManCornerRadius(this) <= 4.0 && !toBeRemoved()) {
        setVisible(true);
        return false;
    }
    return false;

}

Protester::~Protester() {

}

Person::~Person() {

}
