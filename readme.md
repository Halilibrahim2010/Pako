# Pako - Sürdürülebilir, Minimalist ve Basit Paket Yöneticisi

**Pako**, Linux ve Linux-tabanlı sistemlerde çalışan, kullanıcı bazlı, hash tabanlı ve minimalist bir paket yöneticisidir. `.pako` formatındaki paketleri kurma, oluşturma, güncelleme ve metadata okuma işlevlerini sağlar. Atomic kurulum, sandbox ortam ve ram tabanlı extraction ile güvenliği önceliklendirir.

---

## Mimari ve Tasarım

- **Kullanıcı Bazlı Kurulum**: Paketler `~/.local/pako` altında saklanır; root yetkisi gerekmez.  
- **Sandbox ve İzolasyon**: Her paket kendi dizininde izole edilir. `bin`, `lib`, `share` alt dizinleri ile sürümler birbirine müdahale etmez.  
- **Hash Tabanlı Yönetim**: Paketler SHA256 hash ile isimlendirilir; sürümler ve dosyalar çakışmaz.  
- **Atomic Kurulum**: Dosya kopyalamaları tmp file → rename ile yapılır, overwrite ve yarış durumu önlenir.  
- **Ram Tabanlı Extraction**: Paketler SSD’ye yazılmadan önce RAM’de açılır, kurulum sonrası RAM temizlenir.  
- **Cross-Platform**: x86_64 ve ARM64 sistemlerde çalışır.  
- **Bağımlılık Kontrolü**: Paketler metadata üzerinden bağımlılıklarını doğrular. Eksik bağımlılıklar kullanıcıya bildirilir.
- **Minimalist Tasarım**: Sadece gerekli fonksiyonları içerir, gereksiz karmaşıklık yok.
- **Dil Desteği**: `gettext` ile Türkçe, İngilizce ve diğer dillere kolayca uyarlanabilir.
- **Kolay Paket Kurulumu**: `.pako` dosyalarını indirip açarak hızlıca kurulabilir.
- **Paket Oluşturma**: Kendi `.pako` paketlerinizi oluşturabilir ve paylaşabilirsiniz.
- **Paket Bilgisi Görüntüleme**: Metadata bilgilerini kolayca görüntüler.
- **Esnek PATH Yönetimi**: `~/.local/pako/bin` eklenerek terminalden direkt çalıştırılabilir.
---

## Paket Formatı

- `.pako` dosyaları `tar.zst` sıkıştırma ile oluşturulur.  
- Metadata (`metadata.json`) içerir:
  - name: Paket adı  
  - code_name: Paket kod adı (sadece alfanumerik + `_`)  
  - version: Paket sürümü  
  - description: Paket açıklaması  
  - dependencies: Bağımlılıklar listesi  
  - binaries: Çalıştırılabilir dosyalar `{name: path}`  
  - libraries: Kütüphaneler `{name: path}`  
  - mainbinary: Ana çalıştırılabilir dosya  

---

## Güvenlik Önlemleri

1. **Yol Manipülasyonu Koruması**: Paketler kendi dizinini aşamaz (`../` veya absolute path engellenir).  
2. **Atomic Dosya Yazma**: `tmpFile → rename` ile overwrite ve race condition önlenir.  
3. **Dosya İzinleri**:
   - Binaries: owner execute + owner read (`r-x------`), group/other read  
   - Libraries & metadata: owner/group/other read  
4. **Ram Temizliği**: Paket extraction sonrası RAM’de geçici veriler silinir.  
5. **Metadata Doğrulama**: `code_name` alfanumerik ve `_` ile sınırlı.  

---

## Gereksinimler

- C++17 veya üstü (GCC veya Clang)  
- [`nlohmann/json.hpp`](https://github.com/nlohmann/json)  
- OpenSSL (`libssl-dev`) – SHA256 hash  
- `gettext` ve `libintl.h` – yerelleştirme  
- `tar` ve `zstd` – paket oluşturma ve açma  
- `<filesystem>`, `<fstream>`, `<iostream>`, `<sstream>`, `<iomanip>`, `<ctime>`  
- `<unistd.h>`, `<locale.h>` – sistem ve lokalizasyon  

---

## Kurulum
```bash
### Temel Ayarlar
chmod +x menuconfig.sh
make menuconfig

### Derleme
make build

### Yükleme
make install

### Kaldırma
bash
make remove
```
---

## Kullanım
```bash
### Paket Çalıştırma
pako -[version] <package name>  
Varsa `<mainbinary>` çalıştırılır, alt binary opsiyonel.

### Paket Kurulum
pako indir/install -y <package.pako>

### Paket Oluşturma
pako tasarla/create <directory>  
- `metadata.json` şablon oluşturulur veya mevcut metadata kullanılır  
- `libarchive + zstd` ile `.pako` dosyası hazırlanır

### Paket Bilgisi
pako bilgi/info <package.pako> [-json]

### Paket Listesi
pako liste/list

### Paket Kaldırma
pako sil/uninstall <package name>

### Paket Güncelleme
pako guncelle/update <package.pako>

### Versiyon kontrolü
pako --version
### Veya
pako --only-version
```
---

## Versiyon Kontrolü

- Pako kendi sürümünü takip eder.  
- `pako --version` ile sürüm öğrenilebilir.  
- Online kontrol: GitHub/GitLab Release API’den en son sürüm çekilebilir.  

---

## Geliştirici Notları

- Atomic ve ram tabanlı kurulumlar için `tmpDir` ve `fs::rename` kullanıldı.  
- Hash tabanlı yönetim `EVP_SHA256` kullanılarak yapıldı.  
- Paket bağımlılıkları metadata ile kontrol ediliyor; eksik olanlar uyarı veriyor.  
- Sistem path yönetimi ve XDG_DATA_DIRS uyumluluğu sağlandı.  
- Cross-platform uyumluluk: x86_64 ve ARM64 test edildi.  

---

## Dil ve Yerelleştirme

- 🇹🇷 Türkçe  
- 🇬🇧 İngilizce  
- 🇩🇪 Almanca  
- 🇫🇷 Fransızca  

---

## Geliştiriciler

- Halil İbrahim AYKOL – Ana geliştirici  
- İsmail EFE – Dil çevirileri  

Katkılar GitHub üzerinden PR ile kabul edilir.  

---

## Notlar

- Sandbox ve atomic kurulum sayesinde kullanıcı veri güvenliği ve sürüm izolasyonu sağlanır.  
- Paketler sadece owner tarafından execute edilebilir, group/other read ile sınırlıdır.  
- `.pako` formatı sıkıştırılmış ve ram üzerinde açılır, SSD’ye sadece final dosya yazılır.  
