#!/bin/bash

# --- Configuration ---
COMMAND_NAME="jack"
BINARY_NAME="NAND2TETRIS"
BINARY_SOURCE="./cmake-build-debug/$BINARY_NAME"
INSTALL_DIR="$HOME/.jack_toolchain"
OS_SOURCE_DIR="./JackOS"
TOOLS_SOURCE_DIR="./tools"    # <--- NEW: Location of python scripts

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE} 📦 Installing 'jack' Toolchain & DevTools ${NC}"
echo -e "${BLUE}========================================${NC}"

# 1. Locate Binary
echo -e "${GREEN}[1/5] Locating Binary...${NC}"
if [ ! -f "$BINARY_SOURCE" ]; then
    echo -e "${RED}Error: Could not find binary at $BINARY_SOURCE${NC}"
    exit 1
fi

# 2. Setup Directory
echo -e "${GREEN}[2/5] Setting up Install Directory...${NC}"
if [ -d "$INSTALL_DIR" ]; then rm -rf "$INSTALL_DIR"; fi
mkdir -p "$INSTALL_DIR/bin"
mkdir -p "$INSTALL_DIR/os"
mkdir -p "$INSTALL_DIR/tools" # <--- NEW: Folder for python scripts

# 3. Copy Compiler & OS
echo -e "${GREEN}[3/5] Copying Core Files...${NC}"
cp "$BINARY_SOURCE" "$INSTALL_DIR/bin/$BINARY_NAME"
chmod +x "$INSTALL_DIR/bin/$BINARY_NAME"

if [ -d "$OS_SOURCE_DIR" ]; then
    cp "$OS_SOURCE_DIR"/*.jack "$INSTALL_DIR/os/"
else
    echo -e "${RED}Error: Could not find '$OS_SOURCE_DIR'.${NC}"
    exit 1
fi

# 4. Copy Visualization Tools
echo -e "${GREEN}[4/5] Copying Developer Tools...${NC}"
if [ -d "$TOOLS_SOURCE_DIR" ]; then
    # Copy python scripts and D3.js library
    cp "$TOOLS_SOURCE_DIR"/*.py "$INSTALL_DIR/tools/"
    cp "$TOOLS_SOURCE_DIR"/*.js "$INSTALL_DIR/tools/" 2>/dev/null || true
    echo "   -> Python visualizers installed."
else
    echo "   -> Warning: 'tools/' folder not found. Visualizers skipped."
fi

# 5. Create Wrapper
echo -e "${GREEN}[5/5] Creating '$COMMAND_NAME' command...${NC}"

cat <<EOF > "$INSTALL_DIR/wrapper.sh"
#!/bin/bash
COMPILER="$INSTALL_DIR/bin/$BINARY_NAME"
"\$COMPILER" "\$@"
EOF

chmod +x "$INSTALL_DIR/wrapper.sh"

# 6. Link to Global Path
if [ -L "/usr/local/bin/$COMMAND_NAME" ]; then
    sudo rm "/usr/local/bin/$COMMAND_NAME"
fi
sudo ln -s "$INSTALL_DIR/wrapper.sh" "/usr/local/bin/$COMMAND_NAME"

echo -e "\n${BLUE}========================================${NC}"
echo -e "${GREEN} ✅ Installation Complete! ${NC}"
echo -e "${BLUE}========================================${NC}"