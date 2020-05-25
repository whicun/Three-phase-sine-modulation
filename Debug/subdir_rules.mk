################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.asm $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/ti/ccs910/ccs/tools/compiler/ti-cgt-c2000_18.12.2.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="D:/Ti/controlSUITE/device_support/f2833x/v142/DSP2833x_common/include" --include_path="D:/Ti/controlSUITE/device_support/f2833x/v142/DSP2833x_common/lib" --include_path="D:/Ti/controlSUITE/device_support/f2833x/v142/DSP2833x_common/source" --include_path="D:/Ti/controlSUITE/device_support/f2833x/v142/DSP2833x_headers/include" --include_path="D:/Faks/IV/Diplomski/Three phase sine modulation" --include_path="D:/ti/ccs910/ccs/tools/compiler/ti-cgt-c2000_18.12.2.LTS/include" --advice:performance=all -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"D:/ti/ccs910/ccs/tools/compiler/ti-cgt-c2000_18.12.2.LTS/bin/cl2000" -v28 -ml -mt --float_support=fpu32 --include_path="D:/Ti/controlSUITE/device_support/f2833x/v142/DSP2833x_common/include" --include_path="D:/Ti/controlSUITE/device_support/f2833x/v142/DSP2833x_common/lib" --include_path="D:/Ti/controlSUITE/device_support/f2833x/v142/DSP2833x_common/source" --include_path="D:/Ti/controlSUITE/device_support/f2833x/v142/DSP2833x_headers/include" --include_path="D:/Faks/IV/Diplomski/Three phase sine modulation" --include_path="D:/ti/ccs910/ccs/tools/compiler/ti-cgt-c2000_18.12.2.LTS/include" --advice:performance=all -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


