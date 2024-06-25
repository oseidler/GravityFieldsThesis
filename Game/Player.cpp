#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Planetoids.hpp"
#include "Game/GravityFields.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Math/AABB3.hpp"


//
//public game flow functions
//
void Player::Update(float deltaSeconds)
{
	//handle movement input
	m_isSpeedUp = false;
	m_movementIntentions = Vec3();
	m_movementDirection = Vec2();
	m_yawIntentions = 0;

	if (g_theInput->IsKeyDown('W'))
	{
		if (m_isTankTurn) m_movementIntentions.x += 1.0f;
		else m_movementDirection.x += 1.0f;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		if (m_isTankTurn) m_movementIntentions.x -= 1.0f;
		else m_movementDirection.x -= 1.0f;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		if (m_isTankTurn) m_yawIntentions += 1.0f;
		else m_movementDirection.y += 1.0f;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		if (m_isTankTurn) m_yawIntentions -= 1.0f;
		else m_movementDirection.y -= 1.0f;
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		if (m_currentGravitySource == nullptr || m_isFreeFlyMode)
		{
			m_movementIntentions.z -= 1.0f;
		}
	}
	if (g_theInput->IsKeyDown('E'))
	{
		if (m_currentGravitySource == nullptr || m_isFreeFlyMode)
		{
			m_movementIntentions.z += 1.0f;
		}
	}

	if (g_theInput->WasKeyJustPressed('H'))
	{
		Respawn();
	}

	m_isCrouching = false;
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT) && m_isGrounded)
	{
		m_isCrouching = true;
	}
	if (g_theInput->IsKeyDown('R') && !m_isCrouching)
	{
		m_isSpeedUp = true;
	}

	if (g_theInput->WasKeyJustPressed(' '))
	{
		Jump();
	}

	//debug keys
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_cameraMode = static_cast<CameraMode>(m_cameraMode + 1);
		if (m_cameraMode >= NUM_MODES)
		{
			m_cameraMode = FIXED;
		}
		std::string cameraModeString = "";
		switch (m_cameraMode)
		{
			case FIXED: cameraModeString = "Fixed Angle"; break;
			case FREE: cameraModeString = "Free Control"; break;
			case FOLLOW: cameraModeString = "Match Player Angle"; break;
			case FIRST_PERSON: cameraModeString = "First Person"; break;
		}
		std::string changedCameraMessage = Stringf("Changed camera mode: %s", cameraModeString.c_str());
		DebugAddMessage(changedCameraMessage, 4.0f);

		if (m_cameraMode == FREE)
		{
			m_playerCamera.SetUseMatrixOrientationMode(false);
		}
		else
		{
			m_playerCamera.SetUseMatrixOrientationMode(true);
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_invertFreeCamera = !m_invertFreeCamera;
		if (m_invertFreeCamera)
		{
			std::string message = "Invert free camera: on";
			DebugAddMessage(message, 4.0f);
		}
		else
		{
			std::string message = "Invert free camera: off";
			DebugAddMessage(message, 4.0f);
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_rememberLastGravitySource = !m_rememberLastGravitySource;
		if (m_rememberLastGravitySource)
		{
			std::string message = "No-gravity mode: Remember last gravity source";
			DebugAddMessage(message, 4.0f);
		}
		else
		{
			std::string message = "No-gravity mode: Global gravity";
			DebugAddMessage(message, 4.0f);
		}
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		g_theGame->m_isDebugView = !g_theGame->m_isDebugView;
	}
	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		/*m_isTankTurn = !m_isTankTurn;
		std::string turn = "Mimic Joystick";
		if (m_isTankTurn) turn = "Tank Turn";
		std::string message = Stringf("Keyboard turning mode: %s", turn.c_str());
		DebugAddMessage(message, 4.0f);*/

		m_isFreeFlyMode = !m_isFreeFlyMode;

		if (m_isFreeFlyMode)
		{
			DebugAddMessage("Free Fly: True", 5.0f);
			m_orientation = Mat44();
		}
		else
		{
			DebugAddMessage("Free Fly: False", 5.0f);
		}
	}

	//handle mouse input
	IntVec2 mouseDelta = g_theInput->GetCursorClientDelta();
	m_freeCameraOrientation.m_yawDegrees -= (float)mouseDelta.x * m_mouseTurnRate;
	if (m_invertFreeCamera)
	{
		m_freeCameraOrientation.m_pitchDegrees -= (float)mouseDelta.y * m_mouseTurnRate;
	}
	else
	{
		m_freeCameraOrientation.m_pitchDegrees += (float)mouseDelta.y * m_mouseTurnRate;
	}

	if (!m_isTankTurn && (m_movementDirection.x != 0.0f || m_movementDirection.y != 0.0f))
	{
		Vec2 leftStickVector = Vec2(-m_movementDirection.y, m_movementDirection.x);
		Mat44 cameraModelMatrix = m_playerCamera.GetViewMatrix().GetOrthonormalInverse();
		Vec3 moveForward = cameraModelMatrix.GetKBasis3D();
		Vec3 moveForwardOnUpVector = GetProjectedOnto3D(moveForward, GetModelMatrix().GetKBasis3D());
		Vec3 moveForwardOnSurfacePlane = (moveForward - moveForwardOnUpVector).GetNormalized();
		/*if((moveForward-moveForwardOnUpVector).GetLength() < 0.15f)
		{
			moveForward = cameraModelMatrix.GetIBasis3D();
			moveForwardOnUpVector = GetProjectedOnto3D(moveForward, GetModelMatrix().GetKBasis3D());
			moveForwardOnSurfacePlane = (moveForward - moveForwardOnUpVector).GetNormalized();
			std::string mes = "Change to IBasis";
			DebugAddMessage(mes, 0.0f);
		}*/
		Vec3 moveForwardOnSurfacePlaneCorrected = moveForwardOnSurfacePlane;
		if (DotProduct3D(GetModelMatrix().GetKBasis3D(), cameraModelMatrix.GetIBasis3D()) > 0.0f)
		{
			moveForwardOnSurfacePlaneCorrected = -moveForwardOnSurfacePlaneCorrected;
			if (g_theGame->m_isDebugView)
			{
				std::string mes = "Reverse";
				DebugAddMessage(mes, 0.0f);
			}
		}
		//DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlane, 0.05f, 0.0f, Rgba8(255, 127, 0), Rgba8(255, 127, 0), DebugRenderMode::X_RAY);
		//DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneCorrected, 0.05f, 0.0f, Rgba8(127, 0, 0), Rgba8(127, 0, 0), DebugRenderMode::X_RAY);
		Vec3 moveRight = CrossProduct3D(moveForwardOnSurfacePlaneCorrected, GetModelMatrix().GetKBasis3D());
		Vec3 moveRightOnUpVector = GetProjectedOnto3D(moveRight, GetModelMatrix().GetKBasis3D());
		Vec3 moveRightOnSurfacePlane = (moveRight - moveRightOnUpVector).GetNormalized();
		if (DotProduct3D(GetModelMatrix().GetKBasis3D(), cameraModelMatrix.GetIBasis3D()) > 0.0f)
		{
			moveRightOnSurfacePlane = -moveRightOnSurfacePlane;
		}
		//DebugAddWorldArrow(m_position, m_position + moveRightOnSurfacePlane, 0.05f, 0.0f, Rgba8(127, 0, 255), Rgba8(127, 0, 255), DebugRenderMode::X_RAY);
		Vec3 movementDirection = ((moveForwardOnSurfacePlaneCorrected * leftStickVector.y) + (moveRightOnSurfacePlane * leftStickVector.x)).GetNormalized();
		//DebugAddWorldArrow(m_position, m_position + movementDirection, 0.05f, 0.0f, Rgba8(0, 0, 0), Rgba8(0, 0, 0), DebugRenderMode::X_RAY);
		Vec3 moveDirectionRight = CrossProduct3D(movementDirection, GetModelMatrix().GetKBasis3D());
		//DebugAddWorldArrow(m_position, m_position + moveDirectionRight, 0.05f, 0.0f, Rgba8(), Rgba8(), DebugRenderMode::X_RAY);
		Mat44 goalOrientation = Mat44(movementDirection, -moveDirectionRight, GetModelMatrix().GetKBasis3D(), Vec3());
		//DebugAddWorldArrow(m_position, m_position + goalOrientation.GetIBasis3D(), 0.05f, 0.0f, Rgba8(0, 0, 0), Rgba8(0, 0, 0), DebugRenderMode::X_RAY);
		//if (!m_stopMoving)
		{
			//m_orientation = goalOrientation;
			Quaternion startRotQuat = m_orientation.GetAsQuaternion();
			Quaternion endRotQuat = goalOrientation.GetAsQuaternion();
			Quaternion finalRotQuat = Slerp(startRotQuat, endRotQuat, m_turnRate);
			m_orientation = finalRotQuat.GetAsRotMatrix();
			m_orientation.Orthonormalize_XFwd_YLeft_ZUp();
			m_movementIntentions.x += 1.0f;
		}
	}

	UpdateFromController(deltaSeconds);

	m_orientation.AppendZRotation(m_yawIntentions * m_tankTurnRate * deltaSeconds);

	m_freeCameraOrientation.m_pitchDegrees = GetClamped(m_freeCameraOrientation.m_pitchDegrees, -85.0f, 85.0f);

	if (m_movementIntentions != Vec3())
	{
		m_movementIntentions.Normalize();
		m_movementIntentions = GetModelMatrix().TransformVectorQuantity3D(m_movementIntentions);

		MoveInDirection(m_movementIntentions, m_movementSpeed);
	}

	if (m_isSpeedUp)
	{
		deltaSeconds *= m_speedUpFactor;
	}

	if (m_isLongJumping)
	{
		m_longJumpDurationTimer += deltaSeconds;
		AddForce(m_orientation.GetIBasis3D() * m_longJumpForce * deltaSeconds * m_drag);
	}

	/*if (g_theInput->WasKeyJustPressed(KEYCODE_COMMA))
	{
		m_isFixedTimeStep = !m_isFixedTimeStep;
	}*/

	/*if (m_isFixedTimeStep)
	{
		DebugAddMessage("Fixed", 0.0f);
		m_fixedTimeStepTimer += deltaSeconds;
		while (m_fixedTimeStepTimer >= m_fixedTimeStepAmount)
		{
			UpdatePhysics(m_fixedTimeStepAmount);
			m_fixedTimeStepTimer -= m_fixedTimeStepAmount;
		}
	}
	else
	{
		DebugAddMessage("Variable", 0.0f);
		UpdatePhysics(deltaSeconds);
	}*/

	UpdatePhysics(deltaSeconds);

	//handle jump logic
	if (!m_wasGroundedLastFrame && m_isGrounded)
	{
		m_isBackFlipping = false;
		m_backFlipAngle = 0.0f;
		m_isLongJumping = false;
		m_longJumpDurationTimer = 0.0f;
		m_isSideFlipping = false;

		//increment triple jump counter
		if (m_jumpNumber == 3)
		{
			m_jumpNumber = 0;
			m_doTripleJumpFlip = false;
			m_tripleJumpFlipAngle = 0.0f;
		}

		if (m_landingVelocity.GetLength() > TRUE_FALL_THRESHOLD)
		{
			m_tripleJumpTimer = m_tripleJumpTimerMax;
		}
		
		//apply landing squash
		float squashAmount = 1.0f / (m_squashLandBase * m_landingVelocity.GetLength() * m_landSquashScalar);

		if (squashAmount < 1.0f)
		{
			squashAmount = GetClamped(squashAmount, m_maxSquash, 1.0f);
			m_stretchAmount = squashAmount;
		}
	}
	if (m_isGrounded)
	{
		if (m_tripleJumpFlipAngle > 1.0f)
		{
			m_doTripleJumpFlip = false;
			m_tripleJumpFlipAngle = 0.0f;
		}
	}
	if (m_tripleJumpTimer > 0.0f)
	{
		m_tripleJumpTimer -= deltaSeconds;
		if (m_tripleJumpTimer <= 0.0f && (m_isGrounded || m_wasGroundedLastFrame))
		{
			m_jumpNumber = 0;
		}
	}
	if (m_canSideFlipTimer > 0.0f)
	{
		m_canSideFlipTimer -= deltaSeconds;
		if (m_canSideFlipTimer < 0.0f)
		{
			m_canSideFlipTimer = 0.0f;
		}
	}
	if (m_wallJumpTimer > 0.0f)
	{
		m_wallJumpTimer -= deltaSeconds;
		if (m_wallJumpTimer <= 0.0f)
		{
			m_wallJumpTimer = 0.0f;
			m_isWallJumping = false;
		}
	}

	if (m_doTripleJumpFlip)
	{
		m_tripleJumpFlipAngle += m_tripleJumpFlipSpeed * deltaSeconds;
	}
	if (m_isBackFlipping)
	{
		m_backFlipAngle += m_backFlipSpeed * deltaSeconds;
	}

	if (m_stretchAmount > 1.0f)
	{
		m_stretchAmount -= deltaSeconds;
		if (m_stretchAmount < 1.0f)
		{
			m_stretchAmount = 1.0f;
		}
	}
	else if (m_stretchAmount < 1.0f)
	{
		m_stretchAmount += deltaSeconds;
		if (m_stretchAmount > 1.0f)
		{
			m_stretchAmount = 1.0f;
		}
	}

	//print jump debug info
	if (g_theGame->m_isDebugView)
	{
		std::string jumpMessage = Stringf("Current jump: %i  -  Current jump timer: %.3f  -  IsGrounded: %s  -  WasGrounded: %s", m_jumpNumber, m_tripleJumpTimer, m_isGrounded ? "true" : "false", m_wasGroundedLastFrame ? "true" : "false");
		DebugAddMessage(jumpMessage, 0.0f);
		std::string wallSlideMessage = Stringf("IsWallSliding = %s, IsWallJumping = %s", m_isWallSliding ? "true" : "false", m_isWallJumping ? "true" : "false");
		DebugAddMessage(wallSlideMessage, 0.0f);
	}
	/*std::string flipMessage = Stringf("Can Side Flip Timer: %.2f", m_canSideFlipTimer);
	DebugAddMessage(flipMessage, 0.0f);*/

	m_wasGroundedLastFrame = m_isGrounded;
	m_isGrounded = false;
	m_isWallSliding = false;

	//handle camera modes
	switch (m_cameraMode)
	{
		case FIXED: m_playerCamera.SetTransform(m_position + Vec3(m_cameraOffset, 0.0f, 0.0f), Mat44()); break;
		case FREE:	
		{
			m_playerCamera.SetTransform(m_position, m_freeCameraOrientation);
			Vec3 cameraForward = m_playerCamera.GetViewMatrix().GetOrthonormalInverse().GetIBasis3D();
			m_playerCamera.SetTransform(m_position + (cameraForward * m_cameraOffset), m_freeCameraOrientation);
			break;
		}
		case FOLLOW: m_playerCamera.SetTransform(m_position + (GetIBasis() * m_cameraOffset), m_orientation); break;
		case FIRST_PERSON: m_playerCamera.SetTransform(m_position, m_orientation); break;
	}
}


void Player::UpdateFromController(float deltaSeconds)
{
	XboxController const& controller = g_theInput->GetController(0);
	AnalogJoystick const& leftStick = controller.GetLeftStick();
	AnalogJoystick const& rightStick = controller.GetRightStick();

	float leftStickMagnitude = leftStick.GetMagnitude();
	float rightStickMagnitude = rightStick.GetMagnitude();

	//camera controls
	if (rightStickMagnitude > 0.0f)
	{
		Vec2 rightStickVector = Vec2::MakeFromPolarDegrees(rightStick.GetOrientationDegrees());
		m_freeCameraOrientation.m_yawDegrees -= rightStickVector.x * m_controllerTurnRate * deltaSeconds;
		if (m_invertFreeCamera)
		{
			m_freeCameraOrientation.m_pitchDegrees += rightStickVector.y * m_controllerTurnRate * deltaSeconds;
		}
		else
		{
			m_freeCameraOrientation.m_pitchDegrees -= rightStickVector.y * m_controllerTurnRate * deltaSeconds;
		}
	}


	static bool forceIBasis = false;
	/*if (controller.WasButtonJustPressed(XBOX_BUTTON_X))
	{
		forceIBasis = !forceIBasis;
		if (forceIBasis)
		{
			DebugAddMessage("Force IBasis: True", 5.0f);
		}
		else
		{
			DebugAddMessage("Force IBasis: False", 5.0f);
		}
	}*/


	//movement
	constexpr float I_BASIS_THRESHOLD = 0.1f;
	Mat44 cameraModelMatrix = m_playerCamera.GetViewMatrix().GetOrthonormalInverse();
	if (leftStickMagnitude > 0.0f && !m_isBackFlipping && !m_isWallJumping)
	{
		bool usingIBasis = false;

		Vec2 leftStickVector = Vec2::MakeFromPolarDegrees(leftStick.GetOrientationDegrees());
		Vec3 moveForwardKBasis = cameraModelMatrix.GetKBasis3D();
		Vec3 moveForwardOnUpVectorKBasis = GetProjectedOnto3D(moveForwardKBasis, GetModelMatrix().GetKBasis3D());
		Vec3 moveForwardOnSurfacePlaneKBasis = (moveForwardKBasis - moveForwardOnUpVectorKBasis).GetNormalized();

		Vec3 moveForwardIBasis = (cameraModelMatrix.GetKBasis3D() - cameraModelMatrix.GetIBasis3D() * 0.1f).GetNormalized();
		Vec3 moveForwardOnUpVectorIBasis = GetProjectedOnto3D(moveForwardIBasis, GetModelMatrix().GetKBasis3D());
		Vec3 moveForwardOnSurfacePlaneIBasis = (moveForwardIBasis - moveForwardOnUpVectorIBasis).GetNormalized();

		if ((moveForwardKBasis - moveForwardOnUpVectorKBasis).GetLength() < I_BASIS_THRESHOLD)
		{
			usingIBasis = true;

			/*if (g_theGame->m_isDebugView)*/ //DebugAddMessage("Would change to IBasis here", 0.0f);
		}

		Vec3 moveForwardOnSurfacePlaneCorrectedKBasis = moveForwardOnSurfacePlaneKBasis;
		Vec3 moveForwardOnSurfacePlaneCorrectedIBasis = moveForwardOnSurfacePlaneIBasis;
		if (DotProduct3D(GetModelMatrix().GetKBasis3D(), cameraModelMatrix.GetIBasis3D()) > 0.0f)
		{
			moveForwardOnSurfacePlaneCorrectedKBasis = -moveForwardOnSurfacePlaneCorrectedKBasis;
			//moveForwardOnSurfacePlaneCorrectedIBasis = -moveForwardOnSurfacePlaneCorrectedIBasis;

			/*if (g_theGame->m_isDebugView)*/ //DebugAddMessage("Flip KBasis Direction", 0.0f);
		}

		if (DotProduct3D(GetModelMatrix().GetKBasis3D(), cameraModelMatrix.GetKBasis3D()) < 0.0f)
		{
			//moveForwardOnSurfacePlaneCorrectedIBasis = -moveForwardOnSurfacePlaneCorrectedIBasis;

			/*if (g_theGame->m_isDebugView)*/ //DebugAddMessage("Flip IBasis Direction", 0.0f);
		}

		/*if (m_reverse)
		{
			moveForwardOnSurfacePlaneCorrected = -moveForwardOnSurfacePlaneCorrected;
		}*/

		Vec3 moveForwardOnSurfacePlaneCorrected = moveForwardOnSurfacePlaneCorrectedKBasis;
		//Vec3 moveForwardOnSurfacePlaneCorrected = (moveForwardOnSurfacePlaneCorrectedKBasis + moveForwardOnSurfacePlaneCorrectedIBasis).GetNormalized();
		if (usingIBasis || forceIBasis)
		{
			//moveForwardOnSurfacePlaneCorrected = moveForwardOnSurfacePlaneCorrectedIBasis;
			//moveForwardOnSurfacePlaneCorrected = (moveForwardOnSurfacePlaneCorrectedKBasis + moveForwardOnSurfacePlaneCorrectedIBasis).GetNormalized();

			/*if (g_theGame->m_isDebugView)*/ //DebugAddMessage("Change to IBasis", 0.0f);
		}

		if (moveForwardOnSurfacePlaneKBasis == moveForwardOnSurfacePlaneCorrectedKBasis)
		{
			/*if (g_theGame->m_isDebugView)*/ //DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneKBasis, 0.05f, 0.0f, Rgba8(255, 127, 0), Rgba8(255, 127, 0), DebugRenderMode::X_RAY);
		}
		else
		{
			/*if (g_theGame->m_isDebugView)*/ //DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneCorrectedKBasis, 0.05f, 0.0f, Rgba8(127, 0, 0), Rgba8(127, 0, 0), DebugRenderMode::X_RAY);
		}
		if (moveForwardOnSurfacePlaneIBasis == moveForwardOnSurfacePlaneCorrectedIBasis)
		{
			/*if (g_theGame->m_isDebugView)*/ //DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneIBasis, 0.05f, 0.0f, Rgba8(127, 255, 0), Rgba8(127, 255, 0), DebugRenderMode::X_RAY);
		}
		else
		{
			/*if (g_theGame->m_isDebugView)*/ //DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneCorrectedIBasis, 0.05f, 0.0f, Rgba8(0, 127, 0), Rgba8(0, 127, 0), DebugRenderMode::X_RAY);
		}

		/*Vec3 moveRightJBasis = -cameraModelMatrix.GetJBasis3D();
		Vec3 moveRightOnUpVectorJBasis = GetProjectedOnto3D(moveRightJBasis, GetModelMatrix().GetKBasis3D());
		Vec3 moveRightOnSurfacePlaneJBasis = (moveRightJBasis - moveRightOnUpVectorJBasis).GetNormalized();

		Vec3 moveRightOnSurfacePlane = moveRightOnSurfacePlaneJBasis;
		if ((moveRightJBasis - moveRightOnUpVectorJBasis).GetLength() < I_BASIS_THRESHOLD)
		{
			Vec3 moveRight = CrossProduct3D(moveForwardOnSurfacePlaneCorrected, GetModelMatrix().GetKBasis3D());
			Vec3 moveRightOnUpVector = GetProjectedOnto3D(moveRight, GetModelMatrix().GetKBasis3D());
			moveRightOnSurfacePlane = (moveRight - moveRightOnUpVector).GetNormalized();
		}*/

		Vec3 moveRight = CrossProduct3D(moveForwardOnSurfacePlaneCorrected, GetModelMatrix().GetKBasis3D());
		Vec3 moveRightOnUpVector = GetProjectedOnto3D(moveRight, GetModelMatrix().GetKBasis3D());
		Vec3 moveRightOnSurfacePlane = (moveRight - moveRightOnUpVector).GetNormalized();
		
		bool rightFlipped = false;
		if (DotProduct3D(GetModelMatrix().GetKBasis3D(), cameraModelMatrix.GetIBasis3D()) > 0.0f)
		{
			moveRightOnSurfacePlane = -moveRightOnSurfacePlane;
			rightFlipped = true;
		}
		/*if (m_reverse)
		{
			moveRightOnSurfacePlane = -moveRightOnSurfacePlane;
		}*/

		if (!rightFlipped)
		{
			/*if (g_theGame->m_isDebugView)*/ //DebugAddWorldArrow(m_position, m_position + moveRightOnSurfacePlane, 0.05f, 0.0f, Rgba8(127, 0, 255), Rgba8(127, 0, 255), DebugRenderMode::X_RAY);
		}
		else
		{
			/*if (g_theGame->m_isDebugView)*/ //DebugAddWorldArrow(m_position, m_position + moveRightOnSurfacePlane, 0.05f, 0.0f, Rgba8(0, 0, 127), Rgba8(0, 0, 127), DebugRenderMode::X_RAY);
		}
		
		Vec3 movementDirection = ((moveForwardOnSurfacePlaneCorrected * leftStickVector.y) + (moveRightOnSurfacePlane * leftStickVector.x)).GetNormalized();
		//DebugAddWorldArrow(m_position, m_position + movementDirection, 0.05f, 0.0f, Rgba8(0, 0, 0), Rgba8(0, 0, 0), DebugRenderMode::X_RAY);
		Vec3 moveDirectionRight = CrossProduct3D(movementDirection, GetModelMatrix().GetKBasis3D());
		//DebugAddWorldArrow(m_position, m_position + moveDirectionRight, 0.05f, 0.0f, Rgba8(), Rgba8(), DebugRenderMode::X_RAY);
		Mat44 goalOrientation = Mat44(movementDirection, -moveDirectionRight, GetModelMatrix().GetKBasis3D(), Vec3());
		//DebugAddWorldArrow(m_position, m_position + goalOrientation.GetIBasis3D(), 0.05f, 0.0f, Rgba8(0, 0, 0), Rgba8(0, 0, 0), DebugRenderMode::X_RAY);
		
		//m_orientation = goalOrientation;
		Quaternion startRotQuat = m_orientation.GetAsQuaternion();
		Quaternion endRotQuat = goalOrientation.GetAsQuaternion();
		Quaternion finalRotQuat = Slerp(startRotQuat, endRotQuat, m_turnRate);
		m_orientation = finalRotQuat.GetAsRotMatrix();
		m_orientation.Orthonormalize_XFwd_YLeft_ZUp();
		m_movementIntentions.x += 1.0f;

		//determine if player has suddenly changed direction, and can therefore side flip
		if (DotProduct3D(movementDirection, m_velocity) < 0.0f && m_velocity.GetLength() > m_runThreshold)
		{
			m_canSideFlipTimer = m_canSideFlipTimerMax;
		}
	}
	//else if (leftStickMagnitude == 0.0f)
	//{
	//	bool usingIBasis = false;

	//	Vec3 moveForwardKBasis = cameraModelMatrix.GetKBasis3D();
	//	Vec3 moveForwardOnUpVectorKBasis = GetProjectedOnto3D(moveForwardKBasis, GetModelMatrix().GetKBasis3D());
	//	Vec3 moveForwardOnSurfacePlaneKBasis = (moveForwardKBasis - moveForwardOnUpVectorKBasis).GetNormalized();

	//	Vec3 moveForwardIBasis = cameraModelMatrix.GetIBasis3D();
	//	Vec3 moveForwardOnUpVectorIBasis = GetProjectedOnto3D(moveForwardIBasis, GetModelMatrix().GetKBasis3D());
	//	Vec3 moveForwardOnSurfacePlaneIBasis = (moveForwardIBasis - moveForwardOnUpVectorIBasis).GetNormalized();

	//	if ((moveForwardKBasis - moveForwardOnUpVectorKBasis).GetLength() < I_BASIS_THRESHOLD)
	//	{
	//		//usingIBasis = true;

	//		if (g_theGame->m_isDebugView) DebugAddMessage("Would change to IBasis here", 0.0f);
	//	}

	//	Vec3 moveForwardOnSurfacePlaneCorrectedKBasis = moveForwardOnSurfacePlaneKBasis;
	//	Vec3 moveForwardOnSurfacePlaneCorrectedIBasis = moveForwardOnSurfacePlaneIBasis;
	//	if (DotProduct3D(GetModelMatrix().GetKBasis3D(), cameraModelMatrix.GetIBasis3D()) > 0.0f)
	//	{
	//		moveForwardOnSurfacePlaneCorrectedKBasis = -moveForwardOnSurfacePlaneCorrectedKBasis;
	//		moveForwardOnSurfacePlaneCorrectedIBasis = -moveForwardOnSurfacePlaneCorrectedIBasis;

	//		if (g_theGame->m_isDebugView) DebugAddMessage("Flip Direction", 0.0f);
	//	}
	//	/*if (m_reverse)
	//	{
	//		moveForwardOnSurfacePlaneCorrected = -moveForwardOnSurfacePlaneCorrected;
	//	}*/

	//	Vec3 moveForwardOnSurfacePlaneCorrected = moveForwardOnSurfacePlaneCorrectedKBasis;
	//	if (usingIBasis || forceIBasis)
	//	{
	//		moveForwardOnSurfacePlaneCorrected = moveForwardOnSurfacePlaneCorrectedIBasis;

	//		if (g_theGame->m_isDebugView) DebugAddMessage("Change to IBasis", 0.0f);
	//	}

	//	if (moveForwardOnSurfacePlaneKBasis == moveForwardOnSurfacePlaneCorrectedKBasis)
	//	{
	//		if (g_theGame->m_isDebugView) DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneKBasis, 0.05f, 0.0f, Rgba8(255, 127, 0), Rgba8(255, 127, 0), DebugRenderMode::X_RAY);
	//	}
	//	else
	//	{
	//		if (g_theGame->m_isDebugView) DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneCorrectedKBasis, 0.05f, 0.0f, Rgba8(127, 0, 0), Rgba8(127, 0, 0), DebugRenderMode::X_RAY);
	//	}
	//	if (moveForwardOnSurfacePlaneIBasis == moveForwardOnSurfacePlaneCorrectedIBasis)
	//	{
	//		if (g_theGame->m_isDebugView) DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneIBasis, 0.05f, 0.0f, Rgba8(127, 255, 0), Rgba8(127, 255, 0), DebugRenderMode::X_RAY);
	//	}
	//	else
	//	{
	//		if (g_theGame->m_isDebugView) DebugAddWorldArrow(m_position, m_position + moveForwardOnSurfacePlaneCorrectedIBasis, 0.05f, 0.0f, Rgba8(0, 127, 0), Rgba8(0, 127, 0), DebugRenderMode::X_RAY);
	//	}

	//	Vec3 moveRight = CrossProduct3D(moveForwardOnSurfacePlaneCorrected, GetModelMatrix().GetKBasis3D());
	//	Vec3 moveRightOnUpVector = GetProjectedOnto3D(moveRight, GetModelMatrix().GetKBasis3D());
	//	Vec3 moveRightOnSurfacePlane = (moveRight - moveRightOnUpVector).GetNormalized();
	//	bool rightFlipped = false;
	//	if (DotProduct3D(GetModelMatrix().GetKBasis3D(), cameraModelMatrix.GetIBasis3D()) > 0.0f)
	//	{
	//		moveRightOnSurfacePlane = -moveRightOnSurfacePlane;
	//		rightFlipped = true;
	//	}
	//	/*if (m_reverse)
	//	{
	//		moveRightOnSurfacePlane = -moveRightOnSurfacePlane;
	//	}*/

	//	if (!rightFlipped)
	//	{
	//		if (g_theGame->m_isDebugView) DebugAddWorldArrow(m_position, m_position + moveRightOnSurfacePlane, 0.05f, 0.0f, Rgba8(127, 0, 255), Rgba8(127, 0, 255), DebugRenderMode::X_RAY);
	//	}
	//	else
	//	{
	//		if (g_theGame->m_isDebugView) DebugAddWorldArrow(m_position, m_position + moveRightOnSurfacePlane, 0.05f, 0.0f, Rgba8(0, 0, 127), Rgba8(0, 0, 127), DebugRenderMode::X_RAY);
	//	}

	//	/*if (DotProduct3D(GetModelMatrix().GetKBasis3D(), cameraModelMatrix.GetIBasis3D()) > 0.0f)
	//	{
	//		m_reverse = true;
	//	}
	//	else
	//	{
	//		m_reverse = false;
	//	}*/
	//}
	//if (m_reverse) DebugAddMessage("Reverse", 0.0f);






	//buttons
	if (controller.GetLeftTrigger() > 0.0f && m_isGrounded)
	{
		m_isCrouching = true;
	}
	if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_A))
	{
		Jump();
	}
	if (controller.IsButtonDown(XBOX_BUTTON_L))
	{
		m_movementIntentions.z -= 1.0f;
	}
	if (controller.IsButtonDown(XBOX_BUTTON_R))
	{
		m_movementIntentions.z += 1.0f;
	}
	
	if (controller.WasButtonJustPressed(XboxButtonID::XBOX_BUTTON_SELECT))
	{
		Respawn();
	}
}


void Player::Render() const
{
	//render as capsule
	std::vector<Vertex_PCUTBN> meshVerts;

	if (m_isCrouching)
	{
		AddVertsForCapsule3D(meshVerts, Vec3(0.0f, 0.0f, -m_meshHeight * 0.5f), Vec3(0.0f, 0.0f, 0.0f), m_meshRadius + 0.1f);
	}
	else
	{
		AddVertsForCapsule3D(meshVerts, Vec3(0.0f, 0.0f, -m_meshHeight * 0.5f), Vec3(0.0f, 0.0f, m_meshHeight * 0.5f), m_meshRadius);
	}

	Mat44 scaledModelMatrix = GetModelMatrix();
	float squashAmount = 1.0f / m_stretchAmount;
	scaledModelMatrix.AppendScaleNonUniform3D(Vec3(squashAmount, squashAmount, m_stretchAmount));
	if (m_doTripleJumpFlip)
	{
		scaledModelMatrix.AppendYRotation(m_tripleJumpFlipAngle);
	}
	else if (m_isBackFlipping)
	{
		scaledModelMatrix.AppendYRotation(m_backFlipAngle);
	}
	else if (m_isWallSliding)
	{
		scaledModelMatrix.AppendTranslation3D(Vec3(0.25f, 0.0f, 0.0f));
		scaledModelMatrix.AppendYRotation(5.0f);
	}
	scaledModelMatrix.AppendTranslation3D(Vec3(0.0f, 0.0f, -squashAmount + 1.0f));

	g_theRenderer->BindShader(g_theGame->m_lightingShader);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetModelConstants(scaledModelMatrix, m_color);
	g_theRenderer->DrawVertexArray(meshVerts);

	//render collision bounds as wireframe sphere
	if (g_theGame->m_isDebugView)
	{
		std::vector<Vertex_PCU> collisionVerts;

		AddVertsForSphere3D(collisionVerts, Vec3(), m_collisionRadius);

		g_theRenderer->BindShader(nullptr);
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_NONE);
		g_theRenderer->SetModelConstants(GetModelMatrix(), m_debugWireframeColor);
		g_theRenderer->DrawVertexArray(collisionVerts);

		DebugAddWorldArrow(m_position, m_position + m_velocity * 0.1f, 0.05f, 0.0f, Rgba8(255, 255, 0), Rgba8(255, 0, 0), DebugRenderMode::X_RAY);
	}

	DebugAddWorldArrow(m_position, m_position + m_orientation.GetIBasis3D() * 1.5f, 0.05f, 0.0f, Rgba8(255, 0, 0), Rgba8(255, 0, 0));
	DebugAddWorldArrow(m_position, m_position + m_orientation.GetJBasis3D() * 1.5f, 0.05f, 0.0f, Rgba8(0, 255, 0), Rgba8(0, 255, 0));
	DebugAddWorldArrow(m_position, m_position + m_orientation.GetKBasis3D() * 1.5f, 0.05f, 0.0f, Rgba8(0, 0, 255), Rgba8(0, 0, 255));
}


//
//physics functions
//
void Player::UpdatePhysics(float deltaSeconds)
{
	//DebugAddMessage(Stringf("%.4f", deltaSeconds), 0.0f);

	if (m_isFreeFlyMode)
	{
		m_currentGravityCenter = Vec3();
		m_currentGravitySource = nullptr;
		m_currentGravityVector = Vec3();
	}

	AddGravity(m_currentGravityVector);
	if (!m_rememberLastGravitySource)
	{
		m_currentGravityVector = Vec3(0.0f, 0.0f, -100.0f);
	}
	
	Vec3 dragForce = (Vec3() - m_velocity) * m_drag;
	AddForce(dragForce);

	m_velocity += m_acceleration * deltaSeconds;
	//if (g_theGame->m_isDebugView) DebugAddWorldArrow(m_position, m_position + m_velocity * 0.1f, 0.05f, 0.0f, Rgba8(255, 255, 0), Rgba8(255, 255, 0), DebugRenderMode::X_RAY);
	m_position += m_velocity * deltaSeconds;

	/*if (m_rotationAlpha < 1.0f)
	{
		m_rotationAlpha += m_orientationMatchRate * deltaSeconds;
		if (m_rotationAlpha > 1.0f) m_rotationAlpha = 1.0f;
	}*/

	m_acceleration = Vec3();
}


void Player::AddForce(Vec3 const& forceVector)
{
	m_acceleration += forceVector;
}


void Player::AddGravity(Vec3 gravityVector)
{
	if (!m_isGrounded/* || m_movementIntentions != Vec3()*/)
	{	
		if (DotProduct3D(m_velocity, gravityVector) > 0.0f && !m_isLongJumping)
		{
			if (m_isWallSliding)
			{
				AddForce(gravityVector * m_fallSpeedScalar * m_wallSlideResistance);
			}
			else
			{
				AddForce(gravityVector * m_fallSpeedScalar);
			}
		}
		else
		{
			float longJumpScalar = 1.0f;
			if (m_isLongJumping)
			{
				longJumpScalar += m_longJumpDurationTimer * m_longJumpDurationTimerScale;
			}

			AddForce(gravityVector * longJumpScalar);
		}
	}

	if (gravityVector != Vec3())
	{
		if (g_theGame->m_isDebugView) DebugAddWorldArrow(m_position, m_position + gravityVector * 0.01f, 0.05f, 0.0f, Rgba8(255, 0, 255), Rgba8(255, 0, 255), DebugRenderMode::X_RAY);
		
		//lerp orientation to match
		Quaternion startRotQuat = m_orientation.GetAsQuaternion();

		Vec3 kBasis = -(gravityVector).GetNormalized();
		Vec3 iBasis = CrossProduct3D(m_orientation.GetJBasis3D(), kBasis);
		iBasis.Normalize();
		Vec3 jBasis = CrossProduct3D(kBasis, iBasis);
		jBasis.Normalize();
		Mat44 endRotMat = Mat44(iBasis, jBasis, kBasis, Vec3());
		Quaternion endRotQuat = endRotMat.GetAsQuaternion();

		Quaternion finalRotQuat = Slerp(startRotQuat, endRotQuat, m_orientationMatchRate);

		m_orientation = finalRotQuat.GetAsRotMatrix();
		m_orientation.Orthonormalize_XFwd_YLeft_ZUp();

		//rotate velocity by same amount that player rotated
		//Quaternion changeInRot = finalRotQuat - startRotQuat;
		//Mat44 rotChangeMat = changeInRot.GetAsRotMatrix();
		//m_velocity = rotChangeMat.TransformVectorQuantity3D(m_velocity);
		Mat44 startRotMat = startRotQuat.GetAsRotMatrix();
		Vec3 startIBasis = startRotMat.GetIBasis3D();
		//Vec3 finalIBasis = m_orientation.GetIBasis3D();
		float degreesRotated = GetAngleDegreesBetweenVectors3D(startIBasis, iBasis);
		if (g_theGame->m_isDebugView)
		{
			std::string mes = Stringf("Start velocity: %.2f, %.2f, %.2f", m_velocity.x, m_velocity.y, m_velocity.z);
			DebugAddMessage(mes, 0.0f);
			std::string mes1 = Stringf("Degrees rotated: %.2f", degreesRotated);
			DebugAddMessage(mes1, 0.0f);
		}
		if (degreesRotated != 0.0f)
		{
			Vec3 newVelocity = m_velocity.GetRotatedAroundAxisByAngle(jBasis, degreesRotated);
			if (newVelocity.x == newVelocity.x)
			{
				m_velocity = newVelocity;
			}
		}
		if (g_theGame->m_isDebugView)
		{
			std::string mes = Stringf("End velocity: %.2f, %.2f, %.2f", m_velocity.x, m_velocity.y, m_velocity.z);
			DebugAddMessage(mes, 0.0f);
		}
	}
	else
	{
		//lerp orientation to match default
		if (!m_rememberLastGravitySource)
		{
			Quaternion startRotQuat = m_orientation.GetAsQuaternion();

			Vec3 kBasis = Vec3(0.0f, 0.0f, GRAVITY_STANDARD).GetNormalized();
			Vec3 iBasis = CrossProduct3D(m_orientation.GetJBasis3D(), kBasis);
			iBasis.Normalize();
			Vec3 jBasis = CrossProduct3D(kBasis, iBasis);
			jBasis.Normalize();
			Mat44 endRotMat = Mat44(iBasis, jBasis, kBasis, Vec3());
			Quaternion endRotQuat = endRotMat.GetAsQuaternion();

			Quaternion finalRotQuat = Slerp(startRotQuat, endRotQuat, m_orientationMatchRate);

			m_orientation = finalRotQuat.GetAsRotMatrix();
			m_orientation.Orthonormalize_XFwd_YLeft_ZUp();
		}	
	}
}


void Player::SetGravitySource(GravityField const* gravitySource, Vec3 const& gravityCenter, Vec3 const& gravityVector)
{
	if (m_currentGravitySource == nullptr)
	{
		m_currentGravitySource = gravitySource;
		m_currentGravityCenter = gravityCenter;
		m_currentGravityVector = gravityVector;
		//m_rotationAlpha = 0.0f;
	}
	else if (m_currentGravitySource == gravitySource)
	{
		m_currentGravityVector = gravityVector;
		m_currentGravityCenter = gravityCenter;
	}
	else
	{
		if (GetDistanceSquared3D(m_position, gravityCenter) < GetDistanceSquared3D(m_position, m_currentGravityCenter))
		{
			m_currentGravitySource = gravitySource;
			m_currentGravityCenter = gravityCenter;
			m_currentGravityVector = gravityVector;
			//m_rotationAlpha = 0.0f;
		}
	}
}


void Player::AddImpulse(Vec3 const& impulseVector)
{
	m_velocity += impulseVector;
}


void Player::MoveInDirection(Vec3 const& directionNormal, float speed)
{
	Vec3 forceVector = directionNormal.GetNormalized() * speed * m_drag;

	if (m_isCrouching) forceVector *= m_crouchSpeedScale;

	AddForce(forceVector);
}


void Player::Jump()
{
	if (!(m_isGrounded || m_wasGroundedLastFrame || m_isWallSliding)) return;

	if (m_isWallSliding)
	{
		WallJump();
		return;
	}
	
	float modifiedJumpForce = m_jumpForce;
	float modifiedStretch = m_stretchJumpBase;
	m_jumpNumber++;

	if (m_movementIntentions.x == 0 && m_jumpNumber >= 3)
	{
		m_jumpNumber = 1;
	}

	//long jump logic
	if (m_isCrouching && m_movementIntentions.x > 0.0f)
	{
		m_jumpNumber = 0;
		modifiedJumpForce *= m_longJumpHeightScale;
		m_isLongJumping = true;
		g_theGame->m_longJumpTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds());
	}
	//back flip logic
	else if (m_isCrouching)
	{
		m_jumpNumber = 0;
		modifiedJumpForce *= m_backFlipJumpScale;
		m_isBackFlipping = true;
		g_theGame->m_backFlipTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds());
	}
	//side flip logic
	else if (m_canSideFlipTimer > 0.0f)
	{
		m_jumpNumber = 0;
		modifiedJumpForce *= m_sideFlipJumpScale;
		m_isSideFlipping = true;
		g_theGame->m_sideFlipTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds());
	}

	//decide how high to jump based on the jump number
	switch (m_jumpNumber)
	{
		case 1: g_theGame->m_standardJumpTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds()); break;
		case 2: modifiedJumpForce *= m_doubleJumpScalar; modifiedStretch += (m_doubleJumpScalar * 0.25f); g_theGame->m_doubleJumpTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds()); break;
		case 3: modifiedJumpForce *= m_tripleJumpScalar; modifiedStretch += (m_tripleJumpScalar * 0.25f); m_doTripleJumpFlip = true; g_theGame->m_tripleJumpTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds()); break;
	}

	//actually perform the jump
	AddImpulse(m_orientation.GetKBasis3D() * modifiedJumpForce);
	if (m_isBackFlipping)
	{
		AddImpulse(-m_orientation.GetIBasis3D() * m_backFlipMoveImpulse);
	}
	if (m_isSideFlipping)
	{
		//flip orientation
		m_orientation.SetIJK3D(-m_orientation.GetIBasis3D(), -m_orientation.GetJBasis3D(), m_orientation.GetKBasis3D());
		m_orientation.Orthonormalize_XFwd_YLeft_ZUp();

		AddImpulse(m_orientation.GetIBasis3D() * m_sideFlipMoveImpulse);
	}
	modifiedStretch = GetClamped(modifiedStretch, 1.0f, m_maxStretch);
	m_stretchAmount = modifiedStretch;
}


void Player::WallJump()
{
	m_jumpNumber = 0;
	m_isWallSliding = false;
	m_isWallJumping = true;
	m_wallJumpTimer = m_wallJumpTimerMax;
	//m_numWallJumps++;
	g_theGame->m_wallJumpTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds());

	//flip orientation
	m_orientation.SetIJK3D(m_wallSlideNormal, -m_orientation.GetJBasis3D(), m_orientation.GetKBasis3D());
	m_orientation.Orthonormalize_XFwd_YLeft_ZUp();

	//add impulse
	Vec3 wallJumpDirection = (m_orientation.GetIBasis3D() + m_orientation.GetKBasis3D() * 2.0f).GetNormalized();
	AddImpulse(wallJumpDirection * m_wallJumpImpulse);
}


void Player::BecomeGrounded()
{
	m_isGrounded = true;
	m_landingVelocity = m_velocity;
	m_isWallSliding = false;
	m_isWallJumping = false;
}


void Player::StartWallSlide(Vec3 const& wallNormal)
{
	m_isWallSliding = true;
	m_wallSlideNormal = wallNormal;
	m_jumpNumber = 0;
	m_doTripleJumpFlip = false;
	m_tripleJumpFlipAngle = 0.0f;
}


//
//public player utilities
//
Mat44 Player::GetModelMatrix() const
{
	Mat44 modelMatrix = m_orientation;
	modelMatrix.SetTranslation3D(m_position);
	return modelMatrix;
}


Vec3 Player::GetIBasis() const
{
	return GetModelMatrix().GetIBasis3D();
}


Vec3 Player::GetJBasis() const
{
	return GetModelMatrix().GetJBasis3D();
}


Vec3 Player::GetKBasis() const
{
	return GetModelMatrix().GetKBasis3D();
}


void Player::Respawn()
{
	if (g_theGame->m_currentCheckpoint == nullptr)
	{
		m_position = m_playerStartPosition;
	}
	else
	{
		m_position = g_theGame->m_currentCheckpoint->GetCenter();
	}
	m_orientation = Mat44();
	m_currentGravitySource = nullptr;
	m_currentGravityCenter = Vec3();
	m_currentGravityVector = Vec3();
	m_acceleration = Vec3();
	m_velocity = Vec3();

	switch (g_theGame->m_currentSection)
	{
		case 1: /*m_numSection1Respawns++;*/ g_theGame->m_section1RespawnTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds()); break;
		case 2: /*m_numSection2Respawns++;*/ g_theGame->m_section2RespawnTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds()); break;
		case 3: /*m_numSection3Respawns++;*/ g_theGame->m_section3RespawnTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds()); break;
		case 4: /*m_numSection4Respawns++;*/ g_theGame->m_section4RespawnTimes.emplace_back(g_theGame->m_gameClock.GetTotalSeconds()); break;
	}
}
