//==============================================================================

#include <entity_manager.hpp>
#include <entity.hpp>
#include <engine.hpp>

#include <Database.h>
#include <Query.h>
#include <hgeResource.h>

#include <sstream>
#include <cstdarg>

//------------------------------------------------------------------------------
EntityManager::EntityManager()
    :
    m_registry(),
    m_entities(),
    m_sprites(),
    m_names()
{
}

//------------------------------------------------------------------------------
EntityManager::~EntityManager()
{
    fini();
}

//------------------------------------------------------------------------------
void
EntityManager::init()
{
    Database database( "world.db3" );
    Query query( database );

    query.get_result( "SELECT id, name FROM sprites;"  );

    while ( query.fetch_row() )
    {
        sqlite_int64 sprite_id( static_cast< sqlite_int64 >( query.getnum() ) );
        std::string name( query.getstr() );
        m_names[name] = sprite_id;
        m_sprites[sprite_id] = Engine::rm()->GetSprite( name.c_str() );
    }

    query.free_result();
}

//------------------------------------------------------------------------------
void
EntityManager::update( float dt )
{
    std::vector< Entity * >::iterator i;
    for ( i = m_entities.begin(); i != m_entities.end(); ++i )
    {
        ( * i )->update( dt );
    }
}

//------------------------------------------------------------------------------
void
EntityManager::fini()
{
    while ( m_entities.size() > 0 )
    {
        delete m_entities.back();
        m_entities.pop_back();
    }
    m_registry.clear();
    m_sprites.clear();
    m_names.clear();
}

//------------------------------------------------------------------------------
void
EntityManager::registerEntity( unsigned int type, Entity * ( * factory )(),
                               const char * table, const char * query )
{
    std::map< unsigned int, EntityData >::iterator i( m_registry.find( type ) );
    if ( i != m_registry.end() )
    {
        Engine::hge()->System_Log( "Cannot register same entity twice." );
        return;
    }
    EntityData data;
    data.m_factory = factory;
    data.m_table = table;
    data.m_query = query;
    m_registry.insert( std::pair< unsigned int, EntityData >( type, data ) );
}

//------------------------------------------------------------------------------
sqlite_int64
EntityManager::registerSprite( const char * cname )
{
    std::string name( cname );
    std::map< std::string, sqlite_int64 >::iterator i( m_names.find( name ) );
    if ( i != m_names.end() )
    {
        return m_names[name];
    }
    sqlite_int64 id( 0 );
    for ( id = 0; m_sprites.find( id ) != m_sprites.end(); ++id );
    m_names[name] = id;
    m_sprites[id] = Engine::rm()->GetSprite( cname );
    return id;
}

//------------------------------------------------------------------------------
Entity *
EntityManager::factory( unsigned int type, bool add )
{
    std::map< unsigned int, EntityData >::iterator i( m_registry.find( type ) );
    if ( i == m_registry.end() )
    {
        Engine::hge()->System_Log( "Tried to create unregistered entity." );
        return 0;
    }
    Entity * entity( i->second.m_factory() );
    entity->setType( type );
    if ( add )
    {
        m_entities.push_back( entity );
    }
    return entity;
}

//------------------------------------------------------------------------------
std::vector< Entity * >
EntityManager::databaseFactory( unsigned int type )
{
    std::vector< Entity * > entities;

    if ( type == 0 )
    {
        std::map< unsigned int, EntityData >::iterator i;
        for ( i = m_registry.begin(); i != m_registry.end(); ++i )
        {
            std::vector< Entity * > tmp( databaseFactory( i->first ) );
            entities.insert( entities.end(), tmp.begin(), tmp.end() );
        }
        return entities;
    }

    Database database( "world.db3" );
    Query query( database );

    std::map< unsigned int, EntityData >::iterator i( m_registry.find( type ) );
    if ( i != m_registry.end() )
    {
        char data[256];
        sprintf_s( data, 255, "SELECT %s FROM %s;", i->second.m_query,
                                                    i->second.m_table );
        query.get_result( data );
    }
    else
    {
        Engine::hge()->System_Log( "Tried to query unregistered entity." );
    }

    while ( query.fetch_row() )
    {
        entities.push_back( factory( type, false ) );
        entities.back()->initFromQuery( query );    
    }

    query.free_result();

    return entities;
}

//------------------------------------------------------------------------------
sqlite_int64
EntityManager::persistToDatabase( Entity * entity, char * rows[], ... )
{
    sqlite_int64 id( entity->getID() );
    unsigned int type( entity->getType() );

    std::map< unsigned int, EntityData >::iterator i( m_registry.find( type ) );
    if ( i == m_registry.end() )
    {
        Engine::hge()->System_Log( "Cannot persist unregistered entity." );
        return 0;
    }

    int num( 0 );
    char * names[10];
    char * types[10];
    while ( * rows != 0 && num < 10 )
    {
        names[num] = * ( rows++ );
        types[num] = * ( rows++ );
        ++num;
    }

    std::stringstream format;
    if ( id == 0 )
    {
        format << "INSERT INTO " << i->second.m_table << " (";
        for ( int i = 0; i < num; ++i )
        {
            if ( i > 0 ) format << ", ";
            format << names[i];
        }
        format << ") VALUES (";
        for ( int i = 0; i < num; ++i )
        {
            if ( i > 0 ) format << ", ";
            format << types[i];
        }
        format << ")";
    }
    else
    {
        format << "UPDATE " << i->second.m_table << " SET ";
        for ( int i = 0; i < num; ++i )
        {
            if ( i > 0 ) format << ", ";
            format << names[i] << "=" << types[i];
        }
        format << " WHERE id=" << id;
    }

    va_list args;
    char sql[256];
    va_start( args, rows );
    vsprintf_s( sql, 255, format.str().c_str(), args );
    va_end( args );

    Database database( "world.db3" );
    Query query( database );

    if ( ! query.execute( sql ) )
    {
        Engine::hge()->System_Log( "Query Failed: %s", sql );
    }
    else if ( id == 0 )
    {
        id = query.insert_id();
    }

    return id;
}

//------------------------------------------------------------------------------
void
EntityManager::deleteFromDatabase( Entity * entity )
{
    sqlite_int64 id( entity->getID() );
    unsigned int type( entity->getType() );

    if ( id == 0 )
    {
        return;
    }
    std::map< unsigned int, EntityData >::iterator i( m_registry.find( type ) );
    if ( i == m_registry.end() )
    {
        Engine::hge()->System_Log( "Cannot delete unregistered entity." );
        return;
    }
    Database database( "world.db3" );
    Query query( database );
    char sql[256];
    sprintf_s( sql, 255, "DELETE FROM %s WHERE id=%d", i->second.m_table, id );
    if ( ! query.execute( sql ) )
    {
        Engine::hge()->System_Log( "Query Failed: %s", sql );
    }
}

//------------------------------------------------------------------------------
hgeSprite *
EntityManager::getSprite( sqlite_int64 sprite_id )
{
    std::map< sqlite_int64,hgeSprite * >::iterator i(m_sprites.find(sprite_id));
    if ( i == m_sprites.end() )
    {
        Engine::hge()->System_Log( "Cannot find referenced sprite." );
        return 0;
    }
    return m_sprites[sprite_id];
}

//==============================================================================
