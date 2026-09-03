PREFIX   ?= /usr/local
BUILD    ?= build
JOBS     ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

.PHONY: all configure build install uninstall clean

all: build

configure:
	cmake -S . -B $(BUILD) -DCMAKE_BUILD_TYPE=Release \
	      -DCMAKE_INSTALL_PREFIX=$(PREFIX)

build: configure
	cmake --build $(BUILD) -j$(JOBS)

install: build
	cmake --install $(BUILD)
	@if [ "$$(uname)" = "Linux" ] && command -v update-desktop-database >/dev/null 2>&1; then \
		echo "Updating desktop database..."; \
		update-desktop-database $(PREFIX)/share/applications 2>/dev/null || true; \
	fi
	@if [ "$$(uname)" = "Linux" ] && command -v gtk-update-icon-cache >/dev/null 2>&1; then \
		echo "Updating icon cache..."; \
		gtk-update-icon-cache -f -t $(PREFIX)/share/icons/hicolor 2>/dev/null || true; \
	fi
	@echo ""
	@echo "✅ Flying Speed installed to $(PREFIX)"
	@echo "   Binary:  $(PREFIX)/bin/flying-speed"
	@echo "   Assets:  $(PREFIX)/share/flying-speed/assets"
	@if [ "$$(uname)" = "Linux" ]; then \
		echo "   Desktop: $(PREFIX)/share/applications/flying-speed.desktop"; \
		echo "   Icon:    $(PREFIX)/share/icons/hicolor/192x192/apps/flying-speed.png"; \
	fi

uninstall:
	rm -f  $(PREFIX)/bin/flying-speed
	rm -rf $(PREFIX)/share/flying-speed
	rm -f  $(PREFIX)/share/applications/flying-speed.desktop
	rm -f  $(PREFIX)/share/icons/hicolor/192x192/apps/flying-speed.png
	rm -f  $(PREFIX)/share/pixmaps/flying-speed.png
	@if command -v update-desktop-database >/dev/null 2>&1; then \
		update-desktop-database $(PREFIX)/share/applications 2>/dev/null || true; \
	fi
	@echo "Flying Speed uninstalled from $(PREFIX)"

clean:
	rm -rf $(BUILD)
