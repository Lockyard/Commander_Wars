#ifndef PLAYER_H
#define PLAYER_H

#include <QColor>
#include <QObject>
#include "oxygine-flow.h"
#include "game/smartpointers.h"
#include "game/smartCO.h"

#include "coreengine/fileserializable.h"

class CO;

class BaseGameInputIF;

class Player : public QObject, public oxygine::Actor, public FileSerializable
{
    Q_OBJECT

public:
    enum class Alliance
    {
        Friend,
        Enemy
    };
    Q_ENUM(Alliance)

    explicit Player(quint32 id);
    virtual ~Player() override;

    void setPlayerID(const quint32 &value);

    /**
     * @brief serialize stores the object
     * @param pStream
     */
    virtual void serialize(QDataStream& pStream) override;
    /**
     * @brief deserialize restores the object
     * @param pStream
     */
    virtual void deserialize(QDataStream& pStream) override;
    /**
     * @brief getVersion version of the file
     * @return
     */
    inline virtual qint32 getVersion() override
    {
        return 1;
    }
    /**
     * @brief getBaseGameInput pointer to the ai or human player interface
     * @return
     */
    inline BaseGameInputIF* getBaseGameInput()
    {
        return m_pBaseGameInput;
    }
    /**
     * @brief setBaseGameInput sets the player input
     * @param pBaseGameInput
     */
    void setBaseGameInput(BaseGameInputIF *pBaseGameInput);

signals:

public slots:
    /**
     * @brief getColor the color of this player
     * @return
     */
    QColor getColor() const;
    /**
     * @brief setColor sets the color of this player. Note this want update existing sprites
     * @param Color
     */
    void setColor(QColor Color);
    /**
     * @brief getPlayerID player id of this player from 0 to n
     * @return
     */
    quint32 getPlayerID() const;
    /**
     * @brief getArmy the army string id of this player.
     * @return
     */
    QString getArmy();
    /**
     * @brief isEnemy checks the alliance with this player
     * @param pPlayer the player we want to check if he's an enemy
     * @return the alliance of the player with us
     */
    Alliance checkAlliance(Player* pPlayer);
    /**
     * @brief isEnemyUnit checks if the given unit is an enemy
     * @param pUnit
     * @return
     */
    bool isEnemyUnit(Unit* pUnit);
    qint32 getFonds() const;
    /**
     * @brief addFonds increases the money of this player by the given value
     * @param value
     */
    void addFonds(const qint32 &value);
    void setFonds(const qint32 &value);
    /**
     * @brief earnMoney earns money based on the buildings the player has
     * @param modifier multiplier additionaly modifying the income
     */
    void earnMoney(float modifier = 1.0f);
    /**
     * @brief getCO
     * @param id index of the co 0 or 1
     * @return pointer to the co
     */
    CO* getCO(quint8 id);
private:
    quint32 playerID;
    qint32 fonds{0};
    QColor m_Color;
    spCO playerCOs[2] = {nullptr, nullptr};
    /**
     * @brief m_pBaseGameInput pointer to the ai or human player
     */
    BaseGameInputIF* m_pBaseGameInput{nullptr};
};

#endif // PLAYER_H