#include "TransformNode.h"
#include "Libraries/imgui_stdlib.h"
#include "Asset.h"
#include "Track.h"
#include "Tools.h"
#include "Application.h"
#include "UndoManager.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846  /* pi */
#endif

TransformNode::TransformNode(const char* _name) : ::SceneNode(_name) {
	position = ImVec2(0, 0);
	rotation = 0;
	scale = ImVec2(1, 1);
	rtti = NodeType::TRANSFORM_NODE;
}

TransformNode::~TransformNode() { }

void TransformNode::Reset() {
	position = ImVec2(0, 0);
	rotation = 0;
	scale = ImVec2(1, 1);
}

void TransformNode::SetRotation(float val) {
	rotation = val;
	NormalizeRotation();
}

float TransformNode::NormalizedRotation(float rotation) {
	const float range = (float)(M_PI * 2.0);
	while (rotation < 0) {
		rotation += range;
	}
	while (rotation >= range) {
		rotation -= range;
	}
	return rotation;
}

float TransformNode::NormalizeRotation() {
	const float range = (float)(M_PI * 2.0);
	while (rotation < 0) {
		rotation += range;
	}
	while (rotation >= range) {
		rotation -= range;
	}
	return rotation;
}

float TransformNode::GetRotation() {
	NormalizeRotation();
	return rotation;
}

ImMat3 TransformNode::GetMatrix(ImMat3* outPos, ImMat3* outRot, ImMat3* outScl) {
	float rotCos = cosf(rotation);
	float rotSin = sinf(rotation);

	ImMat3 scaleMatrix = ImMat3(
		scale.x, 0, 0,
		0, scale.y, 0,
		0, 0, 1
	);
	if (outScl != 0) {
		*outScl = scaleMatrix;
	}
	ImMat3 rotateMatrix = ImMat3(
		rotCos, -rotSin, 0,
		rotSin, rotCos, 0,
		0, 0, 1
	);
	if (outRot != 0) {
		*outRot = rotateMatrix;
	}
	ImMat3 translateMatrix = ImMat3(
		1, 0, 0,
		0, 1, 0,
		position.x, position.y, 1
	);
	if (outPos != 0) {
		*outPos = translateMatrix;
	}
	return translateMatrix * rotateMatrix * scaleMatrix;
}

void TransformNode::EditorImmediateInspector(class AnimationAsset* selectedAnimation, int selectedFrame) {
	SceneNode::EditorImmediateInspector(selectedAnimation, selectedFrame);
	float minRot = 0.0f;
	const float maxRot = (float)(M_PI * 2.0);

	Application::GetInstance()->PushXenon();
	if (ImGui::CollapsingHeader("Transform Node", ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PopFont();
		ImGui::BeginTable("##transformNodeInspector", 2);

		ImGui::TableSetupColumn("##transformNodeInspectorAAA", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("##transformNodeInspectorBBB", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextColumn();
		ImGui::Text("Position");
		
		if (selectedAnimation == 0) {
			ImGui::SetItemTooltip("Select animation to set keyframes");
		}
		else if (selectedFrame < 0) {
			ImGui::SetItemTooltip("Select frame to set keyframes");
		}
		else {
			ImGui::SetItemTooltip("Click to animate position");
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			if (selectedAnimation != 0 && selectedFrame >= 0) {
				ImGui::OpenPopup("##KEY_POSITION");
			}
		}
		if (ImGui::BeginPopup("##KEY_POSITION")) {
			if (selectedAnimation != 0 && selectedFrame >= 0) { // Not needed...
				Track* x_track = selectedAnimation->GetTrack(this, "position.x");// 
				Track* y_track = selectedAnimation->GetTrack(this, "position.y");//
				float data[2] = { position.x, position.y }; // Default to hierarchy
				if (x_track != 0 && !x_track->IsDeleted() && x_track->HasFrames()) {
					data[0] = x_track->InterpolateF(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
				}
				if (y_track != 0 && !y_track->IsDeleted() && y_track->HasFrames()) {
					data[1] = y_track->InterpolateF(selectedFrame, selectedAnimation->looping);// y_frame->data.asFloat;
				}

				bool key_x = false;
				if (ImGui::Selectable("Keyframe X")) {
					key_x = true;
				}
				bool key_y = false;
				if (ImGui::Selectable("Keyframe Y")) {
					key_y = true;
				}

				if (ImGui::Selectable("Keyframe Both")) {
					key_y = true;
					key_x = true;
				}

				if (key_x) {
					if (x_track == 0 && selectedFrame >= 0) {
						x_track = selectedAnimation->AddTrack(this, "position.x", TrackType::TRACK_FLOAT);
					}
					if (x_track != 0) {
						//x_track->AddFrameF(data[0], selectedFrame);
						UndoManager::GetInstance()->AddFrameF(x_track, data[0], selectedFrame);
					}
				}
				if (key_y) {
					if (y_track == 0 && selectedFrame >= 0) {
						y_track = selectedAnimation->AddTrack(this, "position.y", TrackType::TRACK_FLOAT);
					}
					if (y_track != 0) {
						//y_track->AddFrameF(data[1], selectedFrame);
						UndoManager::GetInstance()->AddFrameF(y_track, data[1], selectedFrame);
					}
				}
			}
			ImGui::EndPopup();
		}


		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		if (selectedAnimation == 0 || selectedAnimation->IsDeleted()) {
			float old[2] = { position.x, position.y }; 
			ImVec2 edit = position;
			if (ImGui::InputFloat2("##positionTransformNode", &edit.x)) {
				if (edit.x != old[0] && edit.y != old[1]) {
					UndoManager::GetInstance()->SetPosition(this, edit, true, true);
				}
				else if (edit.x != old[0]) {
					UndoManager::GetInstance()->SetPosition(this, edit, true, false);
				}
				else if (edit.y != old[1]) {
					UndoManager::GetInstance()->SetPosition(this, edit, false, true);
				}
				else { IM_ASSERT(false); }
				// No need to set position directly, undo manager will handle it
			}
		}
		else {
			Track* x_track = selectedAnimation->GetTrack(this, "position.x");// 
			Track* y_track = selectedAnimation->GetTrack(this, "position.y");//
			
			float data[2] = { position.x, position.y }; // Default to hierarchy
			if (x_track != 0 && !x_track->IsDeleted() && x_track->HasFrames()) {
				data[0] = x_track->InterpolateF(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
			}
			if (y_track != 0 && !y_track->IsDeleted() && y_track->HasFrames()) {
				data[1] = y_track->InterpolateF(selectedFrame, selectedAnimation->looping);// y_frame->data.asFloat;
			}
			float old[2] = { data[0], data[1] };
			
			if (ImGui::InputFloat2("##positionTransformNode", data)) {
				if (data[0] != old[0]) {
					if (x_track == 0 && selectedFrame >= 0) {
						x_track = selectedAnimation->AddTrack(this, "position.x", TrackType::TRACK_FLOAT);
					}
					if (x_track != 0) {
						//x_track->AddFrameF(data[0], selectedFrame);
						UndoManager::GetInstance()->AddFrameF(x_track, data[0], selectedFrame);
					}
				}
				if (data[1] != old[1]) {
					if (y_track == 0 && selectedFrame >= 0) {
						y_track = selectedAnimation->AddTrack(this, "position.y", TrackType::TRACK_FLOAT);
					}
					if (y_track != 0) {
						//y_track->AddFrameF(data[1], selectedFrame);
						UndoManager::GetInstance()->AddFrameF(y_track, data[1], selectedFrame);
					}
				}
			}
		}
		ImGui::PopItemWidth();

		ImGui::TableNextColumn();
		ImGui::Text("Rotation");

		if (selectedAnimation == 0) {
			ImGui::SetItemTooltip("Select animation to set keyframes");
		}
		else if (selectedFrame < 0) {
			ImGui::SetItemTooltip("Select frame to set keyframes");
		}
		else {
			ImGui::SetItemTooltip("Click to animate rotation");
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			ImGui::OpenPopup("##KEY_ROTATION");
		}
		if (ImGui::BeginPopup("##KEY_ROTATION")) {
			float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));

			if (selectedAnimation != 0 && selectedFrame >= 0) {
				if (ImGui::Selectable("Keyframe rotation")) {
					Track* r_track = selectedAnimation->GetTrack(this, "rotation");// 
					float value = rotation;
					if (r_track != 0 && !r_track->IsDeleted() && r_track->HasFrames()) {
						value = r_track->InterpolateF(selectedFrame, selectedAnimation->looping);
					}

					value = TransformNode::NormalizedRotation(value);
					if (r_track == 0 && selectedFrame >= 0) {
						r_track = selectedAnimation->AddTrack(this, "rotation", TrackType::TRACK_FLOAT);
					}
					if (r_track != 0) {
						//r_track->AddFrameF(value, selectedFrame);
						UndoManager::GetInstance()->AddFrameF(r_track, value, selectedFrame);
					}
				}
				Application::GetInstance()->PushXenon();
				ImGui::SeparatorText("Editor");
				ImGui::PopFont();
			}

			bool selected = Application::GetInstance()->GetRotationDisplay() == RotationDisplayType::RADIAN_SLIDER;
			if (ImGui::MenuItem("Radian Slider", "", &selected)) {
				Application::GetInstance()->SetRotationDisplay(RotationDisplayType::RADIAN_SLIDER);
			}
			selected = Application::GetInstance()->GetRotationDisplay() == RotationDisplayType::RADIAN_TEXT;
			if (ImGui::MenuItem("Radian Text", "", &selected)) {
				Application::GetInstance()->SetRotationDisplay(RotationDisplayType::RADIAN_TEXT);
			}
			selected = Application::GetInstance()->GetRotationDisplay() == RotationDisplayType::ANGLE_TEXT;
			if (ImGui::MenuItem("Angle Text", "", &selected)) {
				Application::GetInstance()->SetRotationDisplay(RotationDisplayType::ANGLE_TEXT);
			}

			ImGui::PopStyleVar();
			ImGui::EndPopup();
		}

		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		RotationDisplayType display = Application::GetInstance()->GetRotationDisplay();
		
		if (selectedAnimation == 0 || selectedAnimation->deleted) {
			float activeRotation = rotation;
			if (display == RotationDisplayType::RADIAN_SLIDER) {
				ImGui::SliderScalar("##rotationTransformNodeRs", ImGuiDataType_Float, &activeRotation, &minRot, &maxRot);
			}
			else if (display == RotationDisplayType::RADIAN_TEXT) {
				ImGui::InputFloat("##rotationTransformNodeRt", &activeRotation, 0.01f, 0.1f);
			}
			else if (display == RotationDisplayType::ANGLE_TEXT) {
				activeRotation *= 57.2958f;
				ImGui::InputFloat("##rotationTransformNodeAt", &activeRotation, 0.01f, 0.1f, "%.3f");
				activeRotation *= 0.0174533f;
			}
			else {
				IM_ASSERT(false);
			}

			if (fabsf(activeRotation - rotation) > 0.00001f) {
				UndoManager::GetInstance()->SetRotation(this, activeRotation);
			}
		}
		else {
			Track* r_track = selectedAnimation->GetTrack(this, "rotation");// 
			float value = rotation;
			if (r_track != 0 && !r_track->IsDeleted() && r_track->HasFrames()) {
				value = r_track->InterpolateF(selectedFrame, selectedAnimation->looping);
			}

			if (display == RotationDisplayType::RADIAN_SLIDER) {
				if (ImGui::SliderScalar("##rotationTransformNodeRs", ImGuiDataType_Float, &value, &minRot, &maxRot)) {
					value = TransformNode::NormalizedRotation(value);

					if (r_track == 0 && selectedFrame >= 0) {
						r_track = selectedAnimation->AddTrack(this, "rotation", TrackType::TRACK_FLOAT);
					}
					if (r_track != 0) {
						//r_track->AddFrameF(value, selectedFrame);
						UndoManager::GetInstance()->AddFrameF(r_track, value, selectedFrame);
					}
				}
			}
			else if (display == RotationDisplayType::RADIAN_TEXT) {
				if (ImGui::InputFloat("##rotationTransformNodeRt", &value, 0.01f, 0.1f)) {
					value = TransformNode::NormalizedRotation(value);

					if (r_track == 0 && selectedFrame >= 0) {
						r_track = selectedAnimation->AddTrack(this, "rotation", TrackType::TRACK_FLOAT);
					}
					if (r_track != 0) {
						//r_track->AddFrameF(value, selectedFrame);
						UndoManager::GetInstance()->AddFrameF(r_track, value, selectedFrame);
					}
				}
			}
			else if (display == RotationDisplayType::ANGLE_TEXT) {
				value *= 57.2958f;
				if (ImGui::InputFloat("##rotationTransformNodeAt", &value, 0.01f, 0.1f)) {
					value *= 0.0174533f;
					value = TransformNode::NormalizedRotation(value);

					if (r_track == 0 && selectedFrame >= 0) {
						r_track = selectedAnimation->AddTrack(this, "rotation", TrackType::TRACK_FLOAT);
					}
					if (r_track != 0) {
						//r_track->AddFrameF(value, selectedFrame);
						UndoManager::GetInstance()->AddFrameF(r_track, value, selectedFrame);
					}
				}
			}
			else {
				IM_ASSERT(false);
			}
		}
		ImGui::PopItemWidth();

		ImGui::TableNextColumn();
		ImGui::Text("Scale");

		if (selectedAnimation == 0) {
			ImGui::SetItemTooltip("Select animation to set keyframes");
		}
		else if (selectedFrame < 0) {
			ImGui::SetItemTooltip("Select frame to set keyframes");
		}
		else {
			ImGui::SetItemTooltip("Click to animate scale");
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
			if (selectedAnimation != 0 && selectedFrame >= 0) {
				ImGui::OpenPopup("##KEY_SCALE");
			}
		}
		if (ImGui::BeginPopup("##KEY_SCALE")) {
			if (selectedAnimation != 0 && selectedFrame >= 0) { // Not needed...
				bool scale_x = false;
				bool scale_y = false;
				if (ImGui::Selectable("Keyframe X")) {
					scale_x = true;
				}
				if (ImGui::Selectable("Keyframe Y")) {
					scale_y = true;
				}
				if (ImGui::Selectable("Keyframe Both")) {
					scale_x = true;
					scale_y = true;
				}

				Track* x_track = selectedAnimation->GetTrack(this, "scale.x");// 
				Track* y_track = selectedAnimation->GetTrack(this, "scale.y");//

				float data[2] = { scale.x, scale.y }; // Default to hierarchy
				if (x_track != 0 && !x_track->IsDeleted() && x_track->HasFrames()) {
					data[0] = x_track->InterpolateF(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
				}
				if (y_track != 0 && !y_track->IsDeleted() && y_track->HasFrames()) {
					data[1] = y_track->InterpolateF(selectedFrame, selectedAnimation->looping);// y_frame->data.asFloat;
				}

				if (scale_x) {
					if (x_track == 0 && selectedFrame >= 0) {
						x_track = selectedAnimation->AddTrack(this, "scale.x", TrackType::TRACK_FLOAT);
					}
					if (x_track != 0) {
						//x_track->AddFrameF(data[0], selectedFrame);
						UndoManager::GetInstance()->AddFrameF(x_track, data[0], selectedFrame);
					}
				}
				if (scale_y) {
					if (y_track == 0 && selectedFrame >= 0) {
						y_track = selectedAnimation->AddTrack(this, "scale.y", TrackType::TRACK_FLOAT);
					}
					if (y_track != 0) {
						//y_track->AddFrameF(data[1], selectedFrame);
						UndoManager::GetInstance()->AddFrameF(y_track, data[1], selectedFrame);
					}
				}
			}
			ImGui::EndPopup();
		}


		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		if (selectedAnimation == 0 || selectedAnimation->IsDeleted()) {
			ImVec2 scl = scale;
			ImGui::InputFloat2("##scaleTransformNode", &scl.x);
			bool x_changed = fabsf(scl.x - scale.x) > 0.0001f;
			bool y_changed = fabsf(scl.y - scale.y) > 0.0001f;

			if (x_changed || y_changed) {
				//scale = scl;
				UndoManager::GetInstance()->SetScale(this, scl, x_changed, y_changed);
			}
		}
		else {
			Track* x_track = selectedAnimation->GetTrack(this, "scale.x");// 
			Track* y_track = selectedAnimation->GetTrack(this, "scale.y");//

			float data[2] = { scale.x, scale.y }; // Default to hierarchy
			if (x_track != 0 && !x_track->IsDeleted() && x_track->HasFrames()) {
				data[0] = x_track->InterpolateF(selectedFrame, selectedAnimation->looping);// x_frame->data.asFloat;
			}
			if (y_track != 0 && !y_track->IsDeleted() && y_track->HasFrames()) {
				data[1] = y_track->InterpolateF(selectedFrame, selectedAnimation->looping);// y_frame->data.asFloat;
			}
			float old[2] = { data[0], data[1] };

			if (ImGui::InputFloat2("##scaleTransformNode", data)) {
				if (data[0] != old[0]) {
					if (x_track == 0 && selectedFrame >= 0) {
						x_track = selectedAnimation->AddTrack(this, "scale.x", TrackType::TRACK_FLOAT);
					}
					if (x_track != 0) {
						//x_track->AddFrameF(data[0], selectedFrame);
						UndoManager::GetInstance()->AddFrameF(x_track, data[0], selectedFrame);
					}
				}
				if (data[1] != old[1]) {
					if (y_track == 0 && selectedFrame >= 0) {
						y_track = selectedAnimation->AddTrack(this, "scale.y", TrackType::TRACK_FLOAT);
					}
					if (y_track != 0) {
						//y_track->AddFrameF(data[1], selectedFrame);
						UndoManager::GetInstance()->AddFrameF(y_track, data[1], selectedFrame);
					}
				}
			}
		}
		ImGui::PopItemWidth();
		ImGui::EndTable();
	}
	else {
		ImGui::PopFont();
	}
}

void TransformNode::SerializeInner(std::stringstream& out, int indent) {
	SceneNode::SerializeInner(out, indent);
	if (parent == 0 || isDeleted) {
		return;
	}
	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();

	out << tabs << "\t\"position\": { \"x\": " << position.x << ", \"y\": " << position.y << " }, \n";
	out << tabs << "\t\"rotation\": " << rotation << ",\n";
	out << tabs << "\t\"scale\": { \"x\": " << scale.x << ", \"y\": " << scale.y << " }, \n";

}