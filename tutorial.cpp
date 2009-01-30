//==============================================================================

#include <tutorial.hpp>
#include <engine.hpp>
#include <entity.hpp>
#include <entity_manager.hpp>
#include <viewport.hpp>
#include <fujin.hpp>
#include <cloud.hpp>

#include <hgeresource.h>

//==============================================================================
Tutorial::Tutorial()
    :
    Context()
{
}

//------------------------------------------------------------------------------
Tutorial::~Tutorial()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Tutorial::init()
{
    HGE * hge( Engine::hge() );
    b2World * b2d( Engine::b2d() );
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );

    Fujin::registerEntity();
    Cloud::registerEntity();

    Engine::em()->init();

    vp->offset().x = 0.0f;
    vp->offset().y = 0.0f;
    vp->bounds().x = 8.0f;
    vp->bounds().y = 6.0f;
}

//------------------------------------------------------------------------------
void
Tutorial::fini()
{
    Engine::instance()->getConfig().menu = 2;
    Engine::em()->fini();
}

//------------------------------------------------------------------------------
bool
Tutorial::update( float dt )
{
    HGE * hge( Engine::hge() );

    Engine::em()->update( dt );

    return false;
}

//------------------------------------------------------------------------------
void
Tutorial::render()
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

//==============================================================================
