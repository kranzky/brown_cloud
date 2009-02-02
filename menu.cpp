//==============================================================================

#include <menu.hpp>
#include <menu_item.hpp>
#include <engine.hpp>
#include <viewport.hpp>
#include <config.hpp>

#include <hgeresource.h>
#include <hgefont.h>

//------------------------------------------------------------------------------
Menu::Menu()
    :
    Context(),
    m_font( 0 ),
    m_gui( 0 )
{
}

//------------------------------------------------------------------------------
Menu::~Menu()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Menu::init()
{
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );
    Config & config( Engine::instance()->getConfig() );

    char * label = config.leaderboard ? "Leaderboard" : "High Scores";

    m_font = rm->GetFont( "menu" );
    m_gui = new hgeGUI();
    float cx( 0.5f * vp->screen().x );
    float cy( 0.5f * vp->screen().y );
    m_gui->AddCtrl( new MenuItem( CTRL_TUTORIAL, cx, cy, "Help", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_START, cx, cy + 30, "Start", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_SCORE, cx, cy + 60, label, m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_CREDITS, cx, cy + 90, "Credits",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_EXIT, cx, cy + 150, "Exit", m_font ) );
    m_gui->SetNavMode( HGEGUI_UPDOWN );
    m_gui->SetFocus( Engine::instance()->getConfig().menu );
    m_gui->Enter();
}

//------------------------------------------------------------------------------
void
Menu::fini()
{
    m_gui->DelCtrl( CTRL_TUTORIAL );
    m_gui->DelCtrl( CTRL_START );
    m_gui->DelCtrl( CTRL_SCORE );
    m_gui->DelCtrl( CTRL_CREDITS );
    m_gui->DelCtrl( CTRL_EXIT );
    delete m_gui;
    m_gui = 0;
    m_font = 0;
}

//------------------------------------------------------------------------------
bool
Menu::update( float dt )
{
    HGE * hge( Engine::hge() );
    Engine * engine( Engine::instance() );

    if ( hge->Input_KeyDown( HGEK_ESCAPE ) )
    {
        return true;
    }

    switch ( static_cast< Control >( engine->updateGUI( dt, m_gui,
                                     engine->getConfig().menu, 5 ) ) )
    {
        case CTRL_TUTORIAL:
        {
            Engine::instance()->switchContext( STATE_TUTORIAL );
            break;
        }
        case CTRL_START:
        {
            Engine::instance()->switchContext( STATE_GAME );
            break;
        }
        case CTRL_SCORE:
        {
            Engine::instance()->switchContext( STATE_SCORE );
            break;
        }
        case CTRL_CREDITS:
        {
            Engine::instance()->switchContext( STATE_CREDITS );
            break;
        }
        case CTRL_EXIT:
        {
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void
Menu::render()
{
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );

    m_gui->Render();
    float cx( 0.5f * vp->screen().x );
    rm->GetSprite( "title" )->Render( cx, 150.0f );
}

//==============================================================================
