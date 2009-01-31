//==============================================================================

#include <clump_manager.hpp>
#include <entity.hpp>
#include <engine.hpp>


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
void ClumpManager::update(float dt)
{
	//deal with any interactions that have occurred since the last update
	while (m_interactions.size() > 0)
	{
		InteractionData& i = m_interactions.back();

		b2RevoluteJointDef joint;
		joint.Initialize( i.m_entity->getBody(), i.m_other->getBody(), i.m_collidePosition);
		Engine::instance()->b2d()->CreateJoint( & joint );

		m_interactions.pop_back();


		//std::vector<InteractionData>::iterator i;
		//for (i = m_interactions.begin(); i != m_interactions.end(); ++i)
		//{
		//	
		//}
	}

}

//------------------------------------------------------------------------------
void ClumpManager::fini()
{
	//TODO: any cleanup
}

//------------------------------------------------------------------------------
void ClumpManager::reportCollision(Entity* ent, Entity* other, b2Vec2 collidePosition)
{
	//a new interaction has occurred, so save the data for now, and deal with it in our own update
	InteractionData data;
	data.m_entity = ent;
	data.m_other = other;
	data.m_collidePosition = collidePosition;
	m_interactions.push_back(data);
}


//==============================================================================





