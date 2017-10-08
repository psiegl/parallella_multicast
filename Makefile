# Copyright (c) 2017, Dipl.-Inf. Patrick Siegl
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


TARGET	:= main.elf e_main.srec
default: $(TARGET)

DEPS := $(shell ls .dep_*)
-include $(DEPS)

## ACC  #################
C_PERF := -O3
C_SIZE := -ffunction-sections -fdata-sections
e_%.o: e_%.c
	e-gcc ${C_PERF} ${C_SIZE} -Wall -Wunused-variable -MMD -MF".dep_$@.d" -c -o $@ $<

L_SIZE := -Wl,-static -Wl,-s -Wl,--gc-sections
e_%.elf: e_%.o
	e-gcc -T $(EPIPHANY_HOME)/bsps/current/internal.ldf ${L_SIZE} -o $@ $< -le-lib
	@e-size $@

e_%.srec: e_%.elf
	e-objcopy --srec-forceS3 --output-target srec $^ $@

## HOST #################
%.elf: %.o
	gcc -L ${EPIPHANY_HOME}/tools/host.armv7l/lib -o $@ $< -lm -le-hal -le-loader
	@size $@

%.o: %.c
	gcc -O3 -mfpu=neon -I $(EPIPHANY_HOME)/tools/host.armv7l/include -MMD -MF".dep_$@.d" -c -o $@ $<

## RUN  #################
ELIBS=${ESDK}/tools/host.armv7l/lib:${LD_LIBRARY_PATH}
EHDF=${EPIPHANY_HDF}
run: $(TARGET)
	sudo -E LD_LIBRARY_PATH=${ELIBS} EPIPHANY_HDF=${EHDF} ./main.elf


clean:
	rm -rf $(TARGET) .dep* *.o *.elf *.log
