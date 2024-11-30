#pragma once

#include <vector>
#include <string>
#include "Libraries/imgui.h"

enum TrackType {
	TRACK_INVALID = 0,
	TRACK_FLOAT,
	TRACK_INT,
	TRACK_BOOL,
	TRACK_COLOR
};

class Frame {
public:
	union {
		float asFloat;
		int asInt;
		bool asBool;
		ImU32 asColor;
	} data;
	int frame; // index
};

class Track {
	friend class AnimationAsset;
	friend class DeleteAnimFrameAction;
protected:
	std::vector<Frame> frames;
	TrackType rtti;
	std::string name;
	Track* next; // Singly linked list, head in Animation class
	class AnimationAsset* parent;
	bool deleted;
protected:
	Track(const std::string _name, TrackType _type, AnimationAsset* parent);
	Track(const Track&) = delete;
	Track& operator=(const Track&) = delete;
	int FindLeftFrame(int index); // return an index in the frames array.
	int FindFrame(int index); // returns an index in the frames array.
	Frame* AddFrame(int index); // adds or returns a frame in the frames array.
public:
	Track* GetNext();
	TrackType GetType();
	void SetType(TrackType type);
	const std::string& GetName();

	bool IsDeleted();
	void SetDeleted(bool val);

	bool HasFrames();

	int FrameCount();
	void Sort();

	virtual ~Track();
	void Reset();

	float InterpolateF(int frame, bool loop);
	int   InterpolateI(int frame, bool loop);
	bool  InterpolateB(int frame, bool loop);
	ImU32 InterpolateC(int frame, bool loop);

	void AddFrameF(float data, int frame);
	void AddFrameI(int data,   int frame);
	void AddFrameB(bool data,  int frame);
	void AddFrameC(ImU32 data, int frame);


	Frame* GetFrame(int frame);
	Frame* GetFrameByIndex(int index);
	void RemoveFrame(int frameIndex);
	void DeleteByArrayIndex(int arrayIndex);
	void SerializeInner(std::stringstream& out, const char* target, int indent);
};