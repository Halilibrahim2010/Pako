# Install hedefi
.PHONY: install
install:
	mkdir -p $(HOME)/.local/pako/bin
	mkdir -p $(HOME)/.local/pako/locale
	cp pako $(HOME)/.local/pako/bin/pako
	cp -r locale/* $(HOME)/.local/pako/locale/
	chmod 755 $(HOME)/.local/pako/bin/pako
	echo "Pako başarıyla kuruldu! Çalıştırmak için: $(HOME)/.local/pako/bin/pako"
	
# Remove hedefi
.PHONY: remove
remove:
	rm -rf $(HOME)/.local/pako
	echo "Pako ve tüm dosyaları kaldırıldı."
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS =  -lzstd -lstdc++fs -lssl -lcrypto

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
	make all -j$(NPROC)

# Gettext hedefleri
.PHONY: pot update-po update-mo

# Diller
LANGUAGES = tr fr de

# Klasörler
LOCALE_DIR = locale

# .pot dosyasını oluşturma
pot:
	mkdir -p $(LOCALE_DIR)
	xgettext --from-code=UTF-8 -k_ -d pako -o $(LOCALE_DIR)/pako.pot main.cpp

# PO dosyasını güncelleme
update-po:
	# Her dil için PO dosyasını günceller
	for lang in $(LANGUAGES); do \
		mkdir -p $(LOCALE_DIR)/$$lang/LC_MESSAGES; \
		msgmerge --update $(LOCALE_DIR)/$$lang/LC_MESSAGES/pako.po $(LOCALE_DIR)/pako.pot; \
	done

# MO dosyasını oluşturma
update-mo:
	# Her dil için MO dosyasını oluşturur
	for lang in $(LANGUAGES); do \
		mkdir -p $(LOCALE_DIR)/$$lang/LC_MESSAGES; \
		msgfmt $(LOCALE_DIR)/$$lang/LC_MESSAGES/pako.po -o $(LOCALE_DIR)/$$lang/LC_MESSAGES/pako.mo; \
	done

gettext: pot update-po update-mo

build: gettext parallel
	@echo ""
	@echo "Tüm hedefler başarıyla tamamlandı."
	@echo "Çalıştırmak için ./pako komutunu kullanın."
	@echo "Dil ayarları için locale dizinine bakın."
	@echo "Dil ayarlarını güncellemek için make gettext komutunu kullanın."
	@echo "Paralel derleme için make parallel komutunu kullanın."
	@echo "Temizlemek için make clean komutunu kullanın."
