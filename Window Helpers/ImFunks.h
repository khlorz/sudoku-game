#pragma once

#include "imgui.h"
#include "imgui_internal.h"
#include <string>
#include <vector>

namespace ImGui
{

struct ComboFilterState
{
	int  activeIdx;
	bool selectionChanged;
};

class ComboFilterCallback
{
	public:
		virtual bool ComboFilterShouldOpenPopupCallback(const char* label, char* buffer, int bufferlen,
			const std::vector<std::string>& hints, int num_hints, ImGui::ComboFilterState* s);
};

void Separator(const ImVec2& spacing);

// Combo Filter widget
bool ComboFilter(const char* label, char* buffer, int bufferlen, const std::vector<std::string>& hints, const int& num_hints, ComboFilterState& s, ComboFilterCallback* callback, ImGuiComboFlags flags = 0);

// For centering the image horizontally in a window
void ImageCenter(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));

//
bool Button(const char* label, const ImVec2& sz, const ImVec2& pos);

void StyleColorsNewDark(ImGuiStyle* dst = 0);

} // ImGui namespace

namespace ImFunks
{
// Helper for ComboFilter
class ComboFilter
{
private:
	char                            Preview[100];
	const char*                     Label;
	ImGuiComboFlags                 Flags;
	ImGui::ComboFilterState         State;
	const std::vector<std::string>* List;

	void InitializePreview();
public:
	ComboFilter() = default;
	ComboFilter(const char* _label, const std::vector<std::string>* _list = nullptr, ImGuiComboFlags _flags = 0);
		
	void Reset(const std::vector<std::string>* _list = nullptr, ImGuiComboFlags _flags = 0);
	bool Draw(float w = 0.0f);
	bool IsListEmpty() const;
	int  GetSelectedIdx() const;
};

class LoadingScreen
{
private:
	const char* WindowLabel;
	const char* SpinnerLabel;
public:
	LoadingScreen(const char* w_label, const char* s_label) noexcept;
	LoadingScreen(const LoadingScreen& other) noexcept = delete;
	LoadingScreen& operator = (const LoadingScreen& other) noexcept = delete;

	void StartLoading(const ImVec2& w_sz, const ImVec2& w_pos = ImVec2(-1.0f, -1.0f));
	void RenderLoading(bool is_open, float radius, float thickness = 0.0f, float speed = 3.0f, ImU32 col = 0xffffffff);
};


//class ModalWindow
//{
//protected:
//	const char*      Label;
//	ImVec2           Size;
//	ImGuiWindowFlags Flags;
//
//	//virtual void Close() = 0;
//	virtual void Body() = 0;
//	virtual void Reset() = 0;
//
//public:
//	virtual void RenderModal() = 0;
//	virtual void StartModal() = 0;
//	virtual bool IsClosed() = 0;
//};

}


