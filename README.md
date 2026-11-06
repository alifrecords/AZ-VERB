# AZ VERB — Smart Vocal Reverb
**Azimxodjayev Music** · VST3 / AU / AAX / Standalone · Windows · macOS (Intel + Apple Silicon) · Linux

Daftar DAW'idagi D-Verb asosida qayta yozilgan, mustaqil reverb plagin. Asosiy maqsad vokal uchun zamonaviy, toza, mixda ko'milib ketmaydigan dum.

---

## Tovush xarakteri

| Algoritm | Xarakter | Qanday erishilgan |
|---|---|---|
| **Hall** | Yumshoq qumdek, silliq, metall jiringlashsiz | 8 liniyali FDN, Householder matritsa, liniya ichidagi diffuziya, sekin tasodifiy modulyatsiya (SAND) |
| **Plate** | Kristal, zich, yorqin, sibilyantsiz | Yuqori diffuziya, kam modulyatsiya (pitch toza), kirishda avtomatik de-esser, dumda "air" shelf |
| Church | Katta, sekin gullaydigan fazo | Uzun liniyalar, keng erta aks-sadolar |
| Room I / II | Yaqin, tabiiy xona | Kuchli erta aks-sadolar, qisqa dum |
| Ambient | Yumshoq parda, erta aks-sadolarsiz | Maksimal diffuziya va SAND |
| NonLin | 80-yillar / trap gated | 24 tapli zich "darvoza" |

## AZ fishkalari (o'ziga xos jihatlar)

| Ruchka | Nima qiladi |
|---|---|
| **SAND** | Dumni "qum"dek silliq qiladi: sekin tasodifiy modulyatsiya rezonanslarni yoyadi, metall rang yo'qoladi |
| **CRYSTAL** | Dumga havo va jilo qo'shadi (8.5 kHz shelf + yumshoq garmonika). Kirishdagi de-esser tufayli qattiq chiqmaydi |
| **FOCUS** | Anti-masking: vokal kuylayotganda dumning 300 Hz loyqaligi va 1.6 kHz "so'z zonasi" dinamik bo'shatiladi, ovoz oldinda qoladi |
| **DE-ESS** | Faqat reverbga kirayotgan "s/sh" tovushlarni pasaytiradi (quruq vokalga tegmaydi). Chastotasini Smart Vocal o'zi topadi |
| **DUCK** | Kuylaganda dum −8 dB pasayadi, iboralar orasida "gullaydi" |
| **SYNC** | Pre-delay host tempiga bog'lanadi (1/64 … 1/4, triplet) |
| **FREEZE** | Cheksiz dum: pad va o'tishlar uchun |
| Fazo vizuali | Sichqoncha bilan surilsa, gorizontal SIZE, vertikal DECAY o'zgaradi |

## SMART VOCAL

1. **SMART VOCAL** qutisini bosing va vokal trekni ijro eting (≈8 soniya).
2. Plagin quyidagilarni aniqlaydi:
   - ovoz turi (bas, bariton, tenor, alt, soprano) va F0 chastotasi;
   - tembr (iliq/to'q, muvozanatli, yorqin, burunli);
   - sibilyant chastotasi va kuchi;
   - temp (host'dan, bo'lmasa audiodan) va uslub.
3. 7 ta zamonaviy uslubdan eng moslari tartib bilan, moslik foizi bilan chiqadi. Har biri aynan shu ovozga moslashtiriladi: decay tempoga, de-ess sibilyant chastotasiga, low cut ovoz balandligiga, damp va crystal tembrga qarab.
4. Tugmalar: **QO'LLASH** tavsiyani qo'yadi, **A/B** avvalgi sozlama bilan solishtiradi, **KEYINGI** keyingi variantni ko'rsatadi.

Tahlil butunlay plagin ichida bajariladi: internet va bulut kerak emas.

## O'lchangan natijalar (v1.1)

| Sinov | Natija |
|---|---|
| RT60 aniqligi | ±6% |
| Chastota bo'yicha RT (Hall, 2.5 s) | 125 Hz 2.37 · 500 Hz 2.55 · 2 kHz 2.57 · 4 kHz 2.43 · 8 kHz 2.07 s (tabiiy zal kabi) |
| Dumdagi loyqalik (150–500 Hz ulushi) | 5.5% |
| Dumdagi havo (6–16 kHz ulushi) | 8% |
| Stereo korrelyatsiya (IACC) | ≈0.0: vokalni to'liq o'rab oladi, monoda yo'qotish yo'q |
| Algoritmlar orasidagi balandlik farqi | ≤0.7 dB |
| DE-ESS | sibilyant −8…−12 dB, pastlarga 0 dB |
| CPU (48 kHz stereo) | bitta yadroning ≈1.2% |
| Smart Vocal (sintetik test) | F0 ±2%, sibilyant ±1%, temp ±1 BPM |
| pluginval | strictness 10, 3 xil seed, barcha testlar o'tdi |

## O'zgarishlar

**v1.1.0: ochiq va tabiiy tovush (D-Verb ruhida)**

| Ko'rsatkich | v1.0 | v1.1 |
|---|---|---|
| RT 8 kHz (2.5 s sozlamada) | 0.88 s | 2.07 s |
| Loyqalik ulushi | 11.9% | 5.5% |
| Aniqlik (2–6 kHz) | 28% | 47% |
| Havo (6–16 kHz) | 0.2% | 8% |
| Dum spektr markazi | 1.57 kHz | 2.86 kHz |
| Stereo korrelyatsiya | −0.18 (fazasi buzuq) | 0.0 |

- Lowpass so'nish o'rniga 3 polosali shelf so'nish: bass biroz qisqa, o'rtalar tekis, yuqorilar yumshoq so'nadi.
- Hermite interpolyatsiya: modulyatsiyada yuqori chastotalar yo'qolmaydi.
- Modulyatsiya kamaytirildi: chorus loyqaligi yo'q, pitch toza.
- Erta aks-sadolar kuchaytirildi (Hall, Church, Room): vokal atrofida aniq fazo seziladi.
- WIDTH standarti 100% (maksimal o'rab olish, mono-xavfsiz); presetlar yangilandi.
- Tuzatildi: SYNC, FREEZE va DUCK DAW loyihasini ochganda har doim to'g'ri tiklanadi.

---

## Yig'ish (build)

### Eng oson yo'l: GitHub Actions (kompyuterga hech narsa o'rnatish shart emas)
1. Bu papkani GitHub'ga yangi repo qilib yuklang (masalan `alifrecords/AZ-VERB`).
2. **Actions** bo'limida "AZ VERB build" avtomatik ishga tushadi (~15 daqiqa).
3. Tayyor fayllarni **Artifacts** bo'limidan yuklab oling:
   - `AZ-VERB-Windows`: `AZ-VERB-1.0.0-Windows-Setup.exe` o'rnatuvchisi, `.vst3`, `.aaxplugin`, `.exe`
   - `AZ-VERB-macOS`: `.vst3`, `.component` (AU), `.aaxplugin`, `.app`
4. `v1.0.0` teg qo'yilsa (`git tag v1.0.0 && git push --tags`), fayllar avtomatik **Release** sahifasiga chiqadi.

### O'z kompyuteringizda
- **Windows:** Visual Studio 2022 (C++), CMake.
  ```
  cmake -B build -A x64
  cmake --build build --config Release
  ```
- **macOS:** Xcode, CMake.
  ```
  cmake -B build -G Xcode
  cmake --build build --config Release
  ```
JUCE 8 avtomatik yuklab olinadi.

### Pro Tools (AAX) haqida
AAX fayli yig'iladi, lekin oddiy (retail) Pro Tools uni faqat **PACE/iLok imzosi** bilan yuklaydi. Buning uchun Avid Developer va PACE Eden hisoblarini bepul ochib, `wraptool` bilan imzolash kerak. Imzosiz versiyani faqat Pro Tools Developer build'i ochadi.

## Fayllar tuzilishi
```
Source/dsp/AZVerbEngine.h   — DSP yadrosi (JUCE'siz, toza C++)
Source/VocalAnalyzer.h      — Smart Vocal: tahlil + tavsiyalar
Source/Presets.h            — 16 zavod preseti (Vocal / Instr / FX)
Source/Params.h             — parametrlar (barchasi avtomatizatsiya qilinadi)
Source/PluginProcessor.*    — audio, holat, A/B, undo
Source/PluginEditor.*       — interfeys (o'lchami o'zgaradi, Retina/4K'da tiniq)
tests/                      — DSP va Smart Vocal testlari
installer/                  — Windows o'rnatuvchisi, macOS yo'riqnomasi
```
