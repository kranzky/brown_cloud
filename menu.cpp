//==============================================================================

#include <menu.hpp>
#include <engine.hpp>
#include <viewport.hpp>

#include <hgeresource.h>
#include <hgefont.h>

//------------------------------------------------------------------------------
Menu::Menu()
    :
    Context(),
    m_cursor( 0 ),
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

    m_font = rm->GetFont( "menu" );
    m_cursor = rm->GetSprite( "cursor" );
    m_gui = new hgeGUI();
    float cx( 0.5f * vp->screen().x );
    float cy( 0.5f * vp->screen().y );
    m_gui->AddCtrl( new MenuItem( CTRL_START, cx, cy, "Tutorial", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_START, cx, cy + 30, "Start", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_SCORE, cx, cy + 60, "High Score",
                                  m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_SCORE, cx, cy + 90, "Credits", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_EXIT, cx, cy + 150, "Exit", m_font ) );
    m_gui->SetNavMode( HGEGUI_UPDOWN | HGEGUI_CYCLED );
    m_gui->SetCursor( m_cursor );
    m_gui->SetFocus( 1 );
    m_gui->Enter();
    Engine::instance()->setMouse( "cursor" );
    Engine::instance()->showMouse();
}

//------------------------------------------------------------------------------
void
Menu::fini()
{
    m_gui->DelCtrl( CTRL_TITLES );
    m_gui->DelCtrl( CTRL_START );
    m_gui->DelCtrl( CTRL_SCORE );
    m_gui->DelCtrl( CTRL_EXIT );
    delete m_gui;
    m_gui = 0;
    m_font = 0;
    m_cursor = 0;
}

//------------------------------------------------------------------------------
bool
Menu::update( float dt )
{
    HGE * hge( Engine::hge() );

    switch ( static_cast< Control >( m_gui->Update( dt ) ) )
    {
        case CTRL_TITLES:
        {
            Engine::instance()->switchContext( STATE_SPLASH );
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
    m_font->printf( cx, 80.0f, HGETEXT_CENTER, "B R O W N   C L O U D" );
}

//==============================================================================
MenuItem::MenuItem( Control control, float x, float y, const char * title,
                    hgeFont * font )
    :
    hgeGUIObject(),
    m_title( title ),
    m_font( font )

{
    id = static_cast<int>( control );

    bStatic=false;
    bVisible=true;
    bEnabled=true;

    float width( m_font->GetStringWidth( title ) );
    rect.Set( x - width / 2, y, x + width / 2, y + m_font->GetHeight() );
}
 
//------------------------------------------------------------------------------
void
MenuItem::Render()
{
    m_font->Render( rect.x1, rect.y1, HGETEXT_LEFT, m_title );
}

//------------------------------------------------------------------------------
void
MenuItem::Update( float dt )
{
}

//------------------------------------------------------------------------------
void
MenuItem::Enter()
{
}

//------------------------------------------------------------------------------
void
MenuItem::Leave()
{
}

//------------------------------------------------------------------------------
bool
MenuItem::IsDone()
{
    return true;
}

//------------------------------------------------------------------------------
void
MenuItem::Focus( bool focused )
{
}

//------------------------------------------------------------------------------
void
MenuItem::MouseOver( bool over )
{
    if ( over )
    {
        gui->SetFocus( id );
    }
}

//------------------------------------------------------------------------------
bool
MenuItem::MouseLButton( bool down )
{
    if ( down )
    {
        return true;
    }
    else
    {
        return false;
    }
}

//------------------------------------------------------------------------------
bool
MenuItem::KeyClick( int key, int chr )
{
    if ( key == HGEK_ENTER || key == HGEK_SPACE )
    {
        MouseLButton( true );
        return MouseLButton( false );
    }

    return false;
}

//==============================================================================
