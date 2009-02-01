//==============================================================================

#include <score.hpp>
#include <engine.hpp>
#include <viewport.hpp>
#include <controller.hpp>

#include <hgeresource.h>
#include <hgefont.h>
#include <curl/curl.h>
#include <Box2D.h>

#include <sstream>

//------------------------------------------------------------------------------
namespace
{
    size_t writer( void * ptr, size_t size, size_t nmemb, void * stream )
    {
        if ( stream == 0 || ptr == 0 )  
        {
            return 0;
        }
    
        char * data( static_cast< char * >( stream ) );
    
        while ( data[0] != '\0' )
        {
            ++data;
        }
    
        for ( size_t i = 0; i < nmemb; ++i )
        {
            data[i] = static_cast< char * >( ptr )[i];
            data[i+1] = '\0';
        }
    
        return nmemb;  
    }
}

//------------------------------------------------------------------------------
Score::Score()
    :
    Context(),
    m_cursor( 0 ),
    m_state( SCORE_SHOW ),
    m_timer( 0.0f ),
    m_time_to_show( 0.0f ),
    m_time_to_post( 0.0f ),
    m_high_score(),
    m_name( "" ),
    m_value( 0 ),
    m_highlight( -1 ),
    m_dy( 0.0f )
{
    m_cursor = new hgeSprite( 0, 0, 0, 1, 1 );
    curl_global_init( CURL_GLOBAL_WIN32 );
}

//------------------------------------------------------------------------------
Score::~Score()
{
    curl_global_cleanup();
    delete m_cursor;
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Score::init()
{
    hgeResourceManager * rm( Engine::rm() );
    m_state = SCORE_CALCULATE;
    m_timer = 0.0f;
    m_name = Engine::instance()->getConfig().userName;
    m_value = 0;
    m_highlight = -1;
    m_dy = 0.0f;
    while ( m_name.size() > 15 )
    {
        m_name.erase( m_name.end() - 1 );
    }
    m_time_to_show = 1.0f;
    m_time_to_post = 0.0f;
}

//------------------------------------------------------------------------------
void
Score::fini()
{
}

//------------------------------------------------------------------------------
bool
Score::update( float dt )
{
    HGE * hge( Engine::hge() );
    hgeResourceManager * rm( Engine::rm() );
    hgeParticleManager * pm( Engine::pm() );

    hgeFont * font( rm->GetFont( "menu" ) );

    m_timer += dt;
    
    switch ( m_state )
    {
        case SCORE_CALCULATE:
        {
            if ( m_value > 0 )
            {
                m_state = SCORE_INPUT;
                m_time_to_show = 1.0f;
            }
            else
            {
                m_state = SCORE_SHOW;
                m_high_score.clear();
            }
            break;
        }
        case SCORE_INPUT:
        {
            int character( hge->Input_GetChar() );
            if ( character != 0 )
            {
                if ( ( ( character >= '0' && character <= '9' ) ||
                       ( character >= 'a' && character <= 'z' ) ||
                       ( character >= 'A' && character <= 'Z' ) ) && 
                     m_name.size() < 15 )
                {
                    m_name.push_back( character );
                }
                while ( font->GetStringWidth( m_name.c_str(), false ) > 180.0f )
                {
                    m_name.erase( m_name.end() - 1 );
                }
            }
            if ( hge->Input_KeyDown( HGEK_BACKSPACE ) ||
                 hge->Input_KeyDown( HGEK_DELETE ) )
            {
                if ( m_name.size() > 0 )
                {
                    m_name.erase( m_name.end() - 1 );
                }
            }
            if ( hge->Input_KeyDown( HGEK_ENTER ) )
            {
                if ( m_name.size() == 0 )
                {
                    m_name = "Anonymous";
                }
                Engine::instance()->getConfig().userName = m_name;
                m_time_to_post = 1.0f;
            }
            if ( m_time_to_post > 0.0f )
            {
                m_time_to_post -= dt;
                if ( m_time_to_post <= 0.0f )
                {
                    _insertScore( m_name.c_str(), m_value );
                    m_time_to_show = 1.0f;
                    m_state = SCORE_SHOW;
                    m_high_score.clear();
                }
            }
            break;
        }
        case SCORE_SHOW:
        {
            const Controller & pad( Engine::instance()->getController() );

            if ( m_time_to_show > 0.0f )
            {
                m_time_to_show -= dt;
                if ( m_time_to_show <= 0.0f )
                {
                    _updateScore();
                }
            }

            if ( hge->Input_GetKey() != 0 &&
                 ! Engine::instance()->handledKey() )
            {
                Engine::instance()->switchContext( STATE_MENU );
                return false;
            }
            if ( pad.buttonDown( XPAD_BUTTON_A ) ||
                 pad.buttonDown( XPAD_BUTTON_B ) ||
                 pad.buttonDown( XPAD_BUTTON_START ) ||
                 pad.buttonDown( XPAD_BUTTON_BACK ) )
            {
                Engine::instance()->switchContext( STATE_MENU );
                return false;
            }

            b2Vec2 offset( pad.getStick( XPAD_THUMBSTICK_LEFT ) );
            if ( offset.y > 0.2f || offset.y < -0.2f )
            {
                m_dy += offset.y * dt * 300.0f;
            }
            if ( m_dy > 0.0f )
            {
                m_dy = 0.0f;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void
Score::render()
{
    hgeResourceManager * rm( Engine::rm() );

    hgeFont * font( rm->GetFont( "menu" ) );

    font->SetColor( 0xFFFFFFFF );

    float cx( 0.5f * Engine::vp()->screen().x );
    float cy( 0.5f * Engine::vp()->screen().y );

    switch ( m_state )
    {
        case SCORE_INPUT:
        {
			font->printf( cx, cy + 80.0f, HGETEXT_CENTER,"%d",m_value);
            font->printf( cx, cy + 100.0f, HGETEXT_CENTER, m_name.c_str() );
            if ( static_cast<int>( m_timer * 2.0f ) % 2 != 0 )
            {
                float width = font->GetStringWidth( m_name.c_str() );
                m_cursor->SetColor( 0xFFFFFFFF );
                m_cursor->RenderStretch( cx + width * 0.5f, cy + 125.0f,
                                         cx + width * 0.5f + 16.0f, cy+127.0f );
            }
            if ( m_time_to_post > 0.0f )
            {
                font->printf( cx, cy, HGETEXT_CENTER,
                              "Sending details to server..." );
            }
        }
        case SCORE_CALCULATE:
        {
            font->printf( cx, 80.0f, HGETEXT_CENTER, "F R O W N   T O W N" );
            break;
        }
        case SCORE_SHOW:
        {
            font->printf( cx, 60.0f, HGETEXT_CENTER,
                          "B L O W   H A R D S" );
            font->printf( cx, 2.0f * cy - 90.0f, HGETEXT_CENTER,
                          "Y O U   S U C K   T H E   M O S T" );
            if ( m_time_to_show > 0.0f )
            {
                font->printf( cx, cy, HGETEXT_CENTER,
                              "Requesting scores from server..." );
            }
            else if ( m_high_score.size() == 0 )
            {
                font->printf( cx, cy, HGETEXT_CENTER,
                              "Unable to retrieve high scores :(" );
            }
            font->SetColor( 0xCCFFFFFF );
            Engine::hge()->Gfx_SetClipping( 10, 120,
                static_cast< int >( Engine::vp()->screen().x ) - 20,
                static_cast< int >( Engine::vp()->screen().y ) - 250 );
            std::vector< ScoreData >::iterator j( m_high_score.begin() );
            float my( 0.0f );
            for( int i = 0; j != m_high_score.end(); ++i, ++j )
            {
                my = cy - 180.0f + i * 30.0f;
                if ( my + m_dy < 100.0f )
                {
                    continue;
                }
                if ( m_highlight < 0 && m_value == j->value &&
                     m_name == j->name )
                {
                    m_highlight = i;
                }
                bool clipped( false );
                if ( ! j->clipped )
                {
                    j->clipped = true;
                    while ( font->GetStringWidth( j->name.c_str(),
                            false ) > 180.0f )
                    {
                        clipped = true;
                        j->name.erase( j->name.end() - 1 );
                    }
                    if ( clipped )
                    {
                        j->name.append( "..." );
                    }
                }
                if ( m_highlight == i )
                {
                    font->printf( cx - 340.0f, my + m_dy, HGETEXT_LEFT, "->" );
                    font->printf( cx + 340.0f, my + m_dy, HGETEXT_RIGHT, "<-" );
                }
                font->printf( cx - 300.0f, my + m_dy, HGETEXT_LEFT,
                              "%03d.", i );
                font->printf( cx - 220.0f, my + m_dy, HGETEXT_LEFT,
                              "%s", j->name.c_str() );
                font->printf( cx + 80.0f, my + m_dy, HGETEXT_RIGHT,
                              "%d", j->value );
                font->printf( cx + 120.0f, my + m_dy, HGETEXT_LEFT,
                              "%s", j->timestamp.c_str() );
                if ( my + m_dy > Engine::vp()->screen().y )
                {
                    break;
                }
            }
            if ( my + m_dy < Engine::vp()->screen().y - 160.0f )
            {
                m_dy = Engine::vp()->screen().y - 160.0f - my;
            }
            Engine::hge()->Gfx_SetClipping();
            break;
        }
        default:
        {
            break;
        }
    }
}

//------------------------------------------------------------------------------
void
Score::setValue( int value )
{
    m_value = value;
    m_state = SCORE_CALCULATE;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Score::_updateScore()
{
    hgeFont * font( Engine::rm()->GetFont( "menu" ) );
    CURL * curl( curl_easy_init() );

    if ( curl == 0 )
    {
        return;
    }

    char buffer[65536];
    buffer[0] = '\0';

#ifdef _DEBUG
    curl_easy_setopt( curl, CURLOPT_URL,
                      "http://gamejam.org/teams/KranzkySoftware/scores/" );
#else
    curl_easy_setopt( curl, CURLOPT_URL,
                      "http://gamejam.org/teams/BrownCloud/scores/" );
#endif
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, writer );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, buffer );
    curl_easy_perform( curl );

    curl_easy_cleanup( curl );

    m_high_score.clear();

    std::stringstream stream( buffer );
    char line[256];
    while ( stream.getline( line, 255 ) )
    {
        char name[16];
        int value( 0 );
        char timestamp[64];
        sscanf_s( line, "%[a-zA-Z0-9]:%d:%[^:]", name, 16, & value,
                                                 timestamp, 64 );
		ScoreData data;
		data.name = name;
		data.value = value;
		data.timestamp = timestamp;
        data.clipped = false;
		m_high_score.push_back( data );
	}
}

//------------------------------------------------------------------------------
void
Score::_insertScore( const char * name, int value )
{
    CURL * curl( curl_easy_init() );

    if ( curl == 0 )
    {
        return;
    }

    struct curl_httppost * form( 0 );
    struct curl_httppost * last( 0 );
    char stringValue[64];

    sprintf_s( stringValue, 63, "%d", value );

    curl_formadd( & form, & last, CURLFORM_COPYNAME, "tag",
        CURLFORM_COPYCONTENTS, name,
        CURLFORM_END );
    curl_formadd( & form, & last, CURLFORM_COPYNAME, "value",
        CURLFORM_COPYCONTENTS, stringValue,
        CURLFORM_END );
#ifdef _DEBUG
    curl_formadd( & form, & last, CURLFORM_COPYNAME, "key",
        CURLFORM_COPYCONTENTS, "5ddc27012824dadf705049e39386f1c9",
        CURLFORM_END );
    curl_easy_setopt( curl, CURLOPT_URL,
                      "http://gamejam.org/teams/KranzkySoftware/setscore/" );
#else
    curl_formadd( & form, & last, CURLFORM_COPYNAME, "key",
        CURLFORM_COPYCONTENTS, "79df3c86a699f13c35177e2ce82b8284",
        CURLFORM_END );
    curl_easy_setopt( curl, CURLOPT_URL,
                      "http://gamejam.org/teams/BrownCloud/setscore/" );
#endif

    curl_easy_setopt( curl, CURLOPT_HTTPPOST, form );
    curl_easy_perform( curl );
    curl_formfree( form );

    curl_easy_cleanup( curl );
}
