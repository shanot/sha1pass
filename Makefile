CXXFLAGS+=-std=c++11
CXXFLAGS+=-g
CXXFLAGS+=`pkg-config --cflags --libs gtkmm-3.0`

all:sha1pass


clean:
	${RM} sha1pass
