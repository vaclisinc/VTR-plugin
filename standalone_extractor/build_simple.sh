#!/bin/bash

echo "Building VTR Feature Extractor (Simple)..."

# Create virtual environment
if [ ! -d "venv_simple" ]; then
    python3 -m venv venv_simple
fi

source venv_simple/bin/activate

# Install everything needed
pip install --upgrade pip
pip install librosa pyinstaller

# Clean
rm -rf build dist

# Build with auto-detection
pyinstaller --onefile --name vtr-feature-extractor feature_extractor.py

# Test
if [ -f "dist/vtr-feature-extractor" ]; then
    echo "Testing..."
    ./dist/vtr-feature-extractor --test
fi

deactivate