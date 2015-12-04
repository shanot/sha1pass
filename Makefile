prefix = /usr/local
CXXFLAGS+=-std=c++11
CXXFLAGS+=-g
CXXFLAGS+=`pkg-config --cflags --libs gtkmm-3.0`

all:sha1pass

install:sha1pass
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp sha1pass ${DESTDIR}${PREFIX}/bin


clean:
	${RM} sha1pass
