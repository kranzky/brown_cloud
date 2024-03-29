//==============================================================================

#include <context.hpp>
#include <entity.hpp>

//------------------------------------------------------------------------------
Context::Context()
    :
    m_handles_collisions( false )
{
}

//------------------------------------------------------------------------------
Context::~Context()
{
}

//------------------------------------------------------------------------------
void
Context::notifyOnCollision( bool notify )
{
    m_handles_collisions = notify;
}

//------------------------------------------------------------------------------
bool
Context::handlesCollisions()
{
    return m_handles_collisions;
}

//------------------------------------------------------------------------------
bool
Context::shouldCollide( Entity * left, Entity * right )
{
    return false;
}

//==============================================================================
