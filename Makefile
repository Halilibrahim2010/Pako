# Yapılandırma dosyasını içe aktar
-include .config

# Varsayılan değerler
INSTALL_DIR ?= $(HOME)/.local/pako
TARGET_ARCH ?= native
JOBS ?= 1

CXX = g++
# Mimariyi seç
ifeq ($(TARGET_ARCH),aarch64)
    CXX = aarch64-linux-gnu-g++
    CXXFLAGS = -std=c++17 -Wall -Wextra -O2
else
    CXX = g++
    CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -march=$(TARGET_ARCH)
endif
LDFLAGS = -lzstd -lstdc++fs -lssl -lcrypto

TARGET = pako
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all build clean install uninstall menuconfig pot update-po update-mo gettext

all: build

# Derleme sürecini başlat
build:
	$(MAKE) -j$(JOBS) $(TARGET)
	@echo ""
	@echo "Derleme pırıl pırıl tamamlandı."
	@echo "Kurmak için 'make install' komutunu kullan."

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Menü arayüzünü tetikler
menuconfig:
	@./menuconfig.sh

install:
	mkdir -p $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/locale
	cp $(TARGET) $(INSTALL_DIR)/bin/$(TARGET)
	cp -r locale/* $(INSTALL_DIR)/locale/
	chmod 755 $(INSTALL_DIR)/bin/$(TARGET)
	@echo "Pako başarıyla kuruldu! Yol: $(INSTALL_DIR)/bin/$(TARGET)"

uninstall:
	rm -rf $(INSTALL_DIR)
	@echo "Pako ve tüm dosyaları $(INSTALL_DIR) dizininden kazındı."

clean:
	rm -f $(OBJS) $(TARGET)

# --- Gettext Ayarları ---
LANGUAGES = tr fr de
LOCALE_DIR = locale

pot:
	mkdir -p $(LOCALE_DIR)
	xgettext --from-code=UTF-8 -k_ -d pako -o $(LOCALE_DIR)/pako.pot main.cpp

update-po:
	for lang in $(LANGUAGES); do \
		mkdir -p $(LOCALE_DIR)/$$lang/LC_MESSAGES; \
		msgmerge --update $(LOCALE_DIR)/$$lang/LC_MESSAGES/pako.po $(LOCALE_DIR)/pako.pot; \
	done

update-mo:
	for lang in $(LANGUAGES); do \
		mkdir -p $(LOCALE_DIR)/$$lang/LC_MESSAGES; \
		msgfmt $(LOCALE_DIR)/$$lang/LC_MESSAGES/pako.po -o $(LOCALE_DIR)/$$lang/LC_MESSAGES/pako.mo; \
	done

gettext: pot update-po update-mo