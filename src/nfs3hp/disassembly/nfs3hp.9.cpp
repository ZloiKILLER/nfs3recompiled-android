#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4379b0: native, nfs3hp::frontend::audioRaceMusicMoved */

/* sub_4379d0: native, nfs3hp::frontend::audioEffectsMoved */

/* sub_437a20: native, nfs3hp::frontend::audioEngineMoved */

/* sub_437a70: native, nfs3hp::frontend::audioSpeechMoved */

/* sub_437a90: native, nfs3hp::frontend::audioVolumeDraw */

/* sub_437e00: native, nfs3hp::frontend::audioOnEnter */

/* sub_438070: native, nfs3hp::frontend::audioOnExit */

/* sub_438260: native, nfs3hp::frontend::audioOnFrame */

/* sub_438340: native, nfs3hp::load::endWithSlash */

/* sub_438350: native, nfs3hp::load::folderWritable */

/* sub_4383f0: native, nfs3hp::load::firstFreeHexFolder */

/* sub_4384a0: native, nfs3hp::dead::stub_0x4384a0 */

/* sub_4384f0: native, nfs3hp::load::firstFreePlayerFolder */

/* sub_4385a0: native, nfs3hp::load::freePlayerCarFolder */

/* sub_4385f0: native, nfs3hp::dead::stub_0x4385f0 */

/* sub_4386a0: native, nfs3hp::dead::stub_0x4386a0 */

/* sub_4386b0: native, nfs3hp::load::firstSubfolder */

/* sub_4387e0: native, nfs3hp::load::nextSubfolder */

/* sub_438870: native, nfs3hp::load::firstFile */

/* sub_438970: native, nfs3hp::load::nextFile */

/* sub_4389d0: native, nfs3hp::load::removeFolder */

/* sub_438a80: native, nfs3hp::load::firstCar */

/* sub_438aa0: native, nfs3hp::load::nextCar */

/* sub_438ad0: native, nfs3hp::load::makeCarId */

/* sub_438af0: native, nfs3hp::load::firstCarGlobal */

/* sub_438b10: native, nfs3hp::load::nextCarGlobal */

/* sub_438b40: native, nfs3hp::dead::stub_0x438b40 */

/* sub_438b70: native, nfs3hp::frontend::loadingCarBefore */

/* sub_438b90: native, nfs3hp::load::appendCar */

/* sub_438bd0: native, nfs3hp::load::carCount */

/* sub_438be0: native, nfs3hp::load::countCars */

/* sub_438c10: native, nfs3hp::load::unlinkCar */

/* sub_438c50: native, nfs3hp::load::insertCarByName */

/* sub_438cc0: native, nfs3hp::load::joinCarLists */

/* sub_438d20: native, nfs3hp::load::carListOf */

/* sub_438d60: native, nfs3hp::load::takePoliceCars */

/* sub_438dc0: native, nfs3hp::load::takeClasslessCars */

/* sub_438e30: native, nfs3hp::load::takeOrdinaryCars */

/* sub_438eb0: native, nfs3hp::load::sortCars */

/* sub_438f60: native, nfs3hp::frontend::loadingCarStat */

/* sub_438fc0: native, nfs3hp::dead::stub_0x438fc0 */

/* sub_4390f0: native, nfs3hp::dead::stub_0x4390f0 */

/* sub_4391d0: native, nfs3hp::dead::stub_0x4391d0 */

/* sub_439250: native, nfs3hp::load::carName */

/* sub_439290: native, nfs3hp::frontend::showcaseCarFileStem */

/* sub_4392b0: native, nfs3hp::load::carFolder */

/* sub_439300: native, nfs3hp::load::carFilePrefix */

/* sub_439360: native, nfs3hp::port::obj3d::carVivPath */

/* sub_439450: native, nfs3hp::dead::stub_0x439450 */

/* sub_439510: native, nfs3hp::frontend::loadingCarExtras */

/* sub_439520: native, nfs3hp::load::carId */

/* sub_439530: native, nfs3hp::frontend::loadingCarIdIsPlain */

/* sub_439570: native, nfs3hp::load::carFlag40 */

/* sub_439590: native, nfs3hp::dead::stub_0x439590 */

/* sub_4395a0: native, nfs3hp::frontend::loadingCarIsKept */

/* sub_4395c0: native, nfs3hp::load::carFlag4 */

/* sub_4395e0: native, nfs3hp::frontend::loadingCarFlag2 */

/* sub_439600: native, nfs3hp::pieces::stub_0x439600 */

/* sub_439610: native, nfs3hp::load::carHasNumericId */

/* sub_439620: native, nfs3hp::load::idIsNumeric */

/* sub_439660: native, nfs3hp::load::numericId */

/* sub_439680: native, nfs3hp::load::carIsBonus */

/* sub_4396a0: native, nfs3hp::dead::stub_0x4396a0 */

/* sub_4396b0: native, nfs3hp::load::carIsPolice */

/* sub_4396d0: native, nfs3hp::dead::stub_0x4396d0 */

/* sub_4396e0: native, nfs3hp::load::carIsFerrariOrMercedes */

/* sub_439700: native, nfs3hp::dead::stub_0x439700 */

/* sub_439710: native, nfs3hp::frontend::loadingCarByte3a0 */

/* sub_439730: native, nfs3hp::load::carClass */

/* sub_439740: native, nfs3hp::dead::stub_0x439740 */

/* sub_439760: native, nfs3hp::port::fields::carColourCount */

/* sub_439780: native, nfs3hp::port::fields::carCustomColour */

/* sub_439790: native, nfs3hp::port::fields::carColour */

/* sub_4397c0: native, nfs3hp::port::fields::carColourName */

/* sub_4397d0: native, nfs3hp::port::fields::setCarColour */

/* sub_439810: native, nfs3hp::port::fields::carColourRgb */

/* sub_4398c0: native, nfs3hp::port::fields::setCarCustomColour */

/* sub_439930: native, nfs3hp::dead::stub_0x439930 */

/* sub_439940: native, nfs3hp::dead::stub_0x439940 */

/* sub_439950: native, nfs3hp::dead::stub_0x439950 */

/* sub_439960: native, nfs3hp::dead::stub_0x439960 */

/* sub_439970: native, nfs3hp::dead::stub_0x439970 */

/* sub_439980: native, nfs3hp::dead::stub_0x439980 */

/* sub_439990: native, nfs3hp::load::findCarById */

/* sub_4399f0: native, nfs3hp::load::clearCarCache */

/* sub_439a10: native, nfs3hp::frontend::loadingPlayerToListedCar */

/* sub_439a50: native, nfs3hp::load::carRecord */

/* sub_439ab0: native, nfs3hp::port::fields::carText */

/* sub_439af0: native, nfs3hp::load::fecText */

/* sub_439b20: native, nfs3hp::load::findCarByName */

/* sub_439b70: native, nfs3hp::frontend::loadingFreeCars */

/* sub_439bb0: native, nfs3hp::frontend::loadingCarFreeData */

/* sub_439bd0: native, nfs3hp::load::attachFecTexts */

/* sub_439bf0: native, nfs3hp::frontend::loadingCarFreeBlock */

/* sub_439c10: native, nfs3hp::load::bonusCarLocked */

/* sub_439cc0: native, nfs3hp::frontend::simBonusCarLocked */

/* sub_439ce0: native, nfs3hp::frontend::creditsHaveBonusCar */

/* sub_439d20: native, nfs3hp::load::checkDuplicateCars */

/* sub_439d90: native, nfs3hp::load::loadCarList */

/* sub_43a4d0: native, nfs3hp::frontend::loadingCarCompareNames */

}
