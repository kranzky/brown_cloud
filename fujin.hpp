//==============================================================================

#ifndef ArseFujin
#define ArseFujin

#pragma once

#include <entity.hpp>
#include <damageable.hpp>

//------------------------------------------------------------------------------
class Fujin : public Entity, public Damageable
{
  public:
    static const unsigned int TYPE = 1;
    static Entity * factory() { return new Fujin(); }

    Fujin( float max_strength = 1.0f, float scale = 1.0f );
    virtual ~Fujin();

    virtual void collide( Entity * entity, b2ContactPoint * point );

    virtual void persistToDatabase();

    static void registerEntity();

  protected:
    Fujin( const Fujin & );
    Fujin & operator=( const Fujin & );

    virtual void onSetScale();
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender();
    virtual void initFromQuery( Query & query );
};

#endif

//==============================================================================
