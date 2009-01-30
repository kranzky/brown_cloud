//==============================================================================

#include <credits.hpp>
#include <engine.hpp>

#include <hgeresource.h>

//------------------------------------------------------------------------------
Credits::Credits()
    :
    Context(),
    m_timer( 0.0f )
{
}

//------------------------------------------------------------------------------
Credits::~Credits()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Credits::init()
{
    m_timer = 0.0f;
}

//------------------------------------------------------------------------------
void
Credits::fini()
{
}

//------------------------------------------------------------------------------
bool
Credits::update( float dt )
{
    if ( m_timer > 4.0f )
    {
        Engine::instance()->switchContext( STATE_MENU );
    }

    m_timer += dt;

    return false;
}

//------------------------------------------------------------------------------
void
Credits::render()
{
    hgeResourceManager * rm( Engine::rm() );
    hgeSprite * sprite( 0 );

    int width( Engine::hge()->System_GetState( HGE_SCREENWIDTH ) );
    int height( Engine::hge()->System_GetState( HGE_SCREENHEIGHT ) );

    Engine::instance()->setColour( 0xFFFFFFFF );
    if ( m_timer > 2.0f )
    {
        sprite = rm->GetSprite( "publisher" );
    }
    else
    {
        sprite = rm->GetSprite( "developer" );
    }

    sprite->RenderEx( 0.5f * static_cast<float>( width ),
                      0.5f * static_cast<float>( height ), 0.0f, 0.5f );
}

//==============================================================================
