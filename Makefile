CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS =  -lzstd -lstdc++fs

TARGET = pako
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

# Sistemdeki CPU çekirdek sayısını al
NPROC = $(shell nproc)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Paralel derleme için
.NOTPARALLEL:
.PHONY: parallel
parallel:
	make -j$(NPROC)

# Gettext hedefleri
.PHONY: pot update-po update-mo

pot:
	mkdir -p locale
	xgettext --from-code=UTF-8 -k_ -d pako -o locale/pako.pot main.cpp

update-po:
	msgmerge --update locale/tr/LC_MESSAGES/pako.po locale/pako.pot

update-mo:
	mkdir -p locale/tr/LC_MESSAGES
	msgfmt locale/tr/LC_MESSAGES/pako.po -o locale/tr/LC_MESSAGES/pako.mo

gettext: pot update-po update-mo
