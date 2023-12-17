CXX_FLAGS=-std=c++17 -Wall -g -march=native -O3 -m64 -gdwarf-2 -gstrict-dwarf

CXX=g++

INCL=-I./include
LIB=-L./lib

lib=-lconfig -lconfig++ -lthostmduserapi -lthosttraderapi -lexanic -ldstartradeapi -lyd -lxele_futures_trader_api -lciul1 -lrt -lpthread 

all: trader

src=$(wildcard ./*.cpp ./tools/*.cpp ./api/ctp2mini/*.cpp ./api/simulate/*.cpp ./api/efvi/*.cpp ./api/v10/*.cpp ./api/xele/*.cpp ./api/yd/*.cpp ./core/*.cpp ./core/feed/*.cpp ./core/agent/*.cpp ./core/event/*.cpp  ./core/strategy/*.cpp ./core/logger/*.cpp ./core/user/*.cpp)
obj=$(patsubst %.cpp,%.o,$(src))

%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCL) $< -o $@

trader: $(obj)
	$(CXX) $^ $(CXX_FLAGS) $(INCL) $(LIB) $(lib) -o $@

.PHONY : clean dist
clean:
	rm -rf $(obj) trader
