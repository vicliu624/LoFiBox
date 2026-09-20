#include "BoardBase.h"

#if defined(BOARD_TLORA_PAGER)
#include "TLoraPagerBoard.h"
static TLoraPagerBoard board_instance;
#elif defined(BOARD_CARDPUTER_ADV)
#include "CardputerBoard.h"
static CardputerBoard board_instance;
#elif defined(BOARD_M5STACK_CORE2_AUDIO_FACES)
#include "M5Core2AudioFacesBoard.h"
static M5Core2AudioFacesBoard board_instance;
#else
#error "No board selected. Define a supported BOARD_* macro."
#endif

BoardBase &board = board_instance;
