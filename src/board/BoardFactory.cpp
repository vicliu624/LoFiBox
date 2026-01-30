#include "BoardBase.h"
#include "CardputerBoard.h"
#include "TLoraPagerBoard.h"

#if defined(BOARD_TLORA_PAGER)
static TLoraPagerBoard board_instance;
#elif defined(BOARD_CARDPUTER_ADV)
static CardputerBoard board_instance;
#else
#error "No board selected. Define BOARD_TLORA_PAGER or BOARD_CARDPUTER_ADV"
#endif

BoardBase& board = board_instance;