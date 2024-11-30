#include "SceneNode.h"
#include "Asset.h"
#include "Tools.h"
#include "Libraries/imgui.h"
#include "Libraries/imgui_stdlib.h"
#include "TransformNode.h"
#include "SpriteNode.h"
#include "Application.h"
#include "UndoManager.h"
#include "Platform.h"

#include <iostream>

SceneNode* SceneNode::trackingHead = 0;
SceneNode* SceneNode::clicked = 0;

void SceneNode::DestroyAll() {
	for (SceneNode* iter = SceneNode::trackingHead; iter != 0; ) {
		SceneNode* remove = iter;
		iter = iter->trackingNext;
		delete remove;
	}
	SceneNode::trackingHead = 0;
}

class SpriteNode* SceneNode::DeserializeSprite(const char* _name, const char* guid, const ImVec2& pos, float rot, const ImVec2& scl,
	const ImVec2& size, const ImVec2& pivot, const ImVec2& frameMin, const ImVec2& frameMax,
	const ImVec4& tint, int sort, bool visible, ImageAsset* image, AtlasFrame* frame) {

	SpriteNode* result = new SpriteNode(_name);
	result->uuid = guid;
	result->position = pos;
	result->rotation = rot;
	result->scale = scl;

	result->size = size;
	result->tint = tint;
	result->sort = sort;
	result->visible = visible;
	result->pivot = pivot;
	result->frameMin = frameMin;
	result->frameMax = frameMax;

	result->image = image;
	result->frame = frame;
	
	return result;
}

TransformNode* SceneNode::DeserializeTransform(const char* _name, const char* guid, const ImVec2& pos, float rot, const ImVec2& scl) {
	TransformNode* result = new TransformNode(_name);
	result->uuid = guid;
	result->position = pos;
	result->rotation = rot;
	result->scale = scl;
	return result;
}

std::string& SceneNode::GetName() {
	return name;
}

void SceneNode::SetName(const std::string& name) {
	this->name = name;
	this->nameInput = name;
}

SceneNode::SceneNode(const char* name) {
	parent = 0;
	firstChild = 0;
	nextSibling = 0;
	this->name = name == 0? "Scene Node" : name;
	this->nameInput = this->name;
	trackingNext = 0;
	rtti = NodeType::SCENE_NODE;

	trackingNext = trackingHead;
	trackingHead = this;

	isDeleted = false;

	uuid = MakeNewGuid();
}

SceneNode::~SceneNode() { }

void SceneNode::DragAndDropData::Reset() {
	mode = DragDropInsertMode::UNSET;
	source = target = 0;
}

SceneNode* SceneNode::GetParent() { return this->parent; }
SceneNode* SceneNode::GetFirstChild() { return this->firstChild; }
SceneNode* SceneNode::GetNextSibling() { return this->nextSibling; }

SceneNode* SceneNode::GetPrevSibling() {
	if (parent == 0 || parent->firstChild == this) {
		return 0;
	}
	for (SceneNode* iter = parent->firstChild; iter != 0; iter = iter->nextSibling) {
		if (iter->nextSibling == this) {
			return iter;
		}
	}
	return 0;
}

float SceneNode::GetRotation() {
	return 0.0f;
}

ImMat3 SceneNode::GetMatrix(ImMat3* outPos, ImMat3* outRot, ImMat3* outScl) {
	if (outPos != 0) {
		*outPos = ImMat3();
	}
	if (outRot != 0) {
		*outRot = ImMat3();
	}
	if (outScl != 0) {
		*outScl = ImMat3();
	}
	return ImMat3(); // Just give back identity if it's not present
}

float SceneNode::GetWorldRotation(class Tool* tool, class AnimationAsset* anim, int frame) {
	float worldRotation = 0.0f;

	for (SceneNode* iter = this; iter != 0; iter = iter->GetParent()) {
		float iterRot = 0.0f;

		if (anim != 0 && anim->Contains(iter)) {
			iterRot = anim->GetRotation(iter, frame);
		}
		else {
			iterRot = iter->GetRotation();
		}

		if (tool != 0 && tool->isActive && tool->target == iter) {
			if (tool->rtti == ToolNodeType::ROTATE) {
				iterRot += ((RotateTool*)tool)->rotateCurrent;
			}
		}

		worldRotation += iterRot;
	}
	return worldRotation;
}

ImMat3 SceneNode::GetWorldMatrix(Tool* tool, AnimationAsset* anim, int frame) {
	ImMat3 worldMat = ImMat3();
	
	for (SceneNode* iter = this; iter != 0; iter = iter->GetParent()) {
		ImMat3 iterMat;
		ImMat3 iterPos;
		ImMat3 iterRot;
		ImMat3 iterScl;
		if (anim != 0 && anim->Contains(iter)) {
			iterMat = anim->GetMatrix(iter, frame, &iterPos, &iterRot, &iterScl);
		}
		else {
			iterMat = iter->GetMatrix(&iterPos, &iterRot, &iterScl);
		}

		if (tool != 0 && tool->isActive && tool->target == iter) {
			if (tool->rtti == ToolNodeType::TRANSLATE) {
				TranslateTool* translateTool = (TranslateTool*)tool;
				ImVec2 delta = translateTool->objectCurrent - translateTool->objectStart;
				ImMat3 motionMatrix(
					1.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					delta.x, delta.y, 1.0f
				);

				iterMat = motionMatrix * (iterPos * iterRot * iterScl);
			}
			else if (tool->rtti == ToolNodeType::SCALE) {
				ScaleTool* scaleTool = (ScaleTool*)tool;
				float scaleX = (scaleTool->constraint == ToolAxisConstraint::Y) ? 0.0f : scaleTool->currentScale;
				float scaleY = (scaleTool->constraint == ToolAxisConstraint::X) ? 0.0f : scaleTool->currentScale;
				
				iterScl.xx += scaleX;
				iterScl.yy += scaleY;

				iterMat = (iterPos * iterRot * iterScl);
			}
			else if (tool->rtti == ToolNodeType::ROTATE) {
				RotateTool* rotateTool = (RotateTool*)tool;

				ImMat3 rotateMatrix = RotationMatrix(rotateTool->rotateCurrent);
				iterMat = (iterPos * (rotateMatrix * iterRot) * iterScl);
			}
			else if (tool->rtti == ToolNodeType::PIVOT) {
				PivotTool* pivotTool = (PivotTool*)tool;
				ImVec2 delta = pivotTool->objectCurrentPos - pivotTool->objectStartPos;
				ImMat3 motionMatrix(
					1.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
					delta.x, delta.y, 1.0f
				);

				iterMat = motionMatrix * (iterPos * iterRot * iterScl);
			}
		}

		worldMat = iterMat * worldMat;
	}
	return worldMat;
}

bool SceneNode::CheckParent(SceneNode* newParent) {
	if (newParent == 0 || newParent == this || newParent->CanHaveChildren() == false) {
		return false;
	}

	for (SceneNode* iter = newParent; iter != 0; iter = iter->parent) {
		if (iter == this) {
			return false;
		}
	}

	return true;
}

bool SceneNode::CanHaveChildren() {
	return true;
}

void SceneNode::ClearParent() {
	if (parent != 0) {
		parent->RemoveChild(this);
	}

	parent = 0;
	nextSibling = 0;
}

void SceneNode::SetParent(SceneNode* newParent) {
	if (!CheckParent(newParent)) {
		return;
	}

	if (parent != 0) {
		parent->RemoveChild(this);
	}

	parent = 0;
	nextSibling = 0;
	newParent->AddChild(this);
}

#define ADD_CHILD_COMMON() \
	if (newChild == 0 || !newChild->CheckParent(this)) { \
		return; \
	} \
	if (newChild->parent != 0) { \
		newChild->parent->RemoveChild(newChild); \
	} \
	newChild->parent = this; \
	newChild->nextSibling = 0;


void SceneNode::AddChild(SceneNode* newChild) {
	ADD_CHILD_COMMON();

	if (firstChild == 0) {
		firstChild = newChild;
	}
	else {
		for (SceneNode* iter = firstChild; iter != 0; iter = iter->nextSibling) {
			if (iter->nextSibling == 0) {
				iter->nextSibling = newChild;
				break;
			}
		}
	}
}

void SceneNode::AddChildAfter(SceneNode* prevOrNull, SceneNode* newChild) {
	ADD_CHILD_COMMON();

	if (prevOrNull == 0 || firstChild == 0) {
		newChild->nextSibling = firstChild;
		firstChild = newChild;
	}
	else {
		for (SceneNode* iter = firstChild; iter != 0; iter = iter->nextSibling) {
			if (iter == prevOrNull) {
				newChild->nextSibling = iter->nextSibling;
				iter->nextSibling = newChild;
				break;
			}
		}
	}
}

void SceneNode::AddChildBefore(SceneNode* newChild, SceneNode* nextOrNull) {
	ADD_CHILD_COMMON();

	if (firstChild == 0) { // this used to also check nextOrNull, but i decided that should go last
		newChild->nextSibling = firstChild;
		firstChild = newChild;
	}
	else if (nextOrNull == 0 && firstChild == 0) {
		newChild->nextSibling = firstChild;
		firstChild = newChild;
	}
	else {
		SceneNode* prev = 0;
		bool done = false;
		for (SceneNode* iter = firstChild; iter != 0; iter = iter->nextSibling) {
			if (iter == nextOrNull) {
				newChild->nextSibling = iter;

				if (prev == 0) {
					firstChild = newChild;
				}
				else {
					prev->nextSibling = newChild;
				}
				done = true;
				break;
			}
			prev = iter;
		}

		if (!done && prev != 0) {
			prev->nextSibling = newChild;
		}
	}
}

bool SceneNode::RemoveChild(SceneNode* child) {
	if (firstChild == child) {
		firstChild = child->nextSibling;
		child->parent = 0;
		child->nextSibling = 0;
		return true;
	}

	for (SceneNode* iter = firstChild; iter != 0; iter = iter->nextSibling) {
		if (iter->nextSibling == child) {
			iter->nextSibling = child->nextSibling;
			child->parent = 0;
			child->nextSibling = 0;
			return true;
		}
	}

	return false;
}

void SceneNode::DepthFirst(void(*callback)(SceneNode* node, void* userData), void* userData) {
	SceneNode* root = this;
	SceneNode* itr = this;
	bool traversing = true;

	while (traversing) {
		callback(itr, userData);

		if (itr->firstChild) {
			itr = itr->firstChild;
		}
		else {
			while (itr->nextSibling == 0) {
				if (itr == root) {
					traversing = false;
					break;
				}
				itr = itr->parent;
			}
			if (itr == root) { // Prevent stepping to the roots sibling
				traversing = false;
				break;
			}
			itr = itr->nextSibling;
		}
	}
}

SceneNode* SceneNode::DepthFirst(SceneNode* iter, SceneNode& root) {
	if (iter == 0) {
		return &root;
	}

	if (iter->firstChild) {
		iter = iter->firstChild;
	}
	else {
		while (iter->nextSibling == 0) {
			if (iter == &root) {
				return 0;
			}
			iter = iter->parent;
		}
		if (iter == &root) {
			return 0;
		}
		iter = iter->nextSibling;
	}

	return iter;
}

SceneNode* SceneNode::EditorImguiHierarchy(SceneNode* selected, DragAndDropData* dragDropInfo, ImGuiTextFilter* filter, bool* outActivated, bool showDragDrop) {
	SceneNode* result = selected;
	const void* ptr_id = (void*)(intptr_t)this;
	
	bool filtered = true;
	if (filter != 0 && parent != 0 && filter->IsActive()) {
		filtered = filter->PassFilter(name.c_str());
	}

	if (parent != 0 && (filter == 0 || !filter->IsActive())) {
		//if (showDragDrop) {
			ImGui::SeparatorText(0); // To make this work, modify: 
			// const char* ImGui::FindRenderedTextEnd(const char* text, const char* text_end) -> needs to not do anything if text is null, everything will be null
			// void ImGui::SeparatorTextEx(ImGuiID id, const char* label, const char* label_end, float extra_w) -> text height should be 0 if label, and label_end are null
		//}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Hierarchy")) {
				dragDropInfo->mode = SceneNode::DragDropInsertMode::BEFORE;
				dragDropInfo->target = this;
			}
			ImGui::EndDragDropTarget();
		}
	}

	ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
		//ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_SpanAvailWidth |
		ImGuiTreeNodeFlags_DefaultOpen;/* |
									ImGuiTreeNodeFlags_Framed;*/

	bool isLeaf = EditorIsLeaf();
	if (isLeaf) {
		node_flags |= /*ImGuiTreeNodeFlags_Leaf | */ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
	}
	
	if (selected == this || SceneNode::clicked == this) {
		node_flags |= ImGuiTreeNodeFlags_Selected;
	}

	bool editorNodeOpen = true;

	if (parent != 0 && filtered) {
		if (showDragDrop) {
			if (rtti == NodeType::SPRITE_NODE) {
				ImGui::BeginDisabled();
			}
		}

		editorNodeOpen = ImGui::TreeNodeEx(ptr_id, node_flags, name.c_str()); //ImGui::TreeNode(name.c_str());

		if (showDragDrop) {
			if (rtti == NodeType::SPRITE_NODE) {
				ImGui::EndDisabled();
			}
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && ImGui::IsItemHovered()) {
			ImGui::OpenPopup(uuid.c_str());
		}

		float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));
		if (ImGui::BeginPopup(uuid.c_str())) {
			SceneNode::clicked = this;
			if (ImGui::Selectable("Delete Node##RightClick")) {
				Application::GetInstance()->DeleteAtEndOfFrame(this);
			}

			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();

		{
			//ImGuiID imgui_id = ImGui::GetCurrentWindow()->GetID(ptr_id);
			
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
				SceneNode::clicked = this;
				//std::cout << "Clicked " << imgui_id << "\n";
			}

			if (SceneNode::clicked == this && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				if (ImGui::IsItemHovered()) {
					//std::cout << "Released " << imgui_id << "\n";
					result = this;
					
					if (outActivated != 0) {
						*outActivated = true;
					}
				}
				SceneNode::clicked = 0;
			}
		}



		if (!(showDragDrop && rtti == NodeType::SPRITE_NODE)) {
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Hierarchy")) {
					dragDropInfo->mode = SceneNode::DragDropInsertMode::CHILD;
					dragDropInfo->target = this;
				}
				ImGui::EndDragDropTarget();
			}
		}



		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("Hierarchy", (const void*)(size_t)this, sizeof(size_t));
			dragDropInfo->source = this;
			ImGui::Text(name.c_str());
			ImGui::EndDragDropSource();
		}
	}

	if (editorNodeOpen) {
		for (SceneNode* iter = firstChild; iter != 0; iter = iter->nextSibling) {
			SceneNode* iterResult = iter->EditorImguiHierarchy(result, dragDropInfo/*, forceOpenAllNodes*/, filter, outActivated, showDragDrop);
			if (iterResult != 0) {
				result = iterResult;
			}
		}
		
		if (!isLeaf && parent != 0 && filtered) {
			ImGui::TreePop();
		}
	}

	if (nextSibling == 0 && parent != 0 && (filter == 0 || !filter->IsActive())) {
		//if (showDragDrop) {
			ImGui::SeparatorText(0);// See other one for modifications
		//}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Hierarchy")) {
				dragDropInfo->mode = SceneNode::DragDropInsertMode::AFTER;
				dragDropInfo->target = this;
			}
			ImGui::EndDragDropTarget();
		}
	}

	return result;
}

bool SceneNode::EditorIsLeaf() {
	return firstChild == 0;
}

void SceneNode::EditorImmediateInspector(class AnimationAsset* selectedAnimation, int selectedFrame) {
	Application::GetInstance()->PushXenon();
	if (ImGui::CollapsingHeader("Scene Node", ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PopFont();
		ImGui::Text("Name");
		ImGui::SetItemTooltip("Name is not animatable");
		ImGui::SameLine();
		ImGui::PushItemWidth(-FLT_MIN);
		if (ImGui::InputTextWithHint("##SceneNodeName", "Node Name", &nameInput, ImGuiInputTextFlags_EscapeClearsAll)) {
			UndoManager::GetInstance()->SetName(this, nameInput);
			// name = nameInput; ^ the undo action here should do the renaming
		}

		ImGui::PopItemWidth();
	}
	else {
		ImGui::PopFont();
	}
}

int SceneNode::EditorGetDepth() {
	int depth = 0;
	for (SceneNode* iter = parent; iter != 0; iter = iter->parent) {
		depth += 1;
	}
	return depth;
}

SceneNode* SceneNode::EditorFindRecursive(const char* guid) {
	return _EditorFindRecursive(guid, this);
}

SceneNode* SceneNode::_EditorFindRecursive(const char* guid, SceneNode* root) {
	for (SceneNode* iter = SceneNode::DepthFirst(0, *root); iter != 0; iter = SceneNode::DepthFirst(iter, *root)) {
		if (strcmp(iter->uuid.c_str(), guid) == 0) {
			return iter;
		}
	}
	return 0;
}


void SceneNode::SerializeInner(std::stringstream& out, int indent) {
	if (parent == 0 || isDeleted) {
		return;
	}
	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();

	out << tabs << "\t\"name\": \"" << name << "\",\n";
	out << tabs << "\t\"guid\": \"" << uuid << "\",\n";
	if (rtti == NodeType::SCENE_NODE) {
		out << tabs << "\t\"rtti\": \"SCENE_NODE\",\n";
	}
	else if (rtti == NodeType::TRANSFORM_NODE) {
		out << tabs << "\t\"rtti\": \"TRANSFORM_NODE\",\n";
	}
	else if (rtti == NodeType::SPRITE_NODE) {
		out << tabs << "\t\"rtti\": \"SPRITE_NODE\",\n";
	}
	else {
		IM_ASSERT(false);
	}
}

void SceneNode::Serialize(std::stringstream& out, int indent) {
	if (parent != 0 && !isDeleted) {
		for (int i = 0; i < indent; ++i) { out << "\t"; }
		out <<"{\n";
	}

	SerializeInner(out, indent);

	if (!isDeleted) {
		if (firstChild == 0 && parent != 0) {
			for (int i = 0; i < indent; ++i) { out << "\t"; }
			out << "\t\"children\": [ ]\n";
		}
		else {
			if (parent != 0) {
				for (int i = 0; i < indent; ++i) { out << "\t"; }
				out << "\t\"children\": [\n";
			}
			for (SceneNode* iter = firstChild; iter != 0; iter = iter->nextSibling) {
				iter->Serialize(out, indent + 2);
			}
			if (parent != 0) {
				for (int i = 0; i < indent; ++i) { out << "\t"; }
				out << "\t]\n";
			}
		}

		if (parent != 0 && !isDeleted) {
			for (int i = 0; i < indent; ++i) { out << "\t"; }
			out << "}";
			if (nextSibling == 0) {
				out << "\n";
			}
			else {
				out << ",\n";
			}
		}
	}
}