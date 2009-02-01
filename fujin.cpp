//==============================================================================

#include <fujin.hpp>
#include <engine.hpp>
#include <entity_manager.hpp>
#include <cloud.hpp>

#include <hgeSprite.h>
#include <Box2D.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>
#include <hgeparticle.h>
#include <hgeresource.h>

//==============================================================================
Fujin::Fujin( float max_strength, float scale )
    :
    Entity( scale ),
    Damageable( max_strength ),
    m_AABB(),
	m_suckedClouds(),
	m_timeToNextCloudBlowOut(0.1f),
    m_isBlowing(false),
    m_isSick( false ),
    m_isAsleep( false ),
    m_channel( 0 ),
    m_suck( false )
{
}

//------------------------------------------------------------------------------
Fujin::~Fujin()
{
        if ( m_channel != 0 )
        {
            Engine::instance()->hge()->Channel_Stop( m_channel );
            m_channel = 0;
        }
}

//------------------------------------------------------------------------------
void
Fujin::collide( Entity * entity, b2ContactPoint * point )
{
    float force( b2Clamp( point->normalForce * 0.01f, 0.0f, 1.0f ) );
    if ( force > 0.0f && Engine::instance()->getConfig().vibrate )
    {
        Engine::instance()->getController().rumble( force, force, 0.2f );
    }
    takeDamage( b2Clamp( point->normalForce * 0.01f, 0.0f, 10.0f ) );
}

//------------------------------------------------------------------------------
void
Fujin::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "angle", "%f", "scale", "%f",
                      "sprite_id", "%d" };
    m_id = Engine::em()->persistToDatabase( this, rows, m_body->GetPosition().x,
                                                        m_body->GetPosition().y,
                                                        m_body->GetAngle(),
                                                        m_scale, m_sprite_id );
}

//------------------------------------------------------------------------------
bool
Fujin::isSick()
{
    return m_isSick;
}

//------------------------------------------------------------------------------
void
Fujin::setSick( bool sick )
{
    m_isSick = sick;
}

//------------------------------------------------------------------------------
bool
Fujin::isAsleep()
{
    return m_isAsleep;
}

//------------------------------------------------------------------------------
void
Fujin::setAsleep( bool bsleep )
{
    m_isAsleep = bsleep;
	hgeParticleSystem * sleep( Engine::rm()->GetParticleSystem( "sleep" ) );
    if ( m_isAsleep )
    {
        sleep->Fire();
    }
    else
    {
        sleep->Stop();
    }
}

//------------------------------------------------------------------------------
// static:
//------------------------------------------------------------------------------
void
Fujin::registerEntity()
{
    Engine::em()->registerEntity( Fujin::TYPE, Fujin::factory, "fujins",
                                  "id, x, y, angle, scale, sprite_id" );
}

//------------------------------------------------------------------------------
// protected:
//------------------------------------------------------------------------------
void
Fujin::onSetScale()
{
    if ( m_body == 0 )
    {
        return;
    }
    b2Shape * shape;
    while ( shape = m_body->GetShapeList() )
    {
        m_body->DestroyShape( shape );
    }
    b2PolygonDef shapeDef;
    shapeDef.SetAsBox( 32.0f * m_scale, 32.0f * m_scale );
    shapeDef.density = 1.0f;
    shapeDef.friction =0.0f;
    shapeDef.restitution = 0.3f;
    m_body->CreateShape( & shapeDef );
    m_body->SetMassFromShapes();

	hgeParticleSystem * breath( Engine::rm()->GetParticleSystem( "breath" ) );
	breath->SetScale(m_scale);
	hgeParticleSystem * sleep( Engine::rm()->GetParticleSystem( "sleep" ) );
	sleep->SetScale(m_scale);
}

//------------------------------------------------------------------------------
void
Fujin::doInit()
{
    b2BodyDef bodyDef;
    bodyDef.allowSleep = false;
    bodyDef.userData = static_cast< void * >( this );
    m_body = Engine::b2d()->CreateDynamicBody( & bodyDef );
	m_body->m_linearDamping = 0.8f;
	m_AABB.lowerBound= b2Vec2(-2.0f,-2.0f);
	m_AABB.upperBound= b2Vec2(2.0f,2.0f);
    onSetScale();

    m_isSick = false;
    m_isAsleep = false;
    m_channel = 0;

	Engine::rm()->GetParticleSystem( "breath" )->SetScale( m_scale );
	Engine::rm()->GetParticleSystem( "sleep" )->SetScale( m_scale );

	const Controller & pad( Engine::instance()->getController() );
	Engine::instance()->setMouse("cursor");
}

//------------------------------------------------------------------------------
void
Fujin::doUpdate( float dt )
{
	if (m_timeToNextCloudBlowOut > 0.0f)
		m_timeToNextCloudBlowOut -= dt;

    const Controller & pad( Engine::instance()->getController() );
	const Mouse &mouse(Engine::instance()->getMouse());
	
	const Mouse::MouseButton & leftMouseBtn(mouse.getLeft());

	hgeParticleSystem * breath( Engine::rm()->GetParticleSystem( "breath" ) );
	hgeParticleSystem * sleep( Engine::rm()->GetParticleSystem( "sleep" ) );

    b2Vec2 acceleration( 0.0f, 0.0f );
    float power( 0.0f );

    if ( m_isAsleep )
    {
        m_sprite = Engine::rm()->GetSprite( "fujin_sleep" );
    }
    if ( Engine::instance()->isPaused() || m_isAsleep )
    {
		Engine::instance()->hideMouse();
    }
    else if ( pad.isConnected() )
    {
		Engine::instance()->hideMouse();

        b2Vec2 offset( pad.getStick( XPAD_THUMBSTICK_RIGHT ) );
        offset.y *= -1.0f;
        float angle = lookAt(offset);

        acceleration = pad.getStick( XPAD_THUMBSTICK_LEFT );

        power = pad.getTrigger(XPAD_TRIGGER_LEFT) -
                pad.getTrigger(XPAD_TRIGGER_RIGHT);

    }
	else
	{
		Engine::instance()->showMouse();

		if(Engine::hge()->Input_GetKeyState(HGEK_W))
		{
            acceleration.y += 1.0f;
		}
		if (Engine::hge()->Input_GetKeyState(HGEK_S))
		{
            acceleration.y -= 1.0f;
		}
		if (Engine::hge()->Input_GetKeyState(HGEK_A))
		{
            acceleration.x -= 1.0f;
		}
		if (Engine::hge()->Input_GetKeyState(HGEK_D))
		{
            acceleration.x += 1.0f;
		}

        if ( Engine::hge()->Input_GetKeyState( HGEK_LBUTTON ) )
        {
            power += 1.0f;
        }
        if ( Engine::hge()->Input_GetKeyState( HGEK_RBUTTON ) )
        {
            power -= 1.0f;
        }

		b2Vec2 position (m_body->GetPosition());
		b2Vec2 mousePosition(mouse.getMousePos());
		b2Vec2 newPos = mousePosition - position;
		float angle = lookAt(newPos);
	}

	bool dead( acceleration.LengthSquared() < 0.2f );
	acceleration *= ( 1000.0f * m_scale * dt );
    acceleration.y *= -1.0f;
    b2Vec2 velocity( m_body->GetLinearVelocity() );
    velocity += acceleration;
	if ( dead )
	{
	    velocity *= 0.9f;
	}
	m_body->SetAngularVelocity( 0.0f );
    m_body->SetLinearVelocity( velocity );

	if( power > 0.01f || power < -0.01f )
	{
		breath->Fire();
		Blow( power );
		m_isBlowing=true;
        if ( power > 0.0f )
        {
            if ( m_suck && m_channel != 0 )
            {
            Engine::instance()->hge()->Channel_Stop( m_channel );
            m_channel = 0;
            }
            m_suck = false;
            int volume( static_cast< int >( 10.0f * power ) );
            if ( m_channel == 0 )
            {
		    m_channel = Engine::instance()->hge()->Effect_PlayEx(
                    Engine::rm()->GetEffect( "wind" ), volume, 0, 1, true );
            }
            Engine::instance()->hge()->Channel_SetVolume( m_channel, volume );
			blowOutClouds();
        }
        else
        {
            if ( ! m_suck && m_channel != 0  )
            {
            Engine::instance()->hge()->Channel_Stop( m_channel );
            m_channel = 0;
            }
            m_suck = true;
            int volume( static_cast< int >( - 10.0f * power ) );
            if ( m_channel == 0 )
            {
		    m_channel = Engine::instance()->hge()->Effect_PlayEx(
                    Engine::rm()->GetEffect( "pant" ), volume, 0, 1, true );
            }
            Engine::instance()->hge()->Channel_SetVolume( m_channel, volume );
			suckUpClouds();
        }
		breath->info.nEmission = static_cast< int >( 20.0f * power );
    }
	else
	{
		m_isBlowing=false;
		breath->Stop();
        if ( m_channel != 0 )
        {
            Engine::instance()->hge()->Channel_Stop( m_channel );
            m_channel = 0;
        }
		if ( m_isAsleep )
		{
			m_sprite = Engine::rm()->GetSprite( "fujin_sleep" );
		}
        else if ( m_isSick )
        {
            m_sprite = Engine::rm()->GetSprite( "fujin_sick" );
        }
        else
        {
            m_sprite = Engine::rm()->GetSprite( "fujin" );
        }
	}

	sleep->info.nEmission = 3;

	b2Vec2 position( m_body->GetPosition() );
	b2Vec2 direction( 0.0f, 1.0f );
	direction = b2Mul( m_body->GetXForm().R, -direction );
	position = position + 50.0f * m_scale * direction;
	sleep->MoveTo( position.x / m_scale, position.y / m_scale, false );
	position = m_body->GetPosition() - 50.0f * m_scale * direction;
	breath->MoveTo( position.x / m_scale, position.y / m_scale, true );
    float angle( m_body->GetAngle() );
	breath->info.fDirection= angle -M_PI;
		
	breath->Update( dt );
	sleep->Update( dt );
}

//------------------------------------------------------------------------------
void
Fujin::doRender( float scale )
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
	hgeParticleSystem * breath( Engine::rm()->GetParticleSystem( "breath" ) );
	hgeParticleSystem * sleep( Engine::rm()->GetParticleSystem( "sleep" ) );
    m_sprite->RenderEx( position.x, position.y, angle, m_scale * 1.5f );
	breath->Render();
	sleep->Render();
    renderDamageable( position, m_scale );
	const Mouse &mouse(Engine::instance()->getMouse());
}

//------------------------------------------------------------------------------
void
Fujin::initFromQuery( Query & query )
{
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );

    m_id = static_cast< sqlite_int64 >( query.getnum() );
    position.x = static_cast< float >( query.getnum() );
    position.y = static_cast< float >( query.getnum() );
    angle = static_cast< float >( query.getnum() );
    m_scale = static_cast< float >( query.getnum() );

    setSpriteID( static_cast< sqlite_int64 >( query.getnum() ) );

    init();

    m_body->SetXForm( position, angle );
}

//------------------------------------------------------------------------------
float Fujin::lookAt( const b2Vec2& targetPoint )
{
	b2Vec2 offset(targetPoint );
	float length( offset.Normalize() );
	float angle =0;
	if ( length > 0.9f )
	{
		b2Vec2 vertical( 0.0f, 1.0f );
		angle=( acosf( b2Dot( offset, vertical ) ) );
		if ( b2Cross( vertical, offset ) < 0.0f )
		{
			angle = -angle;
		}
		m_body->SetXForm( m_body->GetPosition(), angle );
	}
	return angle;
}
//==============================================================================


void Fujin::Blow( float power )
{
    if ( power > 0.0f )
    {
        m_sprite = Engine::rm()->GetSprite( "fujin_blow" );
    }
    else
    {
        m_sprite = Engine::rm()->GetSprite( "fujin_suck" );
    }

    b2Vec2 position( m_body->GetPosition() );

	m_AABB.lowerBound= b2Vec2(position.x-200.0f*m_scale,
                              position.y-200.0f*m_scale);

	m_AABB.upperBound= b2Vec2(position.x+200.0f*m_scale,
                              position.y+200.0f*m_scale);

	const int32 k_bufferSize = 10;

	b2Shape *buffer[k_bufferSize];

	int32 count = Engine::b2d()->Query(m_AABB, buffer, k_bufferSize);

	for ( int32 i = 0; i < count; ++i )
	{
        Entity * entity( static_cast< Entity * >(
            buffer[i]->GetBody()->GetUserData() ) );
        if ( entity->getType() != Cloud::TYPE )
        {
            continue;
        }
        if ( m_scale < entity->getScale() * 0.99f ||
             m_scale * 0.99f > entity->getScale() )
        {
            continue;
        }

        b2Vec2 vertical( 0.0f, 1.0f );
        b2Mat22 rotation( m_body->GetAngle() );
        b2Vec2 heading( b2Mul( rotation, vertical ) );

        b2Vec2 offset( entity->getBody()->GetPosition() -
                       m_body->GetPosition() );

        float length( offset.Normalize() );

        float dot( b2Dot( offset, heading ) );

		if ( dot < 0.0f || length < 1.0f )
        {
            continue;
        }

        float ratio( 1.0f - ( length * length ) /
			                ( 200.0f * 200.0f * m_scale * m_scale ) );

		if ( ratio <= 0.1f )
		{
			ratio = 0.1f;
		}

        float force( 1000000.0f * m_scale * m_scale * dot * ratio * power );

		offset *= force;

        entity->getBody()->WakeUp();
        entity->getBody()->ApplyForce( offset,
                                       entity->getBody()->GetPosition() );
	}
}

//------------------------------------------------------------------------------
void Fujin::suckUpClouds()
{
	EntityManager* em = Engine::em();
	std::vector<Entity*> clouds = em->getEntities(Cloud::TYPE);
	std::vector<Entity*>::iterator i;

	for (i = clouds.begin(); i != clouds.end(); ++i)
	{
		Cloud* cloud = static_cast<Cloud*>(*i);
		b2Vec2 meToCloud = cloud->getBody()->GetPosition() - getBody()->GetPosition();
		if (cloud->getScale() > getScale() * 0.99f
			&& cloud->getScale() < getScale() * 1.01f 
			&& meToCloud.Length() < 10.0f)
		{
			bool alreadySucked = false;
			std::vector<Cloud*>::iterator suckedIter;
			for (suckedIter = m_suckedClouds.begin(); suckedIter != m_suckedClouds.end(); ++suckedIter)
			{
				if ((*suckedIter) == cloud)
					alreadySucked = true;
			}

			if (!alreadySucked)
			{
				cloud->removeFromClump();
				cloud->removeFromWorld();
				m_suckedClouds.push_back(cloud);
                Engine::hge()->Effect_PlayEx( Engine::rm()->GetEffect( "eat" ), 100 );
			}
		}
	}

}

//------------------------------------------------------------------------------
void Fujin::blowOutClouds()
{
	if (m_timeToNextCloudBlowOut < 0.0f)
	{
		if (m_suckedClouds.size() > 0)
		{
			Cloud* cloud = m_suckedClouds.back();
			cloud->addToWorld(getBody()->GetPosition(), m_scale);
			m_suckedClouds.pop_back();
            Engine::hge()->Effect_PlayEx( Engine::rm()->GetEffect( "spit" ), 20 );
		}

		m_timeToNextCloudBlowOut = 2.0f;
	}
}
