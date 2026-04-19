#!/usr/bin/env bash

set -e

echo "[zapret-tui] Installing dependencies..."

# Detect package manager
if command -v apt >/dev/null 2>&1; then
    echo "[*] Detected: APT (Debian/Ubuntu)"
    sudo apt update
    sudo apt install -y libconfuse1 libconfuse-dev pkg-config

elif command -v pacman >/dev/null 2>&1; then
    echo "[*] Detected: Pacman (Arch/Manjaro)"
    sudo pacman -Syy --noconfirm
    sudo pacman -S --noconfirm confuse pkgconf


elif command -v dnf >/dev/null 2>&1; then
    echo "[*] Detected: DNF (Fedora)"
    sudo dnf install -y libconfuse libconfuse-devel pkgconf-pkg-config

elif command -v zypper >/dev/null 2>&1; then
    echo "[*] Detected: Zypper (openSUSE)"
    sudo zypper install -y libconfuse-devel pkgconf

elif command -v apk >/dev/null 2>&1; then
    echo "[*] Detected: APK (Alpine)"
    sudo apk add confuse-dev pkgconf

else
    echo "[!] Unsupported package manager"
    echo "Please install libconfuse manually:"
    echo "  - Debian/Ubuntu: libconfuse1 libconfuse-dev"
    echo "  - Arch: confuse"
    echo "  - Fedora: libconfuse libconfuse-devel"
    exit 1
fi

echo ""
echo "[✓] Checking installation..."

if pkg-config --exists libconfuse; then
    echo "[✓] libconfuse is installed correctly"
else
    echo "[!] libconfuse not found in pkg-config"
    echo "Installation may have failed"
    exit 1
fi

echo ""
echo "[✓] All dependencies installed successfully"

