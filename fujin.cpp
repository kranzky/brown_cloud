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
    Damageable( max_strength ), m_isBlowing(false)
{
}

//------------------------------------------------------------------------------
Fujin::~Fujin()
{
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
}

//------------------------------------------------------------------------------
void
Fujin::doInit()
{
    b2BodyDef bodyDef;
    bodyDef.userData = static_cast< void * >( this );
    m_body = Engine::b2d()->CreateDynamicBody( & bodyDef );
	m_body->m_linearDamping = 0.8f;
	m_AABB.lowerBound= b2Vec2(-2.0f,-2.0f);
	m_AABB.upperBound= b2Vec2(2.0f,2.0f);
    onSetScale();

	Engine::rm()->GetParticleSystem( "breath" )->SetScale( m_scale );

	const Controller & pad( Engine::instance()->getController() );
	if(! pad.isConnected())
	{
		Engine::instance()->setMouse("cursor");
		Engine::instance()->showMouse();
	}
	else
	{
		Engine::instance()->hideMouse();
	}
}

//------------------------------------------------------------------------------
void
Fujin::doUpdate( float dt )
{
    const Controller & pad( Engine::instance()->getController() );
	const Mouse &mouse(Engine::instance()->getMouse());
	
	const Mouse::MouseButton & leftMouseBtn(mouse.getLeft());

	hgeParticleSystem * breath( Engine::rm()->GetParticleSystem( "breath" ) );

    if ( pad.isConnected() && ! Engine::instance()->isPaused() )
    {
        b2Vec2 offset( pad.getStick( XPAD_THUMBSTICK_RIGHT ) );
        offset.y *= -1.0f;
     
       float angle = lookAt(offset);

        b2Vec2 acceleration( pad.getStick( XPAD_THUMBSTICK_LEFT )  );
		bool dead( acceleration.LengthSquared() < 0.2f );
		    acceleration *=  ( 1000.0f * m_scale * dt );
            acceleration.y *=  -1.0f;

        float power( pad.getTrigger(XPAD_TRIGGER_LEFT) -
                     pad.getTrigger(XPAD_TRIGGER_RIGHT) );
		if(power>0.01f || power<-0.01f)
		{
			breath->Fire();
			Blow( power );
			m_isBlowing=true;
            if ( power > 0.0f )
            {
            int volume( static_cast< int >( 100.0f * power ) );
			Engine::instance()->hge()->Effect_PlayEx(
                Engine::rm()->GetEffect( "wind" ), volume );
            }
            else
            {
            int volume( static_cast< int >( - 100.0f * power ) );
			Engine::instance()->hge()->Effect_PlayEx(
                Engine::rm()->GetEffect( "pant" ), volume );
            }
			breath->info.nEmission = static_cast< int >( 20.0f * power );
        }
		else
		{
			m_isBlowing=false;
			breath->Stop();
		}

        if ( breath->GetParticlesAlive() > 0 )
        {
			b2Vec2 position( m_body->GetPosition() );
			b2Vec2 direction( 0.0f, 1.0f );
			direction = b2Mul( m_body->GetXForm().R, -direction );
			position = position - 32.0f * m_scale * direction;
			breath->MoveTo( position.x / m_scale, position.y / m_scale, true );
            float angle( m_body->GetAngle() );
			breath->info.fDirection= angle -M_PI;
        }
		
        b2Vec2 velocity( m_body->GetLinearVelocity() );
        velocity += acceleration;
		if ( dead )
		{
			velocity *= 0.9f;
		}
			m_body->SetAngularVelocity( 0.0f );
        m_body->SetLinearVelocity( velocity );
    }
	else if(! Engine::instance()->isPaused() )
	{
		float xForce = 0;
		float yForce = 0;
		b2Vec2 direction( 1.0f, 1.0f );
		if(Engine::hge()->Input_GetKeyState(HGEK_W))
			{

				yForce =  -0.5f ;
				
			
			}
		if (Engine::hge()->Input_GetKeyState(HGEK_S))
		{
				yForce = 0.5f;
		}

		if (Engine::hge()->Input_GetKeyState(HGEK_A))
			{
				xForce=  -0.5f ;
				
			}
			
		if (Engine::hge()->Input_GetKeyState(HGEK_D))
			{
				xForce = 0.5f ;
				
				
			}
		b2Vec2 position (m_body->GetPosition());
		b2Vec2 mousePosition(mouse.getMousePos());
		b2Vec2 newPos = mousePosition - position;
		//newPos.x = newPos.x/newPos.Length();
		//newPos.y = newPos.y/newPos.Length();
		float angle = lookAt(newPos);
		if(leftMouseBtn.dragging())
		{
			// we are blowing, better make sure we are displaying the particles
			b2Vec2 position( m_body->GetPosition() );
			b2Vec2 direction( 0.0f, 1.0f );
			direction = b2Mul( m_body->GetXForm().R, -direction );
			position = position - 32.0f * m_scale * direction;
			breath->MoveTo( position.x / m_scale, position.y / m_scale, true );
			breath->info.fDirection= angle -M_PI;
			breath->Fire();
			Blow();
			m_isBlowing=true;
			Engine::instance()->hge()->Effect_Play( Engine::rm()->GetEffect( "wind" ) );
		}
		
			else
		{
			m_isBlowing=false;
			breath->Stop();
		}
		

		direction.x = xForce * direction.x * 1000000.0f * m_scale;
		direction.y = yForce * direction.y* 1000000.0f * m_scale;
		m_body->ApplyForce(direction, m_body->GetWorldCenter());
		
	}
	breath->Update( dt );

	b2Vec2 position = m_body->GetWorldCenter();
   // updateDamageable( dt );
}

//------------------------------------------------------------------------------
void
Fujin::doRender( float scale )
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
	hgeParticleSystem * breath( Engine::rm()->GetParticleSystem( "breath" ) );
    m_sprite->RenderEx( position.x, position.y, angle, m_scale );
	breath->Render();
    renderDamageable( position, m_scale );
	const Mouse &mouse(Engine::instance()->getMouse());

// 	Engine::hge()->Gfx_RenderLine(mouse.getMousePos().x, mouse.getMousePos().y,
// 								  m_body->GetPosition().x, m_body->GetPosition().y);
	
	
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

        entity->getBody()->ApplyForce( offset,
                                       entity->getBody()->GetPosition() );
	}
}
