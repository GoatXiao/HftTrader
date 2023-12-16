#ifndef  __USER_H__
#define  __USER_H__

#include "user/user_derived.h"

namespace USER
{
	static UserStrategyBase* getUserStrategy(const ThreadConfig& cfg)
	{
		switch (cfg.strategy_id)
		{
		case 0:
        {
			return new UserStrategy(&cfg);
        } break;
		default:
        {
			return nullptr;
        } break;
		}
	}
};
#endif
