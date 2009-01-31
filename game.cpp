//==============================================================================

#include <game.hpp>
#include <engine.hpp>
#include <entity.hpp>
#include <entity_manager.hpp>
#include <clump_manager.hpp>
#include <viewport.hpp>
#include <fujin.hpp>
#include <cloud.hpp>
#include <score.hpp>

#include <hgeresource.h>

//==============================================================================
Game::Game()
    :
    Context()
{
}

//------------------------------------------------------------------------------
Game::~Game()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Game::init()
{
    HGE * hge( Engine::hge() );
    b2World * b2d( Engine::b2d() );
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );

    Fujin::registerEntity();
    Cloud::registerEntity();

    Engine::em()->init();
	Engine::cm()->init();

    vp->offset().x = 0.0f;
    vp->offset().y = 0.0f;
    vp->bounds().x = 8.0f;
    vp->bounds().y = 6.0f;
    vp->setAngle( 0.0f );

    Entity * entity = Engine::em()->factory( Fujin::TYPE );
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );
    entity->setSprite( "fujin" );
    entity->setScale( 0.01f );
    entity->init();
    entity->getBody()->SetXForm( position, angle );

	for (int i = 0; i < 10; ++i)
	{
		Entity* entity = Engine::em()->factory( Cloud::TYPE );
		b2Vec2 position( Engine::hge()->Random_Float(-3.0f, 3.0f), Engine::hge()->Random_Float(-3.0f, 3.0f) );
		float angle( 0.f );
		entity->setSprite( "cloud" );
		entity->setScale( 0.03f );
		entity->init();
		entity->getBody()->SetXForm( position, angle );
	}
}

//------------------------------------------------------------------------------
void
Game::fini()
{
    Engine::cm()->fini();
	Engine::em()->fini();
}

//------------------------------------------------------------------------------
bool
Game::update( float dt )
{
    HGE * hge( Engine::hge() );

    if ( false )
    {
        Engine::instance()->switchContext( STATE_SCORE );
        Context * context( Engine::instance()->getContext() );
        static_cast< Score * >( context )->setValue( 13 );
        return false;
    }

	Engine::cm()->update( dt );
    Engine::em()->update( dt );

    return false;
}

//------------------------------------------------------------------------------
void
Game::render()
{
    ViewPort * vp( Engine::vp() );
    
    vp->setTransform();

    for ( b2Body * body( Engine::b2d()->GetBodyList() ); body != NULL;
          body = body->GetNext() )
    {
        Entity * entity( static_cast< Entity * >( body->GetUserData() ) );
        if ( entity )
        {
            entity->render();
        }
    }
}
