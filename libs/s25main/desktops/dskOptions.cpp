// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dskOptions.h"
#include "GlobalGameSettings.h"
#include "GlobalVars.h"
#include "Loader.h"
#include "MusicPlayer.h"
#include "Settings.h"
#include "WindowManager.h"
#include "controls/ctrlComboBox.h"
#include "controls/ctrlEdit.h"
#include "controls/ctrlGroup.h"
#include "controls/ctrlImageButton.h"
#include "controls/ctrlOptionGroup.h"
#include "controls/ctrlProgress.h"
#include "driver/VideoDriver.h"
#include "drivers/AudioDriverWrapper.h"
#include "drivers/VideoDriverWrapper.h"
#include "dskMainMenu.h"
#include "helpers/containerUtils.h"
#include "helpers/format.hpp"
#include "helpers/mathFuncs.h"
#include "helpers/toString.h"
#include "ingameWindows/iwAddons.h"
#include "ingameWindows/iwMsgbox.h"
#include "ingameWindows/iwMusicPlayer.h"
#include "ingameWindows/iwTextfile.h"
#include "languages.h"
#include "ogl/FontStyle.h"
#include "gameData/PortraitConsts.h"
#include "s25util/StringConversion.h"
#include "s25util/colors.h"
#include <mygettext/mygettext.h>
#include <sstream>

namespace {
enum
{
    ID_btBack = dskMenuBase::ID_FIRST_FREE,
    ID_txtOptions,
    ID_btAddons,
    ID_grpOptions,
    ID_btCommon,
    ID_btGraphics,
    ID_btSound,
    ID_grpCommon,
    ID_grpGraphics,
    ID_grpSound,
    ID_txtName,
    ID_edtName,
    ID_txtLanguage,
    ID_cbLanguage,
    ID_txtKeyboardLayout,
    ID_btKeyboardLayout,
    ID_txtPort,
    ID_edtPort,
    ID_txtIpv6,
    ID_grpIpv6,
    ID_txtProxy,
    ID_edtProxy,
    ID_edtProxyPort,
    ID_txtProxyType,
    ID_cbProxyType,
    ID_txtDebugData,
    ID_grpDebugData,
    ID_txtUPNP,
    ID_grpUPNP,
    ID_txtInvertScroll,
    ID_grpInvertScroll,
    ID_txtSmartCursor,
    ID_grpSmartCursor,
    ID_txtGFInfo,
    ID_grpGFInfo,
    ID_txtResolution,
    ID_cbResolution,
    ID_txtFullscreen,
    ID_grpFullscreen,
    ID_txtFramerate,
    ID_cbFramerate,
    ID_txtVBO,
    ID_grpVBO,
    ID_txtVideoDriver,
    ID_cbVideoDriver,
    ID_txtOptTextures,
    ID_grpOptTextures,
    ID_txtGuiScale,
    ID_cbGuiScale,
    ID_txtAudioDriver,
    ID_cbAudioDriver,
    ID_txtMusic,
    ID_grpMusic,
    ID_pgMusicVol,
    ID_txtEffects,
    ID_grpEffects,
    ID_pgEffectsVol,
    ID_btMusicPlayer,
    ID_txtCommonPortrait,
    ID_btCommonPortrait,
    ID_cbCommonPortrait,
};
// Use these as IDs in dedicated groups
constexpr auto ID_btOn = 1;
constexpr auto ID_btOff = 0;
// Special case: Submit debug data uses "2" for "ask user" and "0" for "unset, ask at start"
constexpr auto ID_btSubmitDebugOn = 1;
constexpr auto ID_btSubmitDebugAsk = 2;

constexpr auto rowHeight = 30;
constexpr auto sectionSpacing = 20;
constexpr auto sectionSpacingCommon = 10;
constexpr auto tabButtonsStartPosition = DrawPoint(80, 510);
constexpr auto optionRowsStartPosition = DrawPoint(80, 80);
} // namespace

static VideoMode getAspectRatio(const VideoMode& vm)
{
    // First some a bit off values where the aspect ratio is defined by convention
    if(vm == VideoMode(1360, 1024))
        return VideoMode(4, 3);
    else if(vm == VideoMode(1360, 768) || vm == VideoMode(1366, 768))
        return VideoMode(16, 9);

    // Normally Aspect ration is simply width/height as integer numbers (e.g. 4:3)
    int divisor = helpers::gcd(vm.width, vm.height);
    VideoMode ratio(vm.width / divisor, vm.height / divisor);
    // But there are some special cases:
    if(ratio == VideoMode(8, 5))
        return VideoMode(16, 10);
    else if(ratio == VideoMode(5, 3))
        return VideoMode(15, 9);
    else
        return ratio;
}

dskOptions::dskOptions() : Desktop(LOADER.GetImageN("setup013", 0))
{
    AddText(ID_txtOptions, DrawPoint(400, 10), _("Options"), COLOR_YELLOW, FontStyle::CENTER, LargeFont);

    ctrlOptionGroup* mainGroup = AddOptionGroup(ID_grpOptions, GroupSelectType::Check);

    DrawPoint curPos = tabButtonsStartPosition;
    mainGroup->AddTextButton(ID_btCommon, DrawPoint(curPos.x, curPos.y), Extent(200, 22), TextureColor::Green2,
                             _("Common"), NormalFont);
    mainGroup->AddTextButton(ID_btGraphics, DrawPoint(curPos.x + 220, curPos.y), Extent(200, 22), TextureColor::Green2,
                             _("Graphics"), NormalFont);
    mainGroup->AddTextButton(ID_btSound, DrawPoint(curPos.x + 440, curPos.y), Extent(200, 22), TextureColor::Green2,
                             _("Sound/Music"), NormalFont);
    curPos.y += rowHeight + sectionSpacingCommon;

    AddTextButton(ID_btBack, DrawPoint(curPos.x + 220, curPos.y), Extent(200, 22), TextureColor::Red1, _("Back"),
                  NormalFont);
    AddTextButton(ID_btAddons, DrawPoint(curPos.x + 440, curPos.y), Extent(200, 22), TextureColor::Green2, _("Addons"),
                  NormalFont);

    ctrlGroup* groupCommon = AddGroup(ID_grpCommon);
    ctrlGroup* groupGraphics = AddGroup(ID_grpGraphics);
    ctrlGroup* groupSound = AddGroup(ID_grpSound);
    ctrlComboBox* combo;

    // Common
    // {

    curPos = optionRowsStartPosition;
    using Offset = DrawPoint;
    constexpr Offset ctrlOffset(200, -5);                       // Offset of control to its description text
    constexpr Offset ctrlOffset2 = ctrlOffset + Offset(200, 0); // Offset of 2nd control to its description text
    constexpr Extent ctrlSize(190, 22);
    constexpr Extent ctrlSizeLarge = ctrlSize + Extent(ctrlOffset2 - ctrlOffset);

    groupCommon->AddText(ID_txtName, curPos, _("Name in Game:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    ctrlEdit* name =
      groupCommon->AddEdit(ID_edtName, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, NormalFont, 15);
    name->SetText(SETTINGS.lobby.name);

    const auto& currentPortrait = Portraits[SETTINGS.lobby.portraitIndex];
    groupCommon->AddImageButton(ID_btCommonPortrait, DrawPoint(500, curPos.y - 5), Extent(40, 54), TextureColor::Grey,
                                LOADER.GetImageN(currentPortrait.resourceId, currentPortrait.resourceIndex));
    curPos.y += rowHeight;

    groupCommon->AddText(ID_txtCommonPortrait, DrawPoint(80, curPos.y), _("Portrait:"), COLOR_YELLOW, FontStyle{},
                         NormalFont);
    combo = groupCommon->AddComboBox(ID_cbCommonPortrait, DrawPoint(280, curPos.y - 5), Extent(190, 20),
                                     TextureColor::Grey, NormalFont, 100);

    for(unsigned i = 0; i < Portraits.size(); ++i)
    {
        combo->AddString(_(Portraits[i].name));
        if(SETTINGS.lobby.portraitIndex == i)
        {
            combo->SetSelection(i);
        }
    }
    curPos.y += rowHeight;

    groupCommon->AddText(ID_txtLanguage, curPos, _("Language:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    combo = groupCommon->AddComboBox(ID_cbLanguage, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, NormalFont, 100);

    bool selected = false;
    for(unsigned i = 0; i < LANGUAGES.size(); ++i)
    {
        const Language& l = LANGUAGES.getLanguage(i);

        combo->AddString(_(l.name));
        if(SETTINGS.language.language == l.code)
        {
            combo->SetSelection(static_cast<unsigned short>(i));
            selected = true;
        }
    }
    if(!selected)
        combo->SetSelection(0);
    curPos.y += rowHeight;

    groupCommon->AddTextButton(ID_btKeyboardLayout, curPos + ctrlOffset, ctrlSizeLarge, TextureColor::Grey,
                               _("Keyboard layout"), NormalFont);
    curPos.y += rowHeight + sectionSpacingCommon;

    groupCommon->AddText(ID_txtPort, curPos, _("Local Port:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    ctrlEdit* edtPort =
      groupCommon->AddEdit(ID_edtPort, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, NormalFont, 15);
    edtPort->SetNumberOnly(true);
    edtPort->SetText(SETTINGS.server.localPort);
    curPos.y += rowHeight;

    // IPv4/6
    groupCommon->AddText(ID_txtIpv6, curPos, _("Use IPv6:"), COLOR_YELLOW, FontStyle{}, NormalFont);

    ctrlOptionGroup* ipv6 = groupCommon->AddOptionGroup(ID_grpIpv6, GroupSelectType::Check);
    ipv6->AddTextButton(ID_btOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("IPv6"), NormalFont);
    ipv6->AddTextButton(ID_btOff, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("IPv4"), NormalFont);
    ipv6->SetSelection(SETTINGS.server.ipv6);
    // ipv6-feld ggf (de-)aktivieren
    ipv6->GetCtrl<ctrlButton>(1)->SetEnabled(SETTINGS.proxy.type != ProxyType::Socks5); //-V807
    curPos.y += rowHeight + sectionSpacingCommon;

    // Proxyserver
    groupCommon->AddText(ID_txtProxy, curPos, _("Proxyserver:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    ctrlEdit* proxy = groupCommon->AddEdit(ID_edtProxy, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, NormalFont);
    proxy->SetText(SETTINGS.proxy.hostname);
    proxy =
      groupCommon->AddEdit(ID_edtProxyPort, curPos + ctrlOffset2, Extent(50, 22), TextureColor::Grey, NormalFont, 5);
    proxy->SetNumberOnly(true);
    proxy->SetText(SETTINGS.proxy.port);
    curPos.y += rowHeight;

    groupCommon->AddText(ID_txtUPNP, curPos, _("Use UPnP"), COLOR_YELLOW, FontStyle{}, NormalFont);
    ctrlOptionGroup* upnp = groupCommon->AddOptionGroup(ID_grpUPNP, GroupSelectType::Check);
    upnp->AddTextButton(ID_btOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("On"), NormalFont);
    upnp->AddTextButton(ID_btOff, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("Off"), NormalFont);
    upnp->SetSelection(SETTINGS.global.use_upnp);
    curPos.y += rowHeight;

    // Proxytyp
    groupCommon->AddText(ID_txtProxyType, curPos, _("Proxytyp:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    combo =
      groupCommon->AddComboBox(ID_cbProxyType, curPos + ctrlOffset, ctrlSizeLarge, TextureColor::Grey, NormalFont, 100);
    combo->AddString(_("No Proxy"));
    combo->AddString(_("Socks v4"));
    // TODO: not implemented
    // combo->AddString(_("Socks v5"));

    switch(SETTINGS.proxy.type)
    {
        default: combo->SetSelection(0); break;
        case ProxyType::Socks4: combo->SetSelection(1); break;
        case ProxyType::Socks5: combo->SetSelection(2); break;
    }
    curPos.y += rowHeight + sectionSpacingCommon;

    groupCommon->AddText(ID_txtInvertScroll, curPos, _("Invert Mouse Pan:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    ctrlOptionGroup* invertScroll = groupCommon->AddOptionGroup(ID_grpInvertScroll, GroupSelectType::Check);
    invertScroll->AddTextButton(ID_btOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("On"), NormalFont,
                                _("Map moves in the opposite direction the mouse is moved when scrolling/panning."));
    invertScroll->AddTextButton(ID_btOff, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("Off"), NormalFont,
                                _("Map moves in the same direction the mouse is moved when scrolling/panning."));
    invertScroll->SetSelection(SETTINGS.interface.invertMouse);
    curPos.y += rowHeight;

    groupCommon->AddText(ID_txtSmartCursor, curPos, _("Smart Cursor"), COLOR_YELLOW, FontStyle{}, NormalFont);
    ctrlOptionGroup* smartCursor = groupCommon->AddOptionGroup(ID_grpSmartCursor, GroupSelectType::Check);
    smartCursor->AddTextButton(ID_btOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("On"), NormalFont,
                               _("Place cursor on default button for new dialogs / action windows (default)"));
    smartCursor->AddTextButton(
      ID_btOff, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("Off"), NormalFont,
      _("Don't move cursor automatically\nUseful e.g. for split-screen / dual-mice multiplayer (see wiki)"));
    smartCursor->SetSelection(SETTINGS.global.smartCursor);
    curPos.y += rowHeight + sectionSpacingCommon;

    groupCommon->AddText(ID_txtDebugData, curPos, _("Submit debug data:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    mainGroup = groupCommon->AddOptionGroup(ID_grpDebugData, GroupSelectType::Check);
    mainGroup->AddTextButton(ID_btSubmitDebugOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("On"),
                             NormalFont);
    mainGroup->AddTextButton(ID_btSubmitDebugAsk, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("Ask always"),
                             NormalFont);

    mainGroup->SetSelection((SETTINGS.global.submit_debug_data == 1) ? ID_btSubmitDebugOn :
                                                                       ID_btSubmitDebugAsk); //-V807
    curPos.y += rowHeight;

    groupCommon->AddText(ID_txtGFInfo, curPos, _("Show GameFrame Info:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    mainGroup = groupCommon->AddOptionGroup(ID_grpGFInfo, GroupSelectType::Check);
    mainGroup->AddTextButton(ID_btOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("On"), NormalFont);
    mainGroup->AddTextButton(ID_btOff, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("Off"), NormalFont);

    mainGroup->SetSelection(SETTINGS.global.showGFInfo);

    curPos = optionRowsStartPosition;
    groupGraphics->AddText(ID_txtResolution, curPos, _("Fullscreen resolution:"), COLOR_YELLOW, FontStyle{},
                           NormalFont);
    groupGraphics->AddComboBox(ID_cbResolution, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, NormalFont, 150);
    curPos.y += rowHeight + sectionSpacing;

    groupGraphics->AddText(ID_txtFullscreen, curPos, _("Mode:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    mainGroup = groupGraphics->AddOptionGroup(ID_grpFullscreen, GroupSelectType::Check);
    mainGroup->AddTextButton(ID_btOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("Fullscreen"), NormalFont);
    mainGroup->AddTextButton(ID_btOff, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("Windowed"), NormalFont);
    curPos.y += rowHeight + sectionSpacing;

    groupGraphics->AddText(ID_txtFramerate, curPos, _("Limit Framerate:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    groupGraphics->AddComboBox(ID_cbFramerate, curPos + ctrlOffset, ctrlSizeLarge, TextureColor::Grey, NormalFont, 150);
    curPos.y += rowHeight + sectionSpacing;

    groupGraphics->AddText(ID_txtVBO, curPos, _("Vertex Buffer Objects:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    mainGroup = groupGraphics->AddOptionGroup(ID_grpVBO, GroupSelectType::Check);
    mainGroup->AddTextButton(ID_btOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("On"), NormalFont);
    mainGroup->AddTextButton(ID_btOff, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("Off"), NormalFont);
    curPos.y += rowHeight + sectionSpacing;

    groupGraphics->AddText(ID_txtVideoDriver, curPos, _("Graphics Driver"), COLOR_YELLOW, FontStyle{}, NormalFont);
    combo = groupGraphics->AddComboBox(ID_cbVideoDriver, curPos + ctrlOffset, ctrlSizeLarge, TextureColor::Grey,
                                       NormalFont, 100);

    const auto video_drivers = drivers::DriverWrapper::LoadDriverList(drivers::DriverType::Video);

    for(const auto& video_driver : video_drivers)
    {
        combo->AddString(video_driver.GetName());
        if(video_driver.GetName() == SETTINGS.driver.video)
            combo->SetSelection(combo->GetNumItems() - 1);
    }
    curPos.y += rowHeight + sectionSpacing;

    groupGraphics->AddText(ID_txtOptTextures, curPos, _("Optimized Textures:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    mainGroup = groupGraphics->AddOptionGroup(ID_grpOptTextures, GroupSelectType::Check);

    mainGroup->AddTextButton(ID_btOn, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("On"), NormalFont);
    mainGroup->AddTextButton(ID_btOff, curPos + ctrlOffset2, ctrlSize, TextureColor::Grey, _("Off"), NormalFont);
    curPos.y += rowHeight + sectionSpacing;

    groupGraphics->AddText(ID_txtGuiScale, curPos, _("GUI Scale:"), COLOR_YELLOW, FontStyle{}, NormalFont);
    groupGraphics->AddComboBox(ID_cbGuiScale, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, NormalFont, 100);
    updateGuiScale();

    curPos = optionRowsStartPosition;
    constexpr Offset bt1Offset(200, -5);
    constexpr Offset bt2Offset(300, -5);
    constexpr Offset volOffset(400, -5);
    constexpr Extent ctrlSizeSmall(90, ctrlSize.y);

    groupSound->AddText(ID_txtMusic, curPos, _("Music"), COLOR_YELLOW, FontStyle{}, NormalFont);
    mainGroup = groupSound->AddOptionGroup(ID_grpMusic, GroupSelectType::Check);
    mainGroup->AddTextButton(ID_btOn, curPos + bt1Offset, ctrlSizeSmall, TextureColor::Grey, _("On"), NormalFont);
    mainGroup->AddTextButton(ID_btOff, curPos + bt2Offset, ctrlSizeSmall, TextureColor::Grey, _("Off"), NormalFont);

    ctrlProgress* Mvolume =
      groupSound->AddProgress(ID_pgMusicVol, curPos + volOffset, ctrlSize, TextureColor::Grey, 139, 138, 100);
    Mvolume->SetPosition((SETTINGS.sound.musicVolume * 100) / 255); //-V807
    curPos.y += rowHeight + sectionSpacing;

    groupSound->AddText(ID_txtEffects, curPos, _("Effects"), COLOR_YELLOW, FontStyle{}, NormalFont);
    mainGroup = groupSound->AddOptionGroup(ID_grpEffects, GroupSelectType::Check);
    mainGroup->AddTextButton(ID_btOn, curPos + bt1Offset, ctrlSizeSmall, TextureColor::Grey, _("On"), NormalFont);
    mainGroup->AddTextButton(ID_btOff, curPos + bt2Offset, ctrlSizeSmall, TextureColor::Grey, _("Off"), NormalFont);

    ctrlProgress* FXvolume =
      groupSound->AddProgress(ID_pgEffectsVol, curPos + volOffset, ctrlSize, TextureColor::Grey, 139, 138, 100);
    FXvolume->SetPosition((SETTINGS.sound.effectsVolume * 100) / 255);
    curPos.y += rowHeight + sectionSpacing;

    groupSound->AddTextButton(ID_btMusicPlayer, curPos + ctrlOffset, ctrlSize, TextureColor::Grey, _("Music player"),
                              NormalFont);
    curPos.y += rowHeight + sectionSpacing;

    groupSound->AddText(ID_txtAudioDriver, curPos, _("Sounddriver"), COLOR_YELLOW, FontStyle{}, NormalFont);
    combo = groupSound->AddComboBox(ID_cbAudioDriver, curPos + ctrlOffset, ctrlSizeLarge, TextureColor::Grey,
                                    NormalFont, 100);

    const auto audio_drivers = drivers::DriverWrapper::LoadDriverList(drivers::DriverType::Audio);

    for(const auto& audio_driver : audio_drivers)
    {
        combo->AddString(audio_driver.GetName());
        if(audio_driver.GetName() == SETTINGS.driver.audio)
            combo->SetSelection(combo->GetNumItems() - 1);
    }

    // "Allgemein" auswählen
    mainGroup = GetCtrl<ctrlOptionGroup>(ID_grpOptions);
    mainGroup->SetSelection(ID_btCommon, true);

    // Graphics
    // {

    loadVideoModes();

    // Und zu der Combobox hinzufügen
    ctrlComboBox& cbVideoModes = *groupGraphics->GetCtrl<ctrlComboBox>(ID_cbResolution);
    for(const auto& videoMode : video_modes)
    {
        VideoMode ratio = getAspectRatio(videoMode);
        s25util::ClassicImbuedStream<std::ostringstream> str;
        str << videoMode.width << "x" << videoMode.height;
        // Make the length always the same as 'iiiixiiii' to align the ratio
        int len = str.str().length();
        for(int i = len; i < 4 + 1 + 4; i++)
            str << " ";
        str << " (" << ratio.width << ":" << ratio.height << ")";

        cbVideoModes.AddString(str.str());

        // Ist das die aktuelle Auflösung? Dann selektieren
        if(videoMode == SETTINGS.video.fullscreenSize) //-V807
            cbVideoModes.SetSelection(cbVideoModes.GetNumItems() - 1);
    }

    // "Vollbild" setzen
    groupGraphics->GetCtrl<ctrlOptionGroup>(ID_grpFullscreen)->SetSelection(SETTINGS.video.fullscreen); //-V807

    // "Limit Framerate" füllen
    auto* cbFrameRate = groupGraphics->GetCtrl<ctrlComboBox>(ID_cbFramerate);
    if(VIDEODRIVER.HasVSync())
        cbFrameRate->AddString(_("Dynamic (Limits to display refresh rate, works with most drivers)"));
    for(int framerate : Settings::SCREEN_REFRESH_RATES)
    {
        if(framerate == -1)
            cbFrameRate->AddString(_("Disabled"));
        else
            cbFrameRate->AddString(helpers::toString(framerate) + " FPS");
        if(SETTINGS.video.framerate == framerate)
            cbFrameRate->SetSelection(cbFrameRate->GetNumItems() - 1);
    }
    if(!cbFrameRate->GetSelection())
        cbFrameRate->SetSelection(0);

    groupGraphics->GetCtrl<ctrlOptionGroup>(ID_grpVBO)->SetSelection(SETTINGS.video.vbo);

    groupGraphics->GetCtrl<ctrlOptionGroup>(ID_grpOptTextures)->SetSelection(SETTINGS.video.shared_textures);
    // }

    // Sound
    // {

    groupSound->GetCtrl<ctrlOptionGroup>(ID_grpMusic)->SetSelection(SETTINGS.sound.musicEnabled);
    groupSound->GetCtrl<ctrlOptionGroup>(ID_grpEffects)->SetSelection(SETTINGS.sound.effectsEnabled);

    // }

    // Load game settings
    ggs.LoadSettings();
}

dskOptions::~dskOptions()
{
    // Save game settings
    ggs.SaveSettings();
}

void dskOptions::Msg_Group_ProgressChange(const unsigned /*group_id*/, const unsigned ctrl_id,
                                          const unsigned short position)
{
    switch(ctrl_id)
    {
        case ID_pgEffectsVol:
            SETTINGS.sound.effectsVolume = static_cast<uint8_t>((position * 255) / 100);
            AUDIODRIVER.SetMasterEffectVolume(SETTINGS.sound.effectsVolume);
            break;
        case ID_pgMusicVol:
            SETTINGS.sound.musicVolume = static_cast<uint8_t>((position * 255) / 100);
            AUDIODRIVER.SetMusicVolume(SETTINGS.sound.musicVolume);
            break;
    }
}

void dskOptions::Msg_Group_ComboSelectItem(const unsigned group_id, const unsigned ctrl_id, const unsigned selection)
{
    auto* group = GetCtrl<ctrlGroup>(group_id);
    auto* combo = group->GetCtrl<ctrlComboBox>(ctrl_id);

    switch(ctrl_id)
    {
        case ID_cbCommonPortrait:
            SETTINGS.lobby.portraitIndex = selection;
            updatePortraitControls();
            break;
        case ID_cbLanguage:
        {
            // Language changed?
            std::string old_lang = SETTINGS.language.language; //-V807
            SETTINGS.language.language = LANGUAGES.setLanguage(selection);
            if(SETTINGS.language.language != old_lang)
                WINDOWMANAGER.Switch(std::make_unique<dskOptions>());
        }
        break;
        case ID_cbProxyType:
            switch(selection)
            {
                case 0: SETTINGS.proxy.type = ProxyType::None; break;
                case 1: SETTINGS.proxy.type = ProxyType::Socks4; break;
                case 2: SETTINGS.proxy.type = ProxyType::Socks5; break;
            }

            // ipv6 gleich sichtbar deaktivieren
            if(SETTINGS.proxy.type == ProxyType::Socks4 && SETTINGS.server.ipv6)
            {
                GetCtrl<ctrlGroup>(ID_grpCommon)->GetCtrl<ctrlOptionGroup>(ID_grpIpv6)->SetSelection(0);
                GetCtrl<ctrlGroup>(ID_grpCommon)
                  ->GetCtrl<ctrlOptionGroup>(ID_grpIpv6)
                  ->GetCtrl<ctrlButton>(1)
                  ->SetEnabled(false);
                SETTINGS.server.ipv6 = false;
            }

            if(SETTINGS.proxy.type != ProxyType::Socks4)
                GetCtrl<ctrlGroup>(ID_grpCommon)
                  ->GetCtrl<ctrlOptionGroup>(ID_grpIpv6)
                  ->GetCtrl<ctrlButton>(1)
                  ->SetEnabled(true);
            break;
        case ID_cbResolution: SETTINGS.video.fullscreenSize = video_modes[selection]; break;
        case ID_cbFramerate:
            if(VIDEODRIVER.HasVSync())
            {
                if(selection == 0)
                    SETTINGS.video.framerate = 0;
                else
                    SETTINGS.video.framerate = Settings::SCREEN_REFRESH_RATES[selection - 1];
            } else
                SETTINGS.video.framerate = Settings::SCREEN_REFRESH_RATES[selection];

            VIDEODRIVER.setTargetFramerate(SETTINGS.video.framerate);
            break;
        case ID_cbVideoDriver: SETTINGS.driver.video = combo->GetText(selection); break;
        case ID_cbGuiScale:
            SETTINGS.video.guiScale = guiScales_[selection];
            VIDEODRIVER.setGuiScalePercent(SETTINGS.video.guiScale);
            break;
        case ID_cbAudioDriver: SETTINGS.driver.audio = combo->GetText(selection); break;
    }
}

void dskOptions::Msg_Group_OptionGroupChange(const unsigned /*group_id*/, const unsigned ctrl_id,
                                             const unsigned selection)
{
    const bool enabled = selection == ID_btOn;
    switch(ctrl_id)
    {
        case ID_grpIpv6: SETTINGS.server.ipv6 = enabled; break;
        case ID_grpFullscreen: SETTINGS.video.fullscreen = enabled; break;
        case ID_grpVBO: SETTINGS.video.vbo = enabled; break;
        case ID_grpOptTextures: SETTINGS.video.shared_textures = enabled; break;
        case ID_grpMusic:
            SETTINGS.sound.musicEnabled = enabled;
            if(enabled)
                MUSICPLAYER.Play();
            else
                MUSICPLAYER.Stop();
            break;
        case ID_grpEffects: SETTINGS.sound.effectsEnabled = enabled; break;
        case ID_grpDebugData:
            // Special case: Uses e.g. ID_btSubmitDebugOn directly
            SETTINGS.global.submit_debug_data = selection;
            break;
        case ID_grpUPNP: SETTINGS.global.use_upnp = enabled; break;
        case ID_grpInvertScroll: SETTINGS.interface.invertMouse = enabled; break;
        case ID_grpSmartCursor:
            SETTINGS.global.smartCursor = enabled;
            VIDEODRIVER.SetMouseWarping(enabled);
            break;
        case ID_grpGFInfo: SETTINGS.global.showGFInfo = enabled; break;
    }
}

void dskOptions::Msg_OptionGroupChange(const unsigned ctrl_id, const unsigned selection)
{
    if(ctrl_id == ID_grpOptions)
    {
        const auto visGrp = selection + ID_grpCommon - ID_btCommon;
        for(const unsigned id : {ID_grpCommon, ID_grpGraphics, ID_grpSound})
            GetCtrl<ctrlGroup>(id)->SetVisible(id == visGrp);
    }
}

/// Check that the port is valid and sets outPort to it. Shows an error otherwise
static bool validatePort(const std::string& sPort, uint16_t& outPort)
{
    boost::optional<uint16_t> port = validate::checkPort(sPort);
    if(port)
        outPort = *port;
    else
    {
        WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(_("Error"),
                                                      _("Invalid port. The valid port-range is 1 to 65535!"), nullptr,
                                                      MsgboxButton::Ok, MsgboxIcon::ExclamationRed, 1));
    }
    return static_cast<bool>(port);
}

void dskOptions::Msg_ButtonClick(const unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btBack:
        {
            auto* groupCommon = GetCtrl<ctrlGroup>(ID_grpCommon);

            // Name abspeichern
            SETTINGS.lobby.name = groupCommon->GetCtrl<ctrlEdit>(ID_edtName)->GetText();
            if(!validatePort(groupCommon->GetCtrl<ctrlEdit>(ID_edtPort)->GetText(), SETTINGS.server.localPort))
                return;

            SETTINGS.proxy.hostname = groupCommon->GetCtrl<ctrlEdit>(ID_edtProxy)->GetText();
            if(!validatePort(groupCommon->GetCtrl<ctrlEdit>(ID_edtProxyPort)->GetText(), SETTINGS.proxy.port))
                return;

            SETTINGS.Save();

            // Is the selected backend required to support GUI scaling to fullfill the user's choice?
            // If so, warn the user if the backend is unable to support GUI scaling.
            if(VIDEODRIVER.getGuiScale().percent() == 100
               && (SETTINGS.video.guiScale != 100
                   || (SETTINGS.video.guiScale == 0 && VIDEODRIVER.getGuiScaleRange().recommendedPercent != 100)))
            {
                WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(
                  _("Sorry!"), _("The selected video driver does not support GUI scaling! Setting won't be used."),
                  this, MsgboxButton::Ok, MsgboxIcon::ExclamationGreen, 1));
            }

            if((SETTINGS.video.fullscreen && SETTINGS.video.fullscreenSize != VIDEODRIVER.GetWindowSize()) //-V807
               || SETTINGS.video.fullscreen != VIDEODRIVER.IsFullscreen())
            {
                const auto screenSize =
                  SETTINGS.video.fullscreen ? SETTINGS.video.fullscreenSize : SETTINGS.video.windowedSize;
                if(!VIDEODRIVER.ResizeScreen(screenSize, SETTINGS.video.fullscreen))
                {
                    WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(
                      _("Sorry!"), _("You need to restart your game to change the screen resolution!"), this,
                      MsgboxButton::Ok, MsgboxIcon::ExclamationGreen, 1));
                    return;
                }
            }
            if(SETTINGS.driver.video != VIDEODRIVER.GetName() || SETTINGS.driver.audio != AUDIODRIVER.GetName())
            {
                WINDOWMANAGER.Show(std::make_unique<iwMsgbox>(
                  _("Sorry!"), _("You need to restart your game to change the video or audio driver!"), this,
                  MsgboxButton::Ok, MsgboxIcon::ExclamationGreen, 1));
                return;
            }

            WINDOWMANAGER.Switch(std::make_unique<dskMainMenu>());
        }
        break;
        case ID_btAddons: WINDOWMANAGER.ToggleWindow(std::make_unique<iwAddons>(ggs)); break;
    }
}

void dskOptions::Msg_Group_ButtonClick(const unsigned /*group_id*/, const unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        default: break;
        case ID_btCommonPortrait:
            SETTINGS.lobby.portraitIndex = (SETTINGS.lobby.portraitIndex + 1) % Portraits.size();
            updatePortraitControls();
            break;
        case ID_btMusicPlayer: WINDOWMANAGER.ToggleWindow(std::make_unique<iwMusicPlayer>()); break;
        case ID_btKeyboardLayout:
            WINDOWMANAGER.ToggleWindow(std::make_unique<iwTextfile>("keyboardlayout.txt", _("Keyboard layout")));
            break;
    }
}

void dskOptions::Msg_MsgBoxResult(const unsigned msgbox_id, const MsgboxResult /*mbr*/)
{
    switch(msgbox_id)
    {
        default: break;
        // "You need to restart your game ..."
        // "The selected video driver does not support GUI scaling!"
        case 1: WINDOWMANAGER.Switch(std::make_unique<dskMainMenu>()); break;
    }
}

static bool cmpVideoModes(const VideoMode& left, const VideoMode& right)
{
    if(left.width == right.width)
        return left.height > right.height;
    else
        return left.width > right.width;
}

void dskOptions::loadVideoModes()
{
    // Get available modes
    VIDEODRIVER.ListVideoModes(video_modes);
    // Remove everything below 800x600
    helpers::erase_if(video_modes, [](const auto& it) { return it.width < 800 && it.height < 600; });
    // Sort by aspect ratio
    std::sort(video_modes.begin(), video_modes.end(), cmpVideoModes);
}

void dskOptions::Msg_ScreenResize(const ScreenResizeEvent& sr)
{
    Desktop::Msg_ScreenResize(sr);
    updateGuiScale();
}

bool dskOptions::Msg_WheelUp(const MouseCoords& mc)
{
    if(VIDEODRIVER.GetModKeyState().ctrl)
    {
        scrollGuiScale(true);
        return true;
    } else
        return Desktop::Msg_WheelUp(mc);
}

bool dskOptions::Msg_WheelDown(const MouseCoords& mc)
{
    if(VIDEODRIVER.GetModKeyState().ctrl)
    {
        scrollGuiScale(false);
        return true;
    } else
        return Desktop::Msg_WheelDown(mc);
}

void dskOptions::updateGuiScale()
{
    // generate GUI scale percentages in 10% increments
    constexpr auto stepSize = 10u;
    const auto roundGuiScale = [=](unsigned percent) {
        return helpers::iround<unsigned>(static_cast<float>(percent) / stepSize) * stepSize;
    };

    const auto range = VIDEODRIVER.getGuiScaleRange();
    const auto recommendedPercentRounded = roundGuiScale(range.recommendedPercent);
    auto* combo = GetCtrl<ctrlGroup>(ID_grpGraphics)->GetCtrl<ctrlComboBox>(ID_cbGuiScale);

    guiScales_.clear();
    combo->DeleteAllItems();

    guiScales_.push_back(0);
    combo->AddString(helpers::format(_("Auto (%u%%)"), range.recommendedPercent));
    if(SETTINGS.video.guiScale == 0)
        combo->SetSelection(0);

    for(unsigned percent = roundGuiScale(range.minPercent); percent <= range.maxPercent; percent += stepSize)
    {
        if(percent == recommendedPercentRounded)
            recommendedGuiScaleIndex_ = guiScales_.size();
        guiScales_.push_back(percent);

        combo->AddString(helpers::toString(percent) + "%");
        if(percent == SETTINGS.video.guiScale)
            combo->SetSelection(combo->GetNumItems() - 1);
    }

    // if GUI scale exceeds maximum, lower it to keep UI elements on screen
    if(SETTINGS.video.guiScale > guiScales_.back())
    {
        combo->SetSelection(combo->GetNumItems() - 1);
        SETTINGS.video.guiScale = guiScales_.back();
        VIDEODRIVER.setGuiScalePercent(SETTINGS.video.guiScale);
    }
}

void dskOptions::scrollGuiScale(bool up)
{
    auto* combo = GetCtrl<ctrlGroup>(ID_grpGraphics)->GetCtrl<ctrlComboBox>(ID_cbGuiScale);
    const auto& selection = combo->GetSelection();
    unsigned newSelection = 0;
    if(!selection || *selection == 0) // No selection or "Auto" item selected
        newSelection = recommendedGuiScaleIndex_;
    else
        newSelection = std::clamp<unsigned>(*selection + (up ? 1 : -1), 1, combo->GetNumItems() - 1);

    if(newSelection != selection)
    {
        combo->SetSelection(newSelection);
        SETTINGS.video.guiScale = guiScales_[newSelection];
        VIDEODRIVER.setGuiScalePercent(SETTINGS.video.guiScale);
    }
}

void dskOptions::updatePortraitControls()
{
    const auto& newPortrait = Portraits[SETTINGS.lobby.portraitIndex];
    auto* groupCommon = GetCtrl<ctrlGroup>(ID_grpCommon);

    auto* portraitButton = groupCommon->GetCtrl<ctrlImageButton>(ID_btCommonPortrait);
    auto* newPortraitTexture = LOADER.GetTextureN(newPortrait.resourceId, newPortrait.resourceIndex);
    portraitButton->SetImage(newPortraitTexture);

    auto* portraitCombo = groupCommon->GetCtrl<ctrlComboBox>(ID_cbCommonPortrait);
    portraitCombo->SetSelection(SETTINGS.lobby.portraitIndex);
}
