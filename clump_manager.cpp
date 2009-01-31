//==============================================================================

#include <clump_manager.hpp>
#include <cloud.hpp>
#include <engine.hpp>



//------------------------------------------------------------------------------
Clump::Clump()
{
}

//------------------------------------------------------------------------------
Clump::~Clump()
{
}

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
ClumpManager::ClumpManager()
: m_interactions(),
  m_clumps()
{
}


//------------------------------------------------------------------------------
ClumpManager::~ClumpManager()
{
	fini();
}

//------------------------------------------------------------------------------
void 
ClumpManager::init()
{
}

//------------------------------------------------------------------------------
void joinEntities(Entity* entity, Entity* otherEntity, b2Vec2 position)
{
	b2RevoluteJointDef joint;
	joint.Initialize( entity->getBody(), otherEntity->getBody(), position);
	Engine::instance()->b2d()->CreateJoint( & joint );
}

//------------------------------------------------------------------------------
void ClumpManager::update(float dt)
{
	//deal with any interactions that have occurred since the last update
	while (m_interactions.size() > 0)
	{
		InteractionData& i = m_interactions.back();
		
		const Clump* entClump = i.m_entity->getClump();
		const Clump* otherClump = i.m_other->getClump();

		if (entClump == NULL && otherClump == NULL)
		{
			//neither entity is in a clump, so create a new clump and put them both in it
			Clump* newClump = new Clump();
			m_clumps.push_back(newClump);

			i.m_entity->setClump(newClump);
			i.m_other->setClump(newClump);
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
		}
		else if (entClump == NULL)
		{
			//first entity is not in a clump, but other entity is, so first needs to join other
			i.m_entity->setClump(i.m_other->getClump());
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
		}
		else if (otherClump == NULL)
		{
			//first entity is in a clump, but other entity is not, so other entity should join first entity's clump
			i.m_other->setClump(i.m_entity->getClump());
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
		}
		else
		{
			//both entities already in clumps, need to combine clumps as long as the clumps are different
			if (entClump != otherClump)
			{
			}
		}


		m_interactions.pop_back();
	}

}

//------------------------------------------------------------------------------
void ClumpManager::fini()
{
	//TODO: any cleanup
}

//------------------------------------------------------------------------------
void ClumpManager::reportCollision(Cloud* ent, Cloud* other, b2Vec2 collidePosition)
{
	//a new interaction has occurred, so save the data for now, and deal with it in our own update
	InteractionData data;
	data.m_entity = ent;
	data.m_other = other;
	data.m_collidePosition = collidePosition;
	m_interactions.push_back(data);
}


//==============================================================================





