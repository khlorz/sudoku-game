#include "ImFunks.h"
#include "imgui_internal.h"
#include "imspinner.h"

// ComboFilter Definitions
namespace ImFunks
{

ComboFilter::ComboFilter(const char* _label, const std::vector<std::string>* _list, ImGuiComboFlags _flags) :
	Label(_label), List(_list), Flags(_flags), State({0, false})
{
	InitializePreview();
}

void ComboFilter::InitializePreview()
{
	if (IsListEmpty()) memcpy(Preview, "N/A", strlen("N/A") + 1);
	else               memcpy(Preview, (*List)[0].c_str(), strlen((*List)[0].c_str()) + 1);
}

void ComboFilter::Reset(const std::vector<std::string>* _list, ImGuiComboFlags _flags)
{
	List                   = _list;
	State.activeIdx        = 0;
	State.selectionChanged = false;
	InitializePreview();
}

bool ComboFilter::Draw(float w)
{
	// The Main Combo Filter
	if (w != 0.0f)
		ImGui::PushItemWidth(w);

	ImGui::BeginDisabled(List == nullptr || List->empty());
	static const std::vector<std::string> empty_list = { "N/A" };
	const bool pressed = ImGui::ComboFilter(Label, 
		                                    Preview, 
		                                    100, 
		                                    IsListEmpty() ? empty_list : *List, 
		                                    IsListEmpty() ? 1 : List->size(), 
											State, 
										    NULL, 
											Flags);
	ImGui::EndDisabled();

	return pressed;
}

bool ComboFilter::IsListEmpty() const
{
	return List == nullptr || List->empty();
}

int ComboFilter::GetSelectedIdx() const
{
	return IsListEmpty() ? -1 : State.activeIdx;
}

}

namespace ImFunks
{

LoadingScreen::LoadingScreen(const char* w_label, const char* s_label) noexcept : 
	WindowLabel(w_label), SpinnerLabel(s_label)
{}

void LoadingScreen::StartLoading(const ImVec2& w_sz, const ImVec2& w_pos)
{
	ImGui::OpenPopup(WindowLabel);
	ImGui::SetNextWindowSize(w_sz, ImGuiCond_Always);
	ImGui::SetNextWindowPos(w_pos.x > 0 && w_pos.y > 0 ? w_pos : ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
}

void LoadingScreen::RenderLoading(bool is_open, float radius, float thickness, float speed, ImU32 col)
{
	ImGui::PushStyleColor(ImGuiCol_PopupBg, 0);
	ImGui::PushStyleColor(ImGuiCol_Border, 0);

	if (thickness == 0.0f)
		thickness = radius / 8.0f;
	if (ImGui::BeginPopupModal(WindowLabel, &is_open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
		ImGui::Dummy({ thickness + 1.0f, 0 }); ImGui::SameLine();
		ImSpinner::SpinnerIncDots(SpinnerLabel, radius, thickness, col, speed, 10);

		ImGui::EndPopup();
	}

	ImGui::PopStyleColor(2);
}

}

//--------------------------------------------------------------------------
// ADDITIONAL IMGUI FUNCTIONS
//--------------------------------------------------------------------------
// All functions that are added to the ImGui namespace are here
// 1.) ComboFilter
// 2.) Center Image display
//

namespace ImGui
{
//-----------------------------------------------------
// COMBO FILTER
//-----------------------------------------------------

void Separator(const ImVec2& spacing)
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, spacing);
	ImGui::Separator();
	ImGui::PopStyleVar();
}

// ------------------------------------------------------------
// COMBO FILTER WIDGET!
// ------------------------------------------------------------
// fuzzy-completion combobox code by kovewnikov, from: https://github.com/ocornut/imgui/issues/1658
// corrections and bug fixes by Marcin "Slajerek" Skoczylas

bool ComboFilter(const char* label, char* buffer, int bufferlen, const std::vector<std::string>& hints, const int& num_hints, ComboFilterState& s, ComboFilterCallback* callback, ImGuiComboFlags flags)
{
	s.selectionChanged = false;

	// Always consume the SetNextWindowSizeConstraint() call in our early return paths
	ImGuiContext& g = *GImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiID popupId = window->GetID(label);
	bool popupIsAlreadyOpened = IsPopupOpen(popupId, 0); //ImGuiPopupFlags_AnyPopupLevel);
	bool popupNeedsToBeOpened = callback ? callback->ComboFilterShouldOpenPopupCallback(label, buffer, bufferlen, hints, num_hints, &s)
		: (buffer[0] != 0) && strcmp(buffer, hints[s.activeIdx].c_str());
	bool popupJustOpened = false;

	IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

	const ImGuiStyle& style = g.Style;

	const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const float expected_w = CalcItemWidth();
	const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : expected_w;
	const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2.0f));
	const ImRect total_bb(frame_bb.Min, ImVec2((label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f) + frame_bb.Max.x, frame_bb.Max.y));
	const float value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, popupId, &frame_bb))
		return false;


	bool hovered, held;
	bool pressed = ButtonBehavior(frame_bb, popupId, &hovered, &held);

	if (!popupIsAlreadyOpened) {
		const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		RenderNavHighlight(frame_bb, popupId);
		if (!(flags & ImGuiComboFlags_NoPreview))
			window->DrawList->AddRectFilled(frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col, style.FrameRounding, (flags & ImGuiComboFlags_NoArrowButton) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersLeft);
	}
	if (!(flags & ImGuiComboFlags_NoArrowButton))
	{
		ImU32 bg_col = GetColorU32((popupIsAlreadyOpened || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImU32 text_col = GetColorU32(ImGuiCol_Text);
		window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding, (w <= arrow_size) ? ImDrawFlags_RoundCornersAll : ImDrawFlags_RoundCornersRight);
		if (value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x)
			RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col, ImGuiDir_Down, 1.0f);
	}

	if (!popupIsAlreadyOpened)
	{
		RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
		if (buffer != NULL && !(flags & ImGuiComboFlags_NoPreview))

			RenderTextClipped(ImVec2(frame_bb.Min.x + style.FramePadding.x, frame_bb.Min.y + style.FramePadding.y), ImVec2(value_x2, frame_bb.Max.y), buffer, NULL, NULL, ImVec2(0.0f, 0.0f));

		if ((pressed || g.NavActivateId == popupId || popupNeedsToBeOpened) && !popupIsAlreadyOpened)
		{
			if (window->DC.NavLayerCurrent == 0)
				window->NavLastIds[0] = popupId;
			OpenPopupEx(popupId);
			popupIsAlreadyOpened = true;
			popupJustOpened = true;
		}
	}

	if (label_size.x > 0)
	{
		RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);
	}

	if (!popupIsAlreadyOpened) {
		return false;
	}

	const float totalWMinusArrow = w - arrow_size;
	struct ImGuiSizeCallbackWrapper {
		static void sizeCallback(ImGuiSizeCallbackData* data)
		{
			float* totalWMinusArrow = (float*)(data->UserData);
			data->DesiredSize = ImVec2(*totalWMinusArrow, 200.f);
		}
	};
	SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(totalWMinusArrow, 150.f), ImGuiSizeCallbackWrapper::sizeCallback, (void*)&totalWMinusArrow);

	char name[16];
	ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

	// Peek into expected window size so we can position it
	if (ImGuiWindow* popup_window = FindWindowByName(name))
	{
		if (popup_window->WasActive)
		{
			ImVec2 size_expected = CalcWindowNextAutoFitSize(popup_window);
			if (flags & ImGuiComboFlags_PopupAlignLeft)
				popup_window->AutoPosLastDirection = ImGuiDir_Left;
			ImRect r_outer = ImGui::GetPopupAllowedExtentRect(popup_window);
			ImVec2 pos = FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame_bb, ImGuiPopupPositionPolicy_ComboBox);

			pos.y -= label_size.y + style.FramePadding.y * 2.0f;

			SetNextWindowPos(pos);
		}
	}

	// Horizontally align ourselves with the framed text
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
	//    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
	bool ret = Begin(name, NULL, window_flags);

	ImGui::PushItemWidth(ImGui::GetWindowWidth());
	ImGui::SetCursorPos(ImVec2(0.f, window->DC.CurrLineTextBaseOffset));
	if (popupJustOpened)
	{
		ImGui::SetKeyboardFocusHere(0);
	}

	bool done = InputTextEx("##inputText", NULL, buffer, bufferlen, ImVec2(0, 0),
		ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue,
		NULL, NULL);
	ImGui::PopItemWidth();

	if (s.activeIdx < 0) {
		IM_ASSERT(false); //Undefined behaviour
		return false;
	}


	if (!ret)
	{
		ImGui::EndChild();
		ImGui::PopItemWidth();
		EndPopup();
		IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
		return false;
	}


	ImGuiWindowFlags window_flags2 = ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus; //0; //ImGuiWindowFlags_HorizontalScrollbar
	ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false, window_flags2);

	struct fuzzy {
		static int score(const char* str1, const char* str2) {
			int score = 0, consecutive = 0, maxerrors = 0;
			while (*str1 && *str2) {
				int is_leading = (*str1 & 64) && !(str1[1] & 64);
				if ((*str1 & ~32) == (*str2 & ~32)) {
					int had_separator = (str1[-1] <= 32);
					int x = had_separator || is_leading ? 10 : consecutive * 5;
					consecutive = 1;
					score += x;
					++str2;
				}
				else {
					int x = -1, y = is_leading * -3;
					consecutive = 0;
					score += x;
					maxerrors += y;
				}
				++str1;
			}
			return score + (maxerrors < -9 ? -9 : maxerrors);
		}
		static int search(const char* str, int num, const std::vector<std::string>& words) {
			int scoremax = 0;
			int best = -1;
			for (int i = 0; i < num; ++i) {
				int score = fuzzy::score(words[i].c_str(), str);
				int record = (score >= scoremax);
				int draw = (score == scoremax);
				if (record) {
					scoremax = score;
					if (!draw) best = i;
					else best = best >= 0 && strlen(words[best].c_str()) < strlen(words[i].c_str()) ? best : i;
				}
			}
			return best;
		}
	};

	bool selectionChangedLocal = false;
	if (buffer[0] != '\0')
	{
		int new_idx = fuzzy::search(buffer, num_hints, hints);
		int idx = new_idx >= 0 ? new_idx : s.activeIdx;
		s.selectionChanged = s.activeIdx != idx;
		selectionChangedLocal = s.selectionChanged;
		s.activeIdx = idx;
	}

	bool arrowScroll = false;
	int arrowScrollIdx = s.activeIdx;

	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
	{
		if (s.activeIdx > 0)
		{
			s.activeIdx--;
			//			selectionChangedLocal = true;
			arrowScroll = true;
			ImGui::SetWindowFocus();
			//			arrowScrollIdx = s.activeIdx;
		}
	}
	if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
	{
		if (s.activeIdx < num_hints - 1)
		{
			s.activeIdx++;
			//			selectionChangedLocal = true;
			arrowScroll = true;
			ImGui::SetWindowFocus();
			//			arrowScrollIdx = s.activeIdx;
		}
	}

	if (done && !arrowScroll) {
		CloseCurrentPopup();
	}

	bool done2 = false;

	for (int n = 0; n < num_hints; n++)
	{
		bool is_selected = n == s.activeIdx;
		if (is_selected && (IsWindowAppearing() || selectionChangedLocal))
		{
			SetScrollHereY();
			//            ImGui::SetItemDefaultFocus();
		}

		if (is_selected && arrowScroll)
		{
			SetScrollHereY();
		}

		if (ImGui::Selectable(hints[n].c_str(), is_selected))
		{
			s.selectionChanged = s.activeIdx != n;
			s.activeIdx = n;
			strcpy(buffer, hints[n].c_str());
			CloseCurrentPopup();

			done2 = true;
		}

	}

	if (arrowScroll)
	{
		strcpy(buffer, hints[s.activeIdx].c_str());
		ImGuiWindow* window = FindWindowByName(name);
		const ImGuiID id = window->GetID("##inputText");
		ImGuiInputTextState* state = GetInputTextState(id);

		const char* buf_end = NULL;
		state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, state->TextW.Size, buffer, NULL, &buf_end);
		state->CurLenA = (int)(buf_end - buffer);
		state->CursorClamp();
	}

	ImGui::EndChild();
	EndPopup();

	bool ret1 = (s.selectionChanged && !strcmp(hints[s.activeIdx].c_str(), buffer));

	bool widgetRet = done || done2 || ret1;

	return widgetRet;
}

bool ComboFilterCallback::ComboFilterShouldOpenPopupCallback(const char* label, char* buffer, int bufferlen, const std::vector<std::string>& hints, int num_hints, ImGui::ComboFilterState* s)
{
	return (buffer[0] != 0) && strcmp(buffer, hints[s->activeIdx].c_str());
}

//-----------------------------------------------------
// CENTER IMAGE DISPLAY
//-----------------------------------------------------

void ImageCenter(ImTextureID user_texture_id, const ImVec2& imgsize, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;

	ImRect bb;
	bb.Min = ImVec2(window->DC.CursorPos.x + ((window->Size.x - imgsize.x) / 2.0f), window->DC.CursorPos.y);
	bb.Max = ImVec2(imgsize.x + bb.Min.x, imgsize.y + bb.Min.y);
	if (border_col.w > 0.0f)
		bb.Max += ImVec2(2, 2);
	ItemSize(bb);
	if (!ItemAdd(bb, 0))
		return;

	if (border_col.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
		window->DrawList->AddImage(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, GetColorU32(tint_col));
	}
	else
	{
		window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
	}
}

bool Button(const char* label, const ImVec2& sz, const ImVec2& pos)
{
	ImGui::SetCursorPos(pos);
	return ImGui::Button(label, sz);
}

} // ImGui namespace

// New Themes
namespace ImGui
{

void ImGui::StyleColorsNewDark(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	
	colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive]              = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused]           = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImLerp(colors[ImGuiCol_TabActive],    colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.0f, 0.0f, 0.0f, 0.50f);
}

}
