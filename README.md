# Opalys — Motor Multiefectos VST3 (JUCE 8 / C++20)
Inspirado en la arquitectura y estética de Opalys de Midilatino.

## Estructura del Proyecto
- `.github/workflows/build.yml`: Compilación automática CI/CD en GitHub Actions para Windows (MSVC) y macOS.
- `CMakeLists.txt`: Configuración CMake con JUCE 8.0.4 FetchContent y optimizaciones SIMD C++20.
- `Source/PluginProcessor.*`: Procesador de audio y APVTS thread-safe con ruteo de 10 efectos.
- `Source/PluginEditor.*`: Interfaz gráfica con visualizador de plasma pastel y rack inferior.
- `Source/OpalysLookAndFeel.h`: LookAndFeel neumórfico moderno en platino y obsidiana.
- `Source/DSP/`: 10 motores DSP independientes (Saturación, Cinta, Sparkles, Delay, Reverb FDN, Wider, Enhancer, Pitch OLA, Breaker, Ricochet).

---

## Opción 1: Compilar automáticamente con GitHub Actions (Recomendado)
1. Crea un nuevo repositorio en GitHub (público o privado).
2. Sube estos archivos descomprimidos a la raíz del repositorio (`git add . && git commit -m "Init Opalys" && git push`).
3. Ve a la pestaña **Actions** en tu repositorio de GitHub.
4. El workflow se ejecutará automáticamente (o puedes lanzarlo manualmente con *Run workflow*).
5. Al finalizar (2-3 minutos), descarga el artifact **Opalys-VST3-Windows-x64** o **Opalys-VST3-macOS-Universal**.
6. Copia el archivo `Opalys.vst3` en:
   - **Windows:** `C:\Program Files\Common Files\VST3\`
   - **macOS:** `/Library/Audio/Plug-Ins/VST3/`
7. Abre **FL Studio** -> **Options** -> **Manage plugins** -> **Find installed plugins**.

---

## Opción 2: Compilar localmente con CMake y Visual Studio (Windows)
Requisitos: CMake >= 3.24 y Visual Studio 2022 (con la carga de trabajo "Desarrollo para el escritorio con C++").

```bash
# 1. Configurar el proyecto
cmake -B build -G "Visual Studio 17 2022" -A x64

# 2. Compilar en Release
cmake --build build --config Release --parallel

# 3. El plugin compilado estará en:
# build/Opalys_artefacts/Release/VST3/Opalys.vst3
```

## Opción 3: Compilar en macOS (Xcode / Ninja)
```bash
cmake -B build -G Xcode -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```
