TARGETS := $(MAKECMDGOALS)

.PHONY: all $(TARGETS)
all: $(TARGETS)

$(TARGETS):
	mkapp -v -t $@ \
		GroveGPIO/GroveGPIO.adef

	mkapp -C -D$@ -C -DINTERFACE_IOT0 -v -t $@ \
		ledmatrix/LedMatrix.adef
		
	mkapp -v -t $@ \
		fingerprint/FingerPrint.adef

	mkapp -v -t $@ \
		lightSensor/lightSensor.adef


clean:
	rm -rf _build_* *.update

