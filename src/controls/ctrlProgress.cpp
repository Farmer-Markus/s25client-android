// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "defines.h" // IWYU pragma: keep
#include "ctrlProgress.h"

#include "WindowManager.h"
#include "Loader.h"
#include "ogl/glArchivItem_Font.h"
#include "CollisionDetection.h"

#include <sstream>

ctrlProgress::ctrlProgress(Window* parent,
                           const unsigned int id,
                           const unsigned short x,
                           const unsigned short y,
                           const unsigned short width,
                           const unsigned short height,
                           const TextureColor tc,
                           unsigned short button_minus,
                           unsigned short button_plus,
                           const unsigned short maximum,
                           const unsigned short x_padding,
                           const unsigned short y_padding,
                           const unsigned int force_color,
                           const std::string& tooltip,
                           const std::string& button_minus_tooltip,
                           const std::string& button_plus_tooltip,
                           unsigned short* const  /*write_val*/)
    : Window(DrawPoint(x, y), id, parent, width, height, tooltip),
      tc(tc), position(0), maximum(maximum), padding(x_padding, y_padding), force_color(force_color)
{
    const char* str1 = "io", * str2 = "io";
    if(button_minus >= 1000)
    {
        str1 = "io_new";
        button_minus -= 1000;
    }
    if(button_plus >= 1000)
    {
        str2 = "io_new";
        button_plus -= 1000;
    }

    AddImageButton(0, 0,              0, height, height, tc, LOADER.GetImageN(str1, button_minus), (button_minus_tooltip.length() ? button_minus_tooltip : _("Less")) );
    AddImageButton(1, width - height, 0, height, height, tc, LOADER.GetImageN(str2, button_plus),  (button_plus_tooltip.length( ) ? button_plus_tooltip  : _("More")) );

    // Hide left and right 3D border by making the buttons overlap the bar
    padding.x -= 2;
}

/**
 *  setzt die Position an den angegebenen Wert.
 */
void ctrlProgress::SetPosition(unsigned short position)
{
    this->position = (position > maximum ? maximum : position);
}

/**
 *  Zeichenmethode.
 *
 *  @return @p true bei Erfolg, @p false bei Fehler
 */
void ctrlProgress::Draw_()
{
    DrawPoint barPos = GetDrawPos() + padding;
    // Offset by button size
    barPos.x += height_;
    Draw3D(barPos, CalcBarWidth(), height_ - 2 * padding.y, tc, 2);

    // Buttons
    DrawControls();

    const DrawPoint innerPadding(4, 4);
    unsigned int percentage = position * 100 / maximum;
    unsigned int progress = (CalcBarWidth() - innerPadding.x * 2) * position / maximum;

    // Farbe herausfinden
    unsigned int color = 0xFFD70000;

    // Feste Farbe?
    if(force_color)
        color = force_color;
    else
    {
        // Farbe wählen je nachdem wie viel Prozent
        if(percentage >= 60)
            color = 0xFF71B63C;
        else if(percentage >= 30)
            color = 0xFFFFBF33;
        else if(percentage >= 20)
            color = 0xFFDB7428;
    }

    // Leiste
    DrawRectangle(barPos + innerPadding, progress, height_ - 2 * (innerPadding.y + padding.y), color);

    // Prozentzahlen zeichnen
    std::stringstream percent;
    percent << percentage << "%";
    SmallFont->Draw(GetDrawPos() + DrawPoint(width_, height_) / 2, percent.str(), glArchivItem_Font::DF_VCENTER | glArchivItem_Font::DF_CENTER, COLOR_YELLOW);
}

void ctrlProgress::Resize(unsigned short width, unsigned short height)
{
    Window::Resize(width, height);

    Window* lessBt = GetCtrl<Window>(0);
    Window* moreBt = GetCtrl<Window>(1);
    lessBt->Resize(height, height);
    moreBt->Resize(height, height);
    moreBt->Move(width - height, 0);
}

unsigned ctrlProgress::CalcBarWidth() const
{
    return width_ - 2 * (padding.x + height_);
}

void ctrlProgress::Msg_ButtonClick(const unsigned int ctrl_id)
{
    switch(ctrl_id)
    {
        case 0: // Minus
        {
            if(position)
                --position;
            if(parent_)
                parent_->Msg_ProgressChange(GetID(), position);
        } break;
        case 1: // Plus
        {
            if(position < maximum)
                ++position;
            if(parent_)
                parent_->Msg_ProgressChange(GetID(), position);
        } break;
    }
}

bool ctrlProgress::Msg_LeftDown(const MouseCoords& mc)
{
    // Test if clicked on progress bar
    if(IsPointInRect(mc.x, mc.y,
            GetX() + height_ + 2 + padding.x, GetY() + 4 + padding.y,
            width_ - height_ * 2 - 4 - 2 * padding.x, height_ - 8 - 2 * padding.y))
    {
        // The additional check for (position > maximum) is
        // mathematically redundant here; if there was more code than
        // it in SetPosition() we had to call it here instead of simply:
        position = ( mc.x - (GetX() + height_ + 2 + padding.x)
                     + /*rounding:*/ (width_ - height_ * 2 - 4 - 2 * padding.x) / maximum / 2)
                   * maximum / (width_ - height_ * 2 - 4 - 2 * padding.x);

        if(parent_) parent_->Msg_ProgressChange(GetID(), position);
        return true;
    }
    else
        return RelayMouseMessage(&Window::Msg_LeftDown, mc);
}

bool ctrlProgress::Msg_LeftUp(const MouseCoords& mc)
{
    return RelayMouseMessage(&Window::Msg_LeftUp, mc);
}

bool ctrlProgress::Msg_WheelUp(const MouseCoords& mc)
{
    // If mouse is over the controls, simulate button click
    if(IsPointInRect(mc.x, mc.y, GetX(), GetY(), width_, height_))
    {
        Msg_ButtonClick(1);
        return true;
    }
    return false;
}

bool ctrlProgress::Msg_WheelDown(const MouseCoords& mc)
{
    // If mouse is over the controls, simulate button click
    if(IsPointInRect(mc.x, mc.y, GetX(), GetY(), width_, height_))
    {
        Msg_ButtonClick(0);
        return true;
    }
    return false;
}

bool ctrlProgress::Msg_MouseMove(const MouseCoords& mc)
{
    // an Buttons weiterleiten
    RelayMouseMessage(&Window::Msg_MouseMove, mc);

    if(IsPointInRect(mc.x, mc.y, GetX() + height_ + padding.x, GetY(), width_ - height_ * 2 - padding.x * 2, height_))
    {
        WINDOWMANAGER.SetToolTip(this, tooltip_);
        return true;
    }
    else
    {
        WINDOWMANAGER.SetToolTip(this, "");
        return false;
    }
}
