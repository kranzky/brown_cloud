//==============================================================================

#include <engine.hpp>
#include <viewport.hpp>

//------------------------------------------------------------------------------
ViewPort::ViewPort()
    :
    m_offset(),
    m_bounds(),
    m_screen(),
    m_hscale( 0.0f ),
    m_vscale( 0.0f ),
    m_angle( 0.0f )
{
}

//------------------------------------------------------------------------------
ViewPort::~ViewPort()
{
}

//------------------------------------------------------------------------------
float
ViewPort::angle()
{
    return m_angle;
}

//------------------------------------------------------------------------------
b2Vec2 &
ViewPort::offset()
{
    return m_offset;
}

//------------------------------------------------------------------------------
b2Vec2 &
ViewPort::bounds()
{
    return m_bounds;
}

//------------------------------------------------------------------------------
b2Vec2 &
ViewPort::screen()
{
    return m_screen;
}

//------------------------------------------------------------------------------
void
ViewPort::screenToWorld( b2Vec2 & point )
{
    _updateRatios();
	    point.x = point.x - ( 0.5f * m_screen.x - m_offset.x );
	    point.y = point.y - ( 0.5f * m_screen.y - m_offset.y );
		point.x /= m_hscale;
		point.y /= m_vscale;
}

//------------------------------------------------------------------------------
float
ViewPort::hscale() const
{
    _updateRatios();
    return m_hscale;
}

//------------------------------------------------------------------------------
float
ViewPort::vscale() const
{
    _updateRatios();
    return m_vscale;
}

//------------------------------------------------------------------------------
void
ViewPort::setAngle( float angle )
{
    m_angle = angle;
}

//------------------------------------------------------------------------------
void
ViewPort::setTransform( float scale )
{
    _updateRatios();
    Engine::hge()->Gfx_SetTransform( m_offset.x, m_offset.y,
                                     0.5f * m_screen.x, 0.5f * m_screen.y,
                                     m_angle,
                                     scale * m_hscale, scale * m_vscale );
}

//------------------------------------------------------------------------------
bool
ViewPort::restore()
{
    int width( 0 );
    int height( 0 );
    if ( Engine::hge()->System_GetState( HGE_WINDOWED ) )
    {
        width = Engine::hge()->System_GetState( HGE_SCREENWIDTH );
        height = Engine::hge()->System_GetState( HGE_SCREENHEIGHT );
    }
    else
    {
        width = GetSystemMetrics( SM_CXSCREEN );
        height = GetSystemMetrics( SM_CYSCREEN );
    }
    m_screen.x = static_cast<float>( width );
    m_screen.y = static_cast<float>( height );
	return true;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
ViewPort::_updateRatios() const
{
    m_hscale = m_screen.x / m_bounds.x;
    m_vscale = m_screen.y / m_bounds.y;
}

//==============================================================================
