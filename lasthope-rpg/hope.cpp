#include <iostream>
#include <cstdlib>
#include <array>
#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <ctime>
#include <unistd.h>
#include <unordered_set>
#include <iterator>
#include <cstdlib>
#include <limits>
#include <tuple>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>

#include "chooseCharacter.h"
#include "systemf.h"

#define TTY_PATH ""
#define STTY_US "true"
#define STTY_DEF "true"


const int WIDT = 45;
const int HEIGH = 20;
int plx, ply;

class Entity;

class Armor {
public:
    Armor() {}
    Armor(const std::string& name, const std::string& description, int defense, int cost, const std::array<int, 2>& levels)
            : name(name), description(description), defense(defense), cost(cost), levels(levels) {}

    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    int getDefense() const { return defense; }
    int getCost() const { return cost; }
    const std::array<int, 2>& getLevels() const {return levels;}
private:
    std::string name;
    std::string description;
    int defense;
    int cost;
    std::array<int, 2> levels;
};

class Weapon {
public:
    Weapon() {}
    Weapon(const std::string& name, const std::string& description, int damage, double criticalHitRate, int cost, const std::array<int, 2>& levels)
            : name(name), description(description), damage(damage), criticalHitRate(criticalHitRate), cost(cost), levels(levels) {}

    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    int getDamage() const { return damage; }
    double getCriticalHitRate() const { return criticalHitRate; }
    int getCost() const { return cost; }
    const std::array<int, 2>& getLevels() const {return levels;}
    void Print() {
        std::cout<<"weapon\n\r";
    }
private:
    std::string name;
    std::string description;
    int damage;
    double criticalHitRate;
    int cost;
    std::array<int, 2> levels;
};

class Item {
public:
    Item() {}
    Item(const std::string& name, const std::string& description, int cost)
            : name(name), description(description), cost(cost) {}

    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    int getCost() const { return cost; }

    virtual ~Item() {}
private:
    std::string name;
    std::string description;
    int cost;
};

class Medicine : public Item {
public:
    Medicine() {}
    Medicine(const std::string& name, const std::string& description, int cost, int healingAmount)
            : Item(name, description, cost), healingAmount(healingAmount) {}

    int getHealingAmount() const { return healingAmount; }
    void use(Entity& target);

    virtual ~Medicine() {}
private:
    int healingAmount;
};

class Gadget : public Item {
public:
    Gadget() {}
    Gadget(const std::string& name, const std::string& description, int cost)
            : Item(name, description, cost) {}

    virtual ~Gadget() {}
    virtual void activate(Entity& user, std::vector<Entity*>& enemies) = 0;
};

class ShieldGadget : public Gadget {
public:
    ShieldGadget() {}
    ShieldGadget(const std::string& name, const std::string& description, int cost, int level)
            : Gadget(name, description, cost), level(level), remainingCharges(level) {}

    int getLevel() const { return level; }
    int getRemainingCharges() const { return remainingCharges; }
    void useCharge() { remainingCharges--; }
    void activate(Entity& user, std::vector<Entity*>& enemies) override;
private:
    int level;
    int remainingCharges;
};

class GrenadeGadget : public Gadget {
public:
    GrenadeGadget() {}
    GrenadeGadget(const std::string& name, const std::string& description, int cost, int damage, size_t targets)
            : Gadget(name, description, cost), damage(damage), targets(targets) {}

    int getDamage() const { return damage; }
    size_t getTargets() const { return targets; }
    void activate(Entity& user, std::vector<Entity*>& enemies) override;
private:
    int damage;
    size_t targets;
};

class Backpack {
public:
    void addWeapon(std::shared_ptr<Weapon> weapon) { weapons.push_back(weapon); }
    void addArmor(std::shared_ptr<Armor> armor) { armors.push_back(armor); }
    void addItem(const std::shared_ptr<Item> item) {
        items.push_back(item);
        std::string name = item->getName();
        ItemMap[name]++;
    }

    std::shared_ptr<Weapon> getWeapon(size_t index) {
        if (index < weapons.size()) {
            return weapons[index];
        }
        return nullptr;
    }
    std::shared_ptr<Armor> getArmor(size_t index) {
        if (index < armors.size()) {
            return armors[index];
        }
        return nullptr;
    }

    std::shared_ptr<Item> getItem(size_t index) {
        if (index < items.size()) {
            return items[index];
        }
        return nullptr;
    }

    std::shared_ptr<const Item> getItem(size_t index) const {
        if (index < items.size()) {
            return items[index];
        }
        return nullptr;
    }

    void removeWeapon(size_t index) {
        if (index < weapons.size()) {
            weapons.erase(weapons.begin() + index);
        }
    }
    void removeArmor(size_t index) {
        if (index < armors.size()) {
            armors.erase(armors.begin() + index);
        }
    }

    void removeItem(const std::shared_ptr<Item>& item) {
        //Method 2: auto it = find_if(items.begin(), items.end(), [item](const shared_ptr<Item> p) { return p->getName() == item->getName(); });
        auto it = items.end();
        for(auto it2 = items.begin(); it2 != items.end(); ++it2) {
            if(item->getName() == (*it2)->getName()) {
                it = it2;
                break;
            }
        }
        if (it != items.end()) {
            items.erase(it);
            // Update the item count in the map
            ItemMap[item->getName()]--;
            if (ItemMap[item->getName()] == 0) {
                ItemMap.erase(item->getName());
            }
        }
    }

    void printWeapons(const Entity &entity) const;
    void printArmors(const Entity &entity) const;
    void printItems(const Entity &entity) const;

    size_t weaponSize() const { return weapons.size(); }
    size_t armorSize() const { return armors.size(); }
    size_t getItemSize() const { return ItemMap.size(); }
    int getItemCount(const std::string& item_name) const {
        auto it = ItemMap.find(item_name);
        if (it != ItemMap.end()) {
            return it->second;
        }
        return 0;
    }
private:
    std::vector<std::shared_ptr<Weapon>> weapons;
    std::vector<std::shared_ptr<Armor>> armors;
    std::vector<std::shared_ptr<Item>> items;
    std::map<std::string, int> ItemMap;
};

class Entity {
public:
    Entity() {}

    Entity(const std::string& name, int health, int attackPower, double dodgeRate, double criticalHitRate, int labTokens)
            : name(name), health(health), maxHealth(health), attackPower(attackPower), dodgeRate(dodgeRate), criticalHitRate(criticalHitRate), labTokens(labTokens), level(1), remainingShieldRounds(0) {}

    // Entity functions
    Backpack& getBackpack() { return backpack; }
    const Backpack& getBackpack() const { return backpack; }
    bool isAlive() const { return health > 0; }
    int getHealth() const { return health; }
    const std::string& getName() const { return name; }
    int getLabTokens() const { return labTokens; }
    void setLabTokens(int newLabTokens) { labTokens = newLabTokens; }
    int getAttackPower() const{ return attackPower;}
    double getDodgeRate() const { return dodgeRate; }
    double getCriticalHitRate() const { return criticalHitRate; }
    int getLevel() const { return level;}
    void changeLevel(int i){level = i;}
    int getRemainingShieldRounds() const { return remainingShieldRounds;}


    // Equip functions
    void equipWeapon(size_t index) {
        if (index < backpack.weaponSize()) {
            equippedWeapon = backpack.getWeapon(index);
        }
    }

    void equipArmor(size_t index) {
        if (index < backpack.armorSize()) {
            equippedArmor = backpack.getArmor(index);
        }
    }

    void useShieldGadget(ShieldGadget& gadget) {
        applyShieldGadgetEffect(gadget.getLevel());
        backpack.removeItem(std::make_shared<ShieldGadget>(gadget));
        std::cout << "Using shield gadget: " << gadget.getName() << " Remaining Charge: " << remainingShieldRounds << "\n\r";
    }

    void applyShieldGadgetEffect(int rounds) {
        originalDodgeRate = dodgeRate;
        dodgeRate = 1.0;
        remainingShieldRounds = rounds;
    }

    void resetShieldGadgetEffect() {
        dodgeRate = originalDodgeRate;
    }

    void updateEffects() {
        if (remainingShieldRounds > 0) {
            remainingShieldRounds--;
            if (remainingShieldRounds == 0) {
                resetShieldGadgetEffect();
            }
        }
    }

    void useGrenadeGadget(GrenadeGadget& gadget, std::vector<Entity*>& enemies) {
        backpack.removeItem(std::make_shared<GrenadeGadget>(gadget));
        int enemiesDamaged = 0;
        for (Entity* enemy : enemies) {
            if (enemiesDamaged >= gadget.getTargets()) break;
            enemy->takeDamage(gadget.getDamage());
            enemiesDamaged++;
        }
        sleep(1);
        std::cout << "You dealt " << gadget.getDamage() << " damage to each of " << enemiesDamaged << " Zombies!\n\r";
        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](Entity* e) { return !e->isAlive(); }), enemies.end());
        sleep(1);
        std::cout << "Remaining enemies: " << enemies.size() << "\n\r";
        for (size_t i = 0; i < enemies.size(); i++) {
            std::cout << "Enemy " << (i+1) << " Health: " << enemies[i]->getHealth() << "\n\r";
        }
    }

    int attack(Entity& target) {
        if (target.dodge()) {
            std::cout << "Miss!" << "\n\r";
            return 0;
        }
        int damage = attackPower;
        if (equippedWeapon) {
            damage += equippedWeapon->getDamage();
        }

        // Check for critical hit
        double critRoll = static_cast<double>(rand()) / RAND_MAX;
        if (critRoll < criticalHitRate) {
            damage *= 2;
            std::cout << "Critical hit!" << "\n\r";
        }
        target.takeDamage(damage);
        return damage;
    }

    void takeDamage(int damage) {
        if (equippedArmor) {
            damage -= equippedArmor->getDefense();
            if (damage < 0) damage = 0;
        }
        health -= damage;
        if (health < 0) health = 0;
    }

    void heal(int healthBoost) {
        health += healthBoost;
        if (health > maxHealth) health = maxHealth;
    }

    void transferLabTokens(Entity& target) {
        target.labTokens += labTokens;
    }

    bool isWeaponEquipped(const std::shared_ptr<Weapon>& weapon) const {
        return equippedWeapon == weapon;
    }

    bool isArmorEquipped(const std::shared_ptr<Armor>& armor) const {
        return equippedArmor == armor;
    }

    void printInfo() {
        std::cout<<"*****************************************************************************************\n\r";
        std::cout << "Character: " << name << "  Health: " << health << "  DodgeRate: " << dodgeRate 
        << "  Critical Hit Rate:"<< criticalHitRate << "  LabTokens: " << labTokens << "  Shield Charge: " 
        << remainingShieldRounds << "\n\r";
        backpack.printWeapons(*this);
        backpack.printArmors(*this);
        backpack.printItems(*this);
        std::cout<<"*****************************************************************************************\n\r";
    }

    void equipItems() {
        while (true) {
            std::cout << "Would you like to equip a weapon (w), armor (a), or exit (e)? ";
            char input;
            std::cin >> input;

            if (input == 'w') {
                backpack.printWeapons(*this);
                std::cout << "Enter the index of the weapon you would like to equip (-1 to cancel): ";
                int index;
                std::cin >> index;

                if (!std::cin.fail() && index >= 0 && static_cast<size_t>(index) < backpack.weaponSize()) {
                    equipWeapon(index);
                    std::cout << "Weapon equipped successfully." << "\n\r";
                } else if (!std::cin.fail() && index != -1) {
                    std::cout << "Invalid index. Please try again." << "\n\r";
                }
            } else if (input == 'a') {
                backpack.printArmors(*this);
                std::cout << "Enter the index of the armor you would like to equip (-1 to cancel): ";
                int index;
                std::cin >> index;

                if (!std::cin.fail() && index >= 0 && static_cast<size_t>(index) < backpack.armorSize()) {
                    equipArmor(index);
                    std::cout << "Armor equipped successfully." << "\n\r";
                } else if (!std::cin.fail() && index != -1) {
                    std::cout << "Invalid index. Please try again." << "\n\r";
                }
            } else if (input == 'e') {
                break;
            } else {
                std::cout << "Invalid input. Please try again." << "\n\r";
            }

            // Clear input buffer
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    void combatInteraction(std::vector<Entity*>& enemies) {
        while (true) {
            std::cout << "Choose an action: Equip weapon/armor (e), Use Medicine/Grenade/Shield Items (i), return/Exit (x): ";
            char action;
            std::cin >> action;
            switch(action) {
                case 'e': {
                    equipItems();
                    break;
                }
                case 'i': {
                    std::cout << "Enter the index of the item you would like to use (-1 to cancel): ";
                    int index;
                    std::cin >> index;
                    if (!std::cin.fail() && index >= 0 && static_cast<size_t>(index) < backpack.getItemSize()) {
                        std::shared_ptr<Item> item = backpack.getItem(index);
                        if (dynamic_cast<GrenadeGadget*>(item.get())) {
                            useGrenadeGadget(*dynamic_cast<GrenadeGadget*>(item.get()), enemies);
                            sleep(1);
                            std::cout << std::endl;
                        } else if (dynamic_cast<ShieldGadget*>(item.get())) {
                            useShieldGadget(*dynamic_cast<ShieldGadget*>(item.get()));
                            sleep(1);
                            std::cout << std::endl;
                        } else if (dynamic_cast<Medicine*>(item.get())) {
                            heal(dynamic_cast<Medicine*>(item.get())->getHealingAmount());
                            backpack.removeItem(item);
                            std::cout << "Medicine used successfully." << "\n\r";
                            std::cout << std::endl;
                        } else {
                            std::cout << "Item not usable. Please try again." << "\n\r";
                            std::cout << std::endl;
                        }
                    } else if (!std::cin.fail() && index != -1) {
                        std::cout << "Invalid index. Please try again." << "\n\r";
                        std::cout << std::endl;
                    }
                    break;
                }
                case 'x': {
                    return;
                }
                default: {
                    std::cout << "Invalid action. Please try again." << "\n\r";
                    break;
                }
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
private:
    bool dodge() {
        double dodgeRoll = static_cast<double>(rand()) / RAND_MAX;
        if (dodgeRoll < dodgeRate) {
            return true;
        }
        if (activeShieldGadget && activeShieldGadget->getRemainingCharges() > 0) {
            activeShieldGadget->useCharge();
            if (activeShieldGadget->getRemainingCharges() == 0) {
                removeActiveShieldGadget();
            }
            return true;
        }
        return false;
    }
    void removeActiveShieldGadget() {
        activeShieldGadget.reset();
    }
    std::string name;
    int health;
    int maxHealth;
    int attackPower;
    double dodgeRate;
    double originalDodgeRate;
    int remainingShieldRounds;
    double criticalHitRate;
    int labTokens;
    int level;

    std::shared_ptr<Weapon> equippedWeapon;
    std::shared_ptr<Armor> equippedArmor;
    std::shared_ptr<ShieldGadget> activeShieldGadget;
    Backpack backpack;
};

void ShieldGadget::activate(Entity& user, std::vector<Entity*>& enemies) {
    user.useShieldGadget(*this);
}

void GrenadeGadget::activate(Entity& user, std::vector<Entity*>& enemies) {
    user.useGrenadeGadget(*this, enemies);
}

void Backpack::printWeapons(const Entity& entity) const {
    std::cout << "Weapons in Backpack: " << "\n\r";
    if (weapons.empty()) {
        std::cout << "- No Weapons here" << "\n\r";
        return;
    }
    for (size_t i = 0; i < weapons.size(); ++i) {
        std::cout << "- "<< "Name: " << weapons[i]->getName()
             << ", Index: " << i
             << ", Damage: " << weapons[i]->getDamage()
             << ", Critical Hit Rate: " << weapons[i]->getCriticalHitRate()
             << ", Equipped: " << (entity.isWeaponEquipped(weapons[i]) ? "Yes" : "No") << "\n\r";
    }
}

void Backpack::printArmors(const Entity& entity) const {
    std::cout << "Armors in Backpack: " << "\n\r";
    if (armors.empty()) {
        std::cout << "- No Armors here" << "\n\r";
        return;
    }
    for (size_t i = 0; i < armors.size(); ++i) {
        std::cout << "- " << "Name: " << armors[i]->getName()
             << ", Index: " << i
             << ", Defense: " << armors[i]->getDefense()
             << ", Equipped: " << (entity.isArmorEquipped(armors[i]) ? "Yes" : "No") << "\n\r";
    }
}

void Backpack::printItems(const Entity& entity) const {
    std::cout << "Items in Backpack: " << "\n\r";
    if (entity.getBackpack().getItemSize() == 0) {
        std::cout << "- No Items here" << "\n\r";
        return;
    }
    std::unordered_set<std::string> printedItemNames;
    for (size_t i = 0; i < entity.getBackpack().getItemSize(); ++i) {
        std::shared_ptr<const Item> item = entity.getBackpack().getItem(i);
        // Skip the item if its name is already in the set
        if (printedItemNames.find(item->getName()) != printedItemNames.end()) {
            continue;
        }
        // Otherwise, add its name to the set and print it
        printedItemNames.insert(item->getName());
        std::cout << "- " << item->getName() << " Index: "<<i<< " [Description: '" << item->getDescription() 
        << "', Remain Count: " << entity.getBackpack().getItemCount(item->getName()) << "]" << "\n\r";
    }
}


std::map<std::string, Medicine>& GetMedicineMap() {
    static std::map<std::string, Medicine> m = {
            {"Bandage", {"Bandage", "A simple bandage to patch up wounds.", 5, 25}},
            {"Small Health Kit", {"Small Health Kit", "A small first aid kit that restores a small amount of health.", 10, 35}},
            {"Large Health Kit", {"Large Health Kit", "A large first aid kit that restores a significant amount of health.", 20, 50}},
            {"Antidote", {"Antidote", "A medicine that cures poison.", 30, 80}},
    };
    return m;
}

std::map<std::string, GrenadeGadget>& GetGrenadeGadgetMap() {
    static std::map<std::string, GrenadeGadget> m = {
            {"Basic Grenade", {"Basic Grenade", "A simple explosive grenade.", 50, 40, 3}}, //cost, damage 
            {"Cluster Grenade", {"Cluster Grenade", "An explosive grenade that splits into smaller bomblets.", 80, 60, 5}},
            {"Incendiary Grenade", {"Incendiary Grenade", "An explosive grenade that sets the area on fire.", 120, 80, 3}},
    };
    return m;
}

std::map<std::string, ShieldGadget>& GetShieldGadgetMap() {
    static std::map<std::string, ShieldGadget> m = {
            {"Level 1 Shield", {"Level 1 Shield", "Level 1 Shield desc", 80, 1}},
            {"Level 2 Shield", {"Level 2 Shield", "Level 2 Shield desc", 100, 2}},
            {"Level 3 Shield", {"Level 3 Shield", "Level 3 Shield desc", 120, 3}},
    };
    return m;
}

std::map<std::string, Weapon>& GetWeaponMap() {
    static std::map<std::string, Weapon> m = {
            {"Pacifier", {"Pacifier", "A lightweight handgun for precise shots in tight spaces.", 30, 0.05, 50, {1, 0}}},
            {"Myco-Mauler", {"Myco-Mauler", "A massive sledgehammer for crushing large foes and obstacles.", 40, 0.1, 80, {1, 0}}},
            {"Reclaimer", {"Reclaimer", "A versatile, scoped rifle for accurate medium to long-range engagements.", 45, 0.3, 100, {1, 2}}},
            {"Fungus Cleaver", {"Fungus Cleaver", "A heavy-duty chopping weapon for powerful close-range strikes.", 60, 0.15, 130, {1, 2}}},
            {"Spore Scourge", {"Spore Scourge", "A pump-action shotgun for decimating enemies at close range.", 95, 0.05, 185, {1, 2}}},
            {"Cordyceps Crippler", {"Cordyceps Crippler", "A silent crossbow for stealthy assassinations with neurotoxin bolts.", 120, 0.3, 250, {1, 2}}},
            {"Fungus Fumigator", {"Fungus Fumigator", "A close-range flamethrower for incinerating groups of infected enemies.", 180, 0.05, 350, {2, 3}}},
            {"Spore Sniper", {"Spore Sniper", "A high-caliber rifle for extreme long-range precision.", 180, 0.5, 380, {2, 3}}},
            {"Parasite Pulverizer", {"Parasite Pulverizer", "A rapid-fire submachine gun for dispatching groups of weaker enemies.", 230, 0.05, 440, {3, 9}}},
            {"Ion-Infector", {"Ion-Infector", "This powerful rifle harnesses electric energy to neutralize multiple enemies at once.", 250, 0.3, 500, {3, 9}}}
    };
    return m;
}

std::map<std::string, Armor>& GetArmorMap() {
    static std::map<std::string, Armor> m = {
            {"HKU Lab Coat", {"HKU Lab Coat", "Provides low defense and slightly increased mobility, a memento from the fallen researchers.", 5, 50, {1,0}}},
            {"Hazmat Suit", {"Hazmat Suit", "Offers moderate defense and mobility with added protection against Cordyceps infection.", 15, 100, {1,2}}},
            {"Reinforced Hazmat Suit", {"Reinforced Hazmat Suit", "Improved version of the Hazmat Suit with increased defense and mobility.", 25, 150, {2,3}}},
            {"Exoskeleton", {"Exoskeleton", "The ultimate armor with the highest defense, but significantly decreased mobility.", 40, 200, {2,3}}}
    };
    return m;
}

std::map<std::string, Entity>& GetEntityMap() {
    //random LabToken from 30-80 for player
    int random_integer = rand() % 51 + 30;
    static std::map<std::string, Entity> m = {
            {"Li'ang", {"Li'ang", 200, 10, 0.1, 0.1, random_integer}},
            {"Claire", {"Claire", 80, 20, 0.5, 0.5, random_integer}},
            {"King'Ada", {"King'Ada", 100, 20, 0.2, 0.2, random_integer}},
            {"Jill", {"Jill", 120, 20, 0.2, 0.2, random_integer}},
            {"Alice", {"Alice", 999, 999, 1.0, 1.0, random_integer}}
    };
    return m;
}

std::shared_ptr<GrenadeGadget> BuyGrenadeGadget(std::string name) {
    GrenadeGadget& gadget = GetGrenadeGadgetMap()[name];
    return std::make_shared<GrenadeGadget>(gadget.getName(), gadget.getDescription(), gadget.getCost(), gadget.getDamage(), gadget.getTargets());
}

std::shared_ptr<ShieldGadget> BuyShieldGadget(std::string name) {
    ShieldGadget& gadget = GetShieldGadgetMap()[name];
    return std::make_shared<ShieldGadget>(gadget.getName(), gadget.getDescription(), gadget.getCost(), gadget.getLevel());
}

std::shared_ptr<Medicine> BuyMedicine(std::string name) {
    Medicine& medicine = GetMedicineMap()[name];
    return std::make_shared<Medicine>(medicine.getName(), medicine.getDescription(), medicine.getCost(), medicine.getHealingAmount());
}

std::shared_ptr<Weapon> BuyWeapon(std::string name) {
    Weapon& weapon = GetWeaponMap()[name];
    return std::make_shared<Weapon>(weapon.getName(), weapon.getDescription(), weapon.getDamage(), weapon.getCriticalHitRate(), weapon.getCost(), weapon.getLevels());
}

std::shared_ptr<Armor> BuyArmor(std::string name) {
    Armor& armor = GetArmorMap()[name];
    return std::make_shared<Armor>(armor.getName(), armor.getDescription(), armor.getDefense(), armor.getCost(), armor.getLevels());
}

Entity BuyEntity(std::string name) {
    Entity& entity = GetEntityMap()[name];
    return Entity(entity.getName(), entity.getHealth(), entity.getAttackPower(), entity.getDodgeRate(), entity.getCriticalHitRate(), entity.getLabTokens());
}


class CombatSystem {
public:
    CombatSystem(Entity& player, Entity zombies[], int numZombies)
            : player(player), zombies(zombies), numZombies(numZombies) {}

    void start() {
        while (player.isAlive() && anyZombiesAlive()) {
            printCombatStats(std::vector<Entity *>());

            playerTurn();

            if (!player.isAlive() || !anyZombiesAlive()) {
                break;
            }
            sleep(1);
            zombiesTurn();
            sleep(1);
        }
    }

private:
    bool anyZombiesAlive() {
        for (int i = 0; i < numZombies; ++i) {
            if (zombies[i].isAlive()) {
                return true;
            }
        }
        return false;
    }

    void printCombatStats(std::vector<Entity *> vector) {
        player.printInfo();
        for (int i = 0; i < numZombies; ++i) {
            std::cout << zombies[i].getName() << "'s HP: " << zombies[i].getHealth() << "\n\r";
        }
    }
    void playerTurn() {
        system(STTY_DEF TTY_PATH);
        std::string action;
        std::cout << "Choose an action (attack(a)/item(i)/run(r)): ";
        std::cin >> action;
        if (action == "a") {
            int targetIndex = selectTarget();
            int damage = player.attack(zombies[targetIndex]);
            sleep(1);
            std::cout << "You dealt " << damage << " damage to " << zombies[targetIndex].getName() << "!\n\r";
            zombies[targetIndex].takeDamage(damage);
            sleep(2);
            printf("\033[H\033[2J\033[3J");
            if (!zombies[targetIndex].isAlive()) {
                std::cout << zombies[targetIndex].getName() << " has been defeated!\n\r";
                zombies[targetIndex].transferLabTokens(player);
                sleep(1);
                std::cout << "You received " << zombies[targetIndex].getLabTokens() << " LabTokens from " << zombies[targetIndex].getName() << "!\n\r";
            }
        } else if (action == "i") {
            // Put zombies into a vector for easier handling
            std::vector<Entity*> enemies;
            for (int i = 0; i < numZombies; ++i) {
                enemies.push_back(&zombies[i]);
            }
            player.combatInteraction(enemies);
            //combatInteraction
        } else if (action == "r") {
            std::cout << "There is no way to run!\n\r";
            usleep(1000000);
            std::printf("\033[H\033[2J\033[3J");
        } else {
            std::printf("\033[H\033[2J\033[3J");
            std::cout << "Invalid action. Try again.\n\r";
            start();
        }
        system(STTY_US TTY_PATH);
    }

    int selectTarget() {
        int targetIndex;
        while (true) {
            std::cout << "Select target (1-" << numZombies << "): ";
            std::cin >> targetIndex;
            if (std::cin.fail()) {
                std::cout << "Invalid input. Please enter an integer." << "\n\r";
                std::cin.clear();                // clear error flag on cin object
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // discard invalid input
                continue;                   // loop back and prompt for input again
            }
            if (targetIndex >= 1 && targetIndex <= numZombies) {
                break;  // Exit the loop if input is within range
            }
            std::cout << "Invalid input. Please try again." << "\n\r";
        }
        targetIndex--; // Adjust for 0-based indexing
        if (!zombies[targetIndex].isAlive()) {
            std::cout << "The target zombie is already dead, don't waste your bullet!" << "\n\r";
            targetIndex = selectTarget();
        }
        return targetIndex;
    }

    void zombiesTurn() {
        for (int i = 0; i < numZombies; ++i) {
            if (zombies[i].isAlive()) {
                int damage = zombies[i].attack(player);
                std::cout << zombies[i].getName() << " dealt " << damage << " damage to you!\n\r";
                sleep(1);
                // Check if the player has remaining shield rounds
                if (player.getRemainingShieldRounds() > 0) {
                    player.updateEffects(); // Reduce shield charge
                    std::cout << "Your shield absorbed the damage!\n\r";
                } else {
                    player.takeDamage(damage);
                }
                if (!player.isAlive()) {
                    std::cout << "You have been defeated!\n\r";
                    break;
                }
            }
        }
    }

    Entity& player;
    Entity* zombies;
    int numZombies;
};

void shop(Entity& hero){
  
  
  int playerLevel = hero.getLevel();
  
  // create store items
  auto& weaponmap = GetWeaponMap();
  auto& armormap = GetArmorMap();
  auto& grenadegadgetmap = GetGrenadeGadgetMap();
  auto& shieldgadgetmap = GetShieldGadgetMap();
  auto& medicinemap = GetMedicineMap();
  
  int k;
  system(STTY_DEF TTY_PATH);
  // Display the store menu
  while (true) {
    std::cout << "\n\r" << "===== THE STORE =====\n\r";
    std::cout << "1. Weapons\n\r";
    std::cout << "2. Armor\n\r";
    std::cout << "3. Medicine\n\r";
    std::cout << "4. Gadgets\n\r";
    std::cout << "5. Recycling store\n\r";
    std::cout << "6. Quit\n\r";
    std::cout << "=====================\n\r" << "\n\r";
    std::cout << "You have " << hero.getLabTokens() << " labTokens. What would you like to do? ";

    int choice;
    std::cin >> choice;
    
    if (std::cin.fail()) {
      std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    else {
      switch (choice) {
        case 1: // Weapons
          std::printf("\033[H\033[2J\033[3J");
          std::cout << "\n\r" << "===================== WEAPONS =====================\n\r";
          k = 0;
          for (auto it = weaponmap.begin(); it != weaponmap.end(); ++it) {
          
            auto& weapon = it->second;
            
            k ++;
            std::cout << "- " << k << " - " << weapon.getName() << " - " << weapon.getDescription() << "\n\r";
            std::cout << " Cost: " << weapon.getCost() << "  Damage: " << weapon.getDamage() << "  Critical hit rate: " << weapon.getCriticalHitRate() << "\n\r";
            std::cout << "---------------------------------------------------" << "\n\r"; 
          }
        
          std::cout << "\n\r" << "Which weapon would you like to buy (enter -1 to go back)? ";
          int weaponChoice;
          std::cin >> weaponChoice;
        
          if (std::cin.fail()) {
            std::cout << "\n\r" << "Invalid choice. Please try again." << "\n\r";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
          }
        
          else if (weaponChoice > 0 && weaponChoice <= weaponmap.size() && weaponChoice <= k) {
            auto it = weaponmap.begin();
            advance(it, weaponChoice - 1);
            auto weapon = std::make_shared<Weapon>(it->second);
          
            if (weapon->getCost() > hero.getLabTokens()) {
              std::printf("\033[H\033[2J\033[3J");
              std::cout << "\n\r" << "You don't have enough labTokens to buy this weapon." << "\n\r";
              break;
              
            
            } else {
              hero.setLabTokens(hero.getLabTokens() - weapon->getCost());
              
              hero.getBackpack().addWeapon(BuyWeapon(weapon->getName()));
              std::cout << "\n\r" << "You bought " << weapon->getName() << " for " << weapon->getCost() << " labTokens." 
              << "\n\r";
              usleep(1000000);
              std::printf("\033[H\033[2J\033[3J");
              break;
              
            }
          } else if (weaponChoice == -1) {
            std::printf("\033[H\033[2J\033[3J");
            std::cout << "\n\r" << "Returning..." << "\n\r";
            usleep(1000000);
            std::printf("\033[H\033[2J\033[3J");
            break;
          
          } else {
            std::cout << "\n\r" << "Invalid choice. Please try again." << "\n\r";
            break;
            
          }
                
        
        case 2: // Armor
          std::printf("\033[H\033[2J\033[3J");
          std::cout << "\n\r" << "====================== ARMOR ======================\n\r";
          k = 0;
          for (auto it = armormap.begin(); it != armormap.end(); ++it) {
          
            auto& armor = it->second;
           
            k ++;
            std::cout << "- " << k << " - " << armor.getName() << " - " << armor.getDescription() << "\n\r";
              
            std::cout << "  Cost: " << armor.getCost() << "  Protection: " << armor.getDefense() << "\n\r";
            std::cout << "---------------------------------------------------" << "\n\r";
          
          }
        
          std::cout <<"\n\r" << "Which armor would you like to buy (enter -1 to go back)? ";
          int armorChoice;
          std::cin >> armorChoice;
        
          if (std::cin.fail()) {
            std::cout << "\n\r" << "Invalid choice. Please try again." << "\n\r";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
          }
        
          else if (armorChoice > 0 && armorChoice <= armormap.size() && armorChoice <= k) {
            auto it = armormap.begin();
            advance(it, armorChoice - 1);
            auto armor = std::make_shared<Armor>(it->second);
          
            if (armor->getCost() > hero.getLabTokens()) {
                std::printf("\033[H\033[2J\033[3J");
                std::cout << "\n\r" << "You don't have enough labTokens to buy this armor." << "\n\r";
                break;
                
            
            } else {
              hero.setLabTokens(hero.getLabTokens() - armor->getCost());
              hero.getBackpack().addArmor(BuyArmor(armor->getName()));
              std::cout << "\n\r" << "You bought " << armor->getName() << " for " << armor->getCost() 
              << " labTokens." << "\n\r";
              usleep(1000000);
              std::printf("\033[H\033[2J\033[3J");
              break;
             
            }
          } else if (armorChoice == -1) {
            std::printf("\033[H\033[2J\033[3J");
            std::cout << "\n\r" << "Returning..." << "\n\r";
            usleep(1000000);
            std::printf("\033[H\033[2J\033[3J");
            break;
          
          } else {
            std::cout << "\n\r"<< "Invalid choice. Please try again!" << "\n\r";
            break;
          }
        
                
          
        case 3: // Medicine
          std::printf("\033[H\033[2J\033[3J");
          std::cout <<"\n\r" << "===================== MEDICINE =====================\n\r";
          k = 0;
          for (auto it = medicinemap.begin(); it != medicinemap.end(); ++it) {
            auto& medicine = it->second;
            k++;
            std::cout << "- " << k << " - " << medicine.getName() << " - " << medicine.getDescription() << "\n\r";
            std::cout << "  Cost: " << medicine.getCost() << "  Healing: " << medicine.getHealingAmount() << "\n\r";
            std::cout << "----------------------------------------------------" << "\n\r";
          }

          std::cout << "\n\r" << "Which medicine would you like to buy (enter -1 to go back)? ";
          int medicineChoice;
          std::cin >> medicineChoice;

          if (std::cin.fail()) {
            std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
          }

          else if (medicineChoice > 0 && medicineChoice <= medicinemap.size() && medicineChoice <= k) {
            auto it = medicinemap.begin();
            advance(it, medicineChoice - 1);
            auto medicine = std::make_shared<Medicine>(it->second);
            if (medicine->getCost() > hero.getLabTokens()) {
              std::printf("\033[H\033[2J\033[3J");
              std::cout << "\n\r" << "You don't have enough labTokens to buy this medicine.\n\r";
              break;
            } else {
              hero.setLabTokens(hero.getLabTokens() - medicine->getCost());
              hero.getBackpack().addItem(BuyMedicine(medicine->getName()));
              std::cout << "\n\r" << "You bought " << medicine->getName() << " for " 
              << medicine->getCost() << " labTokens." << "\n\r";
              usleep(1000000);
              std::printf("\033[H\033[2J\033[3J");
              break;
            }

          } else if (medicineChoice == -1) {
            std::printf("\033[H\033[2J\033[3J");
            std::cout << "\n\r" << "Returning..." << "\n\r";
            usleep(1000000);
            std::printf("\033[H\033[2J\033[3J");
            break;

          } else {
            std::cout << "\n\r" << "Invalid choice. Please try again!" << "\n\r";
            break;
          }

        
          
        case 4: // Gadgets
          std::printf("\033[H\033[2J\033[3J");
          std::cout << "\n\r" <<"====== GADGETS ======\n\r";
        
          std::cout << "1. Grenade gadget\n\r";
          std::cout << "2. Shield gadget\n\r";
        
        
          std::cout << "\n\r" << "What would you like to buy (enter -1 to go back)? ";
          int choicegadget;
          std::cin >> choicegadget;
        
          if (std::cin.fail()) {
            std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
          }
        
          else {
            switch (choicegadget) {
              case 1:
                std::printf("\033[H\033[2J\033[3J");
                std::cout << "\n\r" <<"=============== GRENADE GADGET ===============\n\r";
                k = 0;
                for (auto it = grenadegadgetmap.begin(); it != grenadegadgetmap.end(); ++it) {
              
                  auto& grenadegadget = it->second;
                  k ++;
                  std::cout << "- " << k << " - " << grenadegadget.getName() << " - " << grenadegadget.getDescription() << "\n\r";
                  std::cout << "  Cost: " << grenadegadget.getCost() << "  Damage: " << grenadegadget.getDamage() << "\n\r";
                  std::cout << "----------------------------------------------" << "\n\r";
                }
          
                std::cout << "\n\r" << "Which grenade gadge would you like to buy (enter -1 to go back)? ";
                int grenadegadgetChoice;
                std::cin >> grenadegadgetChoice;
            
                if (std::cin.fail()) {
                  std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
                  std::cin.clear();
                  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                  break;
                }
            
                else if (grenadegadgetChoice > 0 && grenadegadgetChoice <= grenadegadgetmap.size() && grenadegadgetChoice <= k) {
                  auto it = grenadegadgetmap.begin();
                  advance(it, grenadegadgetChoice - 1);
                  auto grenadegadget = std::make_shared<GrenadeGadget>(it->second);
                  if (grenadegadget->getCost() > hero.getLabTokens()) {
                    std::printf("\033[H\033[2J\033[3J");
                    std::cout << "\n\r" << "You don't have enough labTokens to buy this grenade gadge.\n\r";
                    break;
                  } else {
                    hero.setLabTokens(hero.getLabTokens() - grenadegadget->getCost());
                    hero.getBackpack().addItem(BuyGrenadeGadget(grenadegadget->getName()));
                    std::cout << "\n\r" << "You bought " << grenadegadget->getName() << " for " 
                    << grenadegadget->getCost() << " labTokens.\n\r";
                    usleep(1000000);
                    std::printf("\033[H\033[2J\033[3J");
                    break;
                  }
              
                } else if (grenadegadgetChoice == -1) {
                  std::printf("\033[H\033[2J\033[3J");
                  std::cout << "\n\r" << "Returning...\n\r";
                  usleep(800000);
                  std::printf("\033[H\033[2J\033[3J");
                  break;
              
                } else {
                  std::cout << "\n\r" << "Invalid choice. Please try again!\n\r";
                  break;
                }
            
          
              case 2:
                std::printf("\033[H\033[2J\033[3J");
                std::cout << "\n\r" <<"============= SHIELD GADGET =============\n\r";
                k = 0;
                for (auto it = shieldgadgetmap.begin(); it != shieldgadgetmap.end(); ++it) {
              
                  auto& shieldgadget = it->second;
                  k ++;
                  
                 std::cout << "- " << k << " - " << shieldgadget.getName() << " - " << shieldgadget.getDescription() << "\n\r";
                 std::cout << "  Cost: " << shieldgadget.getCost() << "\n\r";
                 std::cout << "-----------------------------------------" << "\n\r";
                }
          
                std::cout << "\n\r" << "Which shield gadget would you like to buy (enter -1 to go back)? ";
                int shieldgadgetChoice;
                std::cin >> shieldgadgetChoice;
            
                if (std::cin.fail()) {
                  std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
                  std::cin.clear();
                  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                  break;
                }
            
                else if (shieldgadgetChoice > 0 && shieldgadgetChoice <= shieldgadgetmap.size() && shieldgadgetChoice <= k) {
                  auto it = shieldgadgetmap.begin();
                  advance(it, shieldgadgetChoice - 1);
                  auto shieldgadget = std::make_shared<ShieldGadget>(it->second);
                  if (shieldgadget->getCost() > hero.getLabTokens()) {
                    std::printf("\033[H\033[2J\033[3J");
                    std::printf("\033[H\033[2J\033[3J");
                    std::cout << "\n\r" << "You don't have enough labTokens to buy this shield gadge.\n\r";
                    break;
                  } else {
                    hero.setLabTokens(hero.getLabTokens() - shieldgadget->getCost());
                    hero.getBackpack().addItem(BuyShieldGadget(shieldgadget->getName()));
                    std::cout << "\n\r" << "You bought " << shieldgadget->getName() << " for " << shieldgadget->getCost() 
                    << " labTokens.\n\r";
                    usleep(1000000);
                    std::printf("\033[H\033[2J\033[3J");
                    break;
                  }
              
                } else if (shieldgadgetChoice == -1) {
                  std::printf("\033[H\033[2J\033[3J");
                  std::cout << "\n\r" << "Returning...\n\r";
                  usleep(800000);
                  std::printf("\033[H\033[2J\033[3J");
                  break;
              
                } else {
                  std::cout << "\n\r"
                            << "Invalid choice. Please try again!\n\r";
                  break;
                }
            
            
              case -1:
                std::printf("\033[H\033[2J\033[3J");
                std::cout << "\n\r" << "Returning...\n\r";
                usleep(800000);
                break;
            
              default:
                std::cout << "\n\r"<< "Invalid choice. Please try again.\n\r";
                break;
          
            }
            break;
          }
          
     
        case 5:
          std::printf("\033[H\033[2J\033[3J");
          std::cout << "\n\r" << "====== RECYCLING STORE ======\n\r";
          std::cout << "1. Weapon\n\r";
          std::cout << "2. Armor\n\r";
        
        
          std::cout << "\n\r" << "What would you like to recycle (enter -1 to go back)? ";
          int sell;
          std::cin >> sell;
        
          if (std::cin.fail()) {
            std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
          }
              
          else if (sell == -1) {
            std::printf("\033[H\033[2J\033[3J");
            std::cout << "\n\r" << "Returning...\n\r";
            usleep(800000);
            std::printf("\033[H\033[2J\033[3J");
            break;
          }  
        
          else {
            switch (sell) {
              case 1:
                std::printf("\033[H\033[2J\033[3J");
                hero.getBackpack().printWeapons(hero);
                std::cout << "\n\r" << "Which item would you like to recycle (enter -1 to go back)? ";
                int recycleweapon;
                std::cin >> recycleweapon;

                if (std::cin.fail()) {
                  std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
                  std::cin.clear();
                  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                  break;
                }

                // Remove and sell the selected weapon or armor
                else if (recycleweapon >= 0 && recycleweapon <= hero.getBackpack().weaponSize()) {
                  std::shared_ptr<Weapon> weapon = hero.getBackpack().getWeapon(recycleweapon);
                  hero.getBackpack().removeWeapon(recycleweapon);
                  int sellPrice = weapon->getCost() / 2;
                  hero.setLabTokens(hero.getLabTokens() + sellPrice);
                  std::cout << "\n\r" << "You recycled " << weapon->getName() << " for " << sellPrice << " labTokens.\n\r";
                  usleep(1000000);
                  std::printf("\033[H\033[2J\033[3J");
                  break;
                }
                else if (recycleweapon == -1) {
                  std::printf("\033[H\033[2J\033[3J");
                  std::cout << "\n\r" << "Returning...\n\r";
                  usleep(800000);
                  std::printf("\033[H\033[2J\033[3J");
                  break;
                }
                else {
                  std::cout << "\n\r" << "Invalid choice. Please try again!\n\r";
                  break;
                }

              case 2:
                std::printf("\033[H\033[2J\033[3J");
                hero.getBackpack().printArmors(hero);
                
                std::cout << "\n\r" << "Which item would you like to recycle (enter -1 to go back)? ";
                int recyclearmor;
                std::cin >> recyclearmor;

                if (std::cin.fail()) {
                  std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
                  std::cin.clear();
                  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                  break;
                }

                // Remove and sell the selected weapon or armor
                else if (recyclearmor >= 0 && recyclearmor <= hero.getBackpack().armorSize()) {
                  std::shared_ptr<Armor> armor = hero.getBackpack().getArmor(recyclearmor);
                  hero.getBackpack().removeArmor(recyclearmor);
                  int sellPrice = armor->getCost() / 2;
                  hero.setLabTokens(hero.getLabTokens() + sellPrice);
                  std::cout << "\n\r" << "You recycled " << armor->getName() << " for " << sellPrice << " labTokens." << "\n\r";
                  usleep(1000000);
                  std::printf("\033[H\033[2J\033[3J");
                  break;
                }
                else if (recyclearmor == -1) {
                  std::printf("\033[H\033[2J\033[3J");
                  std::cout << "\n\r" << "Returning...\n\r";
                  usleep(800000);
                  std::printf("\033[H\033[2J\033[3J");
                  break;
                }
                else {
                  std::cout << "\n\r" << "Invalid choice. Please try again!\n\r";
                  break;
                }
              
              default:
                std::printf("\033[H\033[2J\033[3J");
                std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
                break;
            }
          break;
          }
          
        case 6: // Quit
          std::printf("\033[H\033[2J\033[3J");
          std::cout << "\n\r" << "Thanks for shopping!\n\r" << "\n\r";
          usleep(500000);
          system(STTY_US TTY_PATH);
          return;
          
        default:
          std::cout << "\n\r" << "Invalid choice. Please try again.\n\r";
          break;
    
      }
    }
  }
}


std::vector<std::string> Map1 ={
    "###########################################",
    "#.........................................#",
    "#................######...................#",
    "#................#HOME#...................#",
    "#................######...................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#...................................###...#",
    "#....................................@#...#",
    "#...................................###...#",
    "#................###.##...................#",
    "#................#SHOP#...................#",
    "#................######...................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "###########################################",
};
std::vector<std::string> Map2 ={
    "###########################################",
    "#.........................................#",
    "#..................#####.....#####........#",
    "#....................M.#.......F.#........#",
    "#..................#####.....#####........#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#..................#####............###...#",
    "#....................D.#.............@#...#",
    "#..................#####............###...#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.................#####......#####........#",
    "#...................C.#........P.#........#",
    "#.................#####......#####........#",
    "#.........................................#",
    "###########################################",
};
std::vector<std::string> Map3 ={
    "###########################################",
    "#.........................................#",
    "#..................#####.....#####........#",
    "#...................1E.#.......2E#........#",
    "#..................#####.....#####........#",
    "#.........................................#",
    "#.........................................#",
    "#..................#####..................#",
    "#....................3E#..................#",
    "#..................#####............###...#",
    "#....................................@#...#",
    "#...................................###...#",
    "#.................#####......#####........#",
    "#...................4E#........5E#........#",
    "#.................#####......#####........#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "###########################################",
};
std::vector<std::string> Map4 ={
    "###########################################",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#................BBB......................#",
    "#................B..B.....................#",
    "#................B..B.....................#",
    "#................BBB..............####....#",
    "#................B..B...............@#....#",
    "#................B..B.............####....#",
    "#................BBB......................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "#.........................................#",
    "###########################################",
};

std::vector<std::string> maze = {
    "###########################################",
    "#S#.............#.........................#",
    "#.#.#########.###.###.###############.#####",
    "#.#.#.......#.....#...#...........#.@.....#",
    "#.#.#.#####.#######.###.###########.#######",
    "#...#.#...#.#...........#.......#.#.......#",
    "##.##.#.#.###.###########.#####.#.#.#####.#",
    "#.....#.#...#.#...........#.....#.#.#...#.#",
    "#.#######.###.###.####.####.#####.#.#.#.#.#",
    "#.#.......#...#.#.#.....#...#...#.#.#.#.#.#",
    "#.#####.###.###.#.###.#.###.###.#.#.#.#.#.#",
    "#.....#.#...#...#.#...#.#...#...#.#.#...#.#",
    "#####.#.###.###.#.#.###.###.#.###.#.#####.#",
    "#.....#.#.......#.#.#.#...#.#.#...#.......#",
    "#.#####.#########.#.#.###.#.#.#.#########.#",
    "#.#...............#.#.....#...#.#.........#",
    "#.###.######.######.###########.#.#########",
    "#...#.#.........#...#...........#.#.......#",
    "###.#.###.#####.###.#.###########.#.#####.#",
    "###########################################",
};

void print_maze(const std::vector<std::string>& mapc, int &plxc, int &plyc) {
    for (int s = 0; s < HEIGH; ++s) {
        for (int m = 0; m < WIDT; ++m) {
            if (s == plyc && m == plxc) {
                printf("\033[40;31mP\033[0m");
            } else {
                std::cout << mapc[s][m];
            }
        }
        std::cout << "\n\r";
    }
}


//check zombies whether is alive.
bool check_Zombies(int a[],int num){
    if( a[num] == 1){
        return true;
    }else{
        return false;
    }
}

bool check(const std::vector<std::string>& map,int &plx, int &ply,Entity& player, int a[], char input){
    int new_x = plx, new_y = ply;
    
    
    if (input == 'w') {
        new_y--;
    } else if (input == 's') {
        new_y++;
    } else if (input == 'a') {
        new_x--;
    } else if (input == 'd') {
        new_x++;
    }
    
    if (map[new_y][new_x] != '#') {
        plx = new_x;
        ply = new_y;
        
    }
    
    if (map[new_y][new_x] == 'O'){
        printf("\033[H\033[2J\033[3J");
        shop(player);
        return true;
    }
    if (map[new_y][new_x] == 'M'){
        if (check_Zombies(a, 0)){
            Entity zombiesM[2] = {
                Entity("Male Zombie", 50, 10, 0.05, 0.10, 40), // 5% dodge rate, 10% critical hit rate
                Entity("Student Zombie", 40, 8, 0.05, 0.10, 40)  // 5% dodge rate, 10% critical hit rate
            };
            CombatSystem combatSystem(player, zombiesM, 2);
            printf("\033[H\033[2J\033[3J");
            std::cout << "here is your LabTokens: "<<player.getLabTokens()<< "\n\r";
            combatSystem.start();
            if (!zombiesM->isAlive()){
                a[0] = 0;
            }
            return player.isAlive();
        }
        
    }

    if (map[new_y][new_x] == 'F'){
        if (check_Zombies(a, 1)){
            Entity zombiesF[2] = {
                Entity("Female Zombie", 50, 15, 0.05, 0.10, 40), // 5% dodge rate, 10% critical hit rate
                Entity("Student Zombie", 40, 8, 0.05, 0.10, 40)  // 5% dodge rate, 10% critical hit rate
            };
            CombatSystem combatSystem(player, zombiesF, 2);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombiesF->isAlive()){
                a[1] = 0;
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
      
    }
    if (map[new_y][new_x] == 'P'){
        if (check_Zombies(a, 2)){
            Entity zombiesP[1] = {
                Entity("Professor Zombie", 60, 15, 0.05, 0.10, 60)
            };
            CombatSystem combatSystem(player, zombiesP, 1);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombiesP->isAlive()){
                a[2] = 0;
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
        
    }
    if (map[new_y][new_x] == 'D'){
        if (check_Zombies(a, 3)){
            Entity zombiesD[2] = {
                Entity("Dog Zombie", 30, 20, 0.05, 0.10, 40),
                Entity("Dog Zombie", 30, 20, 0.05, 0.10, 40)
            };
            CombatSystem combatSystem(player, zombiesD, 2);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombiesD->isAlive()){
                a[3] = 0;
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
        
    }
    if (map[new_y][new_x] == 'C'){
        if (check_Zombies(a, 4)){
            Entity zombiesD[2] = {
                Entity("Dog Zombie", 40, 20, 0.05, 0.10, 40),
                Entity("Dog Zombie", 40, 20, 0.05, 0.10, 40)
            };
            CombatSystem combatSystem(player, zombiesD, 2);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombiesD->isAlive()){
                a[4] = 0;
            }
            return player.isAlive();
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
    }
    if (map[new_y][new_x] == '1'){
        if (check_Zombies(a, 0)){
            Entity zombies1[1] = {
                Entity("Elite Male Zombie", 120, 40, 0.10, 0.20, 80)
            };
            CombatSystem combatSystem(player, zombies1, 1);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombies1->isAlive()){
                a[0] = 0;
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
    }
    if (map[new_y][new_x] == '2'){
        if (check_Zombies(a, 1)){
            Entity zombies2[1] = {
                Entity("Elite Male Zombie", 120, 40, 0.10, 0.20, 80)
            };
            CombatSystem combatSystem(player, zombies2, 1);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombies2->isAlive()){
                a[1] = 0;
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
        
    }
    if (map[new_y][new_x] == '3'){
        if (check_Zombies(a, 2)){
            Entity zombies3[1] = {
                Entity("Elite Student Zombie", 100, 35, 0.10, 0.20, 80)
            };
            CombatSystem combatSystem(player, zombies3, 1);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombies3->isAlive()){
                a[2] = 0;
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
        
    }
    if (map[new_y][new_x] == '4'){
        if (check_Zombies(a, 3)){
            Entity zombies4[2] = {
                Entity("Elite Male Zombie", 120, 45, 0.10, 0.20, 80),
                Entity("Elite Female Zombie", 120, 40, 0.10, 0.20, 80)
                
            };
            CombatSystem combatSystem(player, zombies4, 2);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombies4->isAlive()){
                a[3] = 0;
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
        
    }
    if (map[new_y][new_x] == '5'){
        if (check_Zombies(a, 4)){
            Entity zombies5[1] = {
                Entity("Elite Professor Zombie", 240, 60, 0.10, 0.20, 80)
            };
            CombatSystem combatSystem(player, zombies5, 1);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens() << "\n\r";
            combatSystem.start();
            if (!zombies5->isAlive()){
                a[4] = 0;
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
        
    }
    if (map[new_y][new_x] == 'B'){
        if (check_Zombies(a, 0)){
            Entity zombiesB[1] = {
                Entity("Albert Wesker", 500, 100, 0.10, 0.20,10000)
            };
            CombatSystem combatSystem(player, zombiesB, 1);
            printf("\033[H\033[2J\033[3J");
            std::cout<<"here is your LabTokens: "<<player.getLabTokens()<<"\n\r";
            combatSystem.start();
            if (!zombiesB->isAlive()){
                for (int i = 0; i <= 4; i++){a[i] = 0;}
            }
            return player.isAlive();
        }else{
            std::cout << "This Zombie has been killed." << "\n\r";
            return player.isAlive();
        }
        
    }
    return true;
}
        
bool portal(const std::vector<std::string>& mapb,int &plxb, int &plyb){
    int new_xb = plxb, new_yb = plyb;
    if (mapb[new_yb][new_xb] == '@'){
        
        return true;
    }else{
        return false;
    };

}
 //each function for one map
bool PlayHome(const std::vector<std::string>& mapa, int &plxa, int &plya, Entity& playera){
    plxa = 1;
    plya = 1;
    int a[] = {0,0,0,0,0};
    while (true){
        
        char input;
        printf("\033[H\033[2J\033[3J");
        printMiniTitle();
        std::cout << std::endl;
        print_maze(mapa, plxa, plya);
        std::cout << "******************************************\n\r";
        std::cout << "Press q to exist, others to continue\n\r";
        std::cout << "Press w to up" << "   " << "|" << "Player Health:      " << playera.getHealth() << "\n\r";
        std::cout << "Press a to left" << " " << "|" << "Player AttackPower: " << playera.getAttackPower() << "\n\r";
        std::cout << "Press s to down" << " " << "|" << "Player DodgeRate:   " << playera.getDodgeRate() << "\n\r";
        std::cout << "Press d to right" << "|" << "Player CriticalHit: " << playera.getCriticalHitRate() << "\n\r";
        std::cout << "******************************************\n\r";
        std::cin >> input;
        if (input != 'q'){
            check(mapa,plxa,plya,playera,a,input);
            if (!playera.isAlive()){return false;}
            if (portal(mapa,plxa,plya)&& a[0]+a[1]+a[2]+a[3]+a[4] == 0){
                plx = 1;
                ply = 1;
                return true;
            }
            }else{
                std::cout << "You choose quit the game!!!" << "\n\r";
                return false;
        
        }
    }
}
bool PlayMap(const std::vector<std::string>& mapa, int &plxa, int &plya, Entity& playera){
    plxa = 1;
    plya = 1;
    int a[] = {1,1,1,1,1};
    while (true){
        
        char input;
        printf("\033[H\033[2J\033[3J");
        printMiniTitle();
        std::cout << std::endl;
        print_maze(mapa, plxa, plya);
        std::cout << "******************************************\n\r";
        std::cout << "Press q to exist, others to continue\n\r";
        std::cout << "Press w to up" << "   " << "|" << "Player Health:      " << playera.getHealth() << "\n\r";
        std::cout << "Press a to left" << " " << "|" << "Player AttackPower: " << playera.getAttackPower() << "\n\r";
        std::cout << "Press s to down" << " " << "|" << "Player DodgeRate:   " << playera.getDodgeRate() << "\n\r";
        std::cout << "Press d to right" << "|" << "Player CriticalHit: " << playera.getCriticalHitRate() << "\n\r";
        std::cout << "******************************************\n\r";
        std::cin >> input;
        if (input != 'q'){
            check(mapa,plxa,plya,playera,a,input);
            if (!playera.isAlive()){return false;}
            if (portal(mapa,plxa,plya)&& a[0]+a[1]+a[2]+a[3]+a[4] == 0){
                plx = 1;
                ply = 1;
                return true;
            }
            }else{
                std::cout << "You choose quit the game!!!" << "\n\r";
                return false;
        
        }
    }
}

int main(){

    system(STTY_US TTY_PATH);

    int plx = 1,ply = 1;

    std::printf("\033[H\033[2J\033[3J");
    
    printTitle();
    
    std::cout<<"\n\r";

    char c;
    c = get_char();
    if(c != 'q')
    {
        printf("\033[H\033[2J\033[3J");
        printIntro();
        sleep(1);
        std::cout << "\n\rPress Q to exist, others to continue, except Enter\n\r";
        std::cin >> c;
        system(STTY_DEF TTY_PATH);
        if(c == 'q') return 0;
        printf("\033[H\033[2J\033[3J");
       
        int choice = chooseCharacter();
        
        std::map<std::string, Entity>& entityMap = GetEntityMap();
        
        Entity player = BuyEntity(character[choice]);
        
        player.printInfo();
        system(STTY_US TTY_PATH);
        
        std::cout << "\n\rPress Q to exist, others to continue, except Enter\n\r";
        if(PlayHome(Map1, plx, ply, player)){
            if(PlayMap(Map2, plx, ply, player)){
                if (player.isAlive()){
                    if (PlayHome(Map1, plx, ply, player)){
                        if(PlayHome(maze, plx, ply, player)){
                            if (PlayMap(Map3, plx, ply, player)){
                                if (player.isAlive()){
                                    if(PlayHome(Map1, plx, ply, player)){
                                        if (PlayMap(Map4, plx, ply, player)){
                                            printGameWin();
                                        }else{
                                            printGameOver();
                                        }
                                        
                                    }else{
                                        printGameOver();
                                    }
                                    
                                }else{
                                    printGameOver();
                                }
                            }else{
                                printGameOver();
                            }
                        }else{
                            printGameOver();
                        }
                            
                    }else{
                        printGameOver();
                    }
                        
                }else{
                        printGameOver();
                }
            }else{
                printGameOver();
            }
        }else{
            printGameOver();
        }
        
       
       
    }
    
    system(STTY_DEF TTY_PATH);
    std::cout << "Thanks for playing\n\r";
    
    return 0;
}
