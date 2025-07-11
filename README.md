1. Configure the project

  cd /Users/vaclis./Documents/project/VTR-plugin
  cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

  2. Build the plugin

  cd build
  ninja

  Or to build specific formats:
  # Build VST3 only
  ninja VTR-smartEQ_VST3

  # Build AU only (macOS)
  ninja VTR-smartEQ_AU

  3. Install locations

  The build automatically installs plugins to:
  - VST3: ~/Library/Audio/Plug-Ins/VST3/VTR-smartEQ.vst3
  - AU: ~/Library/Audio/Plug-Ins/Components/VTR-smartEQ.component

  Build Options

  Debug build (with AddressSanitizer)

  cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

  Clean build

  cd build
  ninja clean

  Validate the plugin

  # Test VST3
  pluginval --strictness-level 2 --validate-in-process
  ~/Library/Audio/Plug-Ins/VST3/VTR-smartEQ.vst3

  # Test AU
  auval -v aufx VsEQ Vacl

  The plugin should now be available in your DAW!