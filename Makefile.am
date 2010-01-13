if CAN_BUILD_FIRMWARES
WAREZ=firmware/1390702_leds firmware/1390702_noleds firmware/1391401 firmware/1392934 firmware/M4-1

bindists: all make_dists.sh
	$(SHELL) make_dists.sh $(WAREZ)
endif

SUBDIRS=tools matrices mappings firmware/common firmware/vusb $(WAREZ)

CIRCUITS=\
 circuits/IBM_M4-1/74hc164.lib\
 circuits/IBM_M4-1/README\
 circuits/IBM_M4-1/schematic.pdf\
 circuits/IBM_M4-1/schematic.pro\
 circuits/IBM_M4-1/schematic.sch\
 circuits/IBM_Model_M/dulcimer.erc\
 circuits/IBM_Model_M/dulcimer.pro\
 circuits/IBM_Model_M/dulcimer-Component.pdf\
 circuits/IBM_Model_M/dulcimer-Copper.pdf\
 circuits/IBM_Model_M/dulcimer-Circuit.pdf\
 circuits/IBM_Model_M/dulcimer.brd\
 circuits/IBM_Model_M/dulcimer.sch\
 circuits/IBM_Model_M/dulcimer.net\
 circuits/IBM_Model_M/dulcimer-SilkS_Cop.pdf\
 circuits/IBM_Model_M/rump.sch\
 circuits/IBM_Model_M/dulcimer.cmp\
 circuits/IBM_Model_M/README\
 circuits/IBM_Model_M/dulcimer.lst\
 circuits/IBM_Model_M/dulcimer.000\
 circuits/IBM_Model_M/dulcimer.stf\
 circuits/IBM_Model_M/dulcimer-SilkS_Cmp.pdf\
 circuits/IBM_Model_M/rump-schematic.png

EXTRA_DIST=$(CIRCUITS) README.git README.matrix
