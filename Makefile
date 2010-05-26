CXX = g++
CXXFLAGS = -ggdb `pkg-config --cflags gtk+-2.0`
LDFLAGS = `pkg-config --libs gtk+-2.0`


all: uzbltab uzblem uzbltreetab uzblcookied

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

TABSRC = $(wildcard src/uzbltab/*.cpp)
TABOBJ = $(TABSRC:.cpp=.o)
$(TABOBJ): $(wildcard $(dir)/*.h)

uzbltab: $(TABOBJ)
	$(CXX) -o $@ $(TABOBJ) $(LDFLAGS)

EMSRC = $(wildcard src/uzblem/*.cpp)
EMOBJ = $(EMSRC:.cpp=.o)
$(EMOBJ): $(wildcard $(dir)/*.h)

uzblem: $(EMOBJ)
	$(CXX) -o $@ $(EMOBJ)

TREESRC = $(wildcard src/uzbltreetab/*.cpp)
TREEOBJ = $(TREESRC:.cpp=.o)
$(TREEOBJ): $(wildcard $(dir)/*.h)

uzbltreetab: $(TREEOBJ)
	$(CXX) -o $@ $(TREEOBJ) $(LDFLAGS)

COOKIESRC = $(wildcard src/uzblcookied/*.cpp)
COOKIEOBJ = $(COOKIESRC:.cpp=.o)
$(COOKIEOBJ): $(wildcard $(dir)/*.h)

uzblcookied: $(COOKIEOBJ)
	$(CXX) -o $@ $(COOKIEOBJ)

clean:
	rm -f src/*/*.o uzbltab uzblem uzbltreetab uzblcookied

dist:
	make clean
	mkdir -p uzbltabem
	cp README uzbltabem
	cp Makefile uzbltabem
	cp ChangeLog uzbltabem
	cp uzbl-browser uzbltabem
	cp -R src uzbltabem

	tar zcvf uzbltabem-.tar.gz uzbltabem

	rm -r uzbltabem

