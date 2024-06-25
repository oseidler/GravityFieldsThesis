#pragma once
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Quaternion.hpp"


//forward declarations
class Game;
class GravityField;


//constants
constexpr float GROUNDED_THRESHOLD = 0.75f;
constexpr float WALL_THRESHOLD = 0.9f;
constexpr float MAX_TRIPLE_JUMP_ANGLE = 1080.0f;
constexpr float TRUE_FALL_THRESHOLD = 15.0f;

//enums
enum CameraMode
{
	FIXED = 0,
	FREE,
	FOLLOW,
	FIRST_PERSON,
	NUM_MODES
};


class Player
{
//public member functions
public:
	//constructor
	Player(Game* owner) : m_game(owner) {}

	//game flow functions
	void Update(float deltaSeconds);
	void UpdateFromController(float deltaSeconds);
	void Render() const;

	//physics functions
	void UpdatePhysics(float deltaSeconds);
	void AddForce(Vec3 const& forceVector);
	void AddGravity(Vec3 gravityVector);
	void SetGravitySource(GravityField const* gravitySource, Vec3 const& gravityCenter, Vec3 const& gravityVector);
	void AddImpulse(Vec3 const& impulseVector);
	void MoveInDirection(Vec3 const& directionNormal, float speed);
	void Jump();
	void WallJump();
	void BecomeGrounded();
	void StartWallSlide(Vec3 const& wallNormal);

	//player utilities
	Mat44 GetModelMatrix() const;
	Vec3 GetIBasis() const;
	Vec3 GetJBasis() const;
	Vec3 GetKBasis() const;
	void Respawn();

//public member variables
public:
	Game* m_game = nullptr;
	Vec3  m_playerStartPosition = Vec3(-5.0f, 0.0f, 0.0f);

	Vec3 m_position = Vec3();
	Vec3 m_velocity = Vec3();
	Vec3 m_acceleration = Vec3();
	Mat44 m_orientation = Mat44();
	EulerAngles m_freeCameraOrientation = EulerAngles();

	Vec3  m_movementIntentions = Vec3();
	Vec2  m_movementDirection = Vec2();
	float m_yawIntentions = 0.0f;
	float m_movementSpeed = 11.0f;
	float m_tankTurnRate = 170.0f;
	float m_turnRate = 0.08f;
	bool  m_isTankTurn = true;
	float m_mouseTurnRate = 0.08f;
	float m_controllerTurnRate = 150.0f;
	//bool  m_isHoldingMove = false;	//UNIMPLEMENTED
	//bool  m_reverse = false;
	bool  m_stopMoving = false;
	bool  m_isSpeedUp = false;
	float m_speedUpFactor = 2.0f;
	float m_drag = 7.0f;
	bool  m_isCrouching = false;
	float m_crouchSpeedScale = 0.4f;
	float m_runThreshold = 6.0f;

	float m_collisionRadius = 1.0f;
	float m_meshRadius = 0.6f;
	float m_meshHeight = 1.0f;

	float m_jumpForce = 75.0f;
	bool  m_isGrounded = false;
	bool  m_wasGroundedLastFrame = false;
	float m_fallSpeedScalar = 1.75f;

	int   m_jumpNumber = 0;				//1 during a normal jump, 2 during a double jump, 3 during a triple jump
	float m_tripleJumpTimerMax = 0.35f;
	float m_tripleJumpTimer = 0.0f;
	float m_doubleJumpScalar = 1.5f;
	float m_tripleJumpScalar = 2.0f;

	float m_tripleJumpFlipAngle = 0.0f;
	float m_tripleJumpFlipSpeed = 580.0f;
	bool  m_doTripleJumpFlip = false;

	float m_longJumpHeightScale = 0.75f;
	//float m_longJumpImpulse = 90.0f;
	float m_longJumpForce = 2500.0f;
	bool  m_isLongJumping = false;
	float m_longJumpDurationTimer = 0.0f;
	float m_longJumpDurationTimerScale = 1.75f;

	float m_backFlipJumpScale = 1.6f;
	float m_backFlipMoveImpulse = 65.0f;
	bool  m_isBackFlipping = false;

	float m_backFlipAngle = 0.0f;
	float m_backFlipSpeed = -360.0f;

	float m_canSideFlipTimerMax = 0.0f;
	float m_canSideFlipTimer = 0.0f;
	float m_isSideFlipping = false;
	float m_sideFlipMoveImpulse = 50.0f;
	float m_sideFlipJumpScale = 1.5f;

	bool  m_isWallSliding = false;
	bool  m_isWallJumping = false;
	float m_wallSlideResistance = 0.35f;
	Vec3  m_wallSlideNormal = Vec3();
	float m_wallJumpImpulse = 95.0f;
	float m_wallJumpTimerMax = 0.2f;
	float m_wallJumpTimer = 0.0f;

	float m_stretchAmount = 1.0f; //1 is default, higher is stretch, lower is squash
	float m_maxStretch = 2.0f;
	float m_maxSquash = 0.5f;
	float m_squashLandBase = 1.1f;
	float m_stretchJumpBase = 1.25f;
	float m_landSquashScalar = 0.06f;
	Vec3  m_landingVelocity = Vec3();

	GravityField const* m_currentGravitySource = nullptr;
	Vec3 m_currentGravityCenter = Vec3();
	Vec3 m_currentGravityVector = Vec3();
	float m_orientationMatchRate = 0.1f;
	//float m_rotationAlpha = 0.0f;	//CURRENTLY UNUSED
	bool m_rememberLastGravitySource = true;

	Camera m_playerCamera;
	float  m_cameraOffset = -12.5f;
	CameraMode m_cameraMode = FREE;
	bool m_invertFreeCamera = false;

	Rgba8 m_color = Rgba8(100, 200, 0);
	Rgba8 m_debugWireframeColor = Rgba8(255, 0, 0);

	bool m_isFreeFlyMode = false;

	//playtest tracking variables
	/*int m_numStandardJumps = 0;
	int m_numDoubleJumps = 0;
	int m_numTripleJumps = 0;
	int m_numLongJumps = 0;
	int m_numBackFlips = 0;
	int m_numWallJumps = 0;
	int m_numSideFlips = 0;
	int m_numSection1Respawns = 0;
	int m_numSection2Respawns = 0;
	int m_numSection3Respawns = 0;
	int m_numSection4Respawns = 0;*/

	/*bool m_isFixedTimeStep = false;
	float m_fixedTimeStepAmount = 0.005f;
	float m_fixedTimeStepTimer = 0.0f;*/
};
