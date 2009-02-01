//==============================================================================

#include <game.hpp>
#include <engine.hpp>
#include <entity.hpp>
#include <entity_manager.hpp>
#include <clump_manager.hpp>
#include <viewport.hpp>
#include <fujin.hpp>
#include <cloud.hpp>
#include <girder.hpp>
#include <score.hpp>

#include <hgeresource.h>

#include <algorithm>

namespace
{
    const float ZOOM[5] = { 1.0f, 1.8f, 3.2f, 5.8f, 10.5f };

    bool
    lessThan( const Entity * left, const Entity * right )
    {
        if ( left->getScale() < right->getScale() * 0.99f )
        {
            return true;
        }
        if ( left->getScale() * 0.99f > right->getScale() )
        {
            return false;
        }
        if ( left->getType() < right->getType() )
        {
            return true;
        }
        return false;
    }
}

//==============================================================================
Game::Game()
    :
    Context(),
    m_last_zoom( 1.0f ),
    m_zoom( 0 ),
    m_fujin( 0 )
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

    notifyOnCollision( true );

    Fujin::registerEntity();
    Cloud::registerEntity();
    Girder::registerEntity();

    m_zoom = 0;

	m_timeRemaining = 300;
	m_score = 0;
    Engine::em()->init();
	Engine::cm()->init();

    vp->offset().x = 0.0f;
    vp->offset().y = 0.0f;
    vp->bounds().x = 800.0f;
    vp->bounds().y = 600.0f;
    vp->setAngle( 0.0f );
    vp->setScale( ZOOM[m_zoom] );

    m_fujin = static_cast< Fujin * >( Engine::em()->factory( Fujin::TYPE ) );
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );
    m_fujin->setSprite( "fujin" );
    m_fujin->setScale( 1.0f / ZOOM[m_zoom] );
    m_fujin->init();
    m_fujin->getBody()->SetXForm( position, angle );

    for ( int zoom = 0; zoom < 5; ++ zoom )
    {
	for (int i = 0; i < 8; ++i)
	{
		Entity* entity = Engine::em()->factory( Cloud::TYPE );
		b2Vec2 position( Engine::hge()->Random_Float( -400.0f, 400.0f),
                         Engine::hge()->Random_Float( -300.0f, 300.0f) );
		float angle( 0.f );
		entity->setSprite( "cloud" );
		entity->setScale( 1.0f / ZOOM[zoom] );
		entity->init();
		entity->getBody()->SetXForm( position, angle );
        static_cast< Cloud * >( entity )->setZoom( zoom );
	}
    }

    _initArena();
}

//------------------------------------------------------------------------------
void
Game::fini()
{
    notifyOnCollision( false );

    Engine::cm()->fini();
	Engine::em()->fini();
}

//------------------------------------------------------------------------------
bool
Game::update( float dt )
{
	m_timeRemaining -= dt;
    const Controller & pad( Engine::instance()->getController() );
    HGE * hge( Engine::hge() );
    ViewPort * vp( Engine::vp() );

    if ( pad.buttonDown( XPAD_BUTTON_BUTTON_Y ) )
    {
        Engine::instance()->switchContext( STATE_SCORE );
        Context * context( Engine::instance()->getContext() );
        static_cast< Score * >( context )->setValue( 13 );
        return false;
    }

	Engine::cm()->update( dt );
    Engine::em()->update( dt );

    if ( Engine::instance()->isPaused() )
    {
        return false;
    }

    if ( pad.isConnected() )
    {
        if ( pad.buttonDown( XPAD_BUTTON_LEFT_SHOULDER ) && m_zoom > 0 )
        {
            --m_zoom;
        }
        else if ( pad.buttonDown( XPAD_BUTTON_RIGHT_SHOULDER ) && m_zoom < 4 )
        {
            ++m_zoom;
        }
    }
    else
    {
        if ( ( Engine::hge()->Input_KeyDown( HGEK_Q ) ||
               hge->Input_GetMouseWheel() < 0 ) && m_zoom > 0 )
        {
            --m_zoom;
        }
        else if ( ( Engine::hge()->Input_KeyDown( HGEK_E ) ||
                    hge->Input_GetMouseWheel() > 0 ) && m_zoom < 4 )
        {
            ++m_zoom;
        }
    }

    if ( ZOOM[m_zoom] > m_last_zoom )
    {
        m_last_zoom += ( ZOOM[m_zoom] - m_last_zoom ) * dt * 10.0f;
        vp->setScale( m_last_zoom );
        m_fujin->setScale( 1.0f / m_last_zoom );
    }
    else if ( ZOOM[m_zoom] < m_last_zoom )
    {
        m_last_zoom += ( ZOOM[m_zoom] - m_last_zoom ) * dt * 10.0f;
        vp->setScale( m_last_zoom );
        m_fujin->setScale( 1.0f / m_last_zoom );
    }

    vp->offset() = m_fujin->getBody()->GetPosition();

    return false;
}

//------------------------------------------------------------------------------
void
Game::render()
{
	hgeResourceManager * rm( Engine::rm() );
	hgeFont* font = Engine::rm()->GetFont("menu");
	b2Vec2 timeTextLocation (700,10);
	b2Vec2 scoreTextLocation(0,10);
	char timeRemainingText[10];
	sprintf_s(timeRemainingText,"%d:%d",(int)m_timeRemaining/60,(int)(m_timeRemaining)%60);

	char scoreText[15];
	sprintf_s(scoreText,"Score: %5d",m_score);


    ViewPort * vp( Engine::vp() );
	

    vp->setTransform();

	// render time remaining
    rm->GetSprite( "polluted" )->RenderEx( 0.0f, 0.0f, 0.0f, 2.0f );
	
	
//	scoreTextLocation.x =m_zoom-1 * vp->offset().x; 
	//scoreTextLocation.y=m_zoom-1 * vp->offset().y;
	
	
    std::vector< Entity * > entities;
    for ( b2Body * body( Engine::b2d()->GetBodyList() ); body != NULL;
          body = body->GetNext() )
    {
        Entity * entity( static_cast< Entity * >( body->GetUserData() ) );
        if ( entity )
        {
            entities.push_back( entity );
        }
    }

    std::sort( entities.begin(), entities.end(), lessThan );

    std::vector< Entity * >::iterator i;
    float scale( 1.0f / static_cast< float >( ZOOM[m_zoom] ) );
    for ( i = entities.begin(); i != entities.end(); ++i )
    {
        Entity * entity( * i );
        entity->render( scale );
    }

	Engine::hge()->Gfx_SetTransform();
	font->SetColor(0xFFFFFFFF);
	font->Render( timeTextLocation.x, timeTextLocation.y, HGETEXT_LEFT, timeRemainingText );
	font->Render( scoreTextLocation.x, scoreTextLocation.y, HGETEXT_LEFT, scoreText );

	
	
	
}

//------------------------------------------------------------------------------
bool
Game::shouldCollide( Entity * left, Entity * right )
{
    if ( left->getType() == Girder::TYPE ||
         right->getType() == Girder::TYPE )
    {
        return true;
    }

    if ( left->getScale() > right->getScale() * 0.99f &&
         left->getScale() * 0.99f < right->getScale() )
    {
        return left->getType() == right->getType();
    }

    return false;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Game::_initArena()
{
    b2Vec2 position( 0.0f, 0.0f );
    b2Vec2 dimensions( 0.0f, 0.0f );
    Entity * entity( 0 );

    for ( int i = 0; i < 4; ++i )
    {
        switch( i )
        {
            case 0:
            {
                dimensions.x = 800.0f;
                dimensions.y = 1.0f;
                position.x = 0.0f;
                position.y = -300.0f;
                break;
            }
            case 1:
            {
                dimensions.x = 1.0f;
                dimensions.y = 600.0f;
                position.x = 400.0f;
                position.y = 0.0f;
                break;
            }
            case 2:
            {
                dimensions.x = 800.0f;
                dimensions.y = 1.0f;
                position.x = 0.0f;
                position.y = 300.0f;
                break;
            }
            case 3:
            {
                dimensions.x = 1.0f;
                dimensions.y = 600.0f;
                position.x = -400.0f;
                position.y = 0.0f;
                break;
            }
        }
        Girder * girder( static_cast< Girder * >(
            Engine::em()->factory( Girder::TYPE ) ) );
        girder->setScale( 1.0f );
        girder->setDimensions( dimensions );
        girder->init();
        girder->getBody()->SetXForm( position, 0.0f );
    }
}
