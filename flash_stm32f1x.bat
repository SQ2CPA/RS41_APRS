openocd -f ./openocd_stm32f1x.cfg -c "program .pio/build/stm32f100c8t6/firmware.elf verify reset exit"