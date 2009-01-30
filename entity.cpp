//==============================================================================

#include <engine.hpp>
#include <entity.hpp>
#include <entity_manager.hpp>

#include <hgeresource.h>
#include <Box2D.h>

#pragma warning( disable : 4351 )

//------------------------------------------------------------------------------
Entity::Entity( float scale )
    :
    m_scale( scale ),
    m_type( 0 ),
    m_id( 0 ),
    m_body( 0 ),
    m_aabb( 0 ),
    m_sprite_id( 0 ),
    m_sprite( 0 )
{
    m_aabb = new b2AABB();
}

//------------------------------------------------------------------------------
Entity::~Entity()
{
    if ( m_body != 0 )
    {
        Engine::b2d()->DestroyBody( m_body );
    }
    delete m_aabb;
}

//------------------------------------------------------------------------------
void
Entity::init()
{
    doInit();
}

//------------------------------------------------------------------------------
void
Entity::update( float dt )
{
    doUpdate( dt );
    b2Shape * shape( getBody()->GetShapeList() );
    shape->ComputeAABB( m_aabb, getBody()->GetXForm() );
}

//------------------------------------------------------------------------------
void
Entity::render()
{
    doRender();
}

//------------------------------------------------------------------------------
b2Body *
Entity::getBody() const
{
    return m_body;
}

//------------------------------------------------------------------------------
b2AABB *
Entity::getAABB() const
{
    return m_aabb;
}

//------------------------------------------------------------------------------
unsigned int
Entity::getType()
{
    return m_type;
}

//------------------------------------------------------------------------------
void
Entity::setID( sqlite3_int64 id )
{
    m_id = id;
}

//------------------------------------------------------------------------------
sqlite3_int64
Entity::getID()
{
    return m_id;
}

//------------------------------------------------------------------------------
void
Entity::deleteFromDatabase()
{
    Engine::em()->deleteFromDatabase( this );
}

//------------------------------------------------------------------------------
// protected:
//------------------------------------------------------------------------------
void
Entity::setType( unsigned int type )
{
    m_type = type;
}

//------------------------------------------------------------------------------
void
Entity::setSpriteID( sqlite3_int64 sprite_id )
{
    m_sprite_id = sprite_id;
    m_sprite = Engine::em()->getSprite( sprite_id );
}

//==============================================================================
