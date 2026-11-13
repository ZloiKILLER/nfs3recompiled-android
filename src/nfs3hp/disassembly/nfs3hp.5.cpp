#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_41e3c0: native, nfs3hp::trackdata::nearestNodeCoarse */

/* sub_41e420: native, nfs3hp::trackdata::followNode */

/* sub_41e470: native, nfs3hp::trackdata::nearestNode */

/* sub_41e6a0: native, nfs3hp::trackdata::quadVertices */

/* sub_41e720: native, nfs3hp::trackdata::quadStepForward */

/* sub_41e770: native, nfs3hp::trackdata::quadStepBack */

/* sub_41e7c0: native, nfs3hp::trackdata::quadToFixed */

/* sub_41e8d0: native, nfs3hp::trackdata::quadSet */

/* sub_41e8f0: native, nfs3hp::trackdata::nodeSection */

/* sub_41e940: native, nfs3hp::trackdata::resetPosition */

/* sub_41e990: native, nfs3hp::trackdata::findQuad */

/* sub_41f090: native, nfs3hp::trackdata::findQuadNearby */

/* sub_41f330: native, nfs3hp::trackdata::findQuadByLinks */

/* sub_41f500: native, nfs3hp::trackdata::findQuadFallback */

/* sub_41f650: native, nfs3hp::trackdata::locateOnRoad */

/* sub_41f6e0: native, nfs3hp::trackdata::updatePosition */

/* sub_41f870: native, nfs3hp::trackdata::findMatchingQuad */

/* sub_41fd60: native, nfs3hp::trackdata::quadEdgeFlags */

/* sub_41ff10: native, nfs3hp::trackdata::sectionItemCount */

/* sub_41ff20: native, nfs3hp::trackdata::sectionItem */

/* sub_41ffa0: native, nfs3hp::trackdata::setLaneOffsets */

/* sub_41ffb0: native, nfs3hp::dead::stub_0x41ffb0 */

/* sub_41ffd0: native, nfs3hp::trackdata::lanePosition */

/* sub_420230: native, nfs3hp::trackdata::positionFrame */

/* sub_4205a0: native, nfs3hp::trackdata::snapToTrack */

/* sub_420650: native, nfs3hp::trackdata::placeOnTrack */

/* sub_4206f0: native, nfs3hp::trackdata::positionColour */

/* sub_4207e0: native, nfs3hp::trackdata::positionSurface */

/* sub_420840: native, nfs3hp::trackdata::surfaceUnder */

/* sub_420bf0: native, nfs3hp::trackdata::inTunnel */

/* sub_420d60: native, nfs3hp::trackdata::carFacingWay */

/* sub_420ff0: native, nfs3hp::port::view::sectionAheadIsTwo */

/* sub_4211c0: native, nfs3hp::trackdata::carSectionKind */

/* sub_421360: native, nfs3hp::carstate::idleHook */

/* sub_421370: native, nfs3hp::dead::stub_0x421370 */

/* sub_421380: native, nfs3hp::carstate::viewFromCamera */

/* sub_4213b0: native, nfs3hp::carstate::viewFromScreenCamera */

/* sub_421450: native, nfs3hp::carstate::chooseView */

/* sub_421480: native, nfs3hp::carstate::idleHookToo */

/* sub_421490: native, nfs3hp::carstate::pickChaseNode */

/* sub_421570: native, nfs3hp::carstate::turnAround */

/* sub_4215c0: native, nfs3hp::carstate::attachCameras */

/* sub_421730: native, nfs3hp::carstate::headTurn */

/* sub_4217e0: native, nfs3hp::carstate::cameraNode */

/* sub_421880: native, nfs3hp::carstate::cameraTrackPlace */

/* sub_421900: native, nfs3hp::carstate::keepOutOfCars */

/* sub_421b90: native, nfs3hp::carstate::keepAboveRoad */

/* sub_421cc0: native, nfs3hp::carstate::keepOnTrack */

/* sub_421e80: native, nfs3hp::carstate::keepFromRightWall */

/* sub_421f90: native, nfs3hp::carstate::keepFromLeftWall */

/* sub_4220a0: native, nfs3hp::carstate::keepAboveGround */

/* sub_422290: native, nfs3hp::carstate::lookAtTarget */

/* sub_4223c0: native, nfs3hp::carstate::blendCamera */

/* sub_422520: native, nfs3hp::carstate::placeCamera */

/* sub_422610: native, nfs3hp::carstate::switchCamera */

/* sub_4227e0: native, nfs3hp::carstate::cameraZoom */

/* sub_422850: native, nfs3hp::carstate::viewMatrix */

/* sub_4228e0: native, nfs3hp::carstate::updateCamera */

/* sub_4229c0: native, nfs3hp::carstate::cameraView */

/* sub_422aa0: native, nfs3hp::carstate::cockpitShown */

/* sub_422b70: native, nfs3hp::dead::stub_0x422b70 */

/* sub_422ba0: native, nfs3hp::carstate::cameraCar */

/* sub_422bd0: native, nfs3hp::carstate::cameraTarget */

/* sub_422c00: native, nfs3hp::pieces::stub_0x422c00 */

/* sub_422c40: native, nfs3hp::dead::stub_0x422c40 */

/* sub_422c80: native, nfs3hp::dead::stub_0x422c80 */

/* sub_422cf0: native, nfs3hp::carstate::initCameras */

/* sub_4238b0: native, nfs3hp::carstate::setCameraMode */

/* sub_423960: native, nfs3hp::carstate::setOverrideMode */

/* sub_423a20: native, nfs3hp::carstate::nextCycleMode */

/* sub_423a60: native, nfs3hp::carstate::buildCameraCycle */

/* sub_423ab0: native, nfs3hp::carstate::modeInCycle */

/* sub_423ae0: native, nfs3hp::carstate::rememberMode */

/* sub_423b00: native, nfs3hp::carstate::cameraMode */

/* sub_423b50: native, nfs3hp::carstate::setModeAnew */

/* sub_423b80: native, nfs3hp::carstate::settleCamera */

/* sub_423d90: native, nfs3hp::carstate::nearestSpot */

/* sub_423fb0: native, nfs3hp::carstate::scaledIfSet */

/* sub_423fe0: native, nfs3hp::carstate::bitsBroken */

/* sub_424030: native, nfs3hp::carstate::dampVector */

/* sub_4240c0: native, nfs3hp::carstate::chaseOffset */

/* sub_424260: native, nfs3hp::carstate::chaseOffsetLevel */

/* sub_4243a0: native, nfs3hp::frontend::simCameraChase */

/* sub_424690: native, nfs3hp::frontend::simCameraOrbit */

/* sub_424740: native, nfs3hp::frontend::simCameraChaseLow */

/* sub_424b50: native, nfs3hp::frontend::simCameraCircleCar */

/* sub_424ce0: native, nfs3hp::frontend::simCameraLookAround */

/* sub_424e30: native, nfs3hp::frontend::simCameraSideOfCar */

/* sub_424f30: native, nfs3hp::frontend::simCameraEaseTurn */

/* sub_424fd0: native, nfs3hp::frontend::simCameraMoveTowards */

/* sub_425060: native, nfs3hp::frontend::simCameraBesideRoad */

/* sub_4251f0: native, nfs3hp::frontend::simCameraNearestSpot */

/* sub_4253d0: native, nfs3hp::frontend::simCameraPlayPath */

/* sub_425640: native, nfs3hp::frontend::simCameraEndPath */

/* sub_425990: native, nfs3hp::frontend::simCameraBesideCar */

/* sub_425ab0: native, nfs3hp::frontend::simCameraOnCar */

/* sub_425b40: native, nfs3hp::frontend::simCameraGlideTowards */

/* sub_425bf0: native, nfs3hp::frontend::simCameraOverRoad */

/* sub_425d50: native, nfs3hp::frontend::simCameraFlyover */

/* sub_426290: native, nfs3hp::frontend::simCameraStill */

}
