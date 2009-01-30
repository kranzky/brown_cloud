//==============================================================================

#ifndef ArseGame
#define ArseGame

#pragma once

#include <context.hpp>

//------------------------------------------------------------------------------
class Game : public Context
{
  public:
    Game();
    virtual ~Game();

  private:
    Game( const Game & );
    Game & operator=( const Game & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();
};

#endif

//==============================================================================
