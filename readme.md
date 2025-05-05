# Pako - Sürdürülebilir, Minimalist ve Basit Bir Paket Yöneticisi

> ⚠️ **Uyarı:** Bu proje şu anda geliştirme aşamasındadır ve kararlı bir sürüm değildir. Sadece test ve deneme amaçlı paylaşılmıştır. Kodlar zamanla değişebilir ve beklenmedik hatalar olabilir. İlerleyen zamanlarda tam sürüm yayınlanacaktır.

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
./pako indir/install -y <package.pako>  # Paket indir ve kur
./pako tasarla/create <directory>      # Paket oluştur
./pako bilgi/info <package.pako>     # Paket bilgilerini göster
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
- İsmail EFE

Eğer bu projeyi beğendiyseniz ve daha fazla gelişmesini isterseniz, katkıda bulunmaktan çekinmeyin!