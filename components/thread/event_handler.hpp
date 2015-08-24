#ifndef OPPVS_EVENT_HANDLER_HPP
#define OPPVS_EVENT_HANDLER_HPP

#include <functional>
#include "datatypes.hpp"

namespace oppvs {
	
	class EventHandler {
	private:
		/* data */
	public:
		template<typename T>
		void addHandler(std::function<T(void)> callback)
		{
			callback();
		}

		template<typename T, typename... Params>
		void addHandler(std::function<T(Params...)> callback, Params... args)
		{
			callback(args...);
		}

		using fp = void (*)(void);
		void addHandler(fp f)
		{
			addHandler(std::function<void()>(f));
		}

	};
} // oppvs

#endif // OPPVS_EVENT_HANDLER_HPP
