################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CU_SRCS += \
../src/OptCUDANTT.cu 

OBJS += \
./src/OptCUDANTT.o 

CU_DEPS += \
./src/OptCUDANTT.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cu
	@echo 'Building file: $<'
	@echo 'Invoking: NVCC Compiler'
	/usr/bin/nvcc -D_FORCE_INLINES -DNDEBUG -O3 -gencode arch=compute_35,code=sm_35  -odir "src" -M -o "$(@:%.o=%.d)" "$<"
	/usr/bin/nvcc -D_FORCE_INLINES -DNDEBUG -O3 --compile --relocatable-device-code=false -gencode arch=compute_35,code=compute_35 -gencode arch=compute_35,code=sm_35  -x cu -o  "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


