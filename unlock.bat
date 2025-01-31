openocd -f ./openocd_stm32f1x.cfg -c "init; halt; flash protect 0 0 63 off; exit"
openocd -f ./openocd_stm32f1x.cfg -c "init; halt; flash protect 0 0 31 off; exit"