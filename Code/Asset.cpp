#include "Asset.h"
#include "TransformNode.h"
#include "UndoManager.h"
#include "Application.h"
#include "Platform.h"

#include "Libraries/imgui.h"
#include "Libraries/imgui_internal.h"
#include "Libraries/imgui_neo_sequencer.h"
#include "Libraries/imgui_stdlib.h"
#include "Libraries/stb_image.h"

#include "Libraries/sokol_app.h"
#include "Libraries/sokol_gfx.h"
#include "Libraries/sokol_imgui.h"

#include "Libraries/base64.h"
#include "Libraries/ImMath.h"

#include <algorithm>

Asset* Asset::clicked = 0;

const std::string& Asset::GetGUID() {
	return uuid;
}

AssetType Asset::GetType() {
	return rtti;
}

Asset::Asset() {
	rtti = AssetType::ASSET_INVALID;

	name = nameInput = uuid = MakeNewGuid();
	deleted = false;
}

bool Asset::IsDeleted() {
	return deleted;
}

ImageAsset::ImageAsset(const std::string& name_, const void* data, size_t data_size) {
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
	if (image_data == 0) {
		id = 0;
		size.x = size.y = 0.0f;
		return;
	}

#if 1
	sg_image_desc image_desc = { 0 };
	image_desc.width = image_width;
	image_desc.height = image_height;
	image_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
	image_desc.data.subimage[0][0].ptr = image_data;
	image_desc.data.subimage[0][0].size = image_width * image_height * 4 * sizeof(char);
	sg_image sk_image = sg_make_image(&image_desc);

	sg_sampler_desc sampler_desc = { 0 };
	sampler_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
	sampler_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
	sampler_desc.min_filter = SG_FILTER_LINEAR;
	sampler_desc.mag_filter = SG_FILTER_LINEAR;
	sg_sampler sk_sampler = sg_make_sampler(&sampler_desc);

	simgui_image_desc_t img_desc = { 0 };
	img_desc.image = sk_image;
	img_desc.sampler = sk_sampler;
	simgui_image_t imgui_image = simgui_make_image(&img_desc);
#else
	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
#endif

	base64 = base64_encode((unsigned char const* )data, data_size);
	stbi_image_free(image_data);

	name = nameInput = name_;
	id = /*(ImTextureID)(intptr_t)*/(ImTextureID)simgui_imtextureid(imgui_image);
	cleanup = sk_image;
	cleanup_gui = imgui_image;
	size.x = (float)image_width;
	size.y = (float)image_height;
	rtti = AssetType::ASSET_IMAGE;
}

void ImageAsset::EditorImmediateInspector(int* selectedFrame) {
	ImGui::BeginDisabled();
	ImGui::Text(uuid.c_str());
	ImGui::EndDisabled();

	ImGui::BeginTable("##newAnimationConfigAll", 2);
	{
		ImGui::TableSetupColumn("##ImageAsset::EditorImmediateInspector::AAA", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("##ImageAsset::EditorImmediateInspector::BBB", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextColumn();
		ImGui::Text("Name");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		if (ImGui::InputText("##imageAssetFileName", &nameInput)) {
			UndoManager::GetInstance()->SetName(this, nameInput);
		}
		ImGui::PopItemWidth();

		ImGui::TableNextColumn();
		ImGui::Text("Width");
		ImGui::TableNextColumn();
		ImGui::BeginDisabled();
		int _width_ = (int)size.x;
		ImGui::PushItemWidth(-FLT_MIN);
		ImGui::InputInt("##imageassetWidth", &_width_);
		ImGui::PopItemWidth();
		ImGui::EndDisabled();

		ImGui::TableNextColumn();
		ImGui::Text("Height");
		ImGui::TableNextColumn();
		ImGui::BeginDisabled();
		int _height_ = (int)size.y;
		ImGui::PushItemWidth(-FLT_MIN);
		ImGui::InputInt("##imageASsetHeight", &_height_);
		ImGui::PopItemWidth();
		ImGui::EndDisabled();
	}
	ImGui::EndTable();

	ImGui::SeparatorText("Sprite Preview");
	ImVec2 windowRegion = ImGui::GetContentRegionAvail();
	float scale = 0.0f;
	if (windowRegion.x > 0) {
		scale = windowRegion.x / size.x;
	}
	ImVec2 scaled(size.x * scale, size.y * scale);
	ImGui::Image(id, scaled, ImVec2(0, 0), ImVec2(1, 1));
}


Asset::~Asset() {}
ImageAsset::~ImageAsset() {
	simgui_destroy_image(cleanup_gui);
	sg_destroy_image(cleanup);
	//glDeleteTextures(1, (const GLuint*)&id);
	id = 0;
}

AtlasAsset::~AtlasAsset() {
	for (int i = 0, size = (int)allFrames.size(); i < size; ++i) {
		delete allFrames[i];
	}
	allFrames.clear();
}

void Asset::EditorImmediateInspector(int* selectedFrame) {
	ImGui::Text("Not Implemented");
}

AnimationAsset::AnimationAsset(const std::string& name_, int numFrames, int fps, bool _looping) {
	name = nameInput = name_;
	frameRate = fps;
	looping = _looping;
	frameCount = numFrames;
	editorFrameToDelete = -1;
	editorTrackToDeleteFrom = 0;
	editorGroupToDelete = 0;
	rtti = AssetType::ASSET_ANIMATION;
}

AnimationAsset::~AnimationAsset() {
	for (std::map<SceneNode*, Track*>::iterator iter = tracks.begin(), end = tracks.end(); iter != end; iter++) {
		Track* trackIter = iter->second;
		while (trackIter != 0) {
			Track* toDelete = trackIter;
			trackIter = trackIter->GetNext();
			delete toDelete;
		}
	}
	tracks.clear();
	uiStateOpen.clear();
}

Track* AnimationAsset::AddTrack(SceneNode* node, const std::string& name, TrackType type) {
	if (tracks.count(node) == 0) {
		tracks[node] = 0;
		uiStateOpen[node] = true;
	}
	
	Track* result = _GetTrack(node, name);
	if (result != 0) {
		if (result->IsDeleted()) {
			result->Reset();
		}
		result->SetType(type);
		return result;
	}

	// Insert newTrack as head
	Track* newTrack = new Track(name, type, this);
	newTrack->next = tracks[node];
	tracks[node] = newTrack;
	return newTrack;
}

float AnimationAsset::GetPlaybackTime() {
	return (float)frameCount * (1.0f / (float)frameRate);
}

Track* AnimationAsset::GetTrack(SceneNode* node, const std::string& name) {
	Track* result = _GetTrack(node, name);
	if (result != 0 && result->IsDeleted()) { // I think this
		return 0;
	}
	return result;
}

Track* AnimationAsset::_GetTrack(SceneNode* node, const std::string& name) {
	if (tracks.count(node) == 0) {
		return 0;
	}

	for (Track* iter = tracks[node]; iter != 0; iter = iter->GetNext()) {
		if (iter->GetName() == name) {
			return iter;
		}
	}
	
	return 0;
}

bool AnimationAsset::Contains(SceneNode* node) {
	int count = 0;
	if (tracks.count(node) > 0) {
		count = 0;
		for (Track* iter = tracks[node]; iter != 0; iter = iter->GetNext()) {
			if (!iter->IsDeleted() && iter->HasFrames()) {
				count += 1;
			}
		}
	}
	return count > 0;
}

bool AnimationAsset::Contains(SceneNode* node, const std::string& trackName) {
	int count = 0;
	if (tracks.count(node) > 0) {
		count = 0;
		for (Track* iter = tracks[node]; iter != 0; iter = iter->GetNext()) {
			if (!iter->IsDeleted() && iter->HasFrames()) {
				if (iter->name == trackName) {
					count += 1;
				}
			}
		}
	}
	return count > 0;
}

float AnimationAsset::GetRotation(SceneNode* node, int frame) {
	if (frame < 0) {
		return 0.0f;
	}

	Track* rot_ = GetTrack(node, "rotation");
	float rot = 0.0f;

	if (node->rtti == NodeType::SPRITE_NODE || node->rtti == NodeType::TRANSFORM_NODE) {
		TransformNode* xform = (TransformNode*)node;
		rot = xform->rotation;
	}
	if (rot_ != 0 && !rot_->IsDeleted() && rot_->HasFrames()) {
		rot = rot_->InterpolateF(frame, looping);
	}

	return rot;
}

ImMat3 AnimationAsset::GetMatrix(SceneNode* node, int frame, ImMat3* outPos, ImMat3* outRot, ImMat3* outScl) {
	if (frame < 0) {
		return ImMat3();
	}

	Track* pos_x = GetTrack(node, "position.x");
	Track* pos_y = GetTrack(node, "position.y");
	Track* rot_ = GetTrack(node, "rotation");
	Track* scl_x = GetTrack(node, "scale.x");
	Track* scl_y = GetTrack(node, "scale.y");

	// Default
	ImVec2 pos(0, 0);
	float rot = 0.0f;
	ImVec2 scl(1, 1);

	if (node->rtti == NodeType::SPRITE_NODE || node->rtti == NodeType::TRANSFORM_NODE) {
		TransformNode* xform = (TransformNode*)node;
		pos = xform->position;
		rot = xform->rotation;
		scl = xform->scale;
	}

	if (pos_x != 0 && !pos_x->IsDeleted() && pos_x->HasFrames()) {
		pos.x = pos_x->InterpolateF(frame, looping);
	}
	if (pos_y != 0 && !pos_y->IsDeleted() && pos_y->HasFrames()) {
		pos.y = pos_y->InterpolateF(frame, looping);
	}
	if (rot_ != 0 && !rot_->IsDeleted() && rot_->HasFrames()) {
		rot = rot_->InterpolateF(frame, looping);
	}
	if (scl_x != 0 && !scl_x->IsDeleted() && scl_x->HasFrames()) {
		scl.x = scl_x->InterpolateF(frame, looping);
	}
	if (scl_y != 0 && !scl_y->IsDeleted() && scl_y->HasFrames()) {
		scl.y = scl_y->InterpolateF(frame, looping);
	}

	/*return ImMat3(pos, rot, scl);*/
	float rotCos = cosf(rot);
	float rotSin = sinf(rot);

	ImMat3 scaleMatrix = ImMat3(
		scl.x, 0, 0,
		0, scl.y, 0,
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
		pos.x, pos.y, 1
	);
	if (outPos != 0) {
		*outPos = translateMatrix;
	}
	
	return translateMatrix * rotateMatrix * scaleMatrix;
}

int AnimationAsset::GetFrameCount() {
	return frameCount;
}

void AnimationAsset::SetFrameCount(int newCount) {
	frameCount = newCount;
	if (frameCount < 0) {
		frameCount = 0;
	}
}

void AnimationAsset::EditorImmediateInspector(int* selectedFrame) {
	ImGui::BeginDisabled();
	ImGui::Text(uuid.c_str());
	ImGui::EndDisabled();

	ImGui::BeginTable("##newAnimationConfigAll", 2);
	{
		ImGui::TableSetupColumn("##AnimationAsset::EditorImmediateInspector::AAA", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("##AnimationAsset::EditorImmediateInspector::BBB", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextColumn();
		ImGui::Text("Name");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		if (ImGui::InputText("##newAnimationName", &nameInput)) {
			UndoManager::GetInstance()->SetName(this, nameInput);
		}
		ImGui::PopItemWidth();

		ImGui::TableNextColumn();
		ImGui::Text("Frame Rate");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		int newFrameRate = frameRate;
		if (ImGui::InputInt("##newAnimationFPS", &newFrameRate)) {
			UndoManager::GetInstance()->SetFrameRate(this, newFrameRate);
		}
		ImGui::PopItemWidth();
		if (frameRate < 0) {
			frameRate = 0;
		}

		ImGui::TableNextColumn();
		ImGui::Text("Frame Count");
		ImGui::TableNextColumn();
		ImGui::PushItemWidth(-FLT_MIN);
		int newFrameCount = frameCount;
		if (ImGui::InputInt("##newAnimationFPC", &newFrameCount)) {
			UndoManager::GetInstance()->SetFrameCount(this, newFrameCount);
		}
		ImGui::PopItemWidth();
		if (frameCount < 2) {
			frameCount = 2;
		}

		if (selectedFrame != 0) {
			if (*selectedFrame >= frameCount) {
				*selectedFrame = frameCount - 1;
				if (*selectedFrame < 0) {
					*selectedFrame = 0;
				}
			}
		}

		ImGui::TableNextColumn();
		ImGui::Text("Looping");
		ImGui::TableNextColumn();
		bool newlooping = looping;
		if (ImGui::Checkbox("##newAnimationIsLooop", &newlooping)) {
			UndoManager::GetInstance()->SetLooping(this, newlooping);
		}
	}
	ImGui::EndTable();
}

int AnimationAsset::ImguiDrawSequencer(int _currentFrame) {
	int32_t currentFrame = _currentFrame;
	int32_t startFrame = 0;
	int32_t endFrame = frameCount - 1;

	bool deleteFrame = false;
	bool deleteTrack = false;
	bool deleteGroup = false;
	bool reset = false;

	ImGuiNeoSequencerFlags sequencerFlags = ImGuiNeoSequencerFlags_AlwaysShowHeader;
	sequencerFlags |= ImGuiNeoSequencerFlags_EnableSelection;
	sequencerFlags |= ImGuiNeoSequencerFlags_Selection_EnableDragging;

	ImGuiNeoTimelineFlags timelineFlags = 0;
	//timelineFlags |= ImGuiNeoTimelineFlags_AllowFrameChanging;
	reoderTracks.clear();

	if (ImGui::BeginNeoSequencer("AnimationNeoSequencer", &currentFrame, &startFrame, &endFrame, { 0, 0 }, sequencerFlags))
	{
		for (std::map<SceneNode*, Track*>::iterator iter = tracks.begin(), end = tracks.end(); iter != end; iter++) {
			{ // Early out if invalid
				if (!Contains(iter->first)) {
					continue;
				}

				bool valid = false;
				for (Track* _track = iter->second; _track != 0; _track = _track->GetNext()) {
					if (!_track->IsDeleted() && _track->HasFrames()) {
						valid = true;
					}
				}
			
				if (!valid) {
					continue;
				}
			}

			if (ImGui::BeginNeoGroup(iter->first->GetName().c_str(), &uiStateOpen[iter->first])) {
				for (Track* _track = iter->second; _track != 0; _track = _track->GetNext()) {
					if (_track->IsDeleted() || !_track->HasFrames()) {
						continue;
					}
					if (ImGui::BeginNeoTimelineEx(_track->GetName().c_str(), 0, timelineFlags)) {

						for (int f = 0, f_size = (int)_track->FrameCount(); f < f_size; ++f) {
							int32_t val = _track->GetFrameByIndex(f)->frame;
							ImGui::NeoKeyframe(&_track->GetFrameByIndex(f)->frame);
							//_track->GetFrameByIndex(f)->frame = val;
							if (_track->GetFrameByIndex(f)->frame != val) {
								reoderTracks.push_back(_track);
							}

							if (ImGui::IsNeoKeyframeRightClicked()/* ||
								ImGui::IsNeoKeyframeLeftClicked()*/) {
								editorGroupToDelete = 0;
								editorFrameToDelete = f;
								editorTrackToDeleteFrom = _track;
								ImGui::OpenPopup("##DeleteKeyframePopup");
							}
						}

						ImGui::EndNeoTimeLine();

						if (ImGui::IsNeoTimelineLeftClicked() ||
							ImGui::IsNeoTimelineRightClicked()) {
							editorGroupToDelete = 0;
							editorTrackToDeleteFrom = _track;
							editorFrameToDelete = -1;
							ImGui::OpenPopup("##DeleteTrackPopup");
						}
					}
				}

				ImGui::EndNeoGroup();

				if (ImGui::IsNeoGroupRightClicked() || ImGui::IsNeoGroupLeftClicked()) {
					editorGroupToDelete = iter->first;
					editorTrackToDeleteFrom = 0;
					editorFrameToDelete = -1;
					ImGui::OpenPopup("##DeleteGroupPopup");
				}
			}
		}

		//Application::GetInstance()->PushDefaultFramePadding();
		float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f * windowDevicePixelRatio, 5.0f * windowDevicePixelRatio));


		if (ImGui::BeginPopup("##DeleteGroupPopup")) {
			Application::GetInstance()->PushXenon();
			ImGui::SeparatorText("Delete Tracks?");
			ImGui::PopFont();

			Application::GetInstance()->PushArgon();
			if (ImGui::Selectable("Delete")) {
				deleteGroup = true;
				reset = true;
			}
			if (ImGui::Selectable("Cancel")) {
				reset = true;
			}
			ImGui::PopFont();
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("##DeleteTrackPopup")) {
			Application::GetInstance()->PushXenon();
			ImGui::SeparatorText("Delete Track?");
			ImGui::PopFont();

			Application::GetInstance()->PushArgon();
			if (ImGui::Selectable("Delete")) {
				deleteTrack = true;
				reset = true;
			}
			if (ImGui::Selectable("Cancel")) {
				reset = true;
			}
			ImGui::PopFont();

			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("##DeleteKeyframePopup")) {
			Application::GetInstance()->PushXenon();
			ImGui::SeparatorText("Delete Frame?");
			ImGui::PopFont();

			Application::GetInstance()->PushArgon();
			if (ImGui::Selectable("Delete")) {
				deleteFrame = true;
				reset = true;
			}
			if (ImGui::Selectable("Cancel")) {
				reset = true;
			}
			ImGui::PopFont();
			ImGui::EndPopup();
		}

		ImGui::PopStyleVar();

		if (deleteFrame || deleteTrack || reset || deleteGroup) {
			ImGui::DeselectNeoTrack();
		}

		ImGui::EndNeoSequencer();
	}

	
	if (reoderTracks.size() > 0) {
		for (int i = 0, size = (int)reoderTracks.size(); i < size; ++i) {
			reoderTracks[i]->Sort();
		}
	}
	reoderTracks.clear();

	if (deleteGroup) {
		if (editorGroupToDelete != 0) {
			/*for (Track* iter = tracks[editorGroupToDelete]; iter != 0; iter = iter->GetNext()) {
				iter->SetDeleted(true);
			}*/
			UndoManager::GetInstance()->DeleteTracks(tracks[editorGroupToDelete]);
		}
	}

	if (deleteFrame) {
		if (editorTrackToDeleteFrom != 0 && editorFrameToDelete >= 0) {
			/*editorTrackToDeleteFrom->DeleteByArrayIndex(editorFrameToDelete);
			if (!editorTrackToDeleteFrom->HasFrames()) {
				editorTrackToDeleteFrom->SetDeleted(true);
			}*/
			UndoManager::GetInstance()->DeleteFrame(editorTrackToDeleteFrom, editorFrameToDelete);
		}
	}

	if (deleteTrack) {
		if (editorTrackToDeleteFrom != 0) {
			//editorTrackToDeleteFrom->SetDeleted(true);
			UndoManager::GetInstance()->DeleteTrack(editorTrackToDeleteFrom);
		}
	}

	if (deleteFrame || deleteTrack || reset || deleteGroup) {
		editorFrameToDelete = -1;
		editorTrackToDeleteFrom = 0;
		editorGroupToDelete = 0;
	}

	return currentFrame;
}

void AtlasAsset::SetGuid(const std::string& guid) {
	uuid = guid;
}


AtlasAsset::AtlasAsset(const std::string& _name) {
	name = nameInput = _name;
	rtti = AssetType::ASSET_ATLAS;
	sourceSize = ImVec2(1, 1);
}

int AtlasAsset::NumFrames() {
	int result = 0;
	for (int i = 0, size = (int)frames.size(); i < size; ++i) {
		if (!frames[i]->deleted) {
			result += 1;
		}
	}
	return result;
	//return (int)frames.size();
}

AtlasFrame* AtlasAsset::GetFrameByIndex(int index) {
	if (index < 0 || index >= frames.size()) {
		return 0;
	}

	int counter = 0;
	for (int i = 0, size = (int)frames.size(); i < size; ++i) {
		if (!frames[i]->deleted) {
			if (counter == index) {
				return frames[i];
			}
			counter += 1;
		}
	}

	return 0;

	/*if (index < 0 || index >= frames.size()) {
		return 0;
	}
	return frames[index];*/
}


AtlasFrame::AtlasFrame(AtlasAsset* owner) {
	rotated = false;
	rtti = AssetType::ASSET_FRAME;
	name = nameInput = uuid;
	atlas = owner;
}

AtlasFrame* AtlasAsset::AddFrame(const std::string& name, const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, bool rotated) {
	AtlasFrame* newFrame = new AtlasFrame(this);
	newFrame->name = newFrame->nameInput = name;
	newFrame->rotated = rotated;

#if 1
	ImVec2 _min = minim(p0, p1, p2, p3);
	ImVec2 _max = maxim(p0, p1, p2, p3);
	newFrame->p0 = _min;
	newFrame->p2 = _max;
	newFrame->p1 = ImVec2(_max.x, _min.y);
	newFrame->p3 = ImVec2(_min.x, _max.y);
#else
	newFrame->p0 = p0;
	newFrame->p1 = p1;
	newFrame->p2 = p2;
	newFrame->p3 = p3;
#endif

	newFrame->r0 = p0;
	newFrame->r1 = p0 + normalized(p1 - p0) * len(p0 - p3);
	newFrame->r3 = p0 + normalized(p3 - p0) * len(p0 - p1);
	newFrame->r2.x = newFrame->r1.x;
	newFrame->r2.y = newFrame->r3.y;

	frames.push_back(newFrame);
	allFrames.push_back(newFrame);

	return newFrame;
}

void AtlasFrame::EditorImmediateInspector() {
	if (deleted) {
		return;
	}
	ImGui::BeginTable("##AtlasFrame::EditorImmediateInspector", 2);

	ImGui::TableSetupColumn("##AtlasFrame::EditorImmediateInspector::AAA", ImGuiTableColumnFlags_WidthFixed);
	ImGui::TableSetupColumn("##AtlasFrame::EditorImmediateInspector::BBB", ImGuiTableColumnFlags_WidthStretch);

	ImGui::TableNextColumn();
	ImGui::Text("Name");
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(-FLT_MIN);
	if (ImGui::InputText(uuid.c_str(), &nameInput, ImGuiInputTextFlags_EscapeClearsAll)) {
		UndoManager::GetInstance()->SetName(this, nameInput);
	}
	ImGui::PopItemWidth();


	//static char nameIdBuffer[128] = { 0 };
	char uuid_0 = uuid[0];
	char uuid_1 = uuid[1];
	char uuid_2 = uuid[2];
	uuid[0] = uuid[1] = '#';


	ImGui::TableNextColumn();
	ImGui::Text("Point 1");
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(-FLT_MIN);
	ImVec2 newPoint = GetRawPoint(0);
	uuid[2] = 'X';
	if (ImGui::InputFloat2(uuid.c_str(), &newPoint.x)) {
		UndoManager::GetInstance()->SetPoint(this, 0, newPoint);
	}
	ImGui::PopItemWidth();

	ImGui::TableNextColumn();
	ImGui::Text("Point 2");
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(-FLT_MIN);
	newPoint = GetRawPoint(1);
	uuid[2] = 'Y';
	if (ImGui::InputFloat2(uuid.c_str(), &newPoint.x)) {
		UndoManager::GetInstance()->SetPoint(this, 1, newPoint);
	}
	ImGui::PopItemWidth();

	ImGui::TableNextColumn();
	ImGui::Text("Point 3");
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(-FLT_MIN);
	newPoint = GetRawPoint(2);
	uuid[2] = 'Z';
	if (ImGui::InputFloat2(uuid.c_str(), &newPoint.x)) {
		UndoManager::GetInstance()->SetPoint(this, 2, newPoint);
	}
	ImGui::PopItemWidth();

	ImGui::TableNextColumn();
	ImGui::Text("Point 4");
	ImGui::TableNextColumn();
	ImGui::PushItemWidth(-FLT_MIN);
	newPoint = GetRawPoint(3);
	uuid[2] = 'W';
	if (ImGui::InputFloat2(uuid.c_str(), &newPoint.x)) {
		UndoManager::GetInstance()->SetPoint(this, 3, newPoint);
	}
	ImGui::PopItemWidth();

	uuid[0] = uuid_0;
	uuid[1] = uuid_1;
	uuid[2] = uuid_2;

	ImGui::TableNextColumn();
	ImGui::Text("Rotated");
	ImGui::TableNextColumn();
	bool newRotated = rotated;
	if (ImGui::Checkbox("##atlasFrameIsRotated", &newRotated)) {
		UndoManager::GetInstance()->SetRotated(this, newRotated);
	}

	ImGui::EndTable();
}

void AtlasFrame::SetGuid(const std::string& guid) {
	uuid = guid;
}

ImVec2 AtlasFrame::GetSize() {
	return GetMax() - GetMin();
}

ImVec2 AtlasFrame::GetMin() {
	return minim(P0(), P1(), P2(), P3());
}

ImVec2 AtlasFrame::GetMax() {
	return maxim(P0(), P1(), P2(), P3());
}

ImVec2 AtlasFrame::Uv0() { 
	ImVec2 size = atlas->sourceSize;
	if (size.x < -0.0001f || size.x > 0.0001f) {
		size.x = 1.0f / size.x;
	}
	if (size.y < -0.0001f || size.y > 0.0001f) {
		size.y = 1.0f / size.y;
	}
	if (rotated) {
		return r1 * size;
	}
	return p0 * size;
}

ImVec2 AtlasFrame::Uv1() { 
	ImVec2 size = atlas->sourceSize;
	if (size.x < -0.0001f || size.x > 0.0001f) {
		size.x = 1.0f / size.x;
	}
	if (size.y < -0.0001f || size.y > 0.0001f) {
		size.y = 1.0f / size.y;
	}

	if (rotated) {
		return r2 * size;
	}
	return p1 * size;
}
ImVec2 AtlasFrame::Uv2() {
	ImVec2 size = atlas->sourceSize;
	if (size.x < -0.0001f || size.x > 0.0001f) {
		size.x = 1.0f / size.x;
	}
	if (size.y < -0.0001f || size.y > 0.0001f) {
		size.y = 1.0f / size.y;
	}
	if (rotated) {
		return r3 * size;
	}
	return p2 * size;
}
ImVec2 AtlasFrame::Uv3() {
	ImVec2 size = atlas->sourceSize;
	if (size.x < -0.0001f || size.x > 0.0001f) {
		size.x = 1.0f / size.x;
	}
	if (size.y < -0.0001f || size.y > 0.0001f) {
		size.y = 1.0f / size.y;
	}
	if (rotated) {
		return r0 * size;
	}
	return p3 * size;
}

ImVec2 AtlasFrame::GetRawPoint(int index) {
	if (index == 0) {
		return p0;
	}
	else if (index == 1) {
		return p1;
	}
	else if (index == 2) {
		return p2;
	}
	else if (index == 3) {
		return p3;
	}
	IM_ASSERT(false);
	return ImVec2();
}

void AtlasFrame::SetRawPoint(int index, const ImVec2& value) {
	IM_ASSERT(index >= 0 && index <= 3);

	if (index == 0) {
		p0 = value;
	}
	else if (index == 1) {
		p1 = value;
	}
	else if (index == 2) {
		p2 = value;
	}
	else if (index == 3) {
		p3 = value;
	}

	r0 = p0;
	r1 = p0 + normalized(p1 - p0) * len(p0 - p3);
	r3 = p0 + normalized(p3 - p0) * len(p0 - p1);
	r2.x = r1.x;
	r2.y = r3.y;
}

ImVec2 AtlasFrame::P0() {
	if (rotated) {
		return r0;
	}
	return p0;
}

ImVec2 AtlasFrame::P1() {
	if (rotated) {
		return r1;
	}
	return p1;
}

ImVec2 AtlasFrame::P2() {
	if (rotated) {
		return r2;
	}
	return p2;
}

ImVec2 AtlasFrame::P3() {
	if (rotated) {
		return r3;
	}
	return p3;
}

void AtlasFrame::EditorImmediateInspector(int* selectedFrame) {
	ImGui::BeginDisabled();
	ImGui::Text(uuid.c_str());
	ImGui::EndDisabled();

	EditorImmediateInspector();
}

void AtlasAsset::ImguiAssetsList(Asset* SelectedAsset, bool FocusNode, AnimationAsset* SelectedAnimation, int SelectedFrame) {
	for (int i = 0, size = (int)frames.size(); i < size; ++i) {
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_SpanAvailWidth;
		Asset* thisAsset = frames[i];
		if (thisAsset->deleted) {
			continue;
		}

		if (Asset::clicked == thisAsset || SelectedAsset == thisAsset) {
			node_flags |= ImGuiTreeNodeFlags_Selected;
		}
		
		bool isOpen = ImGui::TreeNodeEx((void*)frames[i], node_flags, frames[i]->name.c_str());

		{
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
				Asset::clicked = thisAsset;
			}

			if (Asset::clicked == thisAsset && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				if (ImGui::IsItemHovered()) {
					/*SelectedAsset = thisAsset;
					*FocusNode = false;
					*SelectedAnimation = 0;*/

					UndoManager::GetInstance()->SelectAsset(
						SelectedAsset, thisAsset, 
						FocusNode, false, 
						SelectedAnimation, 0, 
						SelectedFrame, 0
					);
				}
				Asset::clicked = 0;
			}
		}

		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("Assets:Frame", (const void*)&thisAsset, sizeof(Asset*));
			ImGui::Text(thisAsset->name.c_str());
			ImGui::EndDragDropSource();
		}

		if (isOpen) {
			ImGui::TreePop();
		}
	}
}

void AtlasAsset::EditorImmediateInspector(int* selectedFrame) {
	float headerHeight = ImGui::GetContentRegionAvail().y;
	float windowDevicePixelRatio = Application::GetInstance()->WindowDevicePixelRatio();

	ImGui::BeginDisabled();
	ImGui::Text(uuid.c_str());
	ImGui::EndDisabled();

	ImGui::Text("Name");
	ImGui::SameLine();

	ImGui::PushItemWidth(-FLT_MIN);
	if (ImGui::InputText("##assetAtlasName", &nameInput, ImGuiInputTextFlags_EscapeClearsAll)) {
		UndoManager::GetInstance()->SetName(this, nameInput);
	}
	ImGui::PopItemWidth();


	

	ImGui::Text("Size");
	ImGui::SameLine();
	ImVec2 newSize = sourceSize;
	ImGui::SetNextItemWidth(-FLT_MIN);
	if (ImGui::InputFloat2("##assetAtlasSize", &newSize.x)) {
		UndoManager::GetInstance()->SetSize(this, newSize);
	}

	ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 91 * windowDevicePixelRatio);
	if (ImGui::Button("Add Frame", ImVec2(100 * windowDevicePixelRatio, 20 * windowDevicePixelRatio))) {
		UndoManager::GetInstance()->AddFrame(this);
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8 * windowDevicePixelRatio);
	ImGui::SeparatorText("");
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 8 * windowDevicePixelRatio);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_None | ImGuiWindowFlags_AlwaysVerticalScrollbar;
	ImGui::BeginChild("##InspectorAtlasDetailsAE", ImVec2(), ImGuiChildFlags_None, window_flags);

	for (int i = 0, size = (int)frames.size(); i < size; ++i) {
		if (!frames[i]->deleted) {
			frames[i]->EditorImmediateInspector();
			if (i != size - 1) {
				ImGui::SeparatorText("");
			}
		}
	}

	ImGui::EndChild();
}

void Asset::SerializeInner(std::stringstream& out, int indent) {
	if (deleted) return;

	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();


	out << tabs << "\t\"name\": \"" << name << "\",\n";
	out << tabs << "\t\"guid\": \"" << uuid << "\",\n";

	if (rtti == AssetType::ASSET_IMAGE) {
		out << tabs << "\t\"rtti\": \"ASSET_IMAGE\",\n";
	}
	else if (rtti == AssetType::ASSET_ATLAS) {
		out << tabs << "\t\"rtti\": \"ASSET_ATLAS\",\n";
	}
	else if (rtti == AssetType::ASSET_ANIMATION) {
		out << tabs << "\t\"rtti\": \"ASSET_ANIMATION\",\n";
	}
	else if (rtti == AssetType::ASSET_FRAME) {
		out << tabs << "\t\"rtti\": \"ASSET_FRAME\",\n";
	}
	else {
		IM_ASSERT(false);
	}



}

void Asset::Serialize(std::stringstream& out, int indent) {
	if (deleted) return;

	for (int i = 0; i < indent; ++i) { out << "\t"; }
	out << "{\n";

	SerializeInner(out, indent);

	if (!deleted) {
		for (int i = 0; i < indent; ++i) { out << "\t"; }
		out << "},\n";
	}
}

void AnimationAsset::SerializeInner(std::stringstream& out, int indent) {
	Asset::SerializeInner(out, indent);
	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();

	out << tabs << "\t\"frameRate\": " << frameRate <<  ",\n";
	out << tabs << "\t\"frameCount\": " << frameCount << ",\n";
	if (looping) {
		out << tabs << "\t\"looping\": true,\n";
	}
	else {
		out << tabs << "\t\"looping\": false,\n";
	}

	if (tracks.size() == 0) {
		out << tabs << "\t\"tracks\": []\n";
	}
	else {
		out << tabs << "\t\"tracks\": [\n";
		bool firstItem = true;
		for (std::map<SceneNode*, Track*>::iterator iter = tracks.begin(), end = tracks.end(); iter != end; iter++) {
			for (Track* inner = iter->second; inner != 0; inner = inner->GetNext()) {
				if (inner->deleted || !inner->HasFrames()) {
					continue;
				}

				if (firstItem) {
					firstItem = false;
				}
				else {
					out.seekp(-1, out.cur);
					out << ",\n";
				}
				inner->SerializeInner(out, iter->first->uuid.c_str(), indent + 2);
			}
		}
		out << tabs << "\t]\n";
	}

}

void AtlasAsset::SerializeInner(std::stringstream& out, int indent) {
	Asset::SerializeInner(out, indent);
	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();

	out << tabs << "\t\"size\": {\"w\": " << ((int)sourceSize.x) << ", \"h\": " << ((int)sourceSize.x) << " },\n";

	if (NumFrames() == 0) {
		out << tabs << "\t\"frames\": []\n";
	}
	else {
		out << tabs << "\t\"frames\": [\n";
		bool firstItem = true;
		for (int i = 0, size = (int)frames.size(); i < size; ++i) {
			if (frames[i]->deleted) {
				continue;
			}
			if (firstItem) {
				firstItem = false;
			}
			else {
				out.seekp(-1, out.cur);
				out << ",\n";
			}
			frames[i]->SerializeInner(out, indent + 1);
		}
		out << tabs << "\t]\n";
	}
}

void AtlasFrame::SerializeInner(std::stringstream& out, int indent) {
	if (deleted) {
		return;
	}
	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();

	out << tabs << "\t{\n";
	Asset::SerializeInner(out, indent + 1);
	//out << tabs << "\t\t\"name\": \"" << name << "\"\n";
	out << tabs << "\t\t\"p0\": { \"x\": " << p0.x << ", \"y\": " << p0.y <<  "},\n";
	out << tabs << "\t\t\"uv0\": { \"x\": " << r0.x << ", \"y\": " << r0.y << "},\n";
	out << tabs << "\t\t\"p1\": { \"x\": " << p1.x << ", \"y\": " << p1.y <<  "},\n";
	out << tabs << "\t\t\"uv1\": { \"x\": " << r1.x << ", \"y\": " << r1.y << "},\n";
	out << tabs << "\t\t\"p2\": { \"x\": " << p2.x << ", \"y\": " << p2.y <<  "},\n";
	out << tabs << "\t\t\"uv2\": { \"x\": " << r2.x << ", \"y\": " << r2.y << "},\n";
	out << tabs << "\t\t\"p3\": { \"x\": " << p3.x << ", \"y\": " << p3.y <<  "},\n";
	out << tabs << "\t\t\"uv3\": { \"x\": " << r3.x << ", \"y\": " << r3.y << "},\n";

	if (rotated) {
		out << tabs << "\t\t\"rotated\": true\n";
	}
	else {
		out << tabs << "\t\t\"rotated\": false\n";
	}

	out << tabs << "\t}\n";
}

void ImageAsset::SerializeInner(std::stringstream& out, int indent) {
	Asset::SerializeInner(out, indent);
	if (deleted) {
		return;
	}
	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();

	out << tabs << "\t\"size\": {\"w\": " << ((int)size.x) << ", \"h\": " << ((int)size.x) << " },\n";
	out << tabs << "\t\"data\": \"" << base64 << "\"\n";
}