#  Copyright (c) 2020, Mike Beaton. All rights reserved.
#  SPDX-License-Identifier: BSD-3-Clause

.PHONY: config
config:
	../../../OpenCorePkg/Library/OcConfigurationLib/PlistToConfig.py -f 0x1f Template.plist --prefix Bh -c BhConfig.c -h BhConfig.h --include '"BhConfig.h"' > mapped.plist
	../../../OpenCorePkg/Library/OcConfigurationLib/CheckSchema.py BhConfig.c
