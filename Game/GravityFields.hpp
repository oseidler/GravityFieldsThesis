#pragma once
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/Rgba8.hpp"


//forward declarations
class Planetoid;
class Player;


//constants
constexpr float GRAVITY_STANDARD = 100.0f;
const Rgba8 g_gravFieldColor = Rgba8(255, 255, 255, 50);
const Rgba8 g_previewGravFieldColor = Rgba8(255, 255, 255, 20);


//abstract base class
class GravityField
{
//public member functions
public:
	//constructor and destructor
	explicit GravityField(Planetoid* planetoid, float force = GRAVITY_STANDARD, Vec3 offset = Vec3()) : m_planetoid(planetoid), m_force(force), m_offset(offset) {}
	virtual ~GravityField() {}
	
	//gravity utilities
	virtual void ApplyGravity(Player* player) const = 0;
	virtual void DebugRender() const = 0;

//public member variables
public:
	Planetoid* m_planetoid = nullptr;
	
	float m_force = GRAVITY_STANDARD;
	Vec3 m_offset = Vec3();
};


//subclasses
class PlaneField : public GravityField
{
//public member functions
public:
	//constructor
	explicit PlaneField(Planetoid* planetoid, float halfLength, float halfWidth, float height, float force = GRAVITY_STANDARD, Vec3 offset = Vec3()) : GravityField(planetoid, force, offset), 
		m_halfLength(halfLength), m_halfWidth(halfWidth), m_height(height) {}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_height = 1.0f;
	float m_halfLength = 1.0f;
	float m_halfWidth = 1.0f;
};


class SphereField : public GravityField
{
//public member functions
public:
	//constructor
	explicit SphereField(Planetoid* planetoid, float radius, float force = GRAVITY_STANDARD, Vec3 offset = Vec3()) : GravityField(planetoid, force, offset), m_radius(radius) {}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_radius = 1.0f;
};


class CapsuleField : public GravityField
{
//public member functions
public:
	//constructor
	explicit CapsuleField(Planetoid* planetoid, float radius, Vec3 boneStart, Vec3 boneEnd, float force = GRAVITY_STANDARD, Vec3 offset = Vec3()) : GravityField(planetoid, force, offset), 
		m_radius(radius), m_boneStart(boneStart), m_boneEnd(boneEnd) {}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_radius = 1.0f;
	Vec3  m_boneStart = Vec3();
	Vec3  m_boneEnd = Vec3();
};


class EllipsoidField : public GravityField
{
//public member functions
public:
	//constructor
	explicit EllipsoidField(Planetoid* planetoid, float xRadius, float yRadius, float zRadius, float force = GRAVITY_STANDARD, Vec3 offset = Vec3())
		: GravityField(planetoid, force, offset)
		, m_xRadius(xRadius)
		, m_yRadius(yRadius)
		, m_zRadius(zRadius)
	{}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_xRadius = 1.0f;
	float m_yRadius = 1.0f;
	float m_zRadius = 1.0f;
};


class RoundCubeField : public GravityField
{
//public member functions
public:
	//constructor
	explicit RoundCubeField(Planetoid* planetoid, float length, float width, float height, float force = GRAVITY_STANDARD, Vec3 offset = Vec3())
		: GravityField(planetoid, force, offset)
		, m_length(length)
		, m_width(width)
		, m_height(height)
	{}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_length = 1.0f;
	float m_width = 1.0f;
	float m_height = 1.0f;
};


class TorusField : public GravityField
{
//public member functions
public:
	//constructor
	explicit TorusField(Planetoid* planetoid, float tubeRadius, float holeRadius, float force = GRAVITY_STANDARD, Vec3 offset = Vec3())
		: GravityField(planetoid, force, offset)
		, m_tubeRadius(tubeRadius)
		, m_holeRadius(holeRadius)
	{}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_tubeRadius = 1.0f;
	float m_holeRadius = 1.0f;
};


class BowlField : public GravityField
{
//public member functions
public:
	//constuctor
	explicit BowlField(Planetoid* planetoid, float radius, float height, float innerRadius, float force = GRAVITY_STANDARD, Vec3 offset = Vec3()) : GravityField(planetoid, force, offset), 
		m_radius(radius), m_height(height), m_innerRadius(innerRadius) {}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_radius = 1.0f;
	float m_height = 1.0f;
	float m_innerRadius = 1.0f;
};


class MobiusField : public GravityField
{
//public member functions
public:
	//constuctor
	//explicit GravityField(Planetoid* planetoid, ) {}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:

};


class WireField : public GravityField
{
//public member functions
public:
	//constructor
	explicit WireField(Planetoid* planetoid, float radius, float force, Vec3 offset = Vec3())
		: GravityField(planetoid, force, offset)
		, m_radius(radius)
	{}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_radius = 1.0f;
};


//prefab-exclusive fields
class CylinderField : public GravityField
{
//public member functions
public:
	//constructor
	explicit CylinderField(Planetoid* planetoid, float outerRadius, float innerRadius, Vec3 start, Vec3 end, float force, Vec3 offset = Vec3())
		: GravityField(planetoid, force, offset)
		, m_outerRadius(outerRadius)
		, m_innerRadius(innerRadius)
		, m_start(start)
		, m_end(end)
	{}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_outerRadius = 1.0f;
	float m_innerRadius = 1.0f;
	Vec3  m_start = Vec3();
	Vec3  m_end = Vec3();
};


class WedgeField : public GravityField
{
//public member functions
public:
	//constructor
	explicit WedgeField(Planetoid* planetoid, float radius, Vec3 start, Vec3 end, float forwardDegrees, float apertureDegrees, float force, Vec3 offset = Vec3())
		: GravityField(planetoid, force, offset)
		, m_radius(radius)
		, m_start(start)
		, m_end(end)
		, m_forwardDegrees(forwardDegrees)
		, m_apertureDegrees(apertureDegrees)
	{}

	//gravity utilities
	virtual void ApplyGravity(Player* player) const override;
	virtual void DebugRender() const override;

//public member variables
public:
	float m_radius = 1.0f;
	Vec3  m_start = Vec3();
	Vec3  m_end = Vec3();
	float m_forwardDegrees = 0.0f;
	float m_apertureDegrees = 90.0f;
};
