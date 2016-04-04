#ifndef FORTH_VIEWER_H
#define FORTH_VIEWER_H

#include <string>
#include "forth_renderer.h"

namespace oppvs {
	struct ForthObject;
	typedef void (*java_callback)();
	class ForthViewer
	{
	public:
		ForthViewer();
		~ForthViewer();
		void attachCallback(java_callback cb);
		void attachRenderer(ForthRenderer* render);
		int startStreaming(const std::string& streamKey, const std::string& serverAddress);

		java_callback m_callback;
		ForthRenderer* m_renderer;
	private:
		ForthObject* m_object;
		

	};

} // oppvs

#endif // FORTH_VIEWER_H
