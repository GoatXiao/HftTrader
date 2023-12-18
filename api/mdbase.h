#ifndef  __MDBASE_H__
#define  __MDBASE_H__

#include "../system/system.h"

class MDBase
{
public:
	MDBase(){}
	virtual ~MDBase(){}
public:
	int m_nRequestID;
public:
	//初始化
	virtual bool init() = 0;

	//反初始化
	virtual bool uninit() = 0;

	virtual void setInsts(std::vector<std::string>& _insts) = 0;

	virtual void setQueue(PRODUTIONLINE_TYPE plt) = 0;
};
#endif
