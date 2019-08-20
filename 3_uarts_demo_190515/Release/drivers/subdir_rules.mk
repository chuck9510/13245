################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
drivers/pinout.obj: F:/USR_3_uart_demo/demo_code/TivaWare_C_Series-2.1.0.12573/examples/boards/ek-tm4c1294xl/drivers/pinout.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv6/tools/compiler/ti-cgt-arm_5.2.2/include" --include_path="F:/USR_3_uart_demo/demo_code/3_uarts_demo_190515" --include_path="G:/Project 2015/Comunication/TI_RTOS/examples/boards/ek-tm4c1294xl" --include_path="G:/Project 2015/Comunication/TI_RTOS" --include_path="G:/Project 2015/Comunication/TI_RTOS/third_party" --include_path="G:/Project 2015/Comunication/TI_RTOS/third_party/lwip-1.4.1/src/include" --include_path="G:/Project 2015/Comunication/TI_RTOS/third_party/lwip-1.4.1/src/include/ipv4" --include_path="G:/Project 2015/Comunication/TI_RTOS/third_party/lwip-1.4.1/ports/tiva-tm4c129/include" --include_path="G:/Project 2015/Comunication/TI_RTOS/third_party/lwip-1.4.1/apps" --include_path="G:/Project 2015/Comunication/TI_RTOS/third_party/FreeRTOS/Source/include" --include_path="G:/Project 2015/Comunication/TI_RTOS/third_party/FreeRTOS" --include_path="G:/Project 2015/Comunication/TI_RTOS/third_party/FreeRTOS/Source/portable/CCS/ARM_CM4F" --gcc --define=ccs="ccs" --define=PART_TM4C1294NCPDT --define=TARGET_IS_TM4C129_RA1 --display_error_number --diag_warning=225 --diag_wrap=off --gen_func_subsections=on --ual --preproc_with_compile --preproc_dependency="drivers/pinout.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


