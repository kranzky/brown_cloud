//==============================================================================

#ifndef ArseClumpManager
#define ArseClumpManager

#pragma once

#include <vector>
#include <Box2D.h>

class Entity;

//------------------------------------------------------------------------------
class Clump
{
public:
	Clump();
	virtual ~Clump();

	void init();
	void fini();

private:
	Clump( const Clump & );
    Clump & operator=( const Clump & );
};


//------------------------------------------------------------------------------
class ClumpManager
{
public:
	ClumpManager();
	virtual ~ClumpManager();

    void init();
    void update( float dt );
    void fini();

	void reportCollision(Entity* ent, Entity* other, b2Vec2 collidePosition);


private:
    ClumpManager( const ClumpManager & );
    ClumpManager & operator=( const ClumpManager & );

	struct InteractionData
	{
		Entity* m_entity;
		Entity* m_other;
		b2Vec2 m_collidePosition;
	};

	std::vector<InteractionData> m_interactions;
	std::vector< Clump *> m_clumps;
};

#endif

//==============================================================================