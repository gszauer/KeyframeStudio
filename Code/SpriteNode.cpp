#include "SpriteNode.h"
#include "Libraries/imgui.h"
#include "Libraries/imgui_stdlib.h"
#include "AssetManager.h"
#include "Asset.h"
#include "UndoManager.h"
#include "Application.h"

SpriteNode::SpriteNode(const char* _name) : ::TransformNode(_name) {
	width = 0;
	height = 0;
	r = 1.0;
	g = 1.0;
	b = 1.0;
	a = 1.0;
	image = 0;
	frame = 0;
	frameMin = ImVec2(0, 0);
	frameMax = ImVec2(1, 1);
	visible = true;
	sort = 0;
	rtti = NodeType::SPRITE_NODE;
	pivot = ImVec2(0.5f, 0.5f);
}

SpriteNode::~SpriteNode() {} 

bool SpriteNode::CanHaveChildren() { return false; }
void SpriteNode::AddChild(SceneNode* newChild) { }
void SpriteNode::AddChildAfter(SceneNode* prevOrNull, SceneNode* newChild) { }
void SpriteNode::AddChildBefore(SceneNode* newChild, SceneNode* nextOrNull) { }
bool SpriteNode::RemoveChild(SceneNode* child) { return false; }

ImVec2 SpriteNode::GetPivot() {
	if (frame != 0 && frame->rotated) {
		return ImVec2(pivot.y, pivot.x);
	}
	return pivot;
}

void SpriteNode::SetPivot(const ImVec2& _pivot) {
	if (frame != 0 && frame->rotated) {
		pivot = ImVec2(_pivot.y, _pivot.x);
	}
	else {
		pivot = _pivot;
	}
}

void SpriteNode::EditorImmediateInspector(class AnimationAsset* selectedAnimation, int selectedFrame) {
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	TransformNode::EditorImmediateInspector(selectedAnimation, selectedFrame);
	Application::GetInstance()->PushXenon();
	if (ImGui::CollapsingHeader("Sprite Node", ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PopFont();
		ImGui::BeginTable("##spriteNodeInspector", 2);

		ImGui::TableSetupColumn("##spriteNodeInspectorAAA", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("##spriteNodeInspectorBBB", ImGuiTableColumnFlags_WidthStretch);

		{
			ImGui::TableNextColumn();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3 * windowDevicePixelRatio);
			ImGui::Text("Sprite");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3 * windowDevicePixelRatio);
			ImGui::SetItemTooltip("Sprite is not animatable");

			ImGui::TableNextColumn();
			
			ImGui::BeginTable("##spriteNodeInspectorClearSprite", 2);
			{
				ImGui::TableSetupColumn("##sprteNdeInspAAAclrSprt", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("##sprteNdeInspBBBclrSprt", ImGuiTableColumnFlags_WidthFixed);

				ImGui::TableNextColumn();
				ImGui::PushItemWidth(-FLT_MIN);
				const char* hint = "Drop Sprite Here\0\0\0\0";
				size_t buf_size = 16;
				if (image != 0 && !image->deleted) {
					hint = image->name.c_str();
					buf_size = (size_t)image->name.size();
				}
				ImGui::InputText((const char*)"##SpriteNodeInspectorSprite", (char*)hint, buf_size, ImGuiInputTextFlags_ReadOnly);
				ImGui::PopItemWidth();

				ImGui::TableNextColumn();
				bool disable_x = image == 0 || image->deleted;
				if (disable_x) {
					ImGui::BeginDisabled();
				}
				Application::GetInstance()->PushIcon();
				if (ImGui::Button("G", ImVec2(17 * windowDevicePixelRatio, 17 * windowDevicePixelRatio))) {
					UndoManager::GetInstance()->SetSpriteRef(this, 0, size, frameMin, frameMax);
				}
				ImGui::PopFont();

				Application::GetInstance()->PushArgon();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
				ImGui::SetItemTooltip("Clear Sprite");
				ImGui::PopStyleVar();
				ImGui::PopFont();

				if (disable_x) {
					ImGui::EndDisabled();
				}
			}
			ImGui::EndTable();

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Assets:Image")) {
					Asset* asset = *(Asset**)payload->Data;
					if (asset != 0 && asset->GetType() == AssetType::ASSET_IMAGE) {
						/*image = (ImageAsset*)asset;
						width = image->size.x;
						height = image->size.y;
						if (frame == 0) {
							frameMin = ImVec2(0, 0);
							frameMax = ImVec2(width, height);
						}
						else {
							frameMin = frame->GetMin();
							frameMax = frame->GetMax();
						}*/

						ImVec2 tmpMin = ImVec2(0, 0);
						ImVec2 tmpMax = ((ImageAsset*)asset)->size;
						if (frame != 0) {
							tmpMin = frame->GetMin();
							tmpMax = frame->GetMax();
						}
						UndoManager::GetInstance()->SetSpriteRef(this, (ImageAsset*)asset, ((ImageAsset*)asset)->size, tmpMin, tmpMax);
					}
				}
				ImGui::EndDragDropTarget();
			}
		}

		ImGui::TableNextColumn();
		ImGui::Text("Size");
		ImGui::SetItemTooltip("Size is not animatable");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		ImVec2 newSize = size;
		if (ImGui::InputFloat2("##spriteInspectorSize", &newSize.x)) {
			bool setX = fabsf(newSize.x - size.x) > 0.00001f;
			bool setY = fabsf(newSize.y - size.y) > 0.00001f;

			if (setX || setY) {
				UndoManager::GetInstance()->SetSize(this, newSize, setX, setY);
			}
		}
		ImGui::PopItemWidth();

		ImGui::TableNextColumn();
		ImGui::Text("Pivot");
		ImGui::SetItemTooltip("Pivot is not animatable");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		ImVec2 newPivot = pivot;
		if (ImGui::InputFloat2("##spriteInspectorPivot", &newPivot.x)) {
			bool setX = fabsf(newPivot.x - pivot.x) > 0.00001f;
			bool setY = fabsf(newPivot.y - pivot.y) > 0.00001f;

			if (setX || setY) {
				UndoManager::GetInstance()->SetPivot(this, newPivot, setX, setY);
			}
		}
		ImGui::PopItemWidth();
		Clamp01(pivot);

		{
			ImGui::TableNextColumn();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3 * windowDevicePixelRatio);
			ImGui::Text("Frame");
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3 * windowDevicePixelRatio);
			ImGui::SetItemTooltip("Frame is not animatable");
			ImGui::TableNextColumn();

			ImGui::BeginTable("##spriteNodeInspectorClearFrame", 2);
			{
				ImGui::TableSetupColumn("##sprteNdeInspAAAclrFrm", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("##sprteNdeInspBBBclrFrm", ImGuiTableColumnFlags_WidthFixed);
			
				ImGui::TableNextColumn();
				ImGui::PushItemWidth(-FLT_MIN);
				const char* hint = "Drop Frame Here\0\0\0\0";
				size_t buf_size = 16;
				if (frame != 0 && !frame->deleted) {
					hint = frame->name.c_str();
					buf_size = (size_t)frame->name.size();
				}
				ImGui::InputText((const char*)"##SpriteNodeInspectorFrame", (char*)hint, buf_size, ImGuiInputTextFlags_ReadOnly);
				ImGui::PopItemWidth();

				ImGui::TableNextColumn();
				bool disable_x = frame == 0 || frame->deleted;
				if (disable_x) {
					ImGui::BeginDisabled();
				}
				Application::GetInstance()->PushIcon();
				if (ImGui::Button("G", ImVec2(17 * windowDevicePixelRatio, 17 * windowDevicePixelRatio))) {
					ImVec2 frmMax = ImVec2(1, 1);
					ImVec2 imgSize = size;
					if (image != 0) {
						frmMax = image->size;
						imgSize = image->size;
					}

					UndoManager::GetInstance()->SetFrameRef(this, 0, imgSize, ImVec2(0, 0), frmMax);
				}
				ImGui::PopFont();

				Application::GetInstance()->PushArgon();
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
				ImGui::SetItemTooltip("Clear Frame");
				ImGui::PopStyleVar();
				ImGui::PopFont();

				if (disable_x) {
					ImGui::EndDisabled();
				}
			}
			ImGui::EndTable();

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Assets:Frame")) {
					Asset* asset = *(Asset**)payload->Data;
					if (asset != 0 && asset->GetType() == AssetType::ASSET_FRAME) {
						/*frame = (AtlasFrame*)asset;
						size = frame->GetSize();
						frameMin = frame->GetMin();
						frameMax = frame->GetMax();*/

						AtlasFrame* af = (AtlasFrame*)asset;
						UndoManager::GetInstance()->SetFrameRef(this, af, af->GetSize(), af->GetMin(), af->GetMax());
					}
				}
				ImGui::EndDragDropTarget();
			}
		}

		if (frame != 0 && !frame->deleted) {
			ImGui::BeginDisabled();
		}

		ImGui::TableNextColumn();
		ImGui::Text("Frame min");
		ImGui::SetItemTooltip("Frame is not animatable");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		ImVec2 newFrameMin = frameMin;
		if (ImGui::InputFloat2("##spriteInspectorFrameMin", &newFrameMin.x)) {
			bool newX = fabsf(newFrameMin.x - frameMin.x) > 0.00001f;
			bool newY = fabsf(newFrameMin.y - frameMin.y) > 0.00001f;

			if (newX || newY) {
				UndoManager::GetInstance()->SetFrameMin(this, newFrameMin, newX, newY);
			}
		}
		ImGui::PopItemWidth();

		ImGui::TableNextColumn();
		ImGui::Text("Frame max");
		ImGui::SetItemTooltip("Frame is not animatable");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		ImVec2 newFrameMax = frameMax;
		if (ImGui::InputFloat2("##spriteInspectorFrameMax", &newFrameMax.x)) {
			bool newX = fabsf(newFrameMax.x - frameMax.x) > 0.00001f;
			bool newY = fabsf(newFrameMax.y - frameMax.y) > 0.00001f;

			if (newX || newY) {
				UndoManager::GetInstance()->SetFrameMax(this, newFrameMax, newX, newY);
			}
		}
		ImGui::PopItemWidth();

		if (frame != 0 && !frame->deleted) {
			ImGui::EndDisabled();
		}

		ImGui::TableNextColumn();
		ImGui::Text("Sort");

		if (selectedAnimation == 0) {
			ImGui::SetItemTooltip("Select animation to set keyframes");
		}
		else if (selectedFrame < 0) {
			ImGui::SetItemTooltip("Select frame to set keyframes");
		}
		else {
			ImGui::SetItemTooltip("Click to animate sorting index");
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			if (selectedAnimation != 0 && selectedFrame >= 0) {
				ImGui::OpenPopup("##KEY_SORT");
			}
		}
		if (ImGui::BeginPopup("##KEY_SORT")) {
			if (ImGui::Selectable("Keyframe sort index")) {
				Track* track = selectedAnimation->GetTrack(this, "sort");
				int data = sort;
				if (track != 0 && !track->IsDeleted() && track->HasFrames()) {
					data = track->InterpolateI(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
				}
				if (track == 0 && selectedFrame >= 0) {
					track = selectedAnimation->AddTrack(this, "sort", TrackType::TRACK_INT);
				}
				if (track != 0) {
					track->AddFrameI(data, selectedFrame);
				}
			}

			ImGui::EndPopup();
		}

		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		if (selectedAnimation == 0 || selectedAnimation->IsDeleted()) {
			int srt = sort;
			ImGui::InputInt("##spriteInspectorSort", &srt);
			if (srt != sort) {
				UndoManager::GetInstance()->SetSort(this, srt);
			}
		}
		else {
			Track* track = selectedAnimation->GetTrack(this, "sort");
			int data = sort;
			if (track != 0 && !track->IsDeleted() && track->HasFrames()) {
				data = track->InterpolateI(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
			}
			int old = data;

			if (ImGui::InputInt("##spriteInspectorSort", &data)) {
				if (data != old) {
					if (track == 0 && selectedFrame >= 0) {
						track = selectedAnimation->AddTrack(this, "sort", TrackType::TRACK_INT);
					}
					if (track != 0) {
						//track->AddFrameI(data, selectedFrame);
						UndoManager::GetInstance()->AddFrameI(track, data, selectedFrame);
					}
				}
			}
		}
		ImGui::PopItemWidth();

		ImGui::TableNextColumn();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3 * windowDevicePixelRatio);
		ImGui::Text("Visible");
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3 * windowDevicePixelRatio);

		if (selectedAnimation == 0) {
			ImGui::SetItemTooltip("Select animation to set keyframes");
		}
		else if (selectedFrame < 0) {
			ImGui::SetItemTooltip("Select frame to set keyframes");
		}
		else {
			ImGui::SetItemTooltip("Click to animate visibility");
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			if (selectedAnimation != 0 && selectedFrame >= 0) {
				ImGui::OpenPopup("##KEY_VISIBILITY");
			}
		}
		if (ImGui::BeginPopup("##KEY_VISIBILITY")) {
			if (ImGui::Selectable("Keyframe visibility")) {
				Track* track = selectedAnimation->GetTrack(this, "visible");
				bool data = visible;
				if (track != 0 && !track->IsDeleted() && track->HasFrames()) {
					data = track->InterpolateB(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
				}

				if (track == 0 && selectedFrame >= 0) {
					track = selectedAnimation->AddTrack(this, "visible", TrackType::TRACK_BOOL);
				}
				if (track != 0) {
					//track->AddFrameB(data, selectedFrame);
					UndoManager::GetInstance()->AddFrameB(track, data, selectedFrame);
				}
			}

			ImGui::EndPopup();
		}

		ImGui::TableNextColumn();
		
		{
			ImGui::BeginTable("##spriteSubTableForVisible", 4);

			ImGui::TableSetupColumn("##spriteSubTableForVisibleAAA", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("##spriteSubTableForVisibleBBB", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("##spriteSubTableForVisibleCCC", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("##spriteSubTableForVisibleDDD", ImGuiTableColumnFlags_WidthFixed);

			ImGui::TableNextColumn();
			if ((selectedAnimation == 0 || selectedAnimation->IsDeleted())) {
				bool vis = visible;
				ImGui::Checkbox("##spriteInspectorVisible", &vis);
				if (vis != visible) {
					UndoManager::GetInstance()->SetVisibility(this, vis);
				}
			}
			else {
				Track* track = selectedAnimation->GetTrack(this, "visible");
				bool data = visible;
				if (track != 0 && !track->IsDeleted() && track->HasFrames()) {
					data = track->InterpolateB(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
				}
				bool old = data;

				if (ImGui::Checkbox("##spriteInspectorVisible", &data)) {
					if (data != old) {
						if (track == 0 && selectedFrame >= 0) {
							track = selectedAnimation->AddTrack(this, "visible", TrackType::TRACK_BOOL);
						}
						if (track != 0) {
							//track->AddFrameB(data, selectedFrame);
							UndoManager::GetInstance()->AddFrameB(track, data, selectedFrame);
						}
					}
				}
			}

			ImGui::TableNextColumn();
			ImGui::Spacing();

			ImGui::TableNextColumn();
			ImGui::Text("Tint");

			if (selectedAnimation == 0) {
				ImGui::SetItemTooltip("Select animation to set keyframes");
			}
			else if (selectedFrame < 0) {
				ImGui::SetItemTooltip("Select frame to set keyframes");
			}
			else {
				ImGui::SetItemTooltip("Click to animate tint");
			}

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
				if (selectedAnimation != 0 && selectedFrame >= 0) {
					ImGui::OpenPopup("##KEY_TINT");
				}
			}
			if (ImGui::BeginPopup("##KEY_TINT")) {
				if (ImGui::Selectable("Keyframe tint")) {
					ImU32 tintVal = ImGui::ColorConvertFloat4ToU32(tint);
					Track* track = (selectedAnimation == 0 || selectedAnimation->IsDeleted()) ? 0 : selectedAnimation->GetTrack(this, "tint");
					if (track != 0 && !track->IsDeleted() && track->HasFrames()) {
						tintVal = track->InterpolateC(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
					}
					if (track == 0 && selectedFrame >= 0) {
						track = (selectedAnimation == 0 || selectedAnimation->IsDeleted()) ? 0 : selectedAnimation->AddTrack(this, "tint", TrackType::TRACK_COLOR);
					}
					if (track != 0) {
						//track->AddFrameC(tintVal, selectedFrame);
						UndoManager::GetInstance()->AddFrameC(track, tintVal, selectedFrame);
					}
				}

				ImGui::EndPopup();
			}

			ImGui::TableNextColumn();
			{
				ImU32 tintVal = ImGui::ColorConvertFloat4ToU32(tint);
				Track* track = (selectedAnimation == 0 || selectedAnimation->deleted)? 0 : selectedAnimation->GetTrack(this, "tint");
				if (track != 0 && !track->IsDeleted() && track->HasFrames()) {
					tintVal = track->InterpolateC(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
				}

				ImVec4 newTint = ImGui::ColorConvertU32ToFloat4(tintVal);
				if (ImGui::ColorEdit4("##SpriteTint", (float*)&newTint.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreview)) {
					tintVal = ImGui::ColorConvertFloat4ToU32(newTint);
					if (track == 0 && selectedFrame >= 0 && selectedAnimation != 0) {
						track = selectedAnimation->AddTrack(this, "tint", TrackType::TRACK_COLOR);
					}
					
					if (track != 0 && !track->IsDeleted()) {
						//track->AddFrameC(tintVal, selectedFrame);
						UndoManager::GetInstance()->AddFrameC(track, tintVal, selectedFrame);
					}
					else {
						//tint = newTint;
						UndoManager::GetInstance()->SetTint(this, tintVal);
					}
				}
			}

			ImGui::EndTable();
		}

		ImGui::EndTable();

		ImGui::SeparatorText("Sprite Preview");
		if (image != 0 && !image->deleted) {
			ImVec2 size = image->size;
			ImVec2 windowRegion = ImGui::GetContentRegionAvail();
			float scale = 0.0f;
			if (windowRegion.x > 0) {
				scale = windowRegion.x / size.x;
			}
			ImVec2 scaled(size.x * scale, size.y * scale);
			ImGui::Image(image->id, scaled);
		}
		else {
			ImGui::Text("No valid texture specified");
		}
	}
	else {
		ImGui::PopFont();
	}
}

void SpriteNode::SerializeInner(std::stringstream& out, int indent) {
	TransformNode::SerializeInner(out, indent);
	if (parent == 0 || isDeleted) {
		return;
	}

	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();

	out << tabs << "\t\"size\": { \"w\": " << size.x << ", \"h\": " << size.y << " }, \n";
	out << tabs << "\t\"pivot\": { \"x\": " << pivot.x << ", \"y\": " << pivot.y << " }, \n";
	out << tabs << "\t\"tint\": { \"r\": " << tint.x << ", \"g\": " << tint.y << ", \"b\": " << tint.z << ", \"a\": " << tint.w << " }, \n";
	out << tabs << "\t\"sort\": " << sort << ",\n";
	if (visible) {
		out << tabs << "\t\"visible\": true,\n";
	}
	else {
		out << tabs << "\t\"visible\": false,\n";
	}
	if (image == 0) {
		out << tabs << "\t\"image\": null, \n";
	}
	else {
		out << tabs << "\t\"image\": \"" << image->GetGUID() << "\", \n";
	}
	if (frame == 0) {
		out << tabs << "\t\"frame\": null, \n";
	}
	else {
		out << tabs << "\t\"frame\": \"" << frame->GetGUID() << "\", \n";
	}

	out << tabs << "\t\"frameMin\": { \"x\": " << frameMin.x << ", \"y\": " << frameMin.y << " }, \n";
	out << tabs << "\t\"frameMax\": { \"x\": " << frameMax.x << ", \"y\": " << frameMax.y << " }, \n";

}