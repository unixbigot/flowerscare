BOARD ?= esp32:esp32:esp32
DEVICE ?= spike01
PORT ?= tty.Repleo-CH341-00001114
LIBDIR ?= $(HOME)/Arduino/libraries
SDKVERSION ?= $(shell ls -1 $(HOME)/.arduino15/packages/esp8266/hardware/esp8266/ | tail -1)
OTAPROG ?= $(HOME)/.arduino15/packages/esp8266/hardware/esp8266/$(SDKVERSION)/tools/espota.py
OTAPASS ?= pamela
PROGRAM ?= flowerscare
CCFLAGS ?=
#CCFLAGS ?= --verbose --warnings all
MAIN = $(PROGRAM).ino
OBJ = $(PROGRAM).ino.bin
SRCS = $(MAIN)

LIBS =
EXTRALIBS =

build: $(OBJ)

$(OBJ): $(SRCS) Makefile
	arduino-cli compile -b $(BOARD) --build-cache-path . $(CCFLAGS) -o $< $(MAIN)

ota: $(OBJ)
	@if [ -z "$$IP" ] ; then \
		IP=`avahi-browse -ptr  "_arduino._tcp" | egrep ^= | cut -d\; -f4,8,9 | grep ^$$DEVICE | cut -d\; -f2` -p `avahi-browse -ptr  "_arduino._tcp" | egrep ^= | cut -d\; -f4,8,9 | grep ^$$DEVICE | cut -d\; -f3` ;\
	fi ;\
	python $(OTAPROG) -i $(IP) "--auth=$(OTAPASS)" -f $(OBJ)

find:
	@if [ `uname -s` = Darwin ] ; then \
		dns-sd -B _arduino._tcp ;\
	else \
		avahi-browse -ptr  "_arduino._tcp" | egrep ^= | cut -d\; -f4,8,9 ;\
	fi

upload: $(OBJ)
	arduino-cli upload -b $(BOARD) -p $(PORT) -i $(OBJ) -v -t

gosho: upload monitor

clean:
	rm -f $(OBJ)

monitor:
	#cu -s 115200 -l $(PORT)
	miniterm --rts 0 --dtr 0 $(PORT) 115200

installcore: cliconfig
	@[ -f $(GOPATH)/bin/arduino-cli ] || go get -v -u github.com/arduino/arduino-cli && arduino-cli core update-index
	@arduino-cli core list | grep ^esp8266:esp8266 >/dev/null || arduino-cli core install esp8266:esp8266

cliconfig:
	@ [ -d $(GOPATH) ] || mkdir -p $(GOPATH)
	@ [ -d $(GOPATH)/bin ] || mkdir -p $(GOPATH)/bin
	@ [ -d $(GOPATH)/src ] || mkdir -p $(GOPATH)/src
	@if [ \! -f $(GOPATH)/bin/.cli-config.yml ] ; then \
	echo "board_manager:" >>$(GOPATH)/bin/.cli-config.yml ; \
	echo "  additional_urls:" >>$(GOPATH)/bin/.cli-config.yml ; \
	echo "    - http://arduino.esp8266.com/stable/package_esp8266com_index.json" >>$(GOPATH)/bin/.cli-config.yml ; \
	echo "    - https://dl.espressif.com/dl/package_esp32_index.json" >>$(GOPATH)/bin/.cli-config.yml ; \
	fi

libs:
	@for lib in $(LIBS) ; do libdir=`echo "$$lib" | sed -e 's/ /_/g'` ; if [ -d "$(LIBDIR)/$$libdir" ] ; then true ; else echo "Installing $$lib" ; arduino-cli lib install "$$lib" ; fi ; done

extralibs:
	@[ -d $(LIBDIR) ] || mkdir -p $(LIBDIR)
	@for lib in $(EXTRALIBS) ; do repo=`echo $$lib | cut -d% -f1` ; dir=`echo $$lib | cut -d% -f2`; if [ -d "$(LIBDIR)/$$dir" ] ; then echo "Found $$dir" ; else echo "Clone $$repo => $$dir" ; cd $(LIBDIR) && git clone $$repo $$dir ; fi ; done[

installdeps: installcore libs extralibs
