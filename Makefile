#
# Copyright (c) 2025-2026 Murilo Ijanc' <murilo@ijanc.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

BIN      := hmf
SRC      := hmf.cpp
MAN      := hmf.1
MODEL    := yunet.onnx
MODEL_URL := https://github.com/opencv/opencv_zoo/raw/main/models/face_detection_yunet/face_detection_yunet_2023mar.onnx

PREFIX   ?= /usr/local
BINDIR   := $(DESTDIR)$(PREFIX)/bin
MANDIR   := $(DESTDIR)$(PREFIX)/share/man/man1
MODELDIR := $(DESTDIR)$(PREFIX)/share/hmf

CXX      ?= g++
CXXFLAGS ?= -O2 -Wall -Wextra -std=c++11
PKG_CFLAGS := $(shell pkg-config --cflags opencv4)
PKG_LIBS   := -lopencv_core -lopencv_imgproc -lopencv_imgcodecs \
              -lopencv_objdetect -lopencv_dnn

.PHONY: all build clean install uninstall fetch-model

all: build

build: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) $(PKG_CFLAGS) $< -o $@ $(PKG_LIBS)

fetch-model: $(MODEL)

$(MODEL):
	curl -fL -o $@ $(MODEL_URL)

clean:
	rm -f $(BIN)

install: $(BIN) $(MODEL)
	install -Dm755 $(BIN) $(BINDIR)/$(BIN)
	install -Dm644 $(MAN) $(MANDIR)/$(MAN)
	install -Dm644 $(MODEL) $(MODELDIR)/$(MODEL)

uninstall:
	rm -f $(BINDIR)/$(BIN)
	rm -f $(MANDIR)/$(MAN)
	rm -f $(MODELDIR)/$(MODEL)
