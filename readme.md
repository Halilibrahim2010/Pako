# Pako - Sürdürülebilir, Minimalist ve Basit Bir Paket Yöneticisi

**Pako**, Linux üzerinde kullanılabilecek minimalist bir paket yöneticisidir. Bu araç, kullanıcıların `.pako` formatındaki paketleri kurmasına, oluşturmasına ve bilgi almasına olanak tanır. Ayrıca, dil desteği sunarak farklı dillerde uygulamanın kullanılmasını sağlar. Başlıca olarak LyOS için geliştirilmiştir.

## Özellikler

- **Minimalist Tasarım**: Sadece gerekli fonksiyonları içerir, gereksiz karmaşıklık yok.
- **Dil Desteği**: `gettext` ile Türkçe, İngilizce ve diğer dillere kolayca uyarlanabilir.
- **Hash Tabanlı Paket Yönetimi**: Paketler çakışmaz, versiyonlar güvenli şekilde yönetilir; eski ve yeni sürümler birbirine karışmaz.
- **Kullanıcı Bazlı Kurulum**: Paketler `~/.local/pako` altında saklanır, root yetkisi gerekmez.
- **Sandbox Ortamı**: Her paket kendi dizininde izole edilir, sürümler birbirine müdahale etmez.
- **Kolay Paket Kurulumu**: `.pako` dosyalarını indirip açarak hızlıca kurulabilir.
- **Paket Oluşturma**: Kendi `.pako` paketlerinizi oluşturabilir ve paylaşabilirsiniz.
- **Paket Bilgisi Görüntüleme**: Metadata bilgilerini kolayca görüntüler.
- **Bağımlılık Yönetimi**: Paketlerin bağımlılıklarını kontrol eder ve listeler.
- **Esnek PATH Yönetimi**: `~/.local/pako/bin` eklenerek terminalden direkt çalıştırılabilir.

## Kurulum
## Gereksinimler

- C++17 veya daha yeni bir derleyici (GCC veya Clang)
- [`nlohmann/json.hpp`](https://github.com/nlohmann/json) JSON işleme kütüphanesi
- `gettext` ve `libintl.h` dil desteği
- OpenSSL (`libssl-dev`) – SHA256 hash işlemleri için
- `tar` ve `zstd` komutları (paket oluşturma ve sıkıştırma için)
- Standart C++ kütüphaneleri:
  - `<filesystem>`
  - `<fstream>`
  - `<iostream>`
  - `<sstream>`
  - `<iomanip>`
  - `<ctime>`
- `unistd.h` ve `<locale.h>` – sistem ve lokalizasyon işlemleri için

### Derleme
```bash
make parallel
```
### Otomatik Yükleme
```bash
make install
```
### Sistemden Kaldırma
```
make remove
```

## Çalıştırma 
- Pako komut satırında şu şekilde çalıştırılabilir:
```bash
pako -[version] <package name>         # İndirilen paketi çalıştırır
pako indir/install -y <package.pako>   # Paket indir ve kur
pako tasarla/create <directory>        # Paket oluştur
pako bilgi/info <package.pako> [-json] # Paket bilgilerini göster
pako liste/list                        # Tüm indirilen paketleri listeler
pako sil/uninstall <package name>      # İndirilen paketi siler
pako guncelle/update <package.pako>    # Paket günceller
```

## Güvenlik
- Pako, paketlerin güvenliğini sağlamak için bazı kontroller yapmaktadır. Örneğin, paket metadata'sındaki code_name değeri yalnızca alfanumerik karakterler ve alt çizgi içerebilir.

## Dil ve Yerelleştirme
Bu proje şu anda aşağıdaki dillerde kullanılabilir:

- 🇹🇷 Türkçe

- 🇬🇧 İngilizce

- 🇩🇪 Almanca

- 🇫🇷 Fransızca

## Geliştiriciler
Bu proje birkaç kişinin emeğiyle ortaya çıktı. Kodlayanlar, fikir verenler ve destek olan herkesin katkısı değerli. Aşağıda ismi geçenler doğrudan geliştirme sürecine katkıda bulundu:

- Halil İbrahim AYKOL (Ana geliştirici)
- İsmail EFE (Dil çevirileri)

Eğer bu projeyi beğendiyseniz ve daha fazla gelişmesini isterseniz, katkıda bulunmaktan çekinmeyin!