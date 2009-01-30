//==============================================================================

#ifndef ArseMenu
#define ArseMenu

#pragma once

#include <hge.h>

#include <context.hpp>

class hgeSprite;
class hgeFont;
class hgeGUI;

enum Control
{
    CTRL_NONE = 0,
    CTRL_CREDITS = 1,
    CTRL_TUTORIAL = 2,
    CTRL_START = 3,
    CTRL_SCORE = 4,
    CTRL_EXIT = 5
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

#endif

//==============================================================================
