CXX_FLAGS=-std=c++17 -Wall -O3 -ffast-math -march=native -m64 -DNDEBUG

CXX=g++

INCL=-I./include
LIB=-L./lib

lib=-lconfig -lconfig++ -lthostmduserapi -lthosttraderapi -lexanic -ldstartradeapi -lyd -lxele_futures_trader_api -lciul1 -lrt -lpthread 

all: trader

src=$(wildcard ./*.cpp ./api/ctp2mini/*.cpp ./api/simulate/*.cpp ./api/v10/*.cpp ./api/xele/*.cpp ./api/yd/*.cpp ./agent/*.cpp ./core/*.cpp ./feed/ctp/*.cpp ./feed/czce/*.cpp ./feed/dce/*.cpp ./feed/shfe/*.cpp ./feed/simulate/*.cpp ./logger/*.cpp ./strategy/*.cpp ./system/*.cpp ./tool/*.cpp ./user/*.cpp)
obj=$(patsubst %.cpp,%.o,$(src))

%.o: %.cpp
	$(CXX) -c $(CXX_FLAGS) $(INCL) $< -o $@

trader: $(obj)
	$(CXX) $^ $(CXX_FLAGS) $(INCL) $(LIB) $(lib) -o $@

.PHONY : clean dist
clean:
	rm -rf $(obj) trader
