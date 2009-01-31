//==============================================================================

#include <cloud.hpp>
#include <engine.hpp>
#include <entity_manager.hpp>
#include <clump_manager.hpp>

#include <hgesprite.h>
#include <Box2D.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>
#include <hgeparticle.h>
#include <hgeresource.h>

namespace
{
    const float RD[5] = { 0.69f, 0.0f, 0.0f, 1.0f, 1.0f };
    const float GN[5] = { 0.44f, 1.0f, 0.0f, 0.0f, 1.0f };
    const float BL[5] = { 0.10f, 0.0f, 1.0f, 0.0f, 1.0f };
}

//==============================================================================
Cloud::Cloud( float scale )
    :
    Entity( scale ),
	m_particles(NULL),
	m_clump(NULL),
    m_zoom( 0 )
{
}

//------------------------------------------------------------------------------
Cloud::~Cloud()
{
    delete m_particles;
}

//------------------------------------------------------------------------------
void
Cloud::collide( Entity * entity, b2ContactPoint * point )
{
	if (entity->getType() == Cloud::TYPE)
	{
		Engine::cm()->reportCollision(this, reinterpret_cast<Cloud*>(entity), point->position);
	}
}

//------------------------------------------------------------------------------
void
Cloud::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "angle", "%f", "scale", "%f",
                      "sprite_id", "%d" };
    m_id = Engine::em()->persistToDatabase( this, rows, m_body->GetPosition().x,
                                                        m_body->GetPosition().y,
                                                        m_body->GetAngle(),
                                                        m_scale, m_sprite_id );
}

//------------------------------------------------------------------------------
// static:
//------------------------------------------------------------------------------
void
Cloud::registerEntity()
{
    Engine::em()->registerEntity( Cloud::TYPE, Cloud::factory, "clouds",
                                  "id, x, y, angle, scale, sprite_id" );
}

//------------------------------------------------------------------------------
// protected:
//------------------------------------------------------------------------------
void
Cloud::doInit()
{
	b2CircleDef shapeDef;
	shapeDef.radius = m_sprite->GetWidth() * m_scale;
	shapeDef.density = 1.0f;
	shapeDef.friction = 0.0f;
	shapeDef.restitution = 0.7f;

    m_zoom = 0;

	b2BodyDef bodyDef;
	bodyDef.userData = static_cast<void*> (this);
	m_body = Engine::b2d()->CreateDynamicBody(&bodyDef);
	m_body->CreateShape(&shapeDef);
	m_body->SetMassFromShapes();
    m_body->m_linearDamping = 0.2f;

	m_particles = new hgeParticleSystem( * Engine::rm()->GetParticleSystem( "cloud" ) );
    m_particles->SetScale( m_scale );
	m_particles->Fire();
    hgeParticleSystemInfo & info( m_particles->info );
    info.fRadialAccelMin = 0.0f;
    info.fRadialAccelMax = 0.0f;
    info.fTangentialAccelMin = 0.0f;
    info.fTangentialAccelMin = 0.0f;
    info.fGravityMin = 0.0f;
    info.fGravityMax = 0.0f;
}

//------------------------------------------------------------------------------
void
Cloud::doUpdate( float dt )
{
	b2Vec2 position( m_body->GetPosition() );
	m_particles->MoveTo( position.x / m_scale, position.y / m_scale );
    m_particles->Update( dt );
}

//------------------------------------------------------------------------------
void
Cloud::doRender( float scale )
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    hgeParticleSystemInfo & info( m_particles->info );
    float alpha( 0.5f );
    float ratio( scale / m_scale );
    if ( ratio > 1.0f )
    {
		ratio = 1.0f / ratio;
	}
    ratio = 1.0f - ratio;
    ratio = 1.0f - ratio * ratio;
	alpha *= ratio;
    info.colColorStart.a = alpha;
    ratio = 1.0f - ratio;
    ratio = 1.0f - ratio * ratio;
	info.colColorStart.r = RD[m_zoom] * ratio;
	info.colColorStart.g = GN[m_zoom] * ratio;
	info.colColorStart.b = BL[m_zoom] * ratio;
	info.colColorEnd.r = 0.1f * info.colColorStart.r + 0.2f;
	info.colColorEnd.g = 0.1f * info.colColorStart.g + 0.2f;
	info.colColorEnd.b = 0.1f * info.colColorStart.b + 0.2f;
    m_particles->Render();
}

//------------------------------------------------------------------------------
void
Cloud::initFromQuery( Query & query )
{
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );

    m_id = static_cast< sqlite_int64 >( query.getnum() );
    position.x = static_cast< float >( query.getnum() );
    position.y = static_cast< float >( query.getnum() );
    angle = static_cast< float >( query.getnum() );
    m_scale = static_cast< float >( query.getnum() );

    setSpriteID( static_cast< sqlite_int64 >( query.getnum() ) );

    init();

    m_body->SetXForm( position, angle );
}

//------------------------------------------------------------------------------
void Cloud::removeFromClump()
{
	if (m_clump != NULL)
	{
		Engine::cm()->removeCloudFromClump(this, m_clump);
		m_clump = NULL;

		//disconnect any physics joints
		while (getBody()->GetJointList() != NULL)
		{
			b2Joint* joint = getBody()->GetJointList()->joint;
			Engine::instance()->b2d()->DestroyJoint(joint);
		}
	}
}

//------------------------------------------------------------------------------
void
Cloud::setZoom( int zoom )
{
    m_zoom = zoom;
}

//------------------------------------------------------------------------------
int
Cloud::getZoom()
{
    return m_zoom;
}

//==============================================================================

