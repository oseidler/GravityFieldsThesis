#pragma once
#include "Game/GravityFields.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"


//forward declarations
class Player;
class Model;


//abstract base class
class Planetoid
{
//public member functions
public:
	//constructor and destructor
	Planetoid(Vec3 position, EulerAngles orientation = EulerAngles(), Rgba8 color = Rgba8()) : m_position(position), m_orientation(orientation), m_color(color) {}
	virtual ~Planetoid() 
	{
		if (m_field != nullptr) delete m_field;
	}

	//game flow functions
	virtual void Render() const = 0;
	virtual void DebugRender() const;

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) = 0;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const = 0;

	//math utilities
	Mat44 GetModelMatrix() const;

//public member variables
public:
	Vec3 m_position;
	EulerAngles m_orientation; //Shouldn't need to bother with quaternions for planetoids since they're stationary
	Rgba8 m_color;

	GravityField* m_field = nullptr;

	std::vector<Vertex_PCUTBN> m_verts;
};


//subclasses
class PlanePLTD : public Planetoid
{
//public member functions
public:
	//constructor
	PlanePLTD(Vec3 position, float halfLength, float halfWidth, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce = GRAVITY_STANDARD, Rgba8 color = Rgba8());
	
	//game flow functions
	virtual void Render() const override;
	static  void RenderPreview(Vec3 position, float halfLength, float halfWidth, EulerAngles orientation, float gravityHeight, Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;

//public member variables
public:
	float m_halfLength = 1.0f;
	float m_halfWidth = 1.0f;
};


class SpherePLTD : public Planetoid
{
//public member functions
public:
	//constructor
	SpherePLTD(Vec3 position, float radius, bool includeField, float gravityRadius, float gravityForce = GRAVITY_STANDARD, Rgba8 color = Rgba8());

	//game flow functions
	virtual void Render() const override;
	static  void RenderPreview(Vec3 position, float radius, float gravityRadius, Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;

//public member variables
public:
	float m_radius = 1.0f;
};


class CapsulePLTD : public Planetoid
{
//public member functions
public:
	//constructor
	CapsulePLTD(Vec3 position, float radius, float boneLength, Vec3 boneDirection, bool includeField, float gravityRadius, float gravityForce = GRAVITY_STANDARD, Rgba8 color = Rgba8());

	//game flow functions
	virtual void Render() const override;
	static void  RenderPreview(Vec3 position, float radius, float boneLength, Vec3 boneDirection, float gravityRadius, Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;

//public member variables
public:
	float m_radius = 1.0f;
	float m_boneLength = 1.0f;
	Vec3  m_boneDirection = Vec3(1.0f, 0.0f, 0.0f);
	Vec3  m_boneEnd = Vec3();
};


class EllipsoidPLTD : public Planetoid
{
//public member functions
public:
	//constructor
	EllipsoidPLTD(Vec3 position, float xRadius, float yRadius, float zRadius, EulerAngles orientation, bool includeField, float gravityXRadius, float gravityYRadius, float gravityZRadius,
		float gravityForce = GRAVITY_STANDARD, Rgba8 color = Rgba8());

	//game flow functions
	virtual void Render() const override;
	static void  RenderPreview(Vec3 position, float xRadius, float yRadius, float zRadius, EulerAngles orientation, float gravityXRadius, float gravityYRadius, float gravityZRadius, 
		Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;

//public member variables
public:
	float m_xRadius = 1.0f;
	float m_yRadius = 1.0f;
	float m_zRadius = 1.0f;
};


class RoundCubePLTD : public Planetoid
{
//public member functions
public:
	//constructor
	RoundCubePLTD(Vec3 position, float length, float width, float height, float roundedness, EulerAngles orientation, bool includeField, float gravityLength, float gravityWidth, float gravityHeight,
		float gravityForce = GRAVITY_STANDARD, Rgba8 color = Rgba8());

	//game flow functions
	virtual void Render() const override;
	static  void RenderPreview(Vec3 position, float length, float width, float height, float roundedness, EulerAngles orientation, float gravityLength, float gravityWidth, float gravityHeight,
		Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;

//public member variables
public:
	float m_length = 1.0f;
	float m_width = 1.0f;
	float m_height = 1.0f;
	float m_roundedness = 1.0f;
};


class TorusPLTD : public Planetoid
{
//public member functions
public:
	//constructor
	TorusPLTD(Vec3 position, float tubeRadius, float holeRadius, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce = GRAVITY_STANDARD, Rgba8 color = Rgba8());

	//game flow functions
	virtual void Render() const override;
	static  void RenderPreview(Vec3 position, float tubeRadius, float holeRadius, EulerAngles orientation, float gravityRadius, Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;

//public member variables
public:
	float m_tubeRadius = 1.0f;
	float m_holeRadius = 1.0f;
};


class BowlPLTD : public Planetoid
{
//public member functions
public:
	//constructor
	BowlPLTD(Vec3 position, float radius, float thickness, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce = GRAVITY_STANDARD, Rgba8 color = Rgba8());

	//game flow functions
	virtual void Render() const override;
	static  void RenderPreview(Vec3 position, float radius, float thickness, EulerAngles orientation, float gravityRadius, Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;
	bool		 PushSphereOutOfPlanetoid(Vec3& sphereCenter, float sphereRadius);

//private member functions
private:
	//bowl-specific stuff
	void AddVertsForBowl();
	static void AddVertsForBowl(std::vector<Vertex_PCU>& verts, float radius, float thickness);

//public member variables
public:
	float m_radius = 1.0f;
	float m_thickness = 0.1f;
};


class MobiusPLTD : public Planetoid
{
//public member functions
public:
	//constructor
	MobiusPLTD(Vec3 position, float radius, float halfWidth, EulerAngles orientation, bool includeField, float gravityHeight, float gravityForce = GRAVITY_STANDARD, Rgba8 color = Rgba8());

	//game flow functions
	virtual void Render() const override;
	static void  RenderPreview(Vec3 position, float radius, float halfWidth, EulerAngles orientation, float gravityHeight, Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;

//public member variables
public:
	float m_radius = 1.0f;
	float m_halfWidth = 1.0f;
};


//structure for all the perlin-related parameters of the wire planetoid
struct WirePerlinParameters
{
	int m_rngSeed = 0;
	int m_minSegments = 1;
	int m_maxSegments = 2;
	float m_segMinLength = 1.0f;
	float m_segMaxLength = 2.0f;
	float m_maxYawChange = 90.0f;
	float m_maxPitchChange = 90.0f;

	float m_perlinScaleYaw = 1.0f;
	int   m_perlinOctavesYaw = 1;
	float m_perlinOctavePersistYaw = 0.5f;
	float m_perlinOctaveScaleYaw = 2.0f;

	float m_perlinScalePitch = 1.0f;
	int   m_perlinOctavesPitch = 1;
	float m_perlinOctavePersistPitch = 0.5f;
	float m_perlinOctaveScalePitch = 2.0f;
};


class WirePLTD : public Planetoid
{
//public member functions
public:
	//constructor
	WirePLTD(Vec3 position, float radius, WirePerlinParameters perlinStruct, EulerAngles orientation, bool includeField, float gravityRadius, float gravityForce, Rgba8 color);

	//game flow functions
	virtual void Render() const override;
	static  void RenderPreview(Vec3 position, float radius, WirePerlinParameters perlinStruct, EulerAngles orientation, float gravityRadius, Rgba8 color);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;
	void		 AddVertsForWire(std::vector<Vertex_PCUTBN>& verts) const;
	static void  AddVertsForWireStatic(std::vector<Vertex_PCU>& verts, std::vector<Vec3> const& wirePositions, float radius);

//public member variables
public:
	float m_radius = 1.0f;
	std::vector<Vec3> m_wirePositions;
};


//prefab planetoids here
class PrefabPLTD : public Planetoid
{
//public member functions
public:
	//constructor
	PrefabPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color) : Planetoid(position, orientation, color), m_scale(scale) {}
	~PrefabPLTD();

	//game flow functions
	virtual void Render() const override;
	static  void RenderPreview(Model* model, Vec3 position, EulerAngles orientation, Rgba8 color, bool includeField, int modelType, float gravScale);

	//planetoid utilities
	virtual bool CollideWithPlayer(Player* player) override;
	virtual Vec3 GetNearestPointOnPlanetoid(Vec3 playerPos) const override;

//public member variables
public:
	Model* m_model = nullptr;
	float  m_scale = 1.0f;
};


class TeapotPLTD : public PrefabPLTD
{
//public member functions
public:
	//constructor
	TeapotPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color, bool includeField, float gravityRadius, float gravityForce);

//public member variables
public:

};


class SkyStationPLTD : public PrefabPLTD
{
//public member functions
public:
	//constructor
	SkyStationPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color, bool includeField, float gravityRadius, float gravityForce);

//public member variables
public:

};


class MountainPLTD : public PrefabPLTD
{
//public member functions
public:
	//constructor
	MountainPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color, bool includeField, float gravityRadius, float gravityForce);

//public member variables
public:

};


class FortressPLTD : public PrefabPLTD
{
//public member functions
public:
	//constructor
	FortressPLTD(Vec3 position, float scale, EulerAngles orientation, Rgba8 color, bool includeField, float gravityHeight, float gravityForce);

//public member variables
public:

};
