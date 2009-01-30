//==============================================================================

#ifndef ArseMenu
#define ArseMenu

#pragma once

#include <hge.h>
#include <hgegui.h>

#include <context.hpp>

class hgeSprite;
class hgeFont;

enum Control
{
    CTRL_NONE = 0,
    CTRL_TITLES = 1,
    CTRL_START = 2,
    CTRL_SCORE = 3,
    CTRL_EXIT = 4
};

//------------------------------------------------------------------------------
class Menu : public Context
{
  public:
    Menu();
    virtual ~Menu();

  private:
    Menu( const Menu & );
    Menu & operator=( const Menu & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    hgeSprite * m_cursor;
    hgeFont * m_font;
    hgeGUI * m_gui;
};

//------------------------------------------------------------------------------
class MenuItem : public hgeGUIObject
{
  public:
    MenuItem( Control control, float x, float y, const char * title,
              hgeFont * font ); 

    virtual void    Render();
    virtual void    Update( float dt );
    virtual void    Enter();
    virtual void    Leave();
    virtual bool    IsDone();
    virtual void    Focus( bool focused );
    virtual void    MouseOver( bool over );
    virtual bool    MouseLButton( bool down );
    virtual bool    KeyClick( int key, int chr );

  private:
    hgeFont * m_font;
    const char * m_title;
};

#endif

//==============================================================================
