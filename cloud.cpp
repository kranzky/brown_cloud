//==============================================================================

#include <cloud.hpp>
#include <engine.hpp>
#include <entity_manager.hpp>

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
    Entity( scale )
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
	shapeDef.radius = 0.5f * m_sprite->GetWidth() * m_scale;
	shapeDef.localPosition.Set(1.0f, 0.0f);
	shapeDef.density = 1.0f;
	shapeDef.friction = 0.3f;

    //b2PolygonDef shapeDef;
    //shapeDef.SetAsBox( 0.5f * m_sprite->GetWidth() * m_scale,
    //                   0.5f * m_sprite->GetHeight() * m_scale );

	b2BodyDef bodyDef;
	bodyDef.userData = static_cast<void*> (this);
	m_body = Engine::b2d()->CreateDynamicBody(&bodyDef);
	m_body->CreateShape(&shapeDef);
	m_body->SetMassFromShapes();

    //b2BodyDef bodyDef;
    //bodyDef.userData = static_cast< void * >( this );
    //m_body = Engine::b2d()->CreateStaticBody( & bodyDef );
    //m_body->CreateShape( & shapeDef );


	hgeParticleSystem * p ( Engine::rm()->GetParticleSystem( "cloud" ) );
    p->SetScale( m_scale * 0.3f );
	p->Fire();
}

//------------------------------------------------------------------------------
void
Cloud::doUpdate( float dt )
{
	b2Vec2 position( m_body->GetPosition() );
	hgeParticleSystem * p ( Engine::rm()->GetParticleSystem( "cloud" ) );
	p->MoveTo( position.x / (m_scale * 0.3f), position.y / (m_scale * 0.3f) );
    p->Update( dt );
}

//------------------------------------------------------------------------------
void
Cloud::doRender()
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    //m_sprite->RenderEx( position.x, position.y, angle, m_scale );
	hgeParticleSystem * p ( Engine::rm()->GetParticleSystem( "cloud" ) );
    p->Render();
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

