#include "ai/AIController.hpp"
#include "game/Vehicle.hpp"
#include "game/GameWorld.hpp"
#include "game/Weapon.hpp"
#include "core/Logger.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <random>
#include <cstdlib>

AIController::AIController() = default;

AIController::~AIController() {
    shutdown();
}

bool AIController::initialize(std::shared_ptr<Vehicle> vehicle, std::shared_ptr<GameWorld> world) {
    if (!vehicle || !world) {
        Logger::error("AIController initialization failed: null vehicle or world");
        return false;
    }
    
    m_vehicle = vehicle;
    m_world = world;
    
    // Set initial AI parameters based on vehicle type
    if (vehicle->getVehicleType() == "enemy") {
        m_behavior = AIBehavior::AGGRESSIVE;
        m_aggressiveness = 0.7f;
        m_skill = 0.6f;
    } else {
        m_behavior = AIBehavior::CAUTIOUS;
        m_aggressiveness = 0.3f;
        m_skill = 0.5f;
    }
    
    // Set base location to spawn position
    m_baseLocation = vehicle->getPosition();
    
    // Generate some patrol points around base
    addPatrolPoint(m_baseLocation + glm::vec3(50.0f, 0.0f, 0.0f));
    addPatrolPoint(m_baseLocation + glm::vec3(0.0f, 0.0f, 50.0f));
    addPatrolPoint(m_baseLocation + glm::vec3(-50.0f, 0.0f, 0.0f));
    addPatrolPoint(m_baseLocation + glm::vec3(0.0f, 0.0f, -50.0f));
    
    m_initialized = true;
    Logger::info("AIController initialized for vehicle: " + vehicle->getVehicleType());
    return true;
}

void AIController::shutdown() {
    Logger::info("Shutting down AIController");
    
    m_target.reset();
    m_vehicle.reset();
    m_world.reset();
    m_patrolPoints.clear();
    
    m_initialized = false;
}

void AIController::update(float deltaTime) {
    if (!m_initialized || !m_vehicle || !m_world) {
        return;
    }
    
    // Update decision timer
    m_decisionTimer += deltaTime;
    if (m_decisionTimer >= m_decisionInterval) {
        m_decisionTimer = 0.0f;
        updateState(deltaTime);
    }
    
    // Update targeting
    updateTargeting(deltaTime);
    
    // Update movement
    updateMovement(deltaTime);
    
    // Update combat
    updateCombat(deltaTime);
    
    // Update state timer
    m_stateTimer += deltaTime;
}

void AIController::setTarget(std::shared_ptr<Vehicle> target) {
    m_target = target;
    
    if (target) {
        Logger::debug("AI target set to vehicle at position: " + 
                    std::to_string(target->getPosition().x) + ", " + 
                    std::to_string(target->getPosition().y) + ", " + 
                    std::to_string(target->getPosition().z));
    }
}

void AIController::clearTarget() {
    m_target.reset();
    Logger::debug("AI target cleared");
}

void AIController::updateState(float deltaTime) {
    AIState newState = evaluateStateTransition();
    if (newState != m_state) {
        m_state = newState;
        m_stateTimer = 0.0f;
        Logger::debug("AI state changed to: " + std::to_string(static_cast<int>(newState)));
    }
}

void AIController::updateMovement(float deltaTime) {
    if (!m_vehicle) return;
    
    switch (m_state) {
        case AIState::IDLE:
            executeIdleState(deltaTime);
            break;
        case AIState::PATROL:
            executePatrolState(deltaTime);
            break;
        case AIState::CHASE:
            executeChaseState(deltaTime);
            break;
        case AIState::ATTACK:
            executeAttackState(deltaTime);
            break;
        case AIState::FLEE:
            executeFleeState(deltaTime);
            break;
        case AIState::EVADE:
            executeEvadeState(deltaTime);
            break;
        case AIState::SEARCH:
            executeSearchState(deltaTime);
            break;
        case AIState::RETURN_TO_BASE:
            executeReturnToBaseState(deltaTime);
            break;
    }
    
    // Apply movement to vehicle
    m_vehicle->setThrottle(m_currentSpeed);
    m_vehicle->setSteering(m_currentSteering);
    m_vehicle->setBrake(m_currentBrake);
}

void AIController::updateCombat(float deltaTime) {
    if (!m_vehicle || !m_target) return;
    
    // Update fire timing
    m_lastFireTime += deltaTime;
    
    // Check if should fire
    if (shouldAttack() && m_lastFireTime >= m_fireRate) {
        fireWeapons();
        m_lastFireTime = 0.0f;
    }
}

void AIController::updateTargeting(float deltaTime) {
    if (!m_vehicle || !m_world) return;
    
    // Find nearest enemy if no target
    if (!m_target) {
        const auto& vehicles = m_world->getVehicles();
        float nearestDistance = m_sightRange;
        std::shared_ptr<Vehicle> nearestEnemy;
        
        for (const auto& vehicle : vehicles) {
            if (!vehicle || vehicle == m_vehicle || vehicle->getTeam() == m_team) {
                continue;
            }
            
            float distance = glm::length(vehicle->getPosition() - m_vehicle->getPosition());
            if (distance < nearestDistance && canSeeTarget()) {
                nearestDistance = distance;
                nearestEnemy = vehicle;
            }
        }
        
        if (nearestEnemy) {
            setTarget(nearestEnemy);
        }
    }
    
    // Clear target if out of range or destroyed
    if (m_target) {
        float distance = glm::length(m_target->getPosition() - m_vehicle->getPosition());
        if (distance > m_sightRange || m_target->isDestroyed()) {
            clearTarget();
        }
    }
}

void AIController::executeIdleState(float deltaTime) {
    // Stop movement
    m_currentSpeed = 0.0f;
    m_currentSteering = 0.0f;
    m_currentBrake = 1.0f;
}

void AIController::executePatrolState(float deltaTime) {
    if (m_patrolPoints.empty()) {
        m_state = AIState::IDLE;
        return;
    }
    
    // Move towards current patrol point
    glm::vec3 targetPoint = m_patrolPoints[m_currentPatrolIndex];
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    
    // Check if reached patrol point
    float distance = glm::length(targetPoint - vehiclePos);
    if (distance < 10.0f) {
        // Move to next patrol point
        m_currentPatrolIndex++;
        if (m_currentPatrolIndex >= m_patrolPoints.size()) {
            if (m_patrolLoop) {
                m_currentPatrolIndex = 0;
            } else {
                m_currentPatrolIndex = m_patrolPoints.size() - 1;
                m_state = AIState::IDLE;
                return;
            }
        }
    }
    
    // Move towards target
    moveTowards(targetPoint, 0.5f);
}

void AIController::executeChaseState(float deltaTime) {
    if (!m_target) {
        m_state = AIState::PATROL;
        return;
    }
    
    // Predict target position
    glm::vec3 predictedPos = getPredictedTargetPosition(1.0f);
    moveTowards(predictedPos, 0.8f);
}

void AIController::executeAttackState(float deltaTime) {
    if (!m_target) {
        m_state = AIState::PATROL;
        return;
    }
    
    glm::vec3 targetPos = m_target->getPosition();
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    float distance = glm::length(targetPos - vehiclePos);
    
    // Maintain optimal attack range
    if (distance > m_attackRange * 1.5f) {
        moveTowards(targetPos, 0.6f);
    } else if (distance < m_attackRange * 0.5f) {
        moveAwayFrom(targetPos, 0.3f);
    } else {
        // Strafe around target
        strafeAround(targetPos, m_attackRange, 0.4f);
    }
    
    // Aim at target
    aimAtTarget();
}

void AIController::executeFleeState(float deltaTime) {
    if (!m_target) {
        m_state = AIState::RETURN_TO_BASE;
        return;
    }
    
    // Move away from threat
    moveAwayFrom(m_target->getPosition(), 1.0f);
    
    // Check if safe distance
    float distance = glm::length(m_target->getPosition() - m_vehicle->getPosition());
    if (distance > m_sightRange) {
        m_state = AIState::RETURN_TO_BASE;
    }
}

void AIController::executeEvadeState(float deltaTime) {
    if (!m_target) {
        m_state = AIState::PATROL;
        return;
    }
    
    // Evasive maneuvers - random direction changes
    float evadeAngle = static_cast<float>(rand()) / RAND_MAX * 2.0f * glm::pi<float>();
    glm::vec3 evadeDirection = glm::vec3(glm::cos(evadeAngle), 0.0f, glm::sin(evadeAngle));
    glm::vec3 evadeTarget = m_vehicle->getPosition() + evadeDirection * 50.0f;
    
    moveTowards(evadeTarget, 0.9f);
}

void AIController::executeSearchState(float deltaTime) {
    // Search around last known target position
    if (!m_target) {
        m_state = AIState::PATROL;
        return;
    }
    
    glm::vec3 searchCenter = m_target->getPosition();
    strafeAround(searchCenter, 30.0f, 0.3f);
    
    // Timeout after searching for a while
    if (m_stateTimer > 10.0f) {
        m_state = AIState::PATROL;
    }
}

void AIController::executeReturnToBaseState(float deltaTime) {
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    float distance = glm::length(m_baseLocation - vehiclePos);
    
    if (distance < m_baseRadius) {
        m_state = AIState::PATROL;
    } else {
        moveTowards(m_baseLocation, 0.6f);
    }
}

void AIController::moveTowards(const glm::vec3& target, float speed) {
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    glm::vec3 direction = glm::normalize(target - vehiclePos);
    
    // Calculate steering angle
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::quat rotation = m_vehicle->getRotation();
    glm::vec3 vehicleForward = rotation * forward;
    
    float angle = glm::acos(glm::clamp(glm::dot(vehicleForward, direction), -1.0f, 1.0f));
    glm::vec3 cross = glm::cross(vehicleForward, direction);
    
    m_currentSteering = glm::sign(cross.y) * angle;
    m_currentSpeed = speed;
    m_currentBrake = 0.0f;
}

void AIController::moveAwayFrom(const glm::vec3& threat, float speed) {
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    glm::vec3 direction = glm::normalize(vehiclePos - threat);
    glm::vec3 target = vehiclePos + direction * 100.0f;
    
    moveTowards(target, speed);
}

void AIController::strafeAround(const glm::vec3& target, float distance, float speed) {
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    glm::vec3 toTarget = target - vehiclePos;
    toTarget.y = 0.0f; // Keep on ground level
    
    float currentDistance = glm::length(toTarget);
    glm::vec3 direction = glm::normalize(toTarget);
    
    // Calculate perpendicular direction for strafing
    glm::vec3 strafeDirection = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    
    // Adjust strafe direction based on desired distance
    if (currentDistance > distance) {
        // Move closer while strafing
        strafeDirection = glm::normalize(strafeDirection + direction * 0.3f);
    } else if (currentDistance < distance) {
        // Move away while strafing
        strafeDirection = glm::normalize(strafeDirection - direction * 0.3f);
    }
    
    glm::vec3 strafeTarget = vehiclePos + strafeDirection * 50.0f;
    moveTowards(strafeTarget, speed);
}

glm::vec3 AIController::getAvoidanceVector() const {
    // Simple obstacle avoidance
    glm::vec3 avoidance(0.0f);
    
    // This would check for nearby obstacles and return avoidance vector
    // For now, return zero vector
    
    return avoidance;
}

glm::vec3 AIController::getPredictedTargetPosition(float predictionTime) const {
    if (!m_target) {
        return glm::vec3(0.0f);
    }
    
    glm::vec3 targetPos = m_target->getPosition();
    glm::vec3 targetVelocity = glm::vec3(0.0f); // Would need to get from target vehicle
    
    return targetPos + targetVelocity * predictionTime;
}

void AIController::fireWeapons() {
    if (!m_vehicle) return;
    
    // Select best weapon for current situation
    selectBestWeapon();
    
    // Fire selected weapon
    m_vehicle->fireWeapon(m_selectedWeapon);
}

void AIController::selectBestWeapon() {
    if (!m_vehicle) return;
    
    // Simple weapon selection based on distance
    if (!m_target) return;
    
    float distance = glm::length(m_target->getPosition() - m_vehicle->getPosition());
    
    if (distance < 30.0f) {
        m_selectedWeapon = 0; // Close range weapon
    } else if (distance < 80.0f) {
        m_selectedWeapon = 1; // Medium range weapon
    } else {
        m_selectedWeapon = 1; // Long range weapon
    }
}

void AIController::aimAtTarget() {
    // Aim calculation would go here
    // This would adjust the vehicle's orientation to face the target
}

bool AIController::hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const {
    if (!m_world) return false;
    
    // Simple line of sight check - for now assume clear line of sight
    // In a real implementation, this would perform a raycast against terrain and objects
    // For now, we'll just check distance and assume clear if within range
    float distance = glm::length(to - from);
    return distance <= m_sightRange;
}

AIState AIController::evaluateStateTransition() {
    // Check for immediate threats
    if (isUnderThreat() && shouldFlee()) {
        return AIState::FLEE;
    }
    
    // Check if we have a target
    if (m_target) {
        float distance = glm::length(m_target->getPosition() - m_vehicle->getPosition());
        
        if (canSeeTarget()) {
            if (isInAttackRange()) {
                return AIState::ATTACK;
            } else {
                return AIState::CHASE;
            }
        } else {
            return AIState::SEARCH;
        }
    }
    
    // Check if should return to base
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    float distanceFromBase = glm::length(m_baseLocation - vehiclePos);
    if (distanceFromBase > m_baseRadius * 3.0f) {
        return AIState::RETURN_TO_BASE;
    }
    
    // Default to patrol
    return AIState::PATROL;
}

float AIController::calculateThreatLevel(const glm::vec3& position) const {
    // Calculate threat level based on nearby enemies
    float threat = 0.0f;
    
    if (!m_world) return threat;
    
    const auto& vehicles = m_world->getVehicles();
    for (const auto& vehicle : vehicles) {
        if (!vehicle || vehicle == m_vehicle || vehicle->getTeam() == m_team) {
            continue;
        }
        
        float distance = glm::length(vehicle->getPosition() - position);
        if (distance < m_sightRange) {
            threat += (1.0f - distance / m_sightRange) * vehicle->getHealth() / 100.0f;
        }
    }
    
    return glm::clamp(threat, 0.0f, 1.0f);
}

glm::vec3 AIController::findBestCoverPosition() const {
    // Find best cover position - simplified for now
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    
    // Use rand() instead of glm::linearRand
    float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * glm::pi<float>();
    return vehiclePos + glm::vec3(glm::cos(angle), 0.0f, glm::sin(angle)) * 30.0f;
}

glm::vec3 AIController::findFlankingPosition() const {
    if (!m_target) return glm::vec3(0.0f);
    
    glm::vec3 targetPos = m_target->getPosition();
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    glm::vec3 toTarget = glm::normalize(targetPos - vehiclePos);
    
    // Calculate flank position
    glm::vec3 flankDirection = glm::normalize(glm::cross(toTarget, glm::vec3(0.0f, 1.0f, 0.0f)));
    return targetPos + flankDirection * m_attackRange;
}

bool AIController::canSeeTarget() const {
    if (!m_target || !m_vehicle) return false;
    
    glm::vec3 vehiclePos = m_vehicle->getPosition();
    glm::vec3 targetPos = m_target->getPosition();
    
    float distance = glm::length(targetPos - vehiclePos);
    if (distance > m_sightRange) return false;
    
    return hasLineOfSight(vehiclePos, targetPos);
}

bool AIController::isInAttackRange() const {
    if (!m_target) return false;
    
    float distance = glm::length(m_target->getPosition() - m_vehicle->getPosition());
    return distance <= m_attackRange;
}

bool AIController::isUnderThreat() const {
    return calculateThreatLevel(m_vehicle->getPosition()) > 0.5f;
}

bool AIController::shouldFlee() const {
    if (!m_vehicle) return false;
    
    float healthRatio = m_vehicle->getHealth() / 100.0f;
    float threatLevel = calculateThreatLevel(m_vehicle->getPosition());
    
    // Flee if health is low or threat is high
    return (healthRatio < 0.3f && threatLevel > 0.3f) || threatLevel > 0.8f;
}

bool AIController::shouldAttack() const {
    if (!m_target || !m_vehicle) return false;
    
    float healthRatio = m_vehicle->getHealth() / 100.0f;
    return healthRatio > 0.2f && canSeeTarget() && isInAttackRange();
}

void AIController::addPatrolPoint(const glm::vec3& point) {
    m_patrolPoints.push_back(point);
}

void AIController::clearPatrolPoints() {
    m_patrolPoints.clear();
    m_currentPatrolIndex = 0;
}
