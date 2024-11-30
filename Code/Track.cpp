#include "Track.h"
#include "Asset.h"
#include <algorithm>

Track::Track(const std::string _name, TrackType _type, AnimationAsset* _parent) {
	name = _name;
	rtti = _type;
	next = 0;
	parent = _parent;
	deleted = false;
}

Track::~Track() {
	frames.clear();
}

int Track::FindFrame(int index) {
	for (int i = 0, size = (int)frames.size(); i < size; ++i) {
		if (frames[i].frame == index) {
			return i;
		}
	}
	return -1;
}

int Track::FindLeftFrame(int index) {
	int result = -1;
	if (frames.size() > 0 && frames[0].frame <= index) {
		for (int i = 0, size = (int)frames.size(); i < size; ++i) {
			if (frames[i].frame <= index) {
				result = i;
			}
			else {
				break;
			}
		}
	}
	return result;
}

Frame* Track::AddFrame(int index) {
	if (index < 0) {
		return 0;
	}
	int search = FindFrame(index);
	if (search >= 0) {
		return &frames[search];
	}

	Frame* result = 0;
	int left = FindLeftFrame(index);
	if (left < 0) {
		frames.insert(frames.begin(), Frame());
		result = &frames[0];
	}
	else if (left + 1 >= frames.size()) {
		frames.push_back(Frame());
		result = &frames[frames.size() - 1];
	}
	else {
		frames.insert(frames.begin() + left + 1, Frame());
		result = &frames[left + 1];
	}

	result->frame = index;
	return result;
}

void Track::AddFrameF(float data, int frame) {
	IM_ASSERT(!deleted);

	Frame* result = AddFrame(frame);
	if (result != 0) {
		result->data.asFloat = data;
	}
}

void Track::AddFrameI(int data, int frame) {
	IM_ASSERT(!deleted);

	Frame* result = AddFrame(frame);
	if (result != 0) {
		result->data.asInt = data;
	}
}

void Track::AddFrameB(bool data, int frame) {
	IM_ASSERT(!deleted);

	Frame* result = AddFrame(frame);
	if (result != 0) {
		result->data.asBool = data;
	}
}

void Track::AddFrameC(ImU32 data, int frame) {
	IM_ASSERT(!deleted);

	Frame* result = AddFrame(frame);
	if (result != 0) {
		result->data.asColor = data;
	}
}

void Track::Reset() {
	frames.clear();
	deleted = false;
}

bool Track::IsDeleted() {
	return deleted;
}

bool Track::HasFrames() {
	return frames.size() > 0;
}

int Track::FrameCount() {
	return (int)frames.size();
}

void Track::Sort() {
	std::sort(frames.begin(), frames.end(),
		[](const Frame& lhs, const Frame& rhs)->bool {
			return lhs.frame < rhs.frame;
		}
	);
}

Track* Track::GetNext() {
	return next;
}

TrackType Track::GetType() {
	return rtti;
}

void Track::SetType(TrackType type) {
	rtti = type;
}

const std::string& Track::GetName() {
	return name;
}

void Track::SetDeleted(bool val) {
	deleted = val;
}

void Track::RemoveFrame(int frameIndex) {
	int arrayIndex = FindFrame(frameIndex);
	DeleteByArrayIndex(arrayIndex);
}

void Track::DeleteByArrayIndex(int arrayIndex) {
	if (arrayIndex >= 0 && arrayIndex < frames.size()) {
		frames.erase(frames.begin() + arrayIndex);
	}
}

Frame* Track::GetFrame(int frame) {
	int index = FindFrame(frame);
	if (index < 0) {
		return 0;
	}
	return &frames[index];
}

Frame* Track::GetFrameByIndex(int index) {
	if (index < 0 || index >= frames.size()) {
		return 0;
	}
	return &frames[index];
}

float Track::InterpolateF(int frame, bool loop) {
	IM_ASSERT(!deleted);

	size_t numFrames = frames.size();
	int frameCount = parent->GetFrameCount();
	if (numFrames == 0) {
		return 0.0f;
	}
	else if (numFrames == 1) {
		return frames[0].data.asFloat;
	}

	// Get left and right frames
	int leftIndex = FindLeftFrame(frame);
	if (leftIndex == -1) {
		leftIndex = 0;
	}
	int rightIndex = (leftIndex + 1) % numFrames;
	if (rightIndex < leftIndex) {
		int tmp = rightIndex;
		rightIndex = leftIndex;
		leftIndex = tmp;
	}
	Frame& leftFrame = frames[leftIndex];
	if (frame < leftFrame.frame) {
		rightIndex = (int)frames.size() - 1;
	}
	Frame& rightFrame = frames[rightIndex];

	float start = leftFrame.data.asFloat;
	float end = rightFrame.data.asFloat;

	int frameDelta = rightFrame.frame - leftFrame.frame;
	int curFrame = frame - leftFrame.frame;

	bool flipped = frame < leftFrame.frame || frame > rightFrame.frame;
	if (flipped) {
		if (!loop) {
			if (frame < leftFrame.frame) {
				return leftFrame.data.asFloat;
			}
			else {
				return rightFrame.data.asFloat;
			}
		}
		else {
			frameDelta = frameCount - (rightFrame.frame - leftFrame.frame);

			if (frame < leftFrame.frame) {
				curFrame = frame + (frameCount - rightFrame.frame);
			}
			else {
				curFrame = frame - rightFrame.frame;
			}

			start = rightFrame.data.asFloat;
			end = leftFrame.data.asFloat;
		}
	}

	float t = (float)curFrame / (float)frameDelta;

	return start + (end - start) * t;
}

ImU32 Track::InterpolateC(int frame, bool loop) {
	IM_ASSERT(!deleted);


	size_t numFrames = frames.size();
	int frameCount = parent->GetFrameCount();
	if (numFrames == 0) {
		return 0;
	}
	else if (numFrames == 1) {
		return frames[0].data.asColor;
	}

	// Get left and right frames
	int leftIndex = FindLeftFrame(frame);
	if (leftIndex == -1) {
		leftIndex = 0;
	}
	int rightIndex = (leftIndex + 1) % numFrames;
	if (rightIndex < leftIndex) {
		int tmp = rightIndex;
		rightIndex = leftIndex;
		leftIndex = tmp;
	}
	Frame& leftFrame = frames[leftIndex];
	if (frame < leftFrame.frame) {
		rightIndex = (int)frames.size() - 1;
	}
	Frame& rightFrame = frames[rightIndex];

	ImU32 start = leftFrame.data.asColor;
	ImU32 end  = rightFrame.data.asColor;

	int frameDelta = rightFrame.frame - leftFrame.frame;
	int curFrame = frame - leftFrame.frame;

	bool flipped = frame < leftFrame.frame || frame > rightFrame.frame;
	if (flipped) {
		if (!loop) {
			if (frame < leftFrame.frame) {
				return leftFrame.data.asColor;
			}
			else {
				return rightFrame.data.asColor;
			}
		}
		else {
			frameDelta = frameCount - (rightFrame.frame - leftFrame.frame);

			if (frame < leftFrame.frame) {
				curFrame = frame + (frameCount - rightFrame.frame);
			}
			else {
				curFrame = frame - rightFrame.frame;
			}

			start = rightFrame.data.asColor;
			end = leftFrame.data.asColor;
		}
	}

	float t = (float)curFrame / (float)frameDelta;

	ImVec4 startRGBA = ImGui::ColorConvertU32ToFloat4(start);
	ImVec4 endRGBA = ImGui::ColorConvertU32ToFloat4(end);

	ImVec4 resultRGBA = startRGBA + (endRGBA - startRGBA) * t;
	return ImGui::ColorConvertFloat4ToU32(resultRGBA);
}

int Track::InterpolateI(int frame, bool loop) {
	IM_ASSERT(!deleted);


	size_t numFrames = frames.size();
	int frameCount = parent->GetFrameCount();
	if (numFrames == 0) {
		return 0;
	}
	else if (numFrames == 1) {
		return frames[0].data.asInt;
	}

	// Get left and right frames
	int leftIndex = FindLeftFrame(frame);
	if (leftIndex == -1) {
		leftIndex = 0;
	}
	int rightIndex = (leftIndex + 1) % numFrames;
	if (rightIndex < leftIndex) {
		int tmp = rightIndex;
		rightIndex = leftIndex;
		leftIndex = tmp;
	}
	Frame& leftFrame = frames[leftIndex];
	if (frame < leftFrame.frame) {
		rightIndex = (int)frames.size() - 1;
	}
	Frame& rightFrame = frames[rightIndex];

	float start = (float)leftFrame.data.asInt;
	float end   = (float)rightFrame.data.asInt;

	int frameDelta = rightFrame.frame - leftFrame.frame;
	int curFrame = frame - leftFrame.frame;

	bool flipped = frame < leftFrame.frame || frame > rightFrame.frame;
	if (flipped) {
		if (!loop) {
			if (frame < leftFrame.frame) {
				return leftFrame.data.asInt;
			}
			else {
				return rightFrame.data.asInt;
			}
		}
		else {
			frameDelta = frameCount - (rightFrame.frame - leftFrame.frame);

			if (frame < leftFrame.frame) {
				curFrame = frame + (frameCount - rightFrame.frame);
			}
			else {
				curFrame = frame - rightFrame.frame;
			}

			start = (float)rightFrame.data.asInt;
			end   = (float)leftFrame.data.asInt;
		}
	}

	float t = (float)curFrame / (float)frameDelta;

	return (int)(start + (end - start) * t);
}

bool Track::InterpolateB(int frame, bool loop) {
	IM_ASSERT(!deleted);


	size_t numFrames = frames.size();
	if (numFrames == 0) {
		return false;
	}
	else if (numFrames == 1) {
		return frames[0].data.asBool;
	}

	if (frame < frames[0].frame) {
		if (loop) {
			return frames[numFrames - 1].data.asBool;
		}
		return frames[0].data.asBool;
	}
	else if (frame > frames[numFrames - 1].frame) {
		return frames[numFrames - 1].data.asBool;
	}

	int leftIndex = FindLeftFrame(frame);
	if (leftIndex == -1) {
		return false;
	}

	return frames[leftIndex].data.asBool;
}

void Track::SerializeInner(std::stringstream& out, const char* target, int indent) {
	if (deleted || frames.size() == 0) {
		return;
	}

	std::stringstream _tabs;
	for (int i = 0; i < indent; ++i) {
		_tabs << "\t";
	}
	std::string tabs = _tabs.str();

	out << tabs << "{\n";

	out << tabs << "\t\"name\": \"" << name << "\",\n";
	out << tabs << "\t\"target\": \"" << target << "\",\n";
	if (rtti == TrackType::TRACK_FLOAT) {
		out << tabs << "\t\"rtti\": \"TRACK_FLOAT\",\n";
	}
	else if (rtti == TrackType::TRACK_INT) {
		out << tabs << "\t\"rtti\": \"TRACK_INT\",\n";
	}
	else if (rtti == TrackType::TRACK_BOOL) {
		out << tabs << "\t\"rtti\": \"TRACK_BOOL\",\n";
	}
	else if (rtti == TrackType::TRACK_COLOR) {
		out << tabs << "\t\"rtti\": \"TRACK_COLOR\",\n";
	}
	else {
		IM_ASSERT(false);
	}

	if (frames.size() == 0) {
		out << tabs << "\t\"frames\": [ ]\n";
	}
	else {
		out << tabs << "\t\"frames\": [\n";
		bool isFirst = true;
		if (rtti == TrackType::TRACK_FLOAT) {
			for (int i = 0, size = (int)frames.size(); i < size; ++i) {
				if (isFirst) {
					isFirst = false;
				}
				else {
					out.seekp(-1, out.cur);
					out << ",\n";
				}

				out << tabs << "\t\t{ \"index\": " << frames[i].frame << ", \"data\": " << frames[i].data.asFloat << " } \n";
			}
		}
		else if (rtti == TrackType::TRACK_INT) {
			for (int i = 0, size = (int)frames.size(); i < size; ++i) {
				if (isFirst) {
					isFirst = false;
				}
				else {
					out.seekp(-1, out.cur);
					out << ",\n";
				}

				out << tabs << "\t\t{ \"index\": " << frames[i].frame << ", \"data\": " << frames[i].data.asInt << " } \n";
			}
		}
		else if (rtti == TrackType::TRACK_BOOL) {
			for (int i = 0, size = (int)frames.size(); i < size; ++i) {
				if (isFirst) {
					isFirst = false;
				}
				else {
					out.seekp(-1, out.cur);
					out << ",\n";
				}

				if (frames[i].data.asBool) {
					out << tabs << "\t\t{ \"index\": " << frames[i].frame << ", \"data\": true } \n";
				}
				else {
					out << tabs << "\t\t{ \"index\": " << frames[i].frame << ", \"data\": false } \n";
				}
			}
		}
		else if (rtti == TrackType::TRACK_COLOR) {
			for (int i = 0, size = (int)frames.size(); i < size; ++i) {
				for (int i = 0, size = (int)frames.size(); i < size; ++i) {
					if (isFirst) {
						isFirst = false;
					}
					else {
						out.seekp(-1, out.cur);
						out << ",\n";
					}

					ImVec4 color = ImGui::ColorConvertU32ToFloat4(frames[i].data.asColor);
					out << tabs << "\t\t{ \"index\": " << frames[i].frame << ", \"data\": {" 
						<< "\"r\": " << color.x << ", \"g\": " << color.y << ", \"b\": " << color.z << ", \"a\": " << color.w << "} } \n";
				}
			}
		}
		out << tabs << "\t]\n";
	}

	out << tabs << "}\n";
}
