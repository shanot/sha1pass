CXXFLAGS+=-std=c++11
CXXFLAGS+=-g
CXXFLAGS+=`pkg-config --cflags --libs gtkmm-3.0`

all:sha1pass

install:sha1pass
	cp sha1pass ${DESTDIR}


clean:
	${RM} sha1pass
