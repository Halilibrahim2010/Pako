#!/bin/bash

echo "=== Pako Yapılandırma Menüsü ==="
read -p "Kurulum Dizini (Varsayılan olarak tam olarak $HOME/.local/pako kullanılacak): " DIR
DIR=${DIR:-$HOME/.local/pako}

echo "Hedef Mimari:"
echo "1) native (sisteminizin mimarisi)"
echo "2) x86_64"
echo "3) aarch64"
read -p "Seçimin: " ARCH_SEL
case $ARCH_SEL in
    2) ARCH="x86_64" ;;
    3) ARCH="aarch64" ;;
    *) ARCH="native" ;;
esac

MAX_CORES=$(nproc)
echo "Derleme için Çekirdek Kullanımı:"
echo "1) Tek çekirdek"
echo "2) Tüm çekirdekler (Tam olarak $MAX_CORES çekirdek)"
echo "3) Özel miktar belirle"
read -p "Seçimin: " CORE_SEL
case $CORE_SEL in
    2) CORES=$MAX_CORES ;;
    3) read -p "Kullanılacak çekirdek sayısını gir: " CORES ;;
    *) CORES=1 ;;
esac

echo "INSTALL_DIR=$DIR" > .config
echo "TARGET_ARCH=$ARCH" >> .config
echo "JOBS=$CORES" >> .config

echo "Harika! Ayarlar .config dosyasına kaydedildi."