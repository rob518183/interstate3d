#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Vehicle;
class Weapon;
class GameWorld;

enum class AIState {
    IDLE,
    PATROL,
    CHASE,
    ATTACK,
    FLEE,
    EVADE,
    SEARCH,
    RETURN_TO_BASE,
    SEEK_COVER,
    FLANK,
    AMBUSH,
    RETREAT
};

enum class AIBehavior {
    AGGRESSIVE,
    DEFENSIVE,
    CAUTIOUS,
    SUICIDAL,
    TACTICAL
};

class AIController {
public:
    AIController();
    ~AIController();
    
    bool initialize(std::shared_ptr<Vehicle> vehicle, std::shared_ptr<GameWorld> world);
    void shutdown();
    
    void update(float deltaTime);
    void setTarget(std::shared_ptr<Vehicle> target);
    void clearTarget();
    
    // AI behavior
    void setBehavior(AIBehavior behavior) { m_behavior = behavior; }
    AIBehavior getBehavior() const { return m_behavior; }
    void setState(AIState state) { m_state = state; }
    AIState getState() const { return m_state; }
    
    // AI parameters
    void setAggressiveness(float aggressiveness) { m_aggressiveness = glm::clamp(aggressiveness, 0.0f, 1.0f); }
    void setSkill(float skill) { m_skill = glm::clamp(skill, 0.0f, 1.0f); }
    void setReactionTime(float reactionTime) { m_reactionTime = reactionTime; }
    void setSightRange(float range) { m_sightRange = range; }
    void setAttackRange(float range) { m_attackRange = range; }
    
    // Patrol points
    void addPatrolPoint(const glm::vec3& point);
    void clearPatrolPoints();
    void setPatrolLoop(bool loop) { m_patrolLoop = loop; }
    
    // Base location
    void setBaseLocation(const glm::vec3& location) { m_baseLocation = location; }
    const glm::vec3& getBaseLocation() const { return m_baseLocation; }
    
    // Team
    void setTeam(int team) { m_team = team; }
    int getTeam() const { return m_team; }
    
    // Decision making
    bool canSeeTarget() const;
    bool isInAttackRange() const;
    bool isUnderThreat() const;
    bool shouldFlee() const;
    bool shouldAttack() const;
    
    // Getters
    std::shared_ptr<Vehicle> getVehicle() const { return m_vehicle; }
    std::shared_ptr<Vehicle> getTarget() const { return m_target; }
    float getAggressiveness() const { return m_aggressiveness; }
    float getSkill() const { return m_skill; }
    
private:
    void updateState(float deltaTime);
    void updateMovement(float deltaTime);
    void updateCombat(float deltaTime);
    void updateTargeting(float deltaTime);
    
    // State-specific behaviors
    void executeIdleState(float deltaTime);
    void executePatrolState(float deltaTime);
    void executeChaseState(float deltaTime);
    void executeAttackState(float deltaTime);
    void executeFleeState(float deltaTime);
    void executeEvadeState(float deltaTime);
    void executeSearchState(float deltaTime);
    void executeReturnToBaseState(float deltaTime);
    void executeSeekCoverState(float deltaTime);
    void executeFlankState(float deltaTime);
    void executeAmbushState(float deltaTime);
    void executeRetreatState(float deltaTime);
    
    // Tactical helpers
    glm::vec3 findNearestCover() const;
    glm::vec3 calculateFlankPosition() const;
    bool isInAmbushPosition() const;
    bool isUnderHeavyFire() const;
    std::vector<glm::vec3> getTeamPositions() const;
    float calculateThreatLevel() const;
    glm::vec3 getAvoidanceVector() const;
    glm::vec3 getPredictedTargetPosition(float predictionTime) const;
    
    // Combat helpers
    void fireWeapons();
    void selectBestWeapon();
    void aimAtTarget();
    bool hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const;
    
    // Decision helpers
    AIState evaluateStateTransition();
    float calculateThreatLevel(const glm::vec3& position) const;
    glm::vec3 findBestCoverPosition() const;
    glm::vec3 findFlankingPosition() const;
    
    std::shared_ptr<Vehicle> m_vehicle;
    std::shared_ptr<GameWorld> m_world;
    std::shared_ptr<Vehicle> m_target;
    
    // AI state
    AIState m_state = AIState::IDLE;
    AIBehavior m_behavior = AIBehavior::CAUTIOUS;
    float m_stateTimer = 0.0f;
    
    // AI parameters
    float m_aggressiveness = 0.5f;    // 0 = passive, 1 = aggressive
    float m_skill = 0.5f;              // 0 = novice, 1 = expert
    float m_reactionTime = 0.5f;       // Seconds to react
    float m_sightRange = 100.0f;       // Detection range
    float m_attackRange = 50.0f;       // Preferred attack range
    float m_decisionTimer = 0.0f;
    float m_decisionInterval = 0.1f;   // Decision update frequency
    
    // Patrol
    std::vector<glm::vec3> m_patrolPoints;
    int m_currentPatrolIndex = 0;
    bool m_patrolLoop = true;
    
    // Base
    glm::vec3 m_baseLocation = glm::vec3(0.0f);
    float m_baseRadius = 50.0f;
    
    // Combat
    float m_lastFireTime = 0.0f;
    float m_fireRate = 1.0f;
    int m_selectedWeapon = 0;
    
    // Movement
    glm::vec3 m_currentTarget;
    float m_currentSpeed = 0.0f;
    float m_currentSteering = 0.0f;
    float m_currentBrake = 0.0f;
    
    // Team
    int m_team = 0;
    
    bool m_initialized = false;
};
