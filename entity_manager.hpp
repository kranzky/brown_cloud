//==============================================================================

#ifndef ArseThingManager
#define ArseThingManager

#pragma once

#include <vector>
#include <map>

#include <sqlite3.h>

class Entity;
class hgeSprite;

//------------------------------------------------------------------------------
class EntityManager
{
  public:
    EntityManager();
    virtual ~EntityManager();

    void init();
    void update( float dt );
    void fini();

    void registerEntity( unsigned int type, Entity * ( * factory )(),
                         const char * table, const char * query );

    Entity * factory( unsigned int type );
    std::vector< Entity * > databaseFactory( unsigned int type = 0 );

    sqlite_int64 persistToDatabase( Entity * entity, char * rows[], ... );
    void deleteFromDatabase( Entity * entity );

    hgeSprite * getSprite( sqlite_int64 sprite_id );

  protected:
    EntityManager( const EntityManager & );
    EntityManager & operator=( const EntityManager & );

  protected:
    struct EntityData
    {
        Entity * ( * m_factory )();
        const char * m_table;
        const char * m_query;
    };
    std::map< unsigned int, EntityData > m_registry;
    std::vector< Entity * > m_entities;
    std::map< sqlite_int64, hgeSprite * > m_sprites;
};

#endif

//==============================================================================
