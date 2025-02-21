#ifndef GSERVER_COMMON_UNCOPYABLE_H
#define GSERVER_COMMON_UNCOPYABLE_H

namespace gserver
{
	class Uncopyable
	{
	protected:
		Uncopyable(){}
		~Uncopyable(){}
	private:
		Uncopyable(const Uncopyable&);
		Uncopyable& operator=(const Uncopyable&);
	};
}

#endif // GSERVER_COMMON_UNCOPYABLE_H