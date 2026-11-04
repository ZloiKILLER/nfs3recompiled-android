#include "nfs3hp.h"
#include <lib/thread.h>

namespace nfs3hp
{

/* sub_4451d0: native, nfs3hp::port::dialogs::entryUnderMouse */

/* sub_4452d0: native, nfs3hp::port::dialogs::placeEntries */

/* sub_445380: native, nfs3hp::port::dialogs::drawList */

/* sub_445520: native, nfs3hp::port::dialogs::listKey */

/* sub_445680: native, nfs3hp::port::dialogs::listPass */

/* sub_445750: native, nfs3hp::port::dialogs::openList */

/* sub_445ac0: native, nfs3hp::port::dialogs::drawDialogHelp */

/* sub_445de0: native, nfs3hp::port::dialogs::loadDialogArt */

/* sub_445e60: native, nfs3hp::port::dialogs::dialogKey */

/* sub_445f00: native, nfs3hp::frontend::dialogFadedIn */

/* sub_445f10: native, nfs3hp::port::dialogs::closeDialog */

/* sub_445f50: native, nfs3hp::port::dialogs::dialogPass */

/* sub_446060: native, nfs3hp::load::readReplayInfo */

/* sub_4461d0: native, nfs3hp::frontend::loadSaveKnockoutInfo */

/* sub_4462e0: native, nfs3hp::frontend::loadSaveTournamentInfo */

/* sub_446420: native, nfs3hp::frontend::loadSaveGhostInfo */

/* sub_446510: native, nfs3hp::frontend::pauseSeriesSaveExists */

/* sub_446550: native, nfs3hp::frontend::pauseWriteSeriesSave */

/* sub_4465e0: native, nfs3hp::frontend::loadSaveLoadKnockout */

/* sub_4466a0: native, nfs3hp::frontend::loadSaveLoadGhost */

/* sub_446740: native, nfs3hp::frontend::pauseGhostExists */

/* sub_446780: native, nfs3hp::load::saveGhostAs */

/* sub_446800: native, nfs3hp::frontend::loadSaveLoadTournament */

/* sub_4468c0: native, nfs3hp::frontend::pauseWriteTournamentSave */

/* sub_446950: native, nfs3hp::frontend::pauseTournamentSaveExists */

/* sub_446990: native, nfs3hp::load::loadKeptReplay */

/* sub_446a10: native, nfs3hp::load::saveReplayAs */

/* sub_446a90: native, nfs3hp::frontend::pauseReplayExists */

/* sub_446ad0: native, nfs3hp::frontend::loadSaveShowTournaments */

/* sub_446b10: native, nfs3hp::frontend::loadSaveShowKnockouts */

/* sub_446b60: native, nfs3hp::frontend::loadSaveShowReplays */

/* sub_446ba0: native, nfs3hp::frontend::loadSaveShowGhosts */

/* sub_446bf0: native, nfs3hp::frontend::loadSaveLoadFailed */

/* sub_446c50: native, nfs3hp::frontend::loadSaveLoadNow */

/* sub_446ea0: native, nfs3hp::frontend::loadSaveDeleteSelected */

/* sub_446ff0: native, nfs3hp::frontend::loadSaveDeleteAnswered */

/* sub_447030: native, nfs3hp::frontend::loadSaveDeleteClicked */

/* sub_4470e0: native, nfs3hp::frontend::loadSaveDrawDescription */

/* sub_4471a0: native, nfs3hp::frontend::loadSaveSelectionMoved */

/* sub_447370: native, nfs3hp::frontend::loadSaveOnFrame */

/* sub_447470: native, nfs3hp::frontend::loadSaveOnEnter */

/* sub_447590: native, nfs3hp::frontend::loadSaveOnExit */

/* sub_4475a0: native, nfs3hp::load::listReplays */

/* sub_447800: native, nfs3hp::frontend::pauseOverwriteAnswered */

/* sub_4478c0: native, nfs3hp::frontend::pauseSaveNameAnswered */

/* sub_447a30: native, nfs3hp::frontend::pauseAskSaveName */

/* sub_447b40: native, nfs3hp::frontend::forceCancelKey */

/* sub_447b60: native, nfs3hp::frontend::forceStrengthMoved */

/* sub_447c40: native, nfs3hp::frontend::forceRoadEffectsMoved */

/* sub_447c70: native, nfs3hp::frontend::forceCollisionMoved */

/* sub_447ca0: native, nfs3hp::frontend::forceEngineMoved */

/* sub_447cd0: native, nfs3hp::frontend::forceBodyRollMoved */

/* sub_447d50: native, nfs3hp::frontend::forceRoadGripMoved */

/* sub_447dd0: native, nfs3hp::frontend::forceOnEnter */

/* sub_4481c0: native, nfs3hp::frontend::forceOnExit */

/* sub_448210: native, nfs3hp::frontend::screenModeListAdd */

/* sub_4482c0: native, nfs3hp::frontend::modesList */

/* sub_4482f0: native, nfs3hp::frontend::modesChosenWidth */

/* sub_448330: native, nfs3hp::frontend::modesCopyChosen */

/* sub_4483a0: native, nfs3hp::frontend::modesChooseByWidth */

/* sub_448400: native, nfs3hp::frontend::displayModeName */

/* sub_448430: native, nfs3hp::frontend::graphicsOnFrame */

/* sub_448440: native, nfs3hp::frontend::graphicsOnEnter */

/* sub_4484e0: native, nfs3hp::frontend::graphicsOnExit */

/* sub_448570: native, nfs3hp::frontend::advancedIntensityMoved */

/* sub_4485a0: native, nfs3hp::frontend::advancedOnFrame */

/* sub_448610: native, nfs3hp::frontend::advancedOnEnter */

/* sub_448770: native, nfs3hp::frontend::advancedOnExit */

/* sub_4487d0: native, nfs3hp::frontend::locationOnFrame */

/* sub_4487e0: native, nfs3hp::frontend::locationOnEnter */

/* sub_4489e0: native, nfs3hp::frontend::locationOnExit */

/* sub_448f00: native, nfs3hp::port::menus::keywordIndex */

/* sub_448f50: native, nfs3hp::port::menus::storeString */

/* sub_448fd0: native, nfs3hp::port::menus::widgetType */

/* sub_449020: native, nfs3hp::port::menus::widgetName */

/* sub_449060: native, nfs3hp::port::menus::initItem */

/* sub_4490e0: native, nfs3hp::port::menus::sectionType */

/* sub_449130: native, nfs3hp::port::menus::parseInt */

/* sub_449170: native, nfs3hp::port::menus::parseLong */

/* sub_4491b0: native, nfs3hp::port::menus::parseFloat */

/* sub_449240: native, nfs3hp::port::menus::setField */

/* sub_4493b0: native, nfs3hp::port::menus::readEntry */

/* sub_449510: native, nfs3hp::port::menus::loadMenus */

/* sub_4498a0: native, nfs3hp::port::menus::checkMenuFile */

/* sub_449960: native, nfs3hp::port::menus::checkMenuFiles */

/* sub_449990: native, nfs3hp::port::menus::callOnLoad */

/* sub_4499b0: native, nfs3hp::port::loop::itemUsable */

/* sub_4499f0: native, nfs3hp::port::loop::keyAllowed */

/* sub_449a60: native, nfs3hp::decompiled::resetMenuHelp */

/* sub_449aa0: native, nfs3hp::port::loop::drawHelp */

/* sub_449df0: native, nfs3hp::port::loop::keysTaken */

/* sub_449e50: native, nfs3hp::port::loop::selectable */

/* sub_449ea0: native, nfs3hp::port::loop::pointAtSelected */

/* sub_449f30: native, nfs3hp::port::loop::selectPrevious */

/* sub_44a0c0: native, nfs3hp::port::loop::selectNext */

/* sub_44a240: native, nfs3hp::port::loop::escapeButton */

/* sub_44a2d0: native, nfs3hp::port::loop::handleKey */

/* sub_44a440: native, nfs3hp::port::loop::selectedIndex */

/* sub_44a470: native, nfs3hp::port::loop::selectItem */

/* sub_44a4f0: native, nfs3hp::port::loop::drawItems */

}
