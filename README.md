# Pako - Sürdürülebilir, Minimalist ve Basit Bir Paket Yöneticisi

**Pako**, Linux üzerinde kullanılabilecek minimalist bir paket yöneticisidir. Bu araç, kullanıcıların `.pako` formatındaki paketleri kurmasına, oluşturmasına ve bilgi almasına olanak tanır. Ayrıca, dil desteği sunarak farklı dillerde uygulamanın kullanılmasını sağlar. Başlıca olarak Dolunay projesi için geliştirilmiştir.

## Proje Amacı
Pako, sürdürülebilir bir yazılım ekosistemi oluşturmayı hedefler. Hem geliştiriciler hem de kullanıcılar için basit, etkili ve çevre dostu bir paket yöneticisi olmayı amaçlar. Proje, hem temel işlevsellik hem de sürdürülebilirlik açısından optimize edilmiştir.

## Özellikler
- **Minimalist Yapı**: Pako, karmaşık olmayan, sadece gerekli fonksiyonları içeren bir tasarıma sahiptir.
- **Dil Destekleri**: `gettext` kullanılarak farklı dillere çeviri desteği eklenmiştir. Türkçe ve İngilizce gibi çeşitli dillere kolayca uyarlanabilir.
- **Kolay Paket Kurulum**: `.pako` paketlerini indirip, açıp ve gerekli dizinlere dosyaları kopyalayarak sisteminize kurulum yapar.
- **Paket Oluşturma**: Kendi `.pako` paketlerinizi oluşturabilirsiniz.
- **Paket Bilgisi Görüntüleme**: Paketlerin metadata bilgilerini gösterir.
- **Bağımlılık Yönetimi**: Paketlerin bağımlılıklarını kontrol eder ve listeler.

## Kurulum
### Gereksinimler
- C++17 veya daha yeni bir derleyici (GCC veya Clang)
- `nlohmann/json.hpp` JSON işleme kütüphanesi
- `gettext` ve `libintl.h` dil desteği
- `tar` ve `zstd` komutları (paket oluşturma ve sıkıştırma için)

### Derleme
```bash
make parallel
```

## Çalıştırma 
- Pako komut satırında şu şekilde çalıştırılabilir:
```bash
./pako indir -y <package.pako>  # Paket indir ve kur
./pako tasarla <directory>      # Paket oluştur
./pako bilgi <package.pako>     # Paket bilgilerini göster
```
### Örnek Kullanım
- Paket Kurulumu:  ```./pako install -y mypackage.pako```
- Paket Oluşturma(geliştiriciler için): ```./pako create /path/to/package-directory```
- Paket ile ilgili bilgi almak için: ```./pako info mypackage.pako```

## Güvenlik
- Pako, paketlerin güvenliğini sağlamak için bazı kontroller yapmaktadır. Örneğin, paket metadata'sındaki code_name değeri yalnızca alfanumerik karakterler ve alt çizgi içerebilir.

Eğer bu projeyi beğendiyseniz ve daha fazla gelişmesini isterseniz, katkıda bulunmaktan çekinmeyin!
