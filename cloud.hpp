//==============================================================================

#ifndef ArseCloud
#define ArseCloud

#pragma once

#include <entity.hpp>

class hgeParticleSystem;
class Clump;

//------------------------------------------------------------------------------
class Cloud : public Entity
{
  public:
    static const unsigned int TYPE = 2;
    static Entity * factory() { return new Cloud(); }

    Cloud( float scale = 1.0f );
    virtual ~Cloud();

    virtual void collide( Entity * entity, b2ContactPoint * point );

    virtual void persistToDatabase();

    static void registerEntity();

  protected:
    Cloud( const Cloud & );
    Cloud & operator=( const Cloud & );

    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender( float scale );
    virtual void initFromQuery( Query & query );

  private:

    hgeParticleSystem* m_particles;
	Clump* m_clump;
	  
};

#endif

//==============================================================================
