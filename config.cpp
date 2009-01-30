//==============================================================================

#include <config.hpp>
#include <engine.hpp>

#include <hge.h>

//------------------------------------------------------------------------------
Config::Config()
    :
    screenWidth( 800 ),
    screenHeight( 600 ),
    fullScreen( false ),
    userName( "Lloyd" )
{
}

//------------------------------------------------------------------------------
Config::~Config()
{
}

//------------------------------------------------------------------------------
void
Config::init()
{
    screenWidth = Engine::hge()->Ini_GetInt( "kranzky", "width",
                                             screenWidth );
    screenHeight = Engine::hge()->Ini_GetInt( "kranzky","height",
                                              screenHeight );
    fullScreen = Engine::hge()->Ini_GetInt( "kranzky", "fullscreen",
                                            fullScreen ? 1 : 0 ) == 1;
    userName = Engine::hge()->Ini_GetString( "kranzky", "username",
                                             userName.c_str() );
}

//------------------------------------------------------------------------------
void
Config::fini()
{
    Engine::hge()->Ini_SetInt( "kranzky", "width", screenWidth );
    Engine::hge()->Ini_SetInt( "kranzky","height", screenHeight );
    Engine::hge()->Ini_SetInt( "kranzky", "fullscreen", fullScreen ? 1 : 0 );
    Engine::hge()->Ini_SetString( "kranzky", "username", userName.c_str() );
}

//==============================================================================
