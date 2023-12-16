#ifndef  __MODULEBASE_H__
#define  __MODULEBASE_H__

class ModuleBase
{
public:
	virtual void start() = 0; 
	virtual void close() = 0;
};

#endif
