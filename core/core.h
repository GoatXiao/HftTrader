#include "../modulebase.h"

class Core
{
public:
	Core(int pltype);
	~Core();

public:
	void LAUNCH_SYSTEM();
    void set_strategy_id(int);

private:
	ModuleBase* m_pModule{ nullptr };
    int strategy_id{ -1 };
	int m_plt;
	
private:
	void init();
};
