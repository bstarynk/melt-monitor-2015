nbuseless=0

#define MOM_HAS_PREDEFINED(Nam,Hash) grep -q #Nam _listpredefs || { nbuseless=$(( $nbuseless + 1 )) ; echo #Nam }

#include "_mom_predef.h"

echo MOM_NB_PREDEFINED predefined items with $nbuseless useless

exit 0

## eof predefgc.cmd
