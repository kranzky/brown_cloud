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

//==============================================================================
Cloud::Cloud( float scale )
    :
    Entity( scale ),
	m_particles(NULL),
	m_clump(NULL)
{
}

//------------------------------------------------------------------------------
Cloud::~Cloud()
{
}

//------------------------------------------------------------------------------
void
Cloud::collide( Entity * entity, b2ContactPoint * point )
{
	if (entity->getType() == Cloud::TYPE)
	{
		Engine::cm()->reportCollision(this, entity, point->position);
	}

	//if (entity->getType() == Cloud::TYPE)
	//{
	//	b2RevoluteJointDef joint;
	//	joint.Initialize( this->getBody(), entity->getBody(), point->position );
	//	Engine::instance()->b2d()->CreateJoint( & joint );
	//}
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
	shapeDef.radius = 0.3f * m_sprite->GetWidth() * m_scale;
	//shapeDef.localPosition.Set(1.0f, 0.0f);
	shapeDef.density = 10.0f;
	shapeDef.friction = 0.3f;

	b2BodyDef bodyDef;
	bodyDef.userData = static_cast<void*> (this);
	m_body = Engine::b2d()->CreateDynamicBody(&bodyDef);
	m_body->CreateShape(&shapeDef);
	m_body->SetMassFromShapes();

	m_particles = new hgeParticleSystem( * Engine::rm()->GetParticleSystem( "cloud" ) );
    m_particles->SetScale( m_scale * 0.3f );
	m_particles->Fire();
}

//------------------------------------------------------------------------------
void
Cloud::doUpdate( float dt )
{
	b2Vec2 position( m_body->GetPosition() );
	m_particles->MoveTo( position.x / (m_scale * 0.3f), position.y / (m_scale * 0.3f) );
    m_particles->Update( dt );
}

//------------------------------------------------------------------------------
void
Cloud::doRender()
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    //m_sprite->RenderEx( position.x, position.y, angle, m_scale );
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

//==============================================================================

