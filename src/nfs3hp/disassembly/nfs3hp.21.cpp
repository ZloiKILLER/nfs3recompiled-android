#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_48ec40: native, nfs3hp::port::soundThread */

/* sub_48ec80: native, nfs3hp::port::setSoundServer */

/* sub_48ed00: native, nfs3hp::port::clearTimerCallbacks */

/* sub_48ed30: native, nfs3hp::port::timerTick */

/* sub_48ee30: native, nfs3hp::port::addTimerCallback */

/* sub_48ee90: native, nfs3hp::port::removeTimerCallback */

/* sub_48eef0: native, nfs3hp::dead::stub_0x48eef0 */

/* sub_48ef20: native, nfs3hp::dead::stub_0x48ef20 */

/* sub_48ef30: native, nfs3hp::dead::stub_0x48ef30 */

/* sub_48ef50: native, nfs3hp::port::stopTimer */

/* sub_48eff0: native, nfs3hp::port::startTimer */

/* sub_48f1f0: native, nfs3hp::controls::bindPlayersControls */

/* sub_48f4f0: native, nfs3hp::controls::startRaceControls */

/* sub_48f550: native, nfs3hp::controls::readRaceControls */

/* sub_48f6a0: native, nfs3hp::controls::takeCarControls */

/* sub_48f6e0: native, nfs3hp::controls::stepMenuChoice */

/* sub_48f710: native, nfs3hp::controls::stepMenuChoiceOfThree */

/* sub_48f740: native, nfs3hp::controls::actionPressed */

/* sub_48f7b0: native, nfs3hp::controls::functionKeyPressed */

/* sub_48f820: native, nfs3hp::controls::pressFunctionKey */

/* sub_48f860: native, nfs3hp::controls::functionKeyRepeats */

/* sub_48f8d0: native, nfs3hp::controls::keyPressed */

/* sub_48f920: native, nfs3hp::controls::firstFunctionKeyHeld */

/* sub_48f940: native, nfs3hp::controls::bindControl */

/* sub_48f970: native, nfs3hp::controls::bindFunctionKeys */

/* sub_48fa10: native, nfs3hp::controls::queueRaceControls */

/* sub_48faa0: native, nfs3hp::dead::stub_0x48faa0 */

/* sub_48fb40: native, nfs3hp::pieces::stub_0x48fb40 */

/* sub_4900b0: native, nfs3hp::dead::stub_0x4900b0 */

/* sub_490110: native, nfs3hp::dead::stub_0x490110 */

/* sub_490140: native, nfs3hp::dead::stub_0x490140 */

/* sub_4902f0: native, nfs3hp::dead::stub_0x4902f0 */

/* sub_490430: native, nfs3hp::pieces::stub_0x490430 */

/* sub_4904d0: native, nfs3hp::controls::controlsFor */

/* sub_4904f0: native, nfs3hp::controls::defaultControls */

/* sub_4905b0: native, nfs3hp::controls::setControls */

/* sub_490610: native, nfs3hp::controls::controllerNamed */

/* sub_490640: native, nfs3hp::controls::knownControllerControls */

/* sub_4906b0: native, nfs3hp::controls::sideWinderGamePadControls */

/* sub_490770: native, nfs3hp::controls::gravisGripControls */

/* sub_490830: native, nfs3hp::controls::wingManForceControls */

/* sub_4908f0: native, nfs3hp::controls::combatStickControls */

/* sub_4909b0: native, nfs3hp::controls::sideWinderProControls */

/* sub_490a70: native, nfs3hp::controls::sideWinderWheelControls */

/* sub_490b30: native, nfs3hp::controls::steeringWheelControls */

/* sub_490c40: native, nfs3hp::controls::keyboardControls */

/* sub_490cb0: native, nfs3hp::controls::gamePadControls */

/* sub_490e30: native, nfs3hp::controls::joystickControls */

/* sub_490fa0: native, nfs3hp::controls::otherControllerControls */

/* sub_491120: native, nfs3hp::frontend::rendBlinkColour */

/* sub_491190: native, nfs3hp::render::glow */

/* sub_491bc0: native, nfs3hp::render::roadLight */

/* sub_492370: native, nfs3hp::vertices::lightGlow */

/* sub_492980: native, nfs3hp::vertices::headlightBeam */

/* sub_493380: native, nfs3hp::frontend::rendConeSideOffset */

/* sub_4934c0: native, nfs3hp::frontend::rendConeEdgePoint */

/* sub_493640: native, nfs3hp::frontend::rendConesPlace */

/* sub_493a30: native, nfs3hp::frontend::rendConeCountPass */

/* sub_493ad0: native, nfs3hp::frontend::rendConesHit */

/* sub_493e50: native, nfs3hp::frontend::rendConesReset */

/* sub_493ef0: native, nfs3hp::frontend::rendConesLoad */

/* sub_493fb0: native, nfs3hp::frontend::rendConesFree */

/* sub_494010: native, nfs3hp::frontend::rendConesStep */

/* sub_494170: native, nfs3hp::frontend::rendConesDraw */

/* sub_494360: native, nfs3hp::frontend::rendBoltsClear */

/* sub_494390: native, nfs3hp::frontend::rendBoltGlow */

/* sub_4943e0: native, nfs3hp::frontend::rendBoltFreeNode */

/* sub_494420: native, nfs3hp::frontend::rendBoltBranch */

/* sub_4945a0: native, nfs3hp::frontend::rendBoltStrike */

/* sub_494630: native, nfs3hp::frontend::rendBoltsStep */

/* sub_4946e0: native, nfs3hp::render::lightning */

/* sub_494bd0: native, nfs3hp::graphics::drawLoadingAborted */

/* sub_494cb0: native, nfs3hp::render::loadingScreen */

/* sub_494fa0: native, nfs3hp::graphics::showLoadingScreen */

/* sub_495190: native, nfs3hp::frontend::loopBarScale */

/* sub_495230: native, nfs3hp::graphics::drawLoadingProgress */

/* sub_495600: native, nfs3hp::frontend::rendBackImage */

/* sub_4956f0: native, nfs3hp::frontend::loadQfs */

/* sub_4957a0: native, nfs3hp::dead::stub_0x4957a0 */

/* sub_495860: native, nfs3hp::dead::stub_0x495860 */

/* sub_495890: native, nfs3hp::dead::stub_0x495890 */

/* sub_4958c0: native, nfs3hp::dead::stub_0x4958c0 */

/* sub_495920: native, nfs3hp::dead::stub_0x495920 */

/* sub_495a00: native, nfs3hp::dead::stub_0x495a00 */

/* sub_495a10: native, nfs3hp::dead::stub_0x495a10 */

/* sub_495a50: native, nfs3hp::dead::stub_0x495a50 */

/* sub_495b10: native, nfs3hp::dead::stub_0x495b10 */

/* sub_495b30: native, nfs3hp::dead::stub_0x495b30 */

/* sub_495b60: native, nfs3hp::frontend::winaMovieNextFrame */

/* sub_495bc0: native, nfs3hp::movie::play */

/* sub_4960e0: native, nfs3hp::frontend::playMovie */

/* sub_4961e0: native, nfs3hp::frontend::resetAttractTimer */

/* sub_496200: native, nfs3hp::frontend::attractMode */

/* sub_496300: native, nfs3hp::pieces::stub_0x496300 */

/* sub_496310: native, nfs3hp::frontend::mathMakeRootTable */

/* sub_496370: native, nfs3hp::dead::stub_0x496370 */

/* sub_496390: native, nfs3hp::dead::stub_0x496390 */

/* sub_496440: native, nfs3hp::dead::stub_0x496440 */

/* sub_496490: native, nfs3hp::dead::stub_0x496490 */

/* sub_4964d0: native, nfs3hp::frontend::rendWrapFraction */

}
