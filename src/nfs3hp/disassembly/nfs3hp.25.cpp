#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4aa79f: native, nfs3hp::handling::resetDriveState */

/* sub_4aa8e7: native, nfs3hp::handling::settleAtRest */

/* sub_4aab41: native, nfs3hp::drivetrain::wheelGrip */

/* sub_4aaed1: native, nfs3hp::drivetrain::engineForce */

/* sub_4abd9c: native, nfs3hp::handling::setupGripLoss */

/* sub_4ac027: native, nfs3hp::handling::setupSlipLimit */

/* sub_4ac094: native, nfs3hp::handling::tyreGrip */

/* sub_4ac3af: native, nfs3hp::handling::tyreSlide */

/* sub_4ac994: native, nfs3hp::handling::logisticCurve */

/* sub_4ac9dd: native, nfs3hp::handling::steeringGrip */

/* sub_4aca4f: native, nfs3hp::handling::axleForces */

/* sub_4ad250: native, nfs3hp::physics::clearWheelForces */

/* sub_4ad2e1: native, nfs3hp::physics::steerAngle */

/* sub_4ad672: native, nfs3hp::physics::sideSlip */

/* sub_4ad6eb: native, nfs3hp::physics::groundSpeed */

/* sub_4ad7a3: native, nfs3hp::physics::physicsStep */

/* sub_4ae4c1: native, nfs3hp::physics::carPhysics */

/* sub_4ae5c2: native, nfs3hp::physics::settleLoadShift */

/* sub_4ae68e: native, nfs3hp::physics::driveForce */

/* sub_4ae79b: native, nfs3hp::physics::holdStill */

/* sub_4ae85b: native, nfs3hp::physics::sideGrip */

/* sub_4ae98b: native, nfs3hp::physics::handbrakeDrag */

/* sub_4aea88: native, nfs3hp::physics::yawDamping */

/* sub_4aeffc: native, nfs3hp::physics::idleDamping */

/* sub_4af140: native, nfs3hp::physics::coastDamping */

/* sub_4af254: native, nfs3hp::physics::spinTorque */

/* sub_4b0700: native, nfs3hp::frontend::carDataReadInteger */

/* sub_4b0790: native, nfs3hp::pieces::stub_0x4b0790 */

/* sub_4b07b0: native, nfs3hp::frontend::carDataReadByte */

/* sub_4b07c0: native, nfs3hp::pieces::stub_0x4b07c0 */

/* sub_4b07d0: native, nfs3hp::frontend::carDataReadFloat */

/* sub_4b0880: native, nfs3hp::pieces::stub_0x4b0880 */

/* sub_4b08b0: native, nfs3hp::frontend::carDataReadFixed */

/* sub_4b08d0: native, nfs3hp::pieces::stub_0x4b08d0 */

/* sub_4b0900: native, nfs3hp::frontend::carDataLineHasValues */

/* sub_4b0940: native, nfs3hp::frontend::carDataFindKey */

/* sub_4b09d0: native, nfs3hp::frontend::carDataNextLine */

/* sub_4b0bb0: native, nfs3hp::frontend::carDataFillDefaults */

/* sub_4b1380: native, nfs3hp::frontend::carDataRead */

/* sub_4b22d0: native, nfs3hp::pieces::stub_0x4b22d0 */

/* sub_4b48d0: native, nfs3hp::pieces::stub_0x4b48d0 */

/* sub_4b54f0: native, nfs3hp::dead::stub_0x4b54f0 */

/* sub_4b56d0: native, nfs3hp::frontend::winaTickCount */

/* sub_4b56e0: native, nfs3hp::render::transformBuffer */

/* sub_4b5700: native, nfs3hp::frontend::rendFreeBlock */

/* sub_4b5710: native, nfs3hp::dead::stub_0x4b5710 */

/* sub_4b5720: native, nfs3hp::frontend::rendChooseDriver */

/* sub_4b5860: native, nfs3hp::frontend::winaCheckLibrary */

/* sub_4b5940: native, nfs3hp::frontend::rendReadDriverFeatures */

/* sub_4b59c0: native, nfs3hp::render::startDriver */

/* sub_4b5e50: native, nfs3hp::frontend::winaFatalMessage */

/* sub_4b5ea0: native, nfs3hp::graphics::warnTextureMemory */

/* sub_4b5f10: native, nfs3hp::frontend::winaCheckMemory */

/* sub_4b5ff0: native, nfs3hp::frontend::winaDeviceAnswer */

/* sub_4b6020: native, nfs3hp::frontend::loopGameInit */

/* sub_4b6250: native, nfs3hp::graphics::shutDown */

/* sub_4b62a0: native, nfs3hp::frontend::loopInstallIsRemote */

/* sub_4b6320: native, nfs3hp::frontend::discCheck */

/* sub_4b6400: native, nfs3hp::frontend::winaThrashDevice */

/* sub_4b64c0: native, nfs3hp::frontend::winaThrashDescription */

/* sub_4b6520: native, nfs3hp::frontend::winaDesktopSetting */

/* sub_4b6590: native, nfs3hp::frontend::winaDirectXVersion */

/* sub_4b67b0: native, nfs3hp::frontend::pauseQuitToAnswered */

/* sub_4b6810: native, nfs3hp::frontend::pauseQuitToAsk */

/* sub_4b6870: native, nfs3hp::frontend::pauseQuitToReplay */

/* sub_4b6890: native, nfs3hp::frontend::pauseQuitToPost */

/* sub_4b68d0: native, nfs3hp::frontend::pauseQuitToMain */

/* sub_4b6910: native, nfs3hp::frontend::pauseQuitToSystem */

/* sub_4b6930: native, nfs3hp::frontend::pauseQuitToOnEnter */

/* sub_4b6a10: native, nfs3hp::pieces::stub_0x4b6a10 */

/* sub_4b6a30: native, nfs3hp::frontend::poseBlend */

/* sub_4b6bb0: native, nfs3hp::dead::stub_0x4b6bb0 */

/* sub_4b6ca0: native, nfs3hp::frontend::poseMatrixOfRotation */

/* sub_4b6d90: native, nfs3hp::frontend::poseRotationNearest */

/* sub_4b6e80: native, nfs3hp::frontend::poseRotationOfMatrix */

/* sub_4b70b0: native, nfs3hp::dead::stub_0x4b70b0 */

/* sub_4b70f0: native, nfs3hp::frontend::simPoseBlend */

/* sub_4b7180: native, nfs3hp::dead::stub_0x4b7180 */

/* sub_4b71e0: native, nfs3hp::frontend::simQuatSlerp */

/* sub_4b7300: native, nfs3hp::frontend::simQuatNearer */

/* sub_4b73c0: native, nfs3hp::frontend::simQuatFromMatrix */

/* sub_4b7590: native, nfs3hp::frontend::simMatrixFromQuat */

/* sub_4b7680: native, nfs3hp::frontend::simMatrixSlerp */

/* sub_4b76d0: native, nfs3hp::dead::stub_0x4b76d0 */

/* sub_4b7720: native, nfs3hp::frontend::simMatrixSubtract */

/* sub_4b7780: native, nfs3hp::frontend::simMatrixAdd */

/* sub_4b77e0: native, nfs3hp::frontend::simMatrixScale */

/* sub_4b7840: native, nfs3hp::frontend::simMatrixOrthonormalise */

/* sub_4b7920: native, nfs3hp::dead::stub_0x4b7920 */

/* sub_4b79c0: native, nfs3hp::frontend::simMatrixQuarterTurn */

/* sub_4b7a60: native, nfs3hp::load::loadVivCarModel */

/* sub_4b7ba0: native, nfs3hp::load::copyVivCarModel */

/* sub_4b7c20: native, nfs3hp::frontend::loadingCopKeepLight */

/* sub_4b7c60: native, nfs3hp::frontend::loadingCopLoadModel */

/* sub_4b7d00: native, nfs3hp::frontend::rendFreePlayerModel */

/* sub_4b7d30: native, nfs3hp::render::carTextures */

/* sub_4b7eb0: native, nfs3hp::frontend::rendFreeCarModel */

/* sub_4b7f30: native, nfs3hp::dead::stub_0x4b7f30 */

/* sub_4b7f80: native, nfs3hp::frontend::rendShadowCorners */

/* sub_4b81a0: native, nfs3hp::frontend::rendCarShadow */

}
