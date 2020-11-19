## @file
# Copyright (C) 2020, Mike Beaton.  All rights reserved.<BR>
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##

# DSC_SPECIFICATION defines the min. platform version, since we're planning to link to OC we use the same value as they do
[Defines]
  PLATFORM_NAME           = BootHelperPkg
  PLATFORM_GUID           = F891ABF3-A30F-4066-93D6-73C11E30EC25
  PLATFORM_VERSION        = 1.0
  SUPPORTED_ARCHITECTURES = X64|IA32
  BUILD_TARGETS           = RELEASE|DEBUG|NOOPT
  SKUID_IDENTIFIER        = DEFAULT
  DSC_SPECIFICATION       = 0x00010006

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibRepStr/BaseMemoryLibRepStr.inf
  DebugLib|OpenCorePkg/Library/OcDebugLogLib/OcDebugLogLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  OcConsoleControlEntryModeGenericLib|OpenCorePkg/Library/OcConsoleControlEntryModeLib/OcConsoleControlEntryModeGenericLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  TimerLib|OpenCorePkg/Library/OcTimerLib/OcTimerLib.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
# Direct or indirect dependencies of OcDebugLogLib
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  OcCpuLib|OpenCorePkg/Library/OcCpuLib/OcCpuLib.inf
  OcDataHubLib|OpenCorePkg/Library/OcDataHubLib/OcDataHubLib.inf
  OcDevicePathLib|OpenCorePkg/Library/OcDevicePathLib/OcDevicePathLib.inf
  OcFileLib|OpenCorePkg/Library/OcFileLib/OcFileLib.inf
  OcGuardLib|OpenCorePkg/Library/OcGuardLib/OcGuardLib.inf
  OcMiscLib|OpenCorePkg/Library/OcMiscLib/OcMiscLib.inf
  OcStringLib|OpenCorePkg/Library/OcStringLib/OcStringLib.inf
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PlatformHookLib|MdeModulePkg/Library/BasePlatformHookLibNull/BasePlatformHookLibNull.inf
  SerialPortLib|MdeModulePkg/Library/BaseSerialPortLib16550/BaseSerialPortLib16550.inf

[Components]
  BootHelperPkg/Application/BootHelper/BootHelper.inf

[PcdsFixedAtBuild]

[BuildOptions]
  MSFT:NOOPT_*_*_CC_FLAGS    = -D OC_TARGET_RELEASE=1 /FAcs -Dinline=__inline
  MSFT:DEBUG_*_*_CC_FLAGS    = -D OC_TARGET_RELEASE=1 /FAcs -Dinline=__inline -DMDEPKG_NDEBUG
  MSFT:RELEASE_*_*_CC_FLAGS  = -D OC_TARGET_RELEASE=1 /FAcs -Dinline=__inline -DMDEPKG_NDEBUG

  XCODE:NOOPT_*_*_CC_FLAGS   = -D OC_TARGET_RELEASE=1 -fno-unwind-tables -O0
  XCODE:DEBUG_*_*_CC_FLAGS   = -D OC_TARGET_RELEASE=1 -fno-unwind-tables -flto -Os -DMDEPKG_NDEBUG
  XCODE:RELEASE_*_*_CC_FLAGS = -D OC_TARGET_RELEASE=1 -fno-unwind-tables -flto -Os -DMDEPKG_NDEBUG

  GCC:NOOPT_*_*_CC_FLAGS     = -D OC_TARGET_RELEASE=1 -Wno-unused-but-set-variable
  GCC:DEBUG_*_*_CC_FLAGS     = -D OC_TARGET_RELEASE=1 -DMDEPKG_NDEBUG -Wno-unused-but-set-variable
  GCC:RELEASE_*_*_CC_FLAGS   = -D OC_TARGET_RELEASE=1 -DMDEPKG_NDEBUG -Wno-unused-but-set-variable
