//==============================================================================

#include <clump_manager.hpp>
#include <cloud.hpp>
#include <engine.hpp>



//------------------------------------------------------------------------------
Clump::Clump()
: m_clouds()
{
}

//------------------------------------------------------------------------------
Clump::~Clump()
{
}

//------------------------------------------------------------------------------
void Clump::addCloud(Cloud* cloud)
{
	m_clouds.push_back(cloud);
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
void joinClumps(Clump* bigClump, Clump* littleClump)
{
	while (littleClump->getClouds()->size() > 0)
	{
		Cloud* cloud = littleClump->getClouds()->back();

		cloud->setClump(bigClump);
		bigClump->addCloud(cloud);

		littleClump->getClouds()->pop_back();
	}
}

//------------------------------------------------------------------------------
void ClumpManager::update(float dt)
{
	//deal with any interactions that have occurred since the last update
	while (m_interactions.size() > 0)
	{
		InteractionData& i = m_interactions.back();
		
		Clump* entClump = i.m_entity->getClump();
		Clump* otherClump = i.m_other->getClump();

		if (entClump == NULL && otherClump == NULL)
		{
			//neither entity is in a clump, so create a new clump and put them both in it
			Clump* newClump = new Clump();
			m_clumps.push_back(newClump);

			newClump->addCloud(i.m_entity);
			newClump->addCloud(i.m_other);
			i.m_entity->setClump(newClump);
			i.m_other->setClump(newClump);
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
		}
		else if (entClump == NULL)
		{
			//first entity is not in a clump, but other entity is, so first needs to join other
			i.m_other->getClump()->addCloud(i.m_entity);
			i.m_entity->setClump(i.m_other->getClump());
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
		}
		else if (otherClump == NULL)
		{
			//first entity is in a clump, but other entity is not, so other entity should join first entity's clump
			i.m_entity->getClump()->addCloud(i.m_other);
			i.m_other->setClump(i.m_entity->getClump());
			joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
		}
		else
		{
			//both entities already in clumps, need to combine clumps as long as the clumps are different
			if (entClump != otherClump)
			{
				if (entClump->getClouds()->size() > otherClump->getClouds()->size())
				{
					//all of the clouds in otherClump need to be put in entClump
					joinClumps(entClump, otherClump);
					//now destroy the empty clump
					m_clumps.remove(otherClump);
					delete otherClump;
				}
				else
				{
					//all of the clouds in entClump need to be put into otherClump
					joinClumps(otherClump, entClump);
					
					//now destroy the empty clump
					m_clumps.remove(entClump);
					delete entClump;
				}

				//make the actual physics connection between the two clouds that collided
				joinEntities(i.m_entity, i.m_other, i.m_collidePosition);
			}
		}


		m_interactions.pop_back();
	}

}

//------------------------------------------------------------------------------
void ClumpManager::fini()
{
	//TODO: any cleanup
	m_interactions.clear();
	while (m_clumps.size() > 0)
	{
		delete m_clumps.back();
		m_clumps.pop_back();
	}
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





