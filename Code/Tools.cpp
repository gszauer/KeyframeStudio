#include "Tools.h"
#include "Asset.h"
#include "Libraries/imgui.h"
#include "UndoManager.h"
#include "SpriteNode.h"
#include "Application.h"

#include <iostream>


// TODO: Still have static data :(
float Tool::toolZoom = 0.0f;

Tool::Tool() {
	target = 0;
	scaled = false;
	isActive = false;
	rtti = ToolNodeType::BASE;
	lastDrawnAsset = 0;
	lastFrame = -1;
	isGlobal = false;
	constraint = ToolAxisConstraint::NONE;
}
Tool::~Tool() {

}
void Tool::ImGuiEditorBar() {
	ImGui::Text("Base tool this should not ever be called!");
	IM_ASSERT(false); // All children must override
}
void Tool::OnMouseDown(float x, float y, ToolMouseButton button) {
}
void Tool::OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button) {
}
void Tool::OnMouseUp(ToolMouseButton button) {
}
void Tool::OnScroll(float wheelDelta) {
}
void Tool::Draw(ImDrawList* list, ImVec2 offset, float zoom, AnimationAsset* anim, int frame) {
}

TranslateTool::TranslateTool() {
	snapValue = 32;
	target = 0;
	isActive = false;
	isGlobal = false;
	snapEnabled = false;
	constraint = ToolAxisConstraint::NONE;
	rtti = ToolNodeType::TRANSLATE;

	arrowLength = 64;
	arrowWidth = 16;
	arrowHeight = 24;
	rectWidth = 4;
	omniBoxSize = 16;
}

TranslateTool::~TranslateTool() {

}
void TranslateTool::OnMouseDown(float x, float y, ToolMouseButton button) {
	if (button != ToolMouseButton::LEFT) {
		return;
	}

	IM_ASSERT(!isActive);
	mouseStart = mouseCurrent = ImVec2(x, y);
	ImVec2 mousePos(x, y);
	constraint = ToolAxisConstraint::NONE;
	if (target != 0) {
		if (target->rtti == NodeType::TRANSFORM_NODE || target->rtti == NodeType::SPRITE_NODE) {
			if (RectangleContains(lastOmniSquare[0], lastOmniSquare[1], lastOmniSquare[2], lastOmniSquare[3], mousePos)) {
				isActive = true;
				constraint = ToolAxisConstraint::NONE;
			}
			else if (TriangleContains(lastUpArrow[0], lastUpArrow[1], lastUpArrow[2], mousePos) ||
					 RectangleContains(lastUpSquare[0], lastUpSquare[1], lastUpSquare[2], lastUpSquare[3], mousePos)) {
				isActive = true;
				constraint = ToolAxisConstraint::Y;
			}
			else if (TriangleContains(lastRightArrow[0], lastRightArrow[1], lastRightArrow[2], mousePos) ||
					 RectangleContains(lastRightSquare[0], lastRightSquare[1], lastRightSquare[2], lastRightSquare[3], mousePos)) {
				isActive = true;
				constraint = ToolAxisConstraint::X;
			}

			if (isActive) {
				TransformNode* xform = (TransformNode*)target;
				objectStart = xform->position;
				if (lastDrawnAsset != 0 && lastDrawnAsset->Contains(target)) {
					Track* x_track = lastDrawnAsset->GetTrack(target, "position.x");
					if (x_track != 0) {
						objectStart.x = x_track->InterpolateF(lastFrame, lastDrawnAsset->looping);
					}
					Track* y_track = lastDrawnAsset->GetTrack(target, "position.y");
					if (y_track != 0) {
						objectStart.y = y_track->InterpolateF(lastFrame, lastDrawnAsset->looping);
					}
				}
				ImMat3 worldMat = target->GetWorldMatrix(0, lastDrawnAsset, lastFrame);
				normalize(worldMat);

				if (constraint == ToolAxisConstraint::X) {
					if (isGlobal) {
						constraintAxis = ImVec2(1.0f, 0.0f);
					}
					else {
						constraintAxis = ImVec2(worldMat.xx, worldMat.xy);
					}
					normalize(constraintAxis);
				}
				else if(constraint == ToolAxisConstraint::Y) {
					if (isGlobal) {
						constraintAxis = ImVec2(0.0f, 1.0f);
					}
					else {
						constraintAxis = ImVec2(worldMat.yx, worldMat.yy);
					}
					normalize(constraintAxis);
				}

				objectCurrent = objectStart;
			}
		}
	}
}
void TranslateTool::OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button) {
	mouseCurrent = ImVec2(x, y);

	if (target != 0 && button == ToolMouseButton::LEFT && isActive) {
		if (target->rtti == NodeType::TRANSFORM_NODE || target->rtti == NodeType::SPRITE_NODE) {
			TransformNode* xform = (TransformNode*)target;

			ImVec2 movementVector = (mouseCurrent - mouseStart);

			if (constraint != ToolAxisConstraint::NONE) {
				movementVector = project(movementVector, constraintAxis);
			}

			ImMat3 parentWorldMat;
			if (xform->GetParent() != 0) {
				parentWorldMat = xform->GetParent()->GetWorldMatrix(this, lastDrawnAsset, lastFrame);
			}

			movementVector = (invert(parentWorldMat) * ImVec3(movementVector, 0.0f)).Vec2();
			movementVector = (movementVector  / Tool::toolZoom);
			if (snapEnabled) {
				movementVector.x = (float)((int)(movementVector.x / (float)snapValue) * snapValue);
				movementVector.y = (float)((int)(movementVector.y / (float)snapValue) * snapValue);
			}
			objectCurrent = objectStart + movementVector;
		}
	}
}
void TranslateTool::OnMouseUp(ToolMouseButton button) {
	if (target != 0 && button == ToolMouseButton::LEFT && isActive) {
		if (target->rtti == NodeType::TRANSFORM_NODE || target->rtti == NodeType::SPRITE_NODE) {
			TransformNode* xform = (TransformNode*)target;

			if (lastDrawnAsset != 0) {
				if (constraint != ToolAxisConstraint::Y) {
					Track* x_track = lastDrawnAsset->GetTrack(target, "position.x");// 
					if (x_track == 0 && lastFrame >= 0) {
						x_track = lastDrawnAsset->AddTrack(target, "position.x", TrackType::TRACK_FLOAT);
					}
					if (x_track != 0) {
						//x_track->AddFrameF(objectCurrent.x, lastFrame);
						UndoManager::GetInstance()->AddFrameF(x_track, objectCurrent.x, lastFrame);
					}
				}

				if (constraint != ToolAxisConstraint::X) {
					Track* y_track = lastDrawnAsset->GetTrack(target, "position.y");//
					if (y_track == 0 && lastFrame >= 0) {
						y_track = lastDrawnAsset->AddTrack(target, "position.y", TrackType::TRACK_FLOAT);
					}
					if (y_track != 0) {
						//y_track->AddFrameF(objectCurrent.y, lastFrame);
						UndoManager::GetInstance()->AddFrameF(y_track, objectCurrent.y, lastFrame);
					}
				}
			}
			else {
				//xform->position = objectCurrent;
				bool x_changed = fabsf(xform->position.x - objectCurrent.x) >= 0.0001f;
				bool y_changed = fabsf(xform->position.y - objectCurrent.y) >= 0.0001f;
				if (x_changed || y_changed) {
					UndoManager::GetInstance()->SetPosition(xform, objectCurrent, x_changed, y_changed);
				}
			}
		}
	}
	isActive = false;
}
void TranslateTool::Draw(ImDrawList* list, ImVec2 offset, float zoom, AnimationAsset* anim, int frame) {
	if (!scaled) {
		float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
		arrowLength *= windowDevicePixelRatio;
		arrowWidth *= windowDevicePixelRatio;
		arrowHeight *= windowDevicePixelRatio;
		rectWidth *= windowDevicePixelRatio;
		omniBoxSize *= windowDevicePixelRatio;
		scaled = true;
	}

	lastDrawnAsset = anim;
	lastFrame = frame;

	if (target == 0) { return; }

	ImMat3 worldMatrix = target->GetWorldMatrix(this, anim, frame);
	if (isGlobal) {
		worldMatrix.xx = 1.0f;
		worldMatrix.xy = 0.0f;
		worldMatrix.yx = 0.0f;
		worldMatrix.yy = 1.0f;
	}
	else {
		normalize(worldMatrix);
	}

	float shapeScale = 1.0f / zoom;

	{ // Up / Down arrow
		ImVec2 q_p1((-rectWidth * 0.5f) * shapeScale, 0);
		ImVec2 q_p2((rectWidth * 0.5f) * shapeScale, 0);
		ImVec2 q_p3((rectWidth * 0.5f) * shapeScale, (arrowLength - arrowHeight + 1) * shapeScale);
		ImVec2 q_p4((-rectWidth * 0.5f) * shapeScale, (arrowLength - arrowHeight + 1) * shapeScale);

		q_p1 = lastUpSquare[0] = ((offset) + (worldMatrix * ImVec3(q_p1, 1)).Vec2() * zoom);
		q_p2 = lastUpSquare[1] = ((offset) + (worldMatrix * ImVec3(q_p2, 1)).Vec2() * zoom);
		q_p3 = lastUpSquare[2] = ((offset) + (worldMatrix * ImVec3(q_p3, 1)).Vec2() * zoom);
		q_p4 = lastUpSquare[3] = ((offset) + (worldMatrix * ImVec3(q_p4, 1)).Vec2() * zoom);

		ImVec2 t_p1 = ImVec2((-arrowWidth * 0.5f) * shapeScale, (arrowLength - arrowHeight) * shapeScale);
		ImVec2 t_p2 = ImVec2((arrowWidth * 0.5f) * shapeScale, (arrowLength - arrowHeight) * shapeScale);
		ImVec2 t_p3 = ImVec2(0.0f, arrowLength * shapeScale);

		t_p1 = lastUpArrow[0] = ((offset) + (worldMatrix * ImVec3(t_p1, 1)).Vec2() * zoom);
		t_p2 = lastUpArrow[1] = ((offset) + (worldMatrix * ImVec3(t_p2, 1)).Vec2() * zoom);
		t_p3 = lastUpArrow[2] = ((offset) + (worldMatrix * ImVec3(t_p3, 1)).Vec2() * zoom);

		ImGuiIO& io = ImGui::GetIO();
		ImU32 color = IM_COL32(0, 255, 0, 255);
		bool triangleContains = TriangleContains(t_p1, t_p2, t_p3, io.MousePos);
		bool rectangleContains = RectangleContains(q_p1, q_p2, q_p3, q_p4, io.MousePos);
		if (isActive || triangleContains || rectangleContains) {
			if (isActive && constraint == ToolAxisConstraint::Y) {
				color = IM_COL32(255, 255, 0, 255);
			}
			else if (!isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
		}

		list->AddQuadFilled(q_p1, q_p2, q_p3, q_p4, color);
		list->AddTriangleFilled(t_p1, t_p2, t_p3, color);
	}
	{ // Left / Right arrow
		ImVec2 q_p1(0, (-rectWidth * 0.5f) * shapeScale);
		ImVec2 q_p2(0, (rectWidth * 0.5f) * shapeScale);
		ImVec2 q_p3((arrowLength - arrowHeight + 1) * shapeScale, (rectWidth * 0.5f)  * shapeScale);
		ImVec2 q_p4((arrowLength - arrowHeight + 1) * shapeScale, (-rectWidth * 0.5f) * shapeScale);

		q_p1 = lastRightSquare[0] = (offset + (worldMatrix * ImVec3(q_p1, 1)).Vec2() * zoom);
		q_p2 = lastRightSquare[1] = (offset + (worldMatrix * ImVec3(q_p2, 1)).Vec2() * zoom);
		q_p3 = lastRightSquare[2] = (offset + (worldMatrix * ImVec3(q_p3, 1)).Vec2() * zoom);
		q_p4 = lastRightSquare[3] = (offset + (worldMatrix * ImVec3(q_p4, 1)).Vec2() * zoom);

		ImVec2 t_p1 = ImVec2((arrowLength - arrowHeight) * shapeScale, (-arrowWidth * 0.5f) * shapeScale);
		ImVec2 t_p2 = ImVec2((arrowLength - arrowHeight) * shapeScale, (arrowWidth  * 0.5f) * shapeScale);
		ImVec2 t_p3 = ImVec2((arrowLength)*shapeScale, 0.0f);

		t_p1 = lastRightArrow[0] = (offset + (worldMatrix * ImVec3(t_p1, 1)).Vec2() * zoom);
		t_p2 = lastRightArrow[1] = (offset + (worldMatrix * ImVec3(t_p2, 1)).Vec2() * zoom);
		t_p3 = lastRightArrow[2] = (offset + (worldMatrix * ImVec3(t_p3, 1)).Vec2() * zoom);

		ImGuiIO& io = ImGui::GetIO();
		ImU32 color = IM_COL32(255, 0, 0, 255);
		bool triangleContains = TriangleContains(t_p1, t_p2, t_p3, io.MousePos);
		bool rectangleContains = RectangleContains(q_p1, q_p2, q_p3, q_p4, io.MousePos);
		if (isActive || triangleContains || rectangleContains) {
			if (isActive && constraint == ToolAxisConstraint::X) {
				color = IM_COL32(255, 255, 0, 255);
			}
			else if (!isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
		}

		list->AddQuadFilled(q_p1, q_p2, q_p3, q_p4, color);
		list->AddTriangleFilled(t_p1, t_p2, t_p3, color);
	}
	{ // Omni Box
		ImVec2 q_p1((-omniBoxSize * 0.5f) * shapeScale, (-omniBoxSize * 0.5f) * shapeScale);
		ImVec2 q_p2((omniBoxSize * 0.5f ) * shapeScale, (-omniBoxSize * 0.5f) * shapeScale);
		ImVec2 q_p3((omniBoxSize * 0.5f ) * shapeScale, (omniBoxSize *  0.5f) * shapeScale);
		ImVec2 q_p4((-omniBoxSize * 0.5f) * shapeScale, (omniBoxSize *  0.5f) * shapeScale);

		q_p1 = lastOmniSquare[0] = (offset + (worldMatrix * ImVec3(q_p1, 1)).Vec2() * zoom);
		q_p2 = lastOmniSquare[1] = (offset + (worldMatrix * ImVec3(q_p2, 1)).Vec2() * zoom);
		q_p3 = lastOmniSquare[2] = (offset + (worldMatrix * ImVec3(q_p3, 1)).Vec2() * zoom);
		q_p4 = lastOmniSquare[3] = (offset + (worldMatrix * ImVec3(q_p4, 1)).Vec2() * zoom);

		ImGuiIO& io = ImGui::GetIO();
		ImU32 color = IM_COL32(0, 0, 255, 255);
		bool rectangleContains = RectangleContains(q_p1, q_p2, q_p3, q_p4, io.MousePos);
		if (isActive || rectangleContains) {
			if (isActive && constraint == ToolAxisConstraint::NONE) {
				color = IM_COL32(255, 255, 0, 255);
			}
			else if (!isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
		}

		list->AddQuadFilled(q_p1, q_p2, q_p3, q_p4, color);
	}
}
void TranslateTool::ImGuiEditorBar() {
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	ImGui::Text("Snap ");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	ImGui::Checkbox("##translateToolLocalSnap", &snapEnabled);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(8 * windowDevicePixelRatio, 5 * windowDevicePixelRatio));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(125 * windowDevicePixelRatio);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	window->DC.CursorPosPrevLine.y -= 2 * windowDevicePixelRatio;
	ImGui::InputInt("##translateToolsnapValue", &snapValue);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	window->DC.CursorPosPrevLine.y += 2 * windowDevicePixelRatio;
	if (snapValue < 0) {
		snapValue = 0;
	}

	ImGui::SameLine();
	ImGui::Text("  World Space ");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	ImGui::Checkbox("##translateToolLocalSpace", &isGlobal);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
}

RotateTool::RotateTool() {
	rtti = ToolNodeType::ROTATE;
	radius = 64.0f;
	thickness = 6.0f;
	arrowWidth = 16.0f;
	arrowHeight = 16.0f;
	snapEnabled = false;
	snapValue = 15;
	rotateStart = 0.0f;
	rotateCurrent = 0.0f;
	lastZoom = 0;
}
RotateTool::~RotateTool() {

}
void RotateTool::ImGuiEditorBar() {
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	ImGui::Text("Snap ");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	ImGui::Checkbox("##rotateToolLocalSnap", &snapEnabled);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(8 * windowDevicePixelRatio, 5 * windowDevicePixelRatio));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(125 * windowDevicePixelRatio);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	window->DC.CursorPosPrevLine.y -= 2 * windowDevicePixelRatio;
	ImGui::InputInt("##rotateToolsnapValue", &snapValue);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	window->DC.CursorPosPrevLine.y += 2 * windowDevicePixelRatio;
	if (snapValue < 0) {
		snapValue = 0;
	}
}
void RotateTool::OnMouseDown(float x, float y, ToolMouseButton button) {
	//float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	if (button == ToolMouseButton::LEFT && target != 0) {
		ImVec2 mouse(x, y);
		ImMat3 worldMatrix = target->GetWorldMatrix(this, lastDrawnAsset, lastFrame);
		ImVec2 gizmoLocation = lastOffset + (worldMatrix * ImVec3(0, 0, 1)).Vec2() * lastZoom;

		if (CircleContains(gizmoLocation, (radius + thickness), mouse) && !CircleContains(gizmoLocation, (radius - thickness), mouse)) {
			mouseStart = mouseCurrent = mouse;

			if (lastDrawnAsset != 0 && lastDrawnAsset->Contains(target)) {
				rotateStart = lastDrawnAsset->GetRotation(target, lastFrame);
			}
			else {
				rotateStart = target->GetRotation();
			}
			rotateCurrent = 0.0f;

			isActive = true;
		}
	}
}
void RotateTool::OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button) {
	if (button == ToolMouseButton::LEFT && isActive) {
		float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
		mouseCurrent = ImVec2(x, y);

		ImMat3 worldMatrix = target->GetWorldMatrix(this, lastDrawnAsset, lastFrame);
		ImVec2 gizmoLocation = lastOffset + (worldMatrix * ImVec3(0, 0, 1)).Vec2() * lastZoom;

		ImVec2 start = normalized(mouseStart - gizmoLocation);
		ImVec2 side = perp(start);
		ImVec2 current = normalized(mouseCurrent - gizmoLocation);

		rotateCurrent = angle(start, current);

		if (snapEnabled) {
			rotateCurrent *= 57.2958f;
			rotateCurrent = (float)((int)(rotateCurrent / (float)snapValue) * snapValue);
			rotateCurrent = rotateCurrent * 0.0174533f;
		}

		if (rotateCurrent != rotateCurrent) {
			rotateCurrent = angle(start, current);
			IM_ASSERT(false);
		}

		if (dot(current, side) <= 0.0f) {
			rotateCurrent = IM_PI + IM_PI - rotateCurrent;
		}
	}
}
void RotateTool::OnMouseUp(ToolMouseButton button) {
	if (button == ToolMouseButton::LEFT && target != 0 && isActive) {
		ImMat3 worldMatrix = target->GetWorldMatrix(this, lastDrawnAsset, lastFrame);
		ImVec2 gizmoLocation = lastOffset + (worldMatrix * ImVec3(0, 0, 1)).Vec2() * lastZoom;

		if (target->rtti == NodeType::TRANSFORM_NODE || target->rtti == NodeType::SPRITE_NODE) {
			TransformNode* xform = (TransformNode*)target;
			if (lastDrawnAsset != 0) {
				Track* track = lastDrawnAsset->GetTrack(target, "rotation");// 
				if (track == 0 && lastFrame >= 0) {
					track = lastDrawnAsset->AddTrack(target, "rotation", TrackType::TRACK_FLOAT);
				}
				if (track != 0) {
					//track->AddFrameF(TransformNode::NormalizedRotation(rotateStart + rotateCurrent), lastFrame);
					UndoManager::GetInstance()->AddFrameF(track, TransformNode::NormalizedRotation(rotateStart + rotateCurrent), lastFrame);
				}
			}
			else {
				//xform->SetRotation(rotateStart + rotateCurrent);
				UndoManager::GetInstance()->SetRotation(xform, rotateStart + rotateCurrent);
			}
		}
	}
	isActive = false;
}
void RotateTool::Draw(ImDrawList* list, ImVec2 offset, float zoom, AnimationAsset* anim, int frame) {
	if (!scaled) {
		scaled = true;
		float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
		radius *= windowDevicePixelRatio;
		thickness *= windowDevicePixelRatio;
		arrowWidth *= windowDevicePixelRatio;
		arrowHeight *= windowDevicePixelRatio;
	}
	if (target == 0) {
		return;
	}

	lastDrawnAsset = anim;
	lastFrame = frame; 
	lastOffset = offset;
	lastZoom = zoom;

	ImMat3 worldMatrix = target->GetWorldMatrix(this, anim, frame);
	ImVec2 gizmoLocation = offset + (worldMatrix * ImVec3(0, 0, 1)).Vec2() * zoom;

	{ // Fix up world matrix
		ImVec2 matrixLocation = (worldMatrix * ImVec3(0, 0, 1)).Vec2();
		worldMatrix = RotationMatrix(target->GetWorldRotation(this, anim, frame));
		worldMatrix.zx = matrixLocation.x;
		worldMatrix.zy = matrixLocation.y;
	}

	ImU32 color = IM_COL32(0, 0, 255, 255);
	ImVec2 mouse = ImGui::GetIO().MousePos;
	bool hover = (CircleContains(gizmoLocation, (radius + thickness), mouse) && !CircleContains(gizmoLocation, (radius - thickness), mouse));

	if (isActive || hover) {
		color = IM_COL32(255, 255, 0, 255);
	}
	list->AddCircle(gizmoLocation, radius, color, 32, thickness);

	if (isActive) { // Start line
		ImVec2 p0 = gizmoLocation + (RotationMatrix(rotateStart) * ImVec3(0, 0, 1)).Vec2() * zoom;
		ImVec2 p1 = p0 + normalized(mouseStart - gizmoLocation) * (radius);
		list->AddLine(p0, p1, color, thickness * 0.5f);
	}

	color = IM_COL32(0, 0, 255, 255);
	if (isActive) { // Current
		//ImVec2 p0 = offset + (worldMatrix * ImVec3(0, 0, 1)).Vec2() * zoom;
		ImVec2 p0 = gizmoLocation + (RotationMatrix(rotateCurrent) * ImVec3(0, 0, 1)).Vec2() * zoom;
		ImVec2 p1 = p0 + normalized(mouseCurrent - gizmoLocation) * (radius);
		list->AddLine(p0, p1, color, thickness * 0.5f);
	}
}

ScaleTool::ScaleTool() {
	rtti = ToolNodeType::SCALE;
	handleSize = 64;
	handleWidth = 4;
	nibSize = 16;
	omniSize = 16;
	snapEnabled = false;
	snapValue = 10;
	extraLength = 0.0f;
	sensitivity = 64.0f;
	currentScale = 0.0f;
}
ScaleTool::~ScaleTool() {

}
void ScaleTool::ImGuiEditorBar() {
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	ImGui::Text("Snap ");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	ImGui::Checkbox("##scaleToolLocalSnap", &snapEnabled);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(8 * windowDevicePixelRatio, 5 * windowDevicePixelRatio));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(125 * windowDevicePixelRatio);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	window->DC.CursorPosPrevLine.y -= 2 * windowDevicePixelRatio;
	ImGui::InputInt("##scaleToolsnapValue", &snapValue);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	window->DC.CursorPosPrevLine.y += 2 * windowDevicePixelRatio;
	if (snapValue < 0) {
		snapValue = 0;
	}

	ImGui::SameLine();
	ImGui::Dummy(ImVec2(5* windowDevicePixelRatio, 5 * windowDevicePixelRatio));
	ImGui::SameLine();
	ImGui::Text(" Sensitivity  ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(250 * windowDevicePixelRatio);
	float minVal = 1.0f;
	float maxVal = 128.0f;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	if (ImGui::SliderScalar("##scaleToolSensitivityValue", ImGuiDataType_Float, &sensitivity, &minVal, &maxVal)) {
		if (sensitivity < minVal) {
			sensitivity = minVal;
		}
		if (sensitivity > maxVal) {
			sensitivity = maxVal;
		}
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
}
void ScaleTool::OnMouseDown(float x, float y, ToolMouseButton button) {
	if (button == ToolMouseButton::LEFT && target != 0) {
		IM_ASSERT(!isActive);
		ImVec2 mouse = mouseStart = mouseCurrent = ImVec2(x, y);
		extraLength = 0;

		constraint = ToolAxisConstraint::NONE;
		if (RectangleContains(lastOmni[0], lastOmni[1], lastOmni[2], lastOmni[3], mouse)) {
			constraint = ToolAxisConstraint::NONE;
			isActive = true;
		}
		else if (RectangleContains(lastUpHandle[0], lastUpHandle[1], lastUpHandle[2], lastUpHandle[3], mouse) ||
				 RectangleContains(lastUpNib[0], lastUpNib[1], lastUpNib[2], lastUpNib[3], mouse)) {
			constraint = ToolAxisConstraint::Y;
			isActive = true;
		}
		else if (RectangleContains(lastRightHandle[0], lastRightHandle[1], lastRightHandle[2], lastRightHandle[3], mouse) ||
			RectangleContains(lastRightNib[0], lastRightNib[1], lastRightNib[2], lastRightNib[3], mouse)) {
			constraint = ToolAxisConstraint::X;
			isActive = true;
		}

		if (isActive) {
			TransformNode* xform = (TransformNode*)target;
			startScale = xform->scale;
			if (lastDrawnAsset != 0 && lastDrawnAsset->Contains(target)) {
				Track* x_track = lastDrawnAsset->GetTrack(target, "scale.x");
				if (x_track != 0 ) {
					startScale.x = x_track->InterpolateF(lastFrame, lastDrawnAsset->looping);
				}
				Track* y_track = lastDrawnAsset->GetTrack(target, "scale.y");
				if (y_track != 0 ) {
					startScale.y = y_track->InterpolateF(lastFrame, lastDrawnAsset->looping);
				}
			}
			currentScale = 0.0f;

			ImMat3 worldMat = target->GetWorldMatrix(0, lastDrawnAsset, lastFrame);
			normalize(worldMat);

			if (constraint == ToolAxisConstraint::X) {
				constraintAxis = ImVec2(worldMat.xx, worldMat.xy);
			}
			else if (constraint == ToolAxisConstraint::Y) {
				constraintAxis = ImVec2(worldMat.yx, worldMat.yy);
			}
			else {
				constraintAxis = ImVec2(worldMat.xx, worldMat.xy) + 
					ImVec2(worldMat.yx, worldMat.yy);
			}
			normalize(constraintAxis);
		}
	}
}
void ScaleTool::OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button) {
	mouseCurrent = ImVec2(x, y);

	if (target != 0 && button == ToolMouseButton::LEFT && isActive) {
		if (target->rtti == NodeType::TRANSFORM_NODE || target->rtti == NodeType::SPRITE_NODE) {
			TransformNode* xform = (TransformNode*)target;

			ImVec2 movementVector = mouseCurrent - mouseStart;
			ImVec2 mouseVector    = mouseCurrent - mouseStart;

			if (snapEnabled) {
				mouseVector.x = (float)((int)(mouseVector.x / (float)snapValue) * snapValue);
				mouseVector.y = (float)((int)(mouseVector.y / (float)snapValue) * snapValue);
			}

			// Even the omni tool is axis constrained
			movementVector = project(mouseVector, constraintAxis);

			// Unlike the move tool, we will be figuring out everything in screen space
			// so there is no need to calculate the space relative to parent

			extraLength = len(movementVector); 
			if (dot(movementVector, constraintAxis) < 0.0f) {
				extraLength *= -1.0f;
			}
			currentScale = extraLength / sensitivity;
		}
	}
}
void ScaleTool::OnMouseUp(ToolMouseButton button) {
	if (target != 0 && button == ToolMouseButton::LEFT && isActive) {
		if (target->rtti == NodeType::TRANSFORM_NODE || target->rtti == NodeType::SPRITE_NODE) {
			TransformNode* xform = (TransformNode*)target;

			if (lastDrawnAsset != 0) {
				if (constraint != ToolAxisConstraint::Y) {
					Track* x_track = lastDrawnAsset->GetTrack(target, "scale.x");// 
					if (x_track == 0 && lastFrame >= 0 && constraint != ToolAxisConstraint::Y) {
						x_track = lastDrawnAsset->AddTrack(target, "scale.x", TrackType::TRACK_FLOAT);
					}
					if (x_track != 0 && constraint != ToolAxisConstraint::Y) {
						//x_track->AddFrameF(startScale.x + currentScale, lastFrame);
						UndoManager::GetInstance()->AddFrameF(x_track, startScale.x + currentScale, lastFrame);
					}
				}

				if (constraint != ToolAxisConstraint::X) {
					Track* y_track = lastDrawnAsset->GetTrack(target, "scale.y");//
					if (y_track == 0 && lastFrame >= 0 && constraint != ToolAxisConstraint::X) {
						y_track = lastDrawnAsset->AddTrack(target, "scale.y", TrackType::TRACK_FLOAT);
					}
					if (y_track != 0 && constraint != ToolAxisConstraint::X) {
						//y_track->AddFrameF(startScale.y + currentScale, lastFrame);
						UndoManager::GetInstance()->AddFrameF(y_track, startScale.y + currentScale, lastFrame);
					}
				}
			}
			else {
				UndoManager::GetInstance()->SetScale(xform, startScale + ImVec2(currentScale, currentScale), constraint != ToolAxisConstraint::Y, constraint != ToolAxisConstraint::X);
			}
		}
	}
	extraLength = 0.0f;
	isActive = false;
}
void ScaleTool::Draw(ImDrawList* list, ImVec2 offset, float zoom, AnimationAsset* anim, int frame) {
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	lastDrawnAsset = anim;
	lastFrame = frame;

	if (target == 0) { return; }
	ImGuiIO& io = ImGui::GetIO();

	ImMat3 worldMatrix = target->GetWorldMatrix(this, anim, frame);
	if (isGlobal) {
		worldMatrix.xx = 1.0f;
		worldMatrix.xy = 0.0f;
		worldMatrix.yx = 0.0f;
		worldMatrix.yy = 1.0f;
	}
	else { //normalize(worldMatrix);
		ImVec2 gizmoLocation = (worldMatrix * ImVec3(0, 0, 1)).Vec2();
		worldMatrix = RotationMatrix(target->GetWorldRotation(this, anim, frame));
		worldMatrix.zx = gizmoLocation.x;
		worldMatrix.zy = gizmoLocation.y;
	}

	float scl = 1.0f / zoom;

	{ // Y Handle
		float el = 0.0f;
		if (constraint == ToolAxisConstraint::Y) {
			el = extraLength / windowDevicePixelRatio;
		}
		ImVec2 q_p1((-handleWidth * 0.5f) * (scl), 0);
		ImVec2 q_p2((handleWidth * 0.5f) *  (scl), 0);
		ImVec2 q_p3((handleWidth * 0.5f) *  (scl), (handleSize + el)*(scl));
		ImVec2 q_p4((-handleWidth * 0.5f) * (scl), (handleSize + el)*(scl));

		ImVec2 q_p1_nib((-nibSize * 0.5f) * (scl), ((-nibSize * 0.5f) + (handleSize + el - nibSize * 0.5f)) * (scl));
		ImVec2 q_p2_nib((nibSize * 0.5f) *  (scl), ((-nibSize * 0.5f) + (handleSize + el - nibSize * 0.5f)) * (scl));
		ImVec2 q_p3_nib((nibSize * 0.5f) *  (scl), ((nibSize * 0.5f)  + (handleSize + el - nibSize * 0.5f) ) * (scl));
		ImVec2 q_p4_nib((-nibSize * 0.5f) * (scl), ((nibSize * 0.5f)  + (handleSize + el - nibSize * 0.5f) ) * (scl));

		q_p1 = lastUpHandle[0] = (offset)+(worldMatrix * ImVec3(q_p1 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p2 = lastUpHandle[1] = (offset)+(worldMatrix * ImVec3(q_p2 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p3 = lastUpHandle[2] = (offset)+(worldMatrix * ImVec3(q_p3 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p4 = lastUpHandle[3] = (offset)+(worldMatrix * ImVec3(q_p4 * windowDevicePixelRatio, 1)).Vec2() * zoom;

		q_p1_nib = lastUpNib[0] = (offset)+(worldMatrix * ImVec3(q_p1_nib * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p2_nib = lastUpNib[1] = (offset)+(worldMatrix * ImVec3(q_p2_nib * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p3_nib = lastUpNib[2] = (offset)+(worldMatrix * ImVec3(q_p3_nib * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p4_nib = lastUpNib[3] = (offset)+(worldMatrix * ImVec3(q_p4_nib * windowDevicePixelRatio, 1)).Vec2() * zoom;

		ImU32 color = IM_COL32(0, 255, 0, 255);
		bool rectangleContains = RectangleContains(q_p1, q_p2, q_p3, q_p4, io.MousePos);
		bool rectangleContains_nib = RectangleContains(q_p1_nib, q_p2_nib, q_p3_nib, q_p4_nib, io.MousePos);
		if (isActive || rectangleContains_nib || rectangleContains) {
			if (isActive && constraint == ToolAxisConstraint::Y) {
				color = IM_COL32(255, 255, 0, 255);
			}
			else if (!isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
		}

		list->AddQuadFilled(q_p1, q_p2, q_p3, q_p4, color);
		list->AddQuadFilled(q_p1_nib, q_p2_nib, q_p3_nib, q_p4_nib, color);
	}
	
	{ // X Handle
		float el = 0.0f;
		if (constraint == ToolAxisConstraint::X) {
			el = extraLength / windowDevicePixelRatio;
		}

		ImVec2 q_p1(0, (-handleWidth * 0.5f) * (scl));
		ImVec2 q_p2(0, (handleWidth * 0.5f)  * (scl));
		ImVec2 q_p3((handleSize + el)*(scl), ( handleWidth * 0.5f) *  (scl));
		ImVec2 q_p4((handleSize + el)*(scl), (-handleWidth * 0.5f) * (scl));

		ImVec2 q_p1_nib(((-nibSize * 0.5f) + (handleSize + el - nibSize * 0.5f) ) * (scl), (-nibSize * 0.5f) * (scl));
		ImVec2 q_p2_nib(((-nibSize * 0.5f) + (handleSize + el - nibSize * 0.5f) ) * (scl), (nibSize * 0.5f)  * (scl));
		ImVec2 q_p3_nib(((nibSize * 0.5f)  + (handleSize + el - nibSize * 0.5f) ) * (scl), (nibSize * 0.5f)  * (scl));
		ImVec2 q_p4_nib(((nibSize * 0.5f)  + (handleSize + el - nibSize * 0.5f) ) * (scl), (-nibSize * 0.5f) * (scl));

		q_p1 = lastRightHandle[0] = (offset)+(worldMatrix * ImVec3(q_p1 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p2 = lastRightHandle[1] = (offset)+(worldMatrix * ImVec3(q_p2 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p3 = lastRightHandle[2] = (offset)+(worldMatrix * ImVec3(q_p3 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p4 = lastRightHandle[3] = (offset)+(worldMatrix * ImVec3(q_p4 * windowDevicePixelRatio, 1)).Vec2() * zoom;

		q_p1_nib = lastRightNib[0] = (offset)+(worldMatrix * ImVec3(q_p1_nib * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p2_nib = lastRightNib[1] = (offset)+(worldMatrix * ImVec3(q_p2_nib * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p3_nib = lastRightNib[2] = (offset)+(worldMatrix * ImVec3(q_p3_nib * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p4_nib = lastRightNib[3] = (offset)+(worldMatrix * ImVec3(q_p4_nib * windowDevicePixelRatio, 1)).Vec2() * zoom;

		ImU32 color = IM_COL32(255, 0, 0, 255);
		bool rectangleContains = RectangleContains(q_p1, q_p2, q_p3, q_p4, io.MousePos);
		bool rectangleContains_nib = RectangleContains(q_p1_nib, q_p2_nib, q_p3_nib, q_p4_nib, io.MousePos);
		if (isActive || rectangleContains_nib || rectangleContains) {
			if (isActive && constraint == ToolAxisConstraint::X) {
				color = IM_COL32(255, 255, 0, 255);
			}
			else if (!isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
		}

		list->AddQuadFilled(q_p1, q_p2, q_p3, q_p4, color);
		list->AddQuadFilled(q_p1_nib, q_p2_nib, q_p3_nib, q_p4_nib, color);
	}

	{ // Omni Box
		ImVec2 q_p1((-omniSize * 0.5f) * (scl), (-omniSize * 0.5f) * (scl));
		ImVec2 q_p2(( omniSize * 0.5f) * (scl), (-omniSize * 0.5f) * (scl));
		ImVec2 q_p3(( omniSize * 0.5f) * (scl), ( omniSize * 0.5f) * (scl));
		ImVec2 q_p4((-omniSize * 0.5f) * (scl), ( omniSize * 0.5f) * (scl));

		q_p1 = lastOmni[0] = offset + (worldMatrix * ImVec3(q_p1 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p2 = lastOmni[1] = offset + (worldMatrix * ImVec3(q_p2 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p3 = lastOmni[2] = offset + (worldMatrix * ImVec3(q_p3 * windowDevicePixelRatio, 1)).Vec2() * zoom;
		q_p4 = lastOmni[3] = offset + (worldMatrix * ImVec3(q_p4 * windowDevicePixelRatio, 1)).Vec2() * zoom;

		ImGuiIO& io = ImGui::GetIO();
		ImU32 color = IM_COL32(0, 0, 255, 255);
		bool rectangleContains = RectangleContains(q_p1, q_p2, q_p3, q_p4, io.MousePos);
		if (isActive || rectangleContains) {
			if (isActive && constraint == ToolAxisConstraint::NONE) {
				color = IM_COL32(255, 255, 0, 255);
			}
			else if (!isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
		}

		list->AddQuadFilled(q_p1, q_p2, q_p3, q_p4, color);
	}

#if 0
	if (isActive && constraint != ToolAxisConstraint::NONE) {
		ImVec2 p1 = ImVec2(0, 0);
		ImVec2 p2 = constraintAxis * 200;
		// White line, constraint axis
		p1 = offset + p1 * zoom; p2 = offset + p2 * zoom;
		list->AddLine(p1, p2, IM_COL32(255, 255, 255, 255), 4.0f);

		p1 = ImVec2(0, 0);
		p2 = (mouseCurrent - mouseStart);// * 100;
		// Black line, mouse axis
		p1 = offset + p1 * zoom; p2 = offset + p2 * zoom;
		list->AddLine(p1, p2, IM_COL32(0, 0, 0, 255), 4.0f);

		p1 = ImVec2(0, 0);
		p2 = project((mouseCurrent - mouseStart), constraintAxis);
		// Light Blue, projected axis
		p1 = offset + p1 * zoom; p2 = offset + p2 * zoom;
		list->AddLine(p1, p2, IM_COL32(125, 125, 255, 255), 4.0f);


		p1 = ImVec2(0, 0);
		p2 = perp(constraintAxis) * 200;;
		// Light green, perpendicular axis
		p1 = offset + p1 * zoom; p2 = offset + p2 * zoom;
		list->AddLine(p1, p2, IM_COL32(125, 255, 125, 255), 4.0f);
	}
#endif
}

PanTool::PanTool() {
	zoom = 1.0f;
	minZoom = 0.05f;
	maxZoom = 20.0f;
	gridSize = 64.0f;
	gridVisible = true;
	rtti = ToolNodeType::PAN;
}
PanTool::~PanTool() {

}
void PanTool::ImGuiEditorBar() {
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	
	ImGui::Text("Pan ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150 * windowDevicePixelRatio);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	window->DC.CursorPosPrevLine.y -= 2 * windowDevicePixelRatio;
	ImGui::InputFloat2("##editorBarPanZoomPanVec2", &pan.x);
	window->DC.CursorPosPrevLine.y += 2 * windowDevicePixelRatio;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);

	ImGui::SameLine();
	ImGui::Text("  Grid ");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	ImGui::Checkbox("##editorBarPanGridVisible", &gridVisible);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(7 * windowDevicePixelRatio, 5 * windowDevicePixelRatio));
	ImGui::SameLine();
	int gridInt = (int)gridSize;
	ImGui::SetNextItemWidth(100 * windowDevicePixelRatio);
	//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	window->DC.CursorPosPrevLine.y -= 2 * windowDevicePixelRatio;
	ImGui::InputInt("##editorBarPanGrid", &gridInt);
	window->DC.CursorPosPrevLine.y += 2 * windowDevicePixelRatio;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	//ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
	gridSize = (float)gridInt;

	ImGui::SameLine();
	ImGui::Text("  Zoom ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 5 * windowDevicePixelRatio);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2 * windowDevicePixelRatio);
	if (ImGui::SliderScalar("##editorBarPanZoom", ImGuiDataType_Float, &zoom, &minZoom, &maxZoom)) {
		if (zoom < minZoom) {
			zoom = minZoom;
		}
		if (zoom > maxZoom) {
			zoom = maxZoom;
		}
	}
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * windowDevicePixelRatio);
}
void PanTool::OnMouseDown(float x, float y, ToolMouseButton button) { }
void PanTool::OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button) {
	//if (button == ToolMouseButton::MIDDLE || button == ToolMouseButton::RIGHT) {
		pan.x += deltaX;
		pan.y += deltaY;
	//}
}
void PanTool::OnMouseUp(ToolMouseButton button) { }
void PanTool::OnScroll(float wheelDelta) {
	zoom += wheelDelta * 0.1f;
	if (zoom < minZoom) {
		zoom = minZoom;
	}
	if (zoom > maxZoom) {
		zoom = maxZoom;
	}
}


// PIVOT
PivotTool::PivotTool() {
	rtti = ToolNodeType::PIVOT;
	lastZoom = 0;
	allowDrag = true;
	radius = 12.0f;
	thickness = 2.0f;
}
PivotTool::~PivotTool() { }

void PivotTool::ImGuiEditorBar() {
	ImGui::Text("Pivot Tool (no settings)");
	/*ImGui::SameLine();
	ImGui::Checkbox("##pivotToolMove", &allowDrag);*/
}

void PivotTool::OnMouseDown(float x, float y, ToolMouseButton button) { 
	mouseStart = mouseCurrent = ImVec2(x, y);
	ImVec2 mousePos(x, y);

	objectCurrentPos = objectCurrentPivot = ImVec2();

	isActive = false;
	if (target != 0 && target->rtti == NodeType::SPRITE_NODE) {
		ImMat3 worldMatrix = target->GetWorldMatrix(0, lastDrawnAsset, lastFrame);
		ImVec2 gizmoLocation = lastOffset + (worldMatrix * ImVec3(0, 0, 1)).Vec2() * lastZoom;

		if (lenSq(gizmoLocation - mousePos) < radius * radius) {
			isActive = true;
		}

		if (isActive) {
			SpriteNode* sprt = (SpriteNode*)target;
			outline = sprt;
			objectStartPos = sprt->position;
			objectStartPivot = sprt->pivot;

			if (lastDrawnAsset != 0 && lastDrawnAsset->Contains(target)) {
				Track* x_track = lastDrawnAsset->GetTrack(target, "position.x");
				if (x_track != 0) {
					objectStartPos.x = x_track->InterpolateF(lastFrame, lastDrawnAsset->looping);
				}
				Track* y_track = lastDrawnAsset->GetTrack(target, "position.y");
				if (y_track != 0) {
					objectStartPos.y = y_track->InterpolateF(lastFrame, lastDrawnAsset->looping);
				}
			}

			objectCurrentPos = objectStartPos;
			objectCurrentPivot = objectStartPivot;
		}
	}
}

void PivotTool::OnMouseDrag(float x, float y, float deltaX, float deltaY, ToolMouseButton button) {
	mouseCurrent = ImVec2(x, y);
	if (target != 0 && button == ToolMouseButton::LEFT && isActive) {
		if (NodeType::SPRITE_NODE) {
			SpriteNode* sprt = (SpriteNode*)target;

			ImVec2 movementVector = (mouseCurrent - mouseStart);

			ImMat3 parentWorldMat;
			if (sprt->GetParent() != 0) {
				parentWorldMat = sprt->GetParent()->GetWorldMatrix(0, lastDrawnAsset, lastFrame);
			}
			movementVector = (invert(parentWorldMat) * ImVec3(movementVector, 0.0f)).Vec2();
			
			ImVec2 invSize = sprt->size;
			if (invSize.x > 0.00001f || invSize.x < 0.00001f) {
				invSize.x = 1.0f / invSize.x;
			}
			if (invSize.y > 0.00001f || invSize.y < 0.00001f) {
				invSize.y = 1.0f / invSize.y;
			}

			if (sprt->frame != 0 && sprt->frame->rotated) {
				invSize = ImVec2(invSize.y, invSize.x);
			}
			
			ImVec2 movementVector2 = (invert(sprt->GetWorldMatrix(0, lastDrawnAsset, lastFrame)) * ImVec3((mouseCurrent - mouseStart), 0.0f)).Vec2();
			objectCurrentPivot = objectStartPivot - (movementVector2 * -1.0f / Tool::toolZoom) * invSize;
			
			objectCurrentPos = objectStartPos + movementVector / Tool::toolZoom;

			if (allowDrag) {
				if (objectCurrentPivot.x < 0.0f) { objectCurrentPivot.x = 0.0f; }
				if (objectCurrentPivot.y < 0.0f) { objectCurrentPivot.y = 0.0f; }
				if (objectCurrentPivot.x > 1.0f) { objectCurrentPivot.x = 1.0f; }
				if (objectCurrentPivot.y > 1.0f) { objectCurrentPivot.y = 1.0f; }
			}
		}
	}

}

void PivotTool::OnMouseUp(ToolMouseButton button) { 
	if (target != 0 && button == ToolMouseButton::LEFT && isActive) {
		if (target->rtti == NodeType::SPRITE_NODE) {
			SpriteNode* sprt = (SpriteNode*)target;

			bool x_changed = (fabsf(sprt->pivot.x - objectCurrentPivot.x) >= 0.0001f) || (fabsf(sprt->position.x - objectCurrentPos.x) >= 0.0001f);
			bool y_changed = (fabsf(sprt->pivot.y - objectCurrentPivot.y) >= 0.0001f) || (fabsf(sprt->position.y - objectCurrentPos.y) >= 0.0001f);
			if (x_changed || y_changed) {
				/*sprt->pivot.x = objectCurrentPivot.x;
				sprt->pivot.y = objectCurrentPivot.y;
				sprt->position.x = objectCurrentPos.x;
				sprt->position.y = objectCurrentPos.y;*/
					
				UndoManager::GetInstance()->SetPivotTool(sprt, objectCurrentPivot, objectCurrentPos);
			}

		}
	}
	isActive = false;
}

void PivotTool::Draw(ImDrawList* list, ImVec2 offset, float zoom, class AnimationAsset* anim, int frame) {
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
	if (!scaled) {
		scaled = true;

		radius *= windowDevicePixelRatio;
		thickness *= windowDevicePixelRatio;
	}
	lastDrawnAsset = anim;
	lastFrame = frame; 
	lastZoom = zoom;
	lastOffset = offset;
	
	if (target == 0) {
		return;
	}

	ImMat3 worldMatrix = target->GetWorldMatrix(this, anim, frame);
	ImVec2 gizmoLocation = offset + (worldMatrix * ImVec3(0, 0, 1)).Vec2() * zoom;

	{ // BACKGROUND
		ImU32 color = IM_COL32(0, 0, 0, 255);
		ImVec2 offset = ImVec2(thickness, thickness);
		{
			ImVec2 p0 = gizmoLocation - ImVec2(radius, 0);// *zoom;
			ImVec2 p1 = gizmoLocation + ImVec2(radius, 0);// *zoom;
			list->AddLine(p0 + offset, p1 + offset, color, thickness);
		}

		{
			ImVec2 p0 = gizmoLocation - ImVec2(0, radius);// *zoom;
			ImVec2 p1 = gizmoLocation + ImVec2(0, radius);// *zoom;
			list->AddLine(p0 + offset, p1 + offset, color, thickness);
		}

		{
			list->AddCircle(gizmoLocation + offset, radius, color, 16, thickness);
		}
	}

	{ // FOREGROUND
		ImU32 color = IM_COL32(200, 200, 200, 255);
		{
			if (isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
			ImVec2 p0 = gizmoLocation - ImVec2(radius, 0);// *zoom;
			ImVec2 p1 = gizmoLocation + ImVec2(radius, 0);// *zoom;
			list->AddLine(p0, p1, color, thickness);
		}

		{
			if (isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
			ImVec2 p0 = gizmoLocation - ImVec2(0, radius);// *zoom;
			ImVec2 p1 = gizmoLocation + ImVec2(0, radius);// *zoom;
			list->AddLine(p0, p1, color, thickness);
		}

		{
			if (isActive) {
				color = IM_COL32(255, 255, 0, 255);
			}
			list->AddCircle(gizmoLocation, radius, color, 16, thickness);
		}
	}
}