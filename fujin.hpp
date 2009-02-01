//==============================================================================

#ifndef ArseFujin
#define ArseFujin

#pragma once

#include <entity.hpp>
#include <damageable.hpp>
#include <Box2D.h>

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
	bool isBlowing();
	const b2AABB& GetWindAABB();

    bool isSick();
    void setSick( bool sick );
    bool isAsleep();
    void setAsleep( bool sleep );

  protected:
    Fujin( const Fujin & );
    Fujin & operator=( const Fujin & );

    virtual void onSetScale();
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender( float scale );
    virtual void initFromQuery( Query & query );
    float lookAt(const b2Vec2& targetPoint);
	void Blow( float power = 1.0f );
	b2AABB m_AABB;
	bool m_isBlowing;
    bool m_isSick;
    bool m_isAsleep;
};

#endif

//==============================================================================
