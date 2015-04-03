/*
	General rendering class
*/

#ifndef OPPVS_RENDER_HPP
#define OPPVS_RENDER_HPP

#include "databytes.hpp"

namespace oppvs
{
	class Render
	{
	public:
		virtual void setup();
		virtual void draw();
	private:
		int m_frameWidth;
		int m_frameHeight;
		int m_viewWidth;
		int m_viewHeight;
	};
}

#endif